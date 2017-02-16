//
//  AvatarMixer.cpp
//  assignment-client/src/avatars
//
//  Created by Stephen Birarda on 9/5/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <cfloat>
#include <random>
#include <memory>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QTimer>
#include <QtCore/QThread>

#include <AABox.h>
#include <LogHandler.h>
#include <NodeList.h>
#include <udt/PacketHeaders.h>
#include <SharedUtil.h>
#include <UUID.h>
#include <TryLocker.h>

#include "AvatarMixer.h"

const QString AVATAR_MIXER_LOGGING_NAME = "avatar-mixer";

// FIXME - what we'd actually like to do is send to users at ~50% of their present rate down to 30hz. Assume 90 for now.
const int AVATAR_MIXER_BROADCAST_FRAMES_PER_SECOND = 45;
const unsigned int AVATAR_DATA_SEND_INTERVAL_MSECS = (1.0f / (float) AVATAR_MIXER_BROADCAST_FRAMES_PER_SECOND) * 1000;

AvatarMixer::AvatarMixer(ReceivedMessage& message) :
    ThreadedAssignment(message)
{
    // make sure we hear about node kills so we can tell the other nodes
    connect(DependencyManager::get<NodeList>().data(), &NodeList::nodeKilled, this, &AvatarMixer::nodeKilled);

    auto& packetReceiver = DependencyManager::get<NodeList>()->getPacketReceiver();
    packetReceiver.registerListener(PacketType::AvatarData, this, "queueIncomingPacket");

    packetReceiver.registerListener(PacketType::ViewFrustum, this, "handleViewFrustumPacket");
    packetReceiver.registerListener(PacketType::AvatarIdentity, this, "handleAvatarIdentityPacket");
    packetReceiver.registerListener(PacketType::KillAvatar, this, "handleKillAvatarPacket");
    packetReceiver.registerListener(PacketType::NodeIgnoreRequest, this, "handleNodeIgnoreRequestPacket");
    packetReceiver.registerListener(PacketType::RadiusIgnoreRequest, this, "handleRadiusIgnoreRequestPacket");
    packetReceiver.registerListener(PacketType::RequestsDomainListData, this, "handleRequestsDomainListDataPacket");

    auto nodeList = DependencyManager::get<NodeList>();
    connect(nodeList.data(), &NodeList::packetVersionMismatch, this, &AvatarMixer::handlePacketVersionMismatch);
}

void AvatarMixer::queueIncomingPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer node) {
    auto start = usecTimestampNow();
    getOrCreateClientData(node)->queuePacket(message, node);
    auto end = usecTimestampNow();
    _queueIncomingPacketElapsedTime += (end - start);
}


AvatarMixer::~AvatarMixer() {
}

// An 80% chance of sending a identity packet within a 5 second interval.
// assuming 60 htz update rate.
const float IDENTITY_SEND_PROBABILITY = 1.0f / 187.0f;

void AvatarMixer::sendIdentityPacket(AvatarMixerClientData* nodeData, const SharedNodePointer& destinationNode) {
    QByteArray individualData = nodeData->getAvatar().identityByteArray();

    auto identityPacket = NLPacket::create(PacketType::AvatarIdentity, individualData.size());

    individualData.replace(0, NUM_BYTES_RFC4122_UUID, nodeData->getNodeID().toRfc4122());

    identityPacket->write(individualData);

    DependencyManager::get<NodeList>()->sendPacket(std::move(identityPacket), *destinationNode);

    ++_sumIdentityPackets;
}

#include <chrono>
#include <thread>

std::chrono::microseconds AvatarMixer::timeFrame(p_high_resolution_clock::time_point& timestamp) {
    // advance the next frame
    auto nextTimestamp = timestamp + std::chrono::microseconds((int)((float)USECS_PER_SECOND / (float)AVATAR_MIXER_BROADCAST_FRAMES_PER_SECOND));
    auto now = p_high_resolution_clock::now();

    // compute how long the last frame took
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - timestamp);

    // set the new frame timestamp
    timestamp = std::max(now, nextTimestamp);

    // sleep until the next frame should start
    // WIN32 sleep_until is broken until VS2015 Update 2
    // instead, std::max (above) guarantees that timestamp >= now, so we can sleep_for
    std::this_thread::sleep_for(timestamp - now);

    return duration;
}


void AvatarMixer::start() {

    auto nodeList = DependencyManager::get<NodeList>();

    auto frameTimestamp = p_high_resolution_clock::now();

    while (!_isFinished) {
        _numTightLoopFrames++;
        _loopRate.increment();

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // DO THIS FIRST!!!!!!!!
        //
        // DONE --- 1) only sleep for remainder
        // 2) clean up stats, add slave stats
        // 3) delete dead code from mixer (now that it's in slave)
        // 4) audit the locking and side-effects to node, otherNode, and nodeData
        // 5) throttling??
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // calculates last frame duration and sleeps for the remainder of the target amount
        auto frameDuration = timeFrame(frameTimestamp);

        int lockWait, nodeTransform, functor;

        // Allow nodes to process any pending/queued packets across our worker threads
        {
            auto start = usecTimestampNow();

            nodeList->nestedEach([&](NodeList::const_iterator cbegin, NodeList::const_iterator cend) {
                auto end = usecTimestampNow();
                _processQueuedAvatarDataPacketsLockWaitElapsedTime += (end - start);

                _slavePool.processIncomingPackets(cbegin, cend);
            }, &lockWait, &nodeTransform, &functor);
            auto end = usecTimestampNow();
            _processQueuedAvatarDataPacketsElapsedTime += (end - start);

            //qDebug() << "PROCESS PACKETS...  " << "lockWait:" << lockWait << "nodeTransform:" << nodeTransform << "functor:" << functor;
        }

        // process pending display names... this doesn't currently run on multiple threads, because it
        // side-effects the mixer's data, which is fine because it's a very low cost operation
        {
            auto start = usecTimestampNow();
            nodeList->nestedEach([&](NodeList::const_iterator cbegin, NodeList::const_iterator cend) {
                std::for_each(cbegin, cend, [&](const SharedNodePointer& node) {
                    manageDisplayName(node);
                });
            }, &lockWait, &nodeTransform, &functor);
            auto end = usecTimestampNow();
            _displayNameManagementElapsedTime += (end - start);

            //qDebug() << "PROCESS PACKETS...  " << "lockWait:" << lockWait << "nodeTransform:" << nodeTransform << "functor:" << functor;
        }

        // this is where we need to put the real work...
        {
            // for now, call the single threaded version
            //broadcastAvatarData();

            auto start = usecTimestampNow();
            nodeList->nestedEach([&](NodeList::const_iterator cbegin, NodeList::const_iterator cend) {
                auto start = usecTimestampNow();
                _slavePool.anotherJob(cbegin, cend);
                auto end = usecTimestampNow();
                _broadcastAvatarDataInner += (end - start);
            }, &lockWait, &nodeTransform, &functor);
            auto end = usecTimestampNow();
            _broadcastAvatarDataElapsedTime += (end - start);

            _broadcastAvatarDataLockWait += lockWait;
            _broadcastAvatarDataNodeTransform += nodeTransform;
            _broadcastAvatarDataNodeFunctor += functor;
        }


        // play nice with qt event-looping
        {
            // since we're a while loop we need to yield to qt's event processing
            auto start = usecTimestampNow();
            QCoreApplication::processEvents();
            if (_isFinished) {
                // alert qt eventing that this is finished
                QCoreApplication::sendPostedEvents(this, QEvent::DeferredDelete);
                break;
            }
            auto end = usecTimestampNow();
            _processEventsElapsedTime += (end - start);
        }
    }
}


// NOTE: nodeData->getAvatar() might be side effected, most be called when access to node/nodeData
// is guarenteed to not be accessed by other thread
void AvatarMixer::manageDisplayName(const SharedNodePointer& node) {
    AvatarMixerClientData* nodeData = reinterpret_cast<AvatarMixerClientData*>(node->getLinkedData());
    if (nodeData && nodeData->getAvatarSessionDisplayNameMustChange()) {
        AvatarData& avatar = nodeData->getAvatar();
        const QString& existingBaseDisplayName = nodeData->getBaseDisplayName();
        if (--_sessionDisplayNames[existingBaseDisplayName].second <= 0) {
            _sessionDisplayNames.remove(existingBaseDisplayName);
        }

        QString baseName = avatar.getDisplayName().trimmed();
        const QRegularExpression curses { "fuck|shit|damn|cock|cunt" }; // POC. We may eventually want something much more elaborate (subscription?).
        baseName = baseName.replace(curses, "*"); // Replace rather than remove, so that people have a clue that the person's a jerk.
        const QRegularExpression trailingDigits { "\\s*_\\d+$" }; // whitespace "_123"
        baseName = baseName.remove(trailingDigits);
        if (baseName.isEmpty()) {
            baseName = "anonymous";
        }

        QPair<int, int>& soFar = _sessionDisplayNames[baseName]; // Inserts and answers 0, 0 if not already present, which is what we want.
        int& highWater = soFar.first;
        nodeData->setBaseDisplayName(baseName);
        QString sessionDisplayName = (highWater > 0) ? baseName + "_" + QString::number(highWater) : baseName;
        avatar.setSessionDisplayName(sessionDisplayName);
        highWater++;
        soFar.second++; // refcount
        nodeData->flagIdentityChange();
        nodeData->setAvatarSessionDisplayNameMustChange(false);
        sendIdentityPacket(nodeData, node); // Tell node whose name changed about its new session display name. Others will find out below.
        qDebug() << "Giving session display name" << sessionDisplayName << "to node with ID" << node->getUUID();
    }
}

static void avatarLoops();

// only send extra avatar data (avatars out of view, ignored) every Nth AvatarData frame
// Extra avatar data will be sent (AVATAR_MIXER_BROADCAST_FRAMES_PER_SECOND/EXTRA_AVATAR_DATA_FRAME_RATIO) times
// per second.
// This value should be a power of two for performance purposes, as the mixer performs a modulo operation every frame
// to determine whether the extra data should be sent.
static const int EXTRA_AVATAR_DATA_FRAME_RATIO = 16;


// FIXME -- this is dead code... it needs to be removed... 
// this "throttle" logic is the old approach. need to consider some
// reasonable throttle approach in new multi-core design
void AvatarMixer::broadcastAvatarData() {
    int idleTime = AVATAR_DATA_SEND_INTERVAL_MSECS;

    if (_lastFrameTimestamp.time_since_epoch().count() > 0) {
        auto idleDuration = p_high_resolution_clock::now() - _lastFrameTimestamp;
        idleTime = std::chrono::duration_cast<std::chrono::microseconds>(idleDuration).count();
    }

    ++_numStatFrames;

    const float STRUGGLE_TRIGGER_SLEEP_PERCENTAGE_THRESHOLD = 0.10f;
    const float BACK_OFF_TRIGGER_SLEEP_PERCENTAGE_THRESHOLD = 0.20f;

    const float RATIO_BACK_OFF = 0.02f;

    const int TRAILING_AVERAGE_FRAMES = 100;
    int framesSinceCutoffEvent = TRAILING_AVERAGE_FRAMES;

    const float CURRENT_FRAME_RATIO = 1.0f / TRAILING_AVERAGE_FRAMES;
    const float PREVIOUS_FRAMES_RATIO = 1.0f - CURRENT_FRAME_RATIO;

    // NOTE: The following code calculates the _performanceThrottlingRatio based on how much the avatar-mixer was
    // able to sleep. This will eventually be used to ask for an additional avatar-mixer to help out. Currently the value
    // is unused as it is assumed this should not be hit before the avatar-mixer hits the desired bandwidth limit per client.
    // It is reported in the domain-server stats for the avatar-mixer.

    _trailingSleepRatio = (PREVIOUS_FRAMES_RATIO * _trailingSleepRatio)
        + (idleTime * CURRENT_FRAME_RATIO / (float) AVATAR_DATA_SEND_INTERVAL_MSECS);

    float lastCutoffRatio = _performanceThrottlingRatio;
    bool hasRatioChanged = false;

    if (framesSinceCutoffEvent >= TRAILING_AVERAGE_FRAMES) {
        if (_trailingSleepRatio <= STRUGGLE_TRIGGER_SLEEP_PERCENTAGE_THRESHOLD) {
            // we're struggling - change our performance throttling ratio
            _performanceThrottlingRatio = _performanceThrottlingRatio + (0.5f * (1.0f - _performanceThrottlingRatio));

            qDebug() << "Mixer is struggling, sleeping" << _trailingSleepRatio * 100 << "% of frame time. Old cutoff was"
                << lastCutoffRatio << "and is now" << _performanceThrottlingRatio;
            hasRatioChanged = true;
        } else if (_trailingSleepRatio >= BACK_OFF_TRIGGER_SLEEP_PERCENTAGE_THRESHOLD && _performanceThrottlingRatio != 0) {
            // we've recovered and can back off the performance throttling
            _performanceThrottlingRatio = _performanceThrottlingRatio - RATIO_BACK_OFF;

            if (_performanceThrottlingRatio < 0) {
                _performanceThrottlingRatio = 0;
            }

            qDebug() << "Mixer is recovering, sleeping" << _trailingSleepRatio * 100 << "% of frame time. Old cutoff was"
                << lastCutoffRatio << "and is now" << _performanceThrottlingRatio;
            hasRatioChanged = true;
        }

        if (hasRatioChanged) {
            framesSinceCutoffEvent = 0;
        }
    }

    if (!hasRatioChanged) {
        ++framesSinceCutoffEvent;
    }

    //avatarLoops();

    _lastFrameTimestamp = p_high_resolution_clock::now();

#ifdef WANT_DEBUG
    auto sinceLastDebug = p_high_resolution_clock::now() - _lastDebugMessage;
    auto sinceLastDebugUsecs = std::chrono::duration_cast<std::chrono::microseconds>(sinceLastDebug).count();
    quint64 DEBUG_INTERVAL = USECS_PER_SECOND * 5;

    if (sinceLastDebugUsecs > DEBUG_INTERVAL) {
        qDebug() << "broadcast rate:" << _broadcastRate.rate() << "hz";
        _lastDebugMessage = p_high_resolution_clock::now();
    }
#endif
}

void AvatarMixer::nodeKilled(SharedNodePointer killedNode) {
    if (killedNode->getType() == NodeType::Agent
        && killedNode->getLinkedData()) {
        auto nodeList = DependencyManager::get<NodeList>();

        {  // decrement sessionDisplayNames table and possibly remove
           QMutexLocker nodeDataLocker(&killedNode->getLinkedData()->getMutex());
           AvatarMixerClientData* nodeData = dynamic_cast<AvatarMixerClientData*>(killedNode->getLinkedData());
           const QString& baseDisplayName = nodeData->getBaseDisplayName();
           // No sense guarding against very rare case of a node with no entry, as this will work without the guard and do one less lookup in the common case.
           if (--_sessionDisplayNames[baseDisplayName].second <= 0) {
               _sessionDisplayNames.remove(baseDisplayName);
           }
        }

        // this was an avatar we were sending to other people
        // send a kill packet for it to our other nodes
        auto killPacket = NLPacket::create(PacketType::KillAvatar, NUM_BYTES_RFC4122_UUID + sizeof(KillAvatarReason));
        killPacket->write(killedNode->getUUID().toRfc4122());
        killPacket->writePrimitive(KillAvatarReason::AvatarDisconnected);

        nodeList->broadcastToNodes(std::move(killPacket), NodeSet() << NodeType::Agent);

        // we also want to remove sequence number data for this avatar on our other avatars
        // so invoke the appropriate method on the AvatarMixerClientData for other avatars
        nodeList->eachMatchingNode(
            [&](const SharedNodePointer& node)->bool {
                if (!node->getLinkedData()) {
                    return false;
                }

                if (node->getUUID() == killedNode->getUUID()) {
                    return false;
                }

                return true;
            },
            [&](const SharedNodePointer& node) {
                QMetaObject::invokeMethod(node->getLinkedData(),
                                          "removeLastBroadcastSequenceNumber",
                                          Qt::AutoConnection,
                                          Q_ARG(const QUuid&, QUuid(killedNode->getUUID())));
            }
        );
    }
}

void AvatarMixer::handleViewFrustumPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    auto start = usecTimestampNow();
    getOrCreateClientData(senderNode);

    if (senderNode->getLinkedData()) {
        AvatarMixerClientData* nodeData = dynamic_cast<AvatarMixerClientData*>(senderNode->getLinkedData());
        if (nodeData != nullptr) {
            nodeData->readViewFrustumPacket(message->getMessage());
        }
    }

    auto end = usecTimestampNow();
    _handleViewFrustumPacketElapsedTime += (end - start);
}

void AvatarMixer::handleRequestsDomainListDataPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    auto start = usecTimestampNow();

    getOrCreateClientData(senderNode);

    if (senderNode->getLinkedData()) {
        AvatarMixerClientData* nodeData = dynamic_cast<AvatarMixerClientData*>(senderNode->getLinkedData());
        if (nodeData != nullptr) {
            bool isRequesting;
            message->readPrimitive(&isRequesting);
            nodeData->setRequestsDomainListData(isRequesting);
        }
    }
    auto end = usecTimestampNow();
    _handleRequestsDomainListDataPacketElapsedTime += (end - start);
}

void AvatarMixer::handleAvatarIdentityPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    auto start = usecTimestampNow();
    auto nodeList = DependencyManager::get<NodeList>();
    getOrCreateClientData(senderNode);

    if (senderNode->getLinkedData()) {
        AvatarMixerClientData* nodeData = dynamic_cast<AvatarMixerClientData*>(senderNode->getLinkedData());
        if (nodeData != nullptr) {
            AvatarData& avatar = nodeData->getAvatar();

            // parse the identity packet and update the change timestamp if appropriate
            AvatarData::Identity identity;
            AvatarData::parseAvatarIdentityPacket(message->getMessage(), identity);
            bool identityChanged = false;
            bool displayNameChanged = false;
            avatar.processAvatarIdentity(identity, identityChanged, displayNameChanged);
            if (identityChanged) {
                QMutexLocker nodeDataLocker(&nodeData->getMutex());
                nodeData->flagIdentityChange();
                if (displayNameChanged) {
                    nodeData->setAvatarSessionDisplayNameMustChange(true);
                }
            }
        }
    }
    auto end = usecTimestampNow();
    _handleAvatarIdentityPacketElapsedTime += (end - start);
}

void AvatarMixer::handleKillAvatarPacket(QSharedPointer<ReceivedMessage> message) {
    auto start = usecTimestampNow();
    DependencyManager::get<NodeList>()->processKillNode(*message);
    auto end = usecTimestampNow();
    _handleKillAvatarPacketElapsedTime += (end - start);
}

void AvatarMixer::handleNodeIgnoreRequestPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    auto start = usecTimestampNow();
    senderNode->parseIgnoreRequestMessage(message);
    auto end = usecTimestampNow();
    _handleNodeIgnoreRequestPacketElapsedTime += (end - start);
}

void AvatarMixer::handleRadiusIgnoreRequestPacket(QSharedPointer<ReceivedMessage> packet, SharedNodePointer sendingNode) {
    auto start = usecTimestampNow();
    sendingNode->parseIgnoreRadiusRequestMessage(packet);
    auto end = usecTimestampNow();
    _handleRadiusIgnoreRequestPacketElapsedTime += (end - start);
}

void AvatarMixer::sendStatsPacket() {
    auto start = usecTimestampNow();


    QJsonObject statsObject;
    statsObject["threads"] = _slavePool.numThreads();

    statsObject["average_listeners_last_second"] = (float) _sumListeners / (float) _numStatFrames;
    statsObject["average_identity_packets_per_frame"] = (float) _sumIdentityPackets / (float) _numStatFrames;

    statsObject["trailing_sleep_percentage"] = _trailingSleepRatio * 100;
    statsObject["performance_throttling_ratio"] = _performanceThrottlingRatio;
    statsObject["broadcast_loop_rate"] = _loopRate.rate();

    // this things all occur on the frequency of the tight loop
    int tightLoopFrames = _numTightLoopFrames;
    int tenTimesPerFrame = tightLoopFrames * 10;
    #define TIGHT_LOOP_STAT(x) (x > tenTimesPerFrame) ? x / tightLoopFrames : ((float)x / (float)tightLoopFrames);

    statsObject["timing_average_y_processEvents"] = TIGHT_LOOP_STAT(_processEventsElapsedTime);
    statsObject["timing_average_y_queueIncomingPacket"] = TIGHT_LOOP_STAT(_queueIncomingPacketElapsedTime);

    statsObject["timing_average_a1_broadcastAvatarData"] = TIGHT_LOOP_STAT(_broadcastAvatarDataElapsedTime);
    statsObject["timing_average_a2_innnerBroadcastAvatarData"] = TIGHT_LOOP_STAT(_broadcastAvatarDataInner);
    statsObject["timing_average_a3_broadcastAvatarDataLockWait"] = TIGHT_LOOP_STAT(_broadcastAvatarDataLockWait);
    statsObject["timing_average_a4_broadcastAvatarDataNodeTransform"] = TIGHT_LOOP_STAT(_broadcastAvatarDataNodeTransform);
    statsObject["timing_average_a5_broadcastAvatarDataNodeFunctor"] = TIGHT_LOOP_STAT(_broadcastAvatarDataNodeFunctor);

    statsObject["timing_average_z_displayNameManagement"] = TIGHT_LOOP_STAT(_displayNameManagementElapsedTime);
    statsObject["timing_average_z_handleAvatarDataPacket"] = TIGHT_LOOP_STAT(_handleAvatarDataPacketElapsedTime);
    statsObject["timing_average_z_handleAvatarIdentityPacket"] = TIGHT_LOOP_STAT(_handleAvatarIdentityPacketElapsedTime);
    statsObject["timing_average_z_handleKillAvatarPacket"] = TIGHT_LOOP_STAT(_handleKillAvatarPacketElapsedTime);
    statsObject["timing_average_z_handleNodeIgnoreRequestPacket"] = TIGHT_LOOP_STAT(_handleNodeIgnoreRequestPacketElapsedTime);
    statsObject["timing_average_z_handleRadiusIgnoreRequestPacket"] = TIGHT_LOOP_STAT(_handleRadiusIgnoreRequestPacketElapsedTime);
    statsObject["timing_average_z_handleRequestsDomainListDataPacket"] = TIGHT_LOOP_STAT(_handleRequestsDomainListDataPacketElapsedTime);
    statsObject["timing_average_z_handleViewFrustumPacket"] = TIGHT_LOOP_STAT(_handleViewFrustumPacketElapsedTime);
    statsObject["timing_average_z_processQueuedAvatarDataPackets"] = TIGHT_LOOP_STAT(_processQueuedAvatarDataPacketsElapsedTime);
    statsObject["timing_average_z_processQueuedAvatarDataPacketsLockWait"] = TIGHT_LOOP_STAT(_processQueuedAvatarDataPacketsLockWaitElapsedTime);

    statsObject["timing_sendStats"] = (float)_sendStatsElapsedTime;


    AvatarMixerSlaveStats aggregateStats;

    QJsonObject slavesObject;
    // gather stats
    int slaveNumber = 1;
    _slavePool.each([&](AvatarMixerSlave& slave) {
        QJsonObject slaveObject;
        AvatarMixerSlaveStats stats;
        slave.harvestStats(stats);
        slaveObject["nodesProcessed"] = TIGHT_LOOP_STAT(stats.nodesProcessed);
        slaveObject["packetsProcessed"] = TIGHT_LOOP_STAT(stats.packetsProcessed);
        slaveObject["timing_1_processIncomingPackets"] = TIGHT_LOOP_STAT(stats.processIncomingPacketsElapsedTime);
        slaveObject["timing_2_ignoreCalculation"] = TIGHT_LOOP_STAT(stats.ignoreCalculationElapsedTime);
        slaveObject["timing_3_toByteArray"] = TIGHT_LOOP_STAT(stats.toByteArrayElapsedTime);
        slaveObject["timing_4_avatarDataPacking"] = TIGHT_LOOP_STAT(stats.avatarDataPackingElapsedTime);
        slaveObject["timing_5_packetSending"] = TIGHT_LOOP_STAT(stats.packetSendingElapsedTime);
        slaveObject["timing_6_jobElapsedTime"] = TIGHT_LOOP_STAT(stats.jobElapsedTime);

        slavesObject[QString::number(slaveNumber)] = slaveObject;
        slaveNumber++;

        aggregateStats += stats;
    });
    statsObject["timing_slaves"] = slavesObject;

    // broadcastAvatarDataElapsed timing details...
    statsObject["aggregate_nodesProcessed"] = TIGHT_LOOP_STAT(aggregateStats.nodesProcessed);
    statsObject["aggregate_packetsProcessed"] = TIGHT_LOOP_STAT(aggregateStats.packetsProcessed);
    statsObject["timing_aggregate_1_processIncomingPackets"] = TIGHT_LOOP_STAT(aggregateStats.processIncomingPacketsElapsedTime);
    statsObject["timing_aggregate_2_ignoreCalculation"] = TIGHT_LOOP_STAT(aggregateStats.ignoreCalculationElapsedTime);
    statsObject["timing_aggregate_3_toByteArray"] = TIGHT_LOOP_STAT(aggregateStats.toByteArrayElapsedTime);
    statsObject["timing_aggregate_4_avatarDataPacking"] = TIGHT_LOOP_STAT(aggregateStats.avatarDataPackingElapsedTime);
    statsObject["timing_aggregate_5_packetSending"] = TIGHT_LOOP_STAT(aggregateStats.packetSendingElapsedTime);
    statsObject["timing_aggregate_6_jobElapsedTime"] = TIGHT_LOOP_STAT(aggregateStats.jobElapsedTime);

    _handleViewFrustumPacketElapsedTime = 0;
    _handleAvatarDataPacketElapsedTime = 0;
    _handleAvatarIdentityPacketElapsedTime = 0;
    _handleKillAvatarPacketElapsedTime = 0;
    _handleNodeIgnoreRequestPacketElapsedTime = 0;
    _handleRadiusIgnoreRequestPacketElapsedTime = 0;
    _handleRequestsDomainListDataPacketElapsedTime = 0;
    _processEventsElapsedTime = 0;
    _queueIncomingPacketElapsedTime = 0;
    _processQueuedAvatarDataPacketsElapsedTime = 0;
    _processQueuedAvatarDataPacketsLockWaitElapsedTime = 0;

    QJsonObject avatarsObject;
    auto nodeList = DependencyManager::get<NodeList>();
    // add stats for each listerner
    nodeList->eachNode([&](const SharedNodePointer& node) {
        QJsonObject avatarStats;

        const QString NODE_OUTBOUND_KBPS_STAT_KEY = "outbound_kbps";
        const QString NODE_INBOUND_KBPS_STAT_KEY = "inbound_kbps";

        // add the key to ask the domain-server for a username replacement, if it has it
        avatarStats[USERNAME_UUID_REPLACEMENT_STATS_KEY] = uuidStringWithoutCurlyBraces(node->getUUID());

        avatarStats[NODE_OUTBOUND_KBPS_STAT_KEY] = node->getOutboundBandwidth();
        avatarStats[NODE_INBOUND_KBPS_STAT_KEY] = node->getInboundBandwidth();

        AvatarMixerClientData* clientData = static_cast<AvatarMixerClientData*>(node->getLinkedData());
        if (clientData) {
            MutexTryLocker lock(clientData->getMutex());
            if (lock.isLocked()) {
                clientData->loadJSONStats(avatarStats);

                // add the diff between the full outbound bandwidth and the measured bandwidth for AvatarData send only
                avatarStats["delta_full_vs_avatar_data_kbps"] =
                    avatarStats[NODE_OUTBOUND_KBPS_STAT_KEY].toDouble() - avatarStats[OUTBOUND_AVATAR_DATA_STATS_KEY].toDouble();
            }
        }

        avatarsObject[uuidStringWithoutCurlyBraces(node->getUUID())] = avatarStats;
    });

    statsObject["z_avatars"] = avatarsObject;

    ThreadedAssignment::addPacketStatsAndSendStatsPacket(statsObject);

    _sumListeners = 0;
    _sumIdentityPackets = 0;
    _numStatFrames = 0;
    _numTightLoopFrames = 0;

    _broadcastAvatarDataElapsedTime = 0;
    _broadcastAvatarDataInner = 0;
    _broadcastAvatarDataLockWait = 0;
    _broadcastAvatarDataNodeTransform = 0;
    _broadcastAvatarDataNodeFunctor = 0;

    _displayNameManagementElapsedTime = 0;
    _ignoreCalculationElapsedTime = 0;
    _avatarDataPackingElapsedTime = 0;
    _packetSendingElapsedTime = 0;


    auto end = usecTimestampNow();
    _sendStatsElapsedTime = (end - start);

}

void AvatarMixer::run() {
    qDebug() << "Waiting for connection to domain to request settings from domain-server.";
    
    // wait until we have the domain-server settings, otherwise we bail
    DomainHandler& domainHandler = DependencyManager::get<NodeList>()->getDomainHandler();
    connect(&domainHandler, &DomainHandler::settingsReceived, this, &AvatarMixer::domainSettingsRequestComplete);
    connect(&domainHandler, &DomainHandler::settingsReceiveFail, this, &AvatarMixer::domainSettingsRequestFailed);
   
    ThreadedAssignment::commonInit(AVATAR_MIXER_LOGGING_NAME, NodeType::AvatarMixer);

}

AvatarMixerClientData* AvatarMixer::getOrCreateClientData(SharedNodePointer node) {
    auto clientData = dynamic_cast<AvatarMixerClientData*>(node->getLinkedData());

    if (!clientData) {
        node->setLinkedData(std::unique_ptr<NodeData> { new AvatarMixerClientData(node->getUUID()) });
        clientData = dynamic_cast<AvatarMixerClientData*>(node->getLinkedData());
        auto& avatar = clientData->getAvatar();
        avatar.setDomainMinimumScale(_domainMinimumScale);
        avatar.setDomainMaximumScale(_domainMaximumScale);
    }

    return clientData;
}

void AvatarMixer::domainSettingsRequestComplete() {
    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->addSetOfNodeTypesToNodeInterestSet({ NodeType::Agent, NodeType::EntityScriptServer });

    // parse the settings to pull out the values we need
    parseDomainServerSettings(nodeList->getDomainHandler().getSettingsObject());
    
    // start our tight loop...
    start();
}

void AvatarMixer::handlePacketVersionMismatch(PacketType type, const HifiSockAddr& senderSockAddr, const QUuid& senderUUID) {
    // if this client is using packet versions we don't expect.
    if ((type == PacketTypeEnum::Value::AvatarIdentity || type == PacketTypeEnum::Value::AvatarData) && !senderUUID.isNull()) {
        // Echo an empty AvatarData packet back to that client.
        // This should trigger a version mismatch dialog on their side.
        auto nodeList = DependencyManager::get<NodeList>();
        auto node = nodeList->nodeWithUUID(senderUUID);
        if (node) {
            auto emptyPacket = NLPacket::create(PacketType::AvatarData, 0);
            nodeList->sendPacket(std::move(emptyPacket), *node);
        }
    }
}

void AvatarMixer::parseDomainServerSettings(const QJsonObject& domainSettings) {
    const QString AVATAR_MIXER_SETTINGS_KEY = "avatar_mixer";
    QJsonObject avatarMixerGroupObject = domainSettings[AVATAR_MIXER_SETTINGS_KEY].toObject();


    const QString NODE_SEND_BANDWIDTH_KEY = "max_node_send_bandwidth";

    const float DEFAULT_NODE_SEND_BANDWIDTH = 5.0f;
    QJsonValue nodeBandwidthValue = avatarMixerGroupObject[NODE_SEND_BANDWIDTH_KEY];
    if (!nodeBandwidthValue.isDouble()) {
        qDebug() << NODE_SEND_BANDWIDTH_KEY << "is not a double - will continue with default value";
    }

    _maxKbpsPerNode = nodeBandwidthValue.toDouble(DEFAULT_NODE_SEND_BANDWIDTH) * KILO_PER_MEGA;
    qDebug() << "The maximum send bandwidth per node is" << _maxKbpsPerNode << "kbps.";

    const QString AUTO_THREADS = "auto_threads";
    bool autoThreads = avatarMixerGroupObject[AUTO_THREADS].toBool();
    if (!autoThreads) {
        bool ok;
        const QString NUM_THREADS = "num_threads";
        int numThreads = avatarMixerGroupObject[NUM_THREADS].toString().toInt(&ok);
        if (!ok) {
            qWarning() << "Avatar mixer: Error reading thread count. Using 1 thread.";
            numThreads = 1;
        }
        qDebug() << "Avatar mixer will use specified number of threads:" << numThreads;
        _slavePool.setNumThreads(numThreads);
    } else {
        qDebug() << "Avatar mixer will automatically determine number of threads to use. Using:" << _slavePool.numThreads() << "threads.";
    }
    
    const QString AVATARS_SETTINGS_KEY = "avatars";

    static const QString MIN_SCALE_OPTION = "min_avatar_scale";
    float settingMinScale = domainSettings[AVATARS_SETTINGS_KEY].toObject()[MIN_SCALE_OPTION].toDouble(MIN_AVATAR_SCALE);
    _domainMinimumScale = glm::clamp(settingMinScale, MIN_AVATAR_SCALE, MAX_AVATAR_SCALE);

    static const QString MAX_SCALE_OPTION = "max_avatar_scale";
    float settingMaxScale = domainSettings[AVATARS_SETTINGS_KEY].toObject()[MAX_SCALE_OPTION].toDouble(MAX_AVATAR_SCALE);
    _domainMaximumScale = glm::clamp(settingMaxScale, MIN_AVATAR_SCALE, MAX_AVATAR_SCALE);

    // make sure that the domain owner didn't flip min and max
    if (_domainMinimumScale > _domainMaximumScale) {
        std::swap(_domainMinimumScale, _domainMaximumScale);
    }

    qDebug() << "This domain requires a minimum avatar scale of" << _domainMinimumScale
        << "and a maximum avatar scale of" << _domainMaximumScale;

}
