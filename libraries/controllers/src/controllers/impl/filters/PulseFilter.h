//
//  Created by Bradley Austin Davis 2015/10/25
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_Controllers_Filters_Pulse_h
#define hifi_Controllers_Filters_Pulse_h

#include "../Filter.h"

namespace controller {


class PulseFilter : public Filter {
    REGISTER_FILTER_CLASS(PulseFilter);
public:
    PulseFilter() = default;
    PulseFilter(float interval) : _interval(interval) {}

    virtual AxisValue apply(AxisValue value) const override;

    virtual Pose apply(Pose value) const override { return value; }

    virtual bool parseParameters(const QJsonValue& parameters) override;

private:
    static const float DEFAULT_LAST_EMIT_TIME;
    mutable float _lastEmitTime { DEFAULT_LAST_EMIT_TIME };
    bool _resetOnZero { false };
    float _interval { 1.0f };
};

}

#endif
