//
//  Created by Bradley Austin Davis 2015/10/23
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Controllers_StandardEndpoint_h
#define hifi_Controllers_StandardEndpoint_h

#include "../Endpoint.h"

#include <DependencyManager.h>

#include "../../InputRecorder.h"
#include "../../UserInputMapper.h"

namespace controller {

class StandardEndpoint : public VirtualEndpoint {
public:
    StandardEndpoint(const Input& input) : VirtualEndpoint(input) {}
    virtual bool writeable() const override { return !_written; }
    virtual bool readable() const override { return !_read; }
    virtual void reset() override {
        apply(AxisValue(), Endpoint::Pointer());
        apply(Pose(), Endpoint::Pointer());
        _written = _read = false;
    }

    virtual AxisValue value() override {
        _read = true;
        return VirtualEndpoint::value();
    }

    virtual void apply(AxisValue value, const Pointer& source) override {
        // For standard endpoints, the first NON-ZERO write counts.
        if (value != AxisValue()) {
            _written = true;
        }
        VirtualEndpoint::apply(value, source);
    }

    virtual Pose pose() override {
        _read = true;
        InputRecorder* inputRecorder = InputRecorder::getInstance();
        if (inputRecorder->isPlayingback()) {
            auto userInputMapper = DependencyManager::get<UserInputMapper>();
            QString actionName =  userInputMapper->getStandardPoseName(_input.getChannel());
            return inputRecorder->getPoseState(actionName);
        }
        return VirtualEndpoint::pose();
    }

    virtual void apply(const Pose& value, const Pointer& source) override {
        if (value != Pose() && value.isValid()) {
            _written = true;
        }
        VirtualEndpoint::apply(value, source);
    }

private:
    bool _written { false };
    bool _read { false };
};

}

#endif
