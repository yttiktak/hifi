//
//  Created by Bradley Austin Davis on 2015/05/29
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <display-plugins/hmd/HmdDisplayPlugin.h>

#include <QTimer>

#include <OVR_CAPI_GL.h>

#define OVRPL_DISABLED
#include <OVR_Platform.h>

#include <graphics/Geometry.h>

class OculusBaseDisplayPlugin : public HmdDisplayPlugin {
    using Parent = HmdDisplayPlugin;
public:
    bool isSupported() const override;
    bool hasAsyncReprojection() const override { return true; }
    bool getSupportsAutoSwitch() override final { return true; }

    glm::mat4 getEyeProjection(Eye eye, const glm::mat4& baseProjection) const override;
    glm::mat4 getCullingProjection(const glm::mat4& baseProjection) const override;

    // Stereo specific methods
    void resetSensors() override final;
    bool beginFrameRender(uint32_t frameIndex) override;
    float getTargetFrameRate() const override { return _hmdDesc.DisplayRefreshRate; }

    QRectF getPlayAreaRect() override;
    QVector<glm::vec3> getSensorPositions() override;

    virtual StencilMaskMode getStencilMaskMode() const override { return StencilMaskMode::MESH; }
    virtual StencilMaskMeshOperator getStencilMaskMeshOperator() override;

protected:
    void customizeContext() override;
    void uncustomizeContext() override;
    bool internalActivate() override;
    void internalDeactivate() override;
    void updatePresentPose() override;

protected:
    ovrSession _session{ nullptr };
    ovrGraphicsLuid _luid;
    std::array<ovrEyeRenderDesc, 2> _eyeRenderDescs;
    std::array<ovrFovPort, 2> _eyeFovs;
    ovrHmdDesc _hmdDesc;
    ovrLayerEyeFov _sceneLayer;
    ovrViewScaleDesc _viewScaleDesc;
    // ovrLayerEyeFovDepth _depthLayer;
    bool _hmdMounted { false };
    bool _visible { true };

    std::array<graphics::MeshPointer, 2> _stencilMeshes;
    bool _stencilMeshesInitialized { false };
};
