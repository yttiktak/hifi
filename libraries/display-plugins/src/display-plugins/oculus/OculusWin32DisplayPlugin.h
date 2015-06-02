//
//  Created by Bradley Austin Davis on 2015/05/29
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include "OculusBaseDisplayPlugin.h"

#include <QTimer>

class OffscreenGlCanvas;
class MirrorFramebufferWrapper;
class SwapFramebufferWrapper;

using SwapFboPtr = QSharedPointer<SwapFramebufferWrapper>;
using MirrorFboPtr = QSharedPointer<MirrorFramebufferWrapper>;

class OculusWin32DisplayPlugin : public OculusBaseDisplayPlugin {
public:
    virtual bool isSupported();
    virtual const QString & getName();

    virtual void activate(PluginContainer * container) override;
    virtual void deactivate() override;

    virtual void display(GLuint sceneTexture, const glm::uvec2& sceneSize,
        GLuint overlayTexture, const glm::uvec2& overlaySize) override;

    virtual bool eventFilter(QObject* receiver, QEvent* event) override;

protected:
    virtual void customizeContext(PluginContainer * container) override;
    virtual void swapBuffers() override;

private:
    static const QString NAME;
    SwapFboPtr          _swapFbo;
    MirrorFboPtr        _mirrorFbo;
    ovrLayerEyeFov      _layer;
};
