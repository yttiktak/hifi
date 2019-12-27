//
//  Created by amantly 2018.06.26
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_OtherAvatar_h
#define hifi_OtherAvatar_h

#include <memory>
#include <vector>

#include <avatars-renderer/Avatar.h>
#include <workload/Space.h>

#include "InterfaceLogging.h"

class AvatarManager;
class AvatarMotionState;
class DetailedMotionState;

class OtherAvatar : public Avatar {
public:
    explicit OtherAvatar(QThread* thread);
    virtual ~OtherAvatar();

    enum BodyLOD {
        Sphere = 0,
        MultiSphereLow, // No finger joints
        MultiSphereHigh // All joints
    };

    virtual void instantiableAvatar() override { };
    virtual void createOrb() override;
    virtual void indicateLoadingStatus(LoadingStatus loadingStatus) override;
    void updateOrbPosition();
    void removeOrb();

    void setSpaceIndex(int32_t index);
    int32_t getSpaceIndex() const { return _spaceIndex; }
    void updateSpaceProxy(workload::Transaction& transaction) const;

    int parseDataFromBuffer(const QByteArray& buffer) override;

    bool isInPhysicsSimulation() const;
    void rebuildCollisionShape() override;

    void setWorkloadRegion(uint8_t region);
    uint8_t getWorkloadRegion() { return _workloadRegion; }
    bool shouldBeInPhysicsSimulation() const;
    bool needsPhysicsUpdate() const;

    const btCollisionShape* createCollisionShape(int32_t jointIndex, bool& isBound, std::vector<int32_t>& boundJoints);
    std::vector<DetailedMotionState*>& getDetailedMotionStates() { return _detailedMotionStates; }
    void forgetDetailedMotionStates();
    BodyLOD getBodyLOD() { return _bodyLOD; }
    void computeShapeLOD();

    void updateCollisionGroup(bool myAvatarCollide);
    bool getCollideWithOtherAvatars() const { return _collideWithOtherAvatars; } 

    void setCollisionWithOtherAvatarsFlags() override;

    void simulate(float deltaTime, bool inView) override;
    void debugJointData() const;
    friend AvatarManager;

protected:
    void handleChangedAvatarEntityData();
    void updateAttachedAvatarEntities();
    void onAddAttachedAvatarEntity(const QUuid& id);
    void onRemoveAttachedAvatarEntity(const QUuid& id);

    class AvatarEntityDataHash {
    public:
        AvatarEntityDataHash(uint32_t h) : hash(h) {};
        uint32_t hash { 0 };
        bool success { false };
    };

    using MapOfAvatarEntityDataHashes = QMap<QUuid, AvatarEntityDataHash>;
    MapOfAvatarEntityDataHashes _avatarEntityDataHashes;

    std::vector<QUuid> _attachedAvatarEntities;
    QUuid _otherAvatarOrbMeshPlaceholderID;
    AvatarMotionState* _motionState { nullptr };
    std::vector<DetailedMotionState*> _detailedMotionStates;
    int32_t _spaceIndex { -1 };
    uint8_t _workloadRegion { workload::Region::INVALID };
    BodyLOD _bodyLOD { BodyLOD::Sphere };
    bool _needsDetailedRebuild { false };
};

using OtherAvatarPointer = std::shared_ptr<OtherAvatar>;
using AvatarPointer = std::shared_ptr<Avatar>;

#endif  // hifi_OtherAvatar_h
