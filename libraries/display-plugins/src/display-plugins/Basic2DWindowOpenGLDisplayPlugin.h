//
//  Created by Bradley Austin Davis on 2015/05/29
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include "OpenGLDisplayPlugin.h"

#if defined(Q_OS_ANDROID)
#include "VirtualPadManager.h"
#endif

const float TARGET_FRAMERATE_Basic2DWindowOpenGL = 60.0f;

class QScreen;
class QAction;

class Basic2DWindowOpenGLDisplayPlugin : public OpenGLDisplayPlugin {
    Q_OBJECT
    using Parent = OpenGLDisplayPlugin;
public:
    virtual const QString getName() const override { return NAME; }

    virtual float getTargetFrameRate() const override { return  _framerateTarget ? (float) _framerateTarget : TARGET_FRAMERATE_Basic2DWindowOpenGL; }

    virtual void customizeContext() override;
    virtual void uncustomizeContext() override;

    virtual bool internalActivate() override;

    virtual bool isThrottled() const override;

    virtual void compositeExtra() override;

    virtual void pluginUpdate() override {};

    virtual gpu::PipelinePointer getRenderTexturePipeline() override;

protected:
    mutable bool _isThrottled = false;

private:
    static const QString NAME;
    QScreen* getFullscreenTarget();
    std::vector<QAction*> _framerateActions;
    QAction* _vsyncAction { nullptr };
    uint32_t _framerateTarget { 0 };
    int _fullscreenTarget{ -1 };

#if defined(Q_OS_ANDROID)
    gpu::TexturePointer _virtualPadStickTexture;
    gpu::TexturePointer _virtualPadStickBaseTexture;
    qreal _virtualPadPixelSize;

    gpu::TexturePointer _virtualPadJumpBtnTexture;
    qreal _virtualPadJumpBtnPixelSize;

    gpu::TexturePointer _virtualPadRbBtnTexture;
    qreal _virtualPadRbBtnPixelSize;

    class VirtualPadButton {
    public:

        VirtualPadButton() {}
        VirtualPadButton(qreal pixelSize, QString iconPath, VirtualPad::Manager::Button button);

        void draw(gpu::Batch& batch, glm::vec2 buttonPosition);

        gpu::TexturePointer _texture;
        qreal _pixelSize;
        VirtualPad::Manager::Button _button;
    };
    QVector<VirtualPadButton> _virtualPadButtons;

#endif
};
