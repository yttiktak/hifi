//
//  AnimBlendLinearMove.cpp
//
//  Created by Anthony J. Thibault on 10/22/15.
//  Copyright (c) 2015 High Fidelity, Inc. All rights reserved.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AnimBlendLinearMove.h"
#include <GLMHelpers.h>
#include "AnimationLogging.h"
#include "AnimUtil.h"
#include "AnimClip.h"

AnimBlendLinearMove::AnimBlendLinearMove(const QString& id, float alpha, float desiredSpeed, const std::vector<float>& characteristicSpeeds) :
    AnimNode(AnimNode::Type::BlendLinearMove, id),
    _alpha(alpha),
    _desiredSpeed(desiredSpeed),
    _characteristicSpeeds(characteristicSpeeds) {

}

AnimBlendLinearMove::~AnimBlendLinearMove() {

}

static float calculateAlpha(const float speed, const std::vector<float>& characteristicSpeeds) {

    assert(characteristicSpeeds.size() > 0);
    // calculate alpha from linear combination of referenceSpeeds.
    float alpha = 0.0f;
    if (speed <= characteristicSpeeds.front()) {
        alpha = 0.0f;
    } else if (speed > characteristicSpeeds.back()) {
        alpha = (float)(characteristicSpeeds.size() - 1);
    } else {
        for (size_t i = 0; i < characteristicSpeeds.size() - 1; i++) {
            if (characteristicSpeeds[i] < speed && speed < characteristicSpeeds[i + 1]) {
                alpha = (float)i + ((speed - characteristicSpeeds[i]) / (characteristicSpeeds[i + 1] - characteristicSpeeds[i]));
                break;
            }
        }
    }
    return alpha;
}

const AnimPoseVec& AnimBlendLinearMove::evaluate(const AnimVariantMap& animVars, const AnimContext& context, float dt, AnimVariantMap& triggersOut) {

    assert(_children.size() == _characteristicSpeeds.size());

    _desiredSpeed = animVars.lookup(_desiredSpeedVar, _desiredSpeed);

    float speed = 0.0f;
    if (_alphaVar.contains("Lateral")) {
        speed = animVars.lookup("moveLateralSpeed", speed);
    } else if (_alphaVar.contains("Backward")) {
        speed = animVars.lookup("moveBackwardSpeed", speed);
    } else {
        //this is forward movement
        speed = animVars.lookup("moveForwardSpeed", speed);
    }
    _alpha = calculateAlpha(speed, _characteristicSpeeds);
    float parentDebugAlpha = context.getDebugAlpha(_id);

    if (_children.size() == 0) {
        for (auto&& pose : _poses) {
            pose = AnimPose::identity;
        }
    } else if (_children.size() == 1) {
        const float alpha = 0.0f;
        const int prevPoseIndex = 0;
        const int nextPoseIndex = 0;
        float prevDeltaTime, nextDeltaTime;
        setFrameAndPhase(dt, alpha, prevPoseIndex, nextPoseIndex, &prevDeltaTime, &nextDeltaTime, triggersOut);
        evaluateAndBlendChildren(animVars, context, triggersOut, alpha, prevPoseIndex, nextPoseIndex, prevDeltaTime, nextDeltaTime);
        context.setDebugAlpha(_children[0]->getID(), parentDebugAlpha, _children[0]->getType());
    } else {
        auto clampedAlpha = glm::clamp(_alpha, 0.0f, (float)(_children.size() - 1));
        auto prevPoseIndex = glm::floor(clampedAlpha);
        auto nextPoseIndex = glm::ceil(clampedAlpha);
        auto alpha = glm::fract(clampedAlpha);
        float prevDeltaTime, nextDeltaTime;
        setFrameAndPhase(dt, alpha, prevPoseIndex, nextPoseIndex, &prevDeltaTime, &nextDeltaTime, triggersOut);
        evaluateAndBlendChildren(animVars, context, triggersOut, alpha, prevPoseIndex, nextPoseIndex, prevDeltaTime, nextDeltaTime);

        if (prevPoseIndex == nextPoseIndex) {
            context.setDebugAlpha(_children[nextPoseIndex]->getID(), parentDebugAlpha, _children[nextPoseIndex]->getType());
        } else {
            context.setDebugAlpha(_children[prevPoseIndex]->getID(), (1.0f - alpha) * parentDebugAlpha, _children[prevPoseIndex]->getType());
            context.setDebugAlpha(_children[nextPoseIndex]->getID(), alpha * parentDebugAlpha, _children[nextPoseIndex]->getType());
        }
    }

    processOutputJoints(triggersOut);

    return _poses;
}

// for AnimDebugDraw rendering
const AnimPoseVec& AnimBlendLinearMove::getPosesInternal() const {
    return _poses;
}

void AnimBlendLinearMove::evaluateAndBlendChildren(const AnimVariantMap& animVars, const AnimContext& context, AnimVariantMap& triggersOut, float alpha,
                                                   size_t prevPoseIndex, size_t nextPoseIndex,
                                                   float prevDeltaTime, float nextDeltaTime) {
    if (prevPoseIndex == nextPoseIndex) {
        // this can happen if alpha is on an integer boundary
        _poses = _children[prevPoseIndex]->evaluate(animVars, context, prevDeltaTime, triggersOut);
    } else {
        // need to eval and blend between two children.
        auto prevPoses = _children[prevPoseIndex]->evaluate(animVars, context, prevDeltaTime, triggersOut);
        auto nextPoses = _children[nextPoseIndex]->evaluate(animVars, context, nextDeltaTime, triggersOut);

        if (prevPoses.size() > 0 && prevPoses.size() == nextPoses.size()) {
            _poses.resize(prevPoses.size());

            ::blend(_poses.size(), &prevPoses[0], &nextPoses[0], alpha, &_poses[0]);
        }
    }
}

void AnimBlendLinearMove::setFrameAndPhase(float dt, float alpha, int prevPoseIndex, int nextPoseIndex,
                                           float* prevDeltaTimeOut, float* nextDeltaTimeOut, AnimVariantMap& triggersOut) {

    const float FRAMES_PER_SECOND = 30.0f;
    auto prevClipNode = std::dynamic_pointer_cast<AnimClip>(_children[prevPoseIndex]);
    assert(prevClipNode);
    auto nextClipNode = std::dynamic_pointer_cast<AnimClip>(_children[nextPoseIndex]);
    assert(nextClipNode);

    float v0 = _characteristicSpeeds[prevPoseIndex];
    float n0 = (prevClipNode->getEndFrame() - prevClipNode->getStartFrame()) + 1.0f;
    float v1 = _characteristicSpeeds[nextPoseIndex];
    float n1 = (nextClipNode->getEndFrame() - nextClipNode->getStartFrame()) + 1.0f;

    // rate of change in phase space, necessary to achive desired speed.
    float omega = (_desiredSpeed * FRAMES_PER_SECOND) / ((1.0f - alpha) * v0 * n0 + alpha * v1 * n1);

    float f0 = prevClipNode->getStartFrame() + _phase * n0;
    prevClipNode->setCurrentFrame(f0);

    float f1 = nextClipNode->getStartFrame() + _phase * n1;
    nextClipNode->setCurrentFrame(f1);

    // integrate phase forward in time.
    _phase += omega * dt;

    if (_phase < 0.0f) {
        _phase = 0.0f;
    }

    // detect loop trigger events
    if (_phase >= 1.0f) {
        triggersOut.setTrigger(_id + "OnLoop");
        _phase = glm::fract(_phase);
    }

    *prevDeltaTimeOut = omega * dt * (n0 / FRAMES_PER_SECOND);
    *nextDeltaTimeOut = omega * dt * (n1 / FRAMES_PER_SECOND);
}

void AnimBlendLinearMove::setCurrentFrameInternal(float frame) {
    assert(_children.size() > 0);
    auto clipNode = std::dynamic_pointer_cast<AnimClip>(_children.front());
    assert(clipNode);
    const float NUM_FRAMES = (clipNode->getEndFrame() - clipNode->getStartFrame()) + 1.0f;
    _phase = fmodf(frame / NUM_FRAMES, 1.0f);
}
