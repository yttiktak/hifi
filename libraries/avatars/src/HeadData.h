//
//  HeadData.h
//  libraries/avatars/src
//
//  Created by Stephen Birarda on 5/20/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_HeadData_h
#define hifi_HeadData_h

#include <iostream>

#include <QVector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <SharedUtil.h>
#include <BlendshapeConstants.h>

// degrees
const float MIN_HEAD_YAW = -180.0f;
const float MAX_HEAD_YAW = 180.0f;
const float MIN_HEAD_PITCH = -60.0f;
const float MAX_HEAD_PITCH = 60.0f;
const float MIN_HEAD_ROLL = -50.0f;
const float MAX_HEAD_ROLL = 50.0f;

class AvatarData;
class QJsonObject;

class HeadData {
public:
    explicit HeadData(AvatarData* owningAvatar);
    virtual ~HeadData() { };

    // degrees
    float getBaseYaw() const { return _baseYaw; }
    void setBaseYaw(float yaw) { _baseYaw = glm::clamp(yaw, MIN_HEAD_YAW, MAX_HEAD_YAW); }
    float getBasePitch() const { return _basePitch; }
    void setBasePitch(float pitch) { _basePitch = glm::clamp(pitch, MIN_HEAD_PITCH, MAX_HEAD_PITCH); }
    float getBaseRoll() const { return _baseRoll; }
    void setBaseRoll(float roll) { _baseRoll = glm::clamp(roll, MIN_HEAD_ROLL, MAX_HEAD_ROLL); }

    virtual float getFinalYaw() const { return _baseYaw; }
    virtual float getFinalPitch() const { return _basePitch; }
    virtual float getFinalRoll() const { return _baseRoll; }
    virtual glm::quat getRawOrientation() const;
    virtual void setRawOrientation(const glm::quat& orientation);

    glm::quat getOrientation() const;
    void setOrientation(const glm::quat& orientation);

    void setBlendshape(QString name, float val);
    int getBlendshapeIndex(const QString& name);
    void getBlendshapeIndices(const std::vector<QString>& blendShapeNames, std::vector<int>& indexes);
    const QVector<float>& getBlendshapeCoefficients() const { return _blendshapeCoefficients; }
    const QVector<float>& getSummedBlendshapeCoefficients();
    int getNumSummedBlendshapeCoefficients() const;
    void setBlendshapeCoefficients(const QVector<float>& blendshapeCoefficients) { _blendshapeCoefficients = blendshapeCoefficients; }
    void clearBlendshapeCoefficients();

    const glm::vec3& getLookAtPosition() const { return _lookAtPosition; }
    virtual void setLookAtPosition(const glm::vec3& lookAtPosition) {
        if (_lookAtPosition != lookAtPosition) {
            _lookAtPositionChanged = usecTimestampNow();
        }
        _lookAtPosition = lookAtPosition;
    }
    bool lookAtPositionChangedSince(quint64 time) { return _lookAtPositionChanged >= time; }

    enum ProceduralAnimationType {
        AudioProceduralBlendshapeAnimation = 0,
        BlinkProceduralBlendshapeAnimation,
        LidAdjustmentProceduralBlendshapeAnimation,
        SaccadeProceduralEyeJointAnimation,
        ProceduralAnimaitonTypeCount,
    };

    // called by scripts to enable or disable procedural blendshape or eye joint animations.
    bool getProceduralAnimationFlag(ProceduralAnimationType type) const;
    void setProceduralAnimationFlag(ProceduralAnimationType type, bool value);

    // called by c++ to suppress, i.e. temporarily disable a procedural animation.
    bool getSuppressProceduralAnimationFlag(ProceduralAnimationType flag) const;
    void setSuppressProceduralAnimationFlag(ProceduralAnimationType flag, bool value);

    // called by scripts to enable/disable manual adjustment of blendshapes
    void setHasScriptedBlendshapes(bool value);
    bool getHasScriptedBlendshapes() const;

    // called by C++ code to denote the presence of manually driven blendshapes.
    void setHasInputDrivenBlendshapes(bool value);
    bool getHasInputDrivenBlendshapes() const;

    friend class AvatarData;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

protected:
    // degrees
    float _baseYaw;
    float _basePitch;
    float _baseRoll;

    glm::vec3 _lookAtPosition;
    quint64 _lookAtPositionChanged { 0 };

    std::vector<bool> _userProceduralAnimationFlags;
    std::vector<bool> _suppressProceduralAnimationFlags;

    bool _hasScriptedBlendshapes { false };
    bool _hasInputDrivenBlendshapes { false };

    float _leftEyeBlink { 0.0f };
    float _rightEyeBlink { 0.0f };
    float _averageLoudness { 0.0f };
    float _browAudioLift { 0.0f };

    QVector<float> _blendshapeCoefficients;
    QVector<float> _transientBlendshapeCoefficients;
    QVector<float> _summedBlendshapeCoefficients;
    QMap<QString, int> _blendshapeLookupMap;
    AvatarData* _owningAvatar;

private:
    // privatize copy ctor and assignment operator so copies of this object cannot be made
    HeadData(const HeadData&);
    HeadData& operator= (const HeadData&);

    void setHeadOrientation(const glm::quat& orientation);
    void computeBlendshapesLookupMap();
};

#endif // hifi_HeadData_h
