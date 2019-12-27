//
//  AnimNode.cpp
//
//  Created by Anthony J. Thibault on 9/2/15.
//  Copyright (c) 2015 High Fidelity, Inc. All rights reserved.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AnimNode.h"

#include <QtGlobal>

AnimNode::Pointer AnimNode::getParent() {
    return _parent.lock();
}

void AnimNode::addChild(Pointer child) {
    _children.push_back(child);
    child->_parent = shared_from_this();
}

void AnimNode::removeChild(Pointer child) {
    auto iter = std::find(_children.begin(), _children.end(), child);
    if (iter != _children.end()) {
        _children.erase(iter);
        child->_parent.reset();
    }
}

void AnimNode::replaceChild(Pointer oldChild, Pointer newChild) {
    auto iter = std::find(_children.begin(), _children.end(), oldChild);
    if (iter != _children.end()) {
        oldChild->_parent.reset();
        newChild->_parent = shared_from_this();
        if (_skeleton) {
            newChild->setSkeleton(_skeleton);
        }
        *iter = newChild;
    }
}

AnimNode::Pointer AnimNode::getChild(int i) const {
    assert(i >= 0 && i < (int)_children.size());
    return _children[i];
}

void AnimNode::setSkeleton(AnimSkeleton::ConstPointer skeleton) {
    setSkeletonInternal(skeleton);
    for (auto&& child : _children) {
        child->setSkeleton(skeleton);
    }
}

void AnimNode::setCurrentFrame(float frame) {
    setCurrentFrameInternal(frame);
    for (auto&& child : _children) {
        child->setCurrentFrameInternal(frame);
    }
}

void AnimNode::setActive(bool active) {
    setActiveInternal(active);
    for (auto&& child : _children) {
        child->setActiveInternal(active);
    }
}

void AnimNode::processOutputJoints(AnimVariantMap& triggersOut) const {
    if (!_skeleton) {
        return;
    }

    for (auto&& jointName : _outputJointNames) {
        // TODO: cache the jointIndices
        int jointIndex = _skeleton->nameToJointIndex(jointName);
        if (jointIndex >= 0) {
            AnimPose pose = _skeleton->getAbsolutePose(jointIndex, getPosesInternal());
            triggersOut.set(_id + jointName + "Rotation", pose.rot());
            triggersOut.set(_id + jointName + "Position", pose.trans());
        }
    }
}
