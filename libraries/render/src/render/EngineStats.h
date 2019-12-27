//
//  EngineStats.h
//  render/src/render
//
//  Created by Sam Gateau on 3/27/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_EngineStats_h
#define hifi_render_EngineStats_h

#include <gpu/Context.h>

#include <QElapsedTimer>

#include "Engine.h"

namespace render {

    // A simple job collecting global stats on the Engine / Scene / GPU
    class EngineStatsConfig : public Job::Config{
        Q_OBJECT

        Q_PROPERTY(quint32 bufferCPUCount MEMBER bufferCPUCount NOTIFY newStats)
        Q_PROPERTY(quint32 bufferGPUCount MEMBER bufferGPUCount NOTIFY newStats)
        Q_PROPERTY(qint64 bufferCPUMemSize MEMBER bufferCPUMemSize NOTIFY newStats)
        Q_PROPERTY(qint64 bufferGPUMemSize MEMBER bufferGPUMemSize NOTIFY newStats)

        Q_PROPERTY(quint32 textureCPUCount MEMBER textureCPUCount NOTIFY newStats)
        Q_PROPERTY(quint32 textureGPUCount MEMBER textureGPUCount NOTIFY newStats)
        Q_PROPERTY(quint32 textureResidentGPUCount MEMBER textureResidentGPUCount NOTIFY newStats)
        Q_PROPERTY(quint32 textureFramebufferGPUCount MEMBER textureFramebufferGPUCount NOTIFY newStats)
        Q_PROPERTY(quint32 textureResourceGPUCount MEMBER textureResourceGPUCount NOTIFY newStats)
        Q_PROPERTY(quint32 textureExternalGPUCount MEMBER textureExternalGPUCount NOTIFY newStats)

        Q_PROPERTY(qint64 textureCPUMemSize MEMBER textureCPUMemSize NOTIFY newStats)
        Q_PROPERTY(qint64 textureGPUMemSize MEMBER textureGPUMemSize NOTIFY newStats)
        Q_PROPERTY(qint64 textureResidentGPUMemSize MEMBER textureResidentGPUMemSize NOTIFY newStats)
        Q_PROPERTY(qint64 textureFramebufferGPUMemSize MEMBER textureFramebufferGPUMemSize NOTIFY newStats)
        Q_PROPERTY(qint64 textureResourceGPUMemSize MEMBER textureResourceGPUMemSize NOTIFY newStats)
        Q_PROPERTY(qint64 textureExternalGPUMemSize MEMBER textureExternalGPUMemSize NOTIFY newStats)

        Q_PROPERTY(quint32 texturePendingGPUTransferCount MEMBER texturePendingGPUTransferCount NOTIFY newStats)
        Q_PROPERTY(qint64 texturePendingGPUTransferSize MEMBER texturePendingGPUTransferSize NOTIFY newStats)
        Q_PROPERTY(qint64 textureResourcePopulatedGPUMemSize MEMBER textureResourcePopulatedGPUMemSize NOTIFY newStats)

        Q_PROPERTY(quint32 frameAPIDrawcallCount MEMBER frameAPIDrawcallCount NOTIFY newStats)
        Q_PROPERTY(quint32 frameDrawcallCount MEMBER frameDrawcallCount NOTIFY newStats)
        Q_PROPERTY(quint32 frameDrawcallRate MEMBER frameDrawcallRate NOTIFY newStats)

        Q_PROPERTY(quint32 frameTriangleCount MEMBER frameTriangleCount NOTIFY newStats)
        Q_PROPERTY(quint32 frameTriangleRate MEMBER frameTriangleRate NOTIFY newStats)

        Q_PROPERTY(quint32 frameTextureCount MEMBER frameTextureCount NOTIFY newStats)
        Q_PROPERTY(quint32 frameTextureRate MEMBER frameTextureRate NOTIFY newStats)
        Q_PROPERTY(quint64 frameTextureMemoryUsage MEMBER frameTextureMemoryUsage NOTIFY newStats)

        Q_PROPERTY(quint32 frameSetPipelineCount MEMBER frameSetPipelineCount NOTIFY newStats)
        Q_PROPERTY(quint32 frameSetInputFormatCount MEMBER frameSetInputFormatCount NOTIFY newStats)


    public:
        EngineStatsConfig() : Job::Config(true) {}

        quint32 bufferCPUCount{ 0 };
        quint32 bufferGPUCount{ 0 };
        qint64 bufferCPUMemSize { 0 };
        qint64 bufferGPUMemSize { 0 };

        quint32 textureCPUCount{ 0 };
        quint32 textureGPUCount { 0 };
        quint32 textureResidentGPUCount { 0 };
        quint32 textureFramebufferGPUCount { 0 };
        quint32 textureResourceGPUCount { 0 };
        quint32 textureExternalGPUCount { 0 };
        quint32 texturePendingGPUTransferCount { 0 };

        qint64 textureCPUMemSize { 0 };
        qint64 textureGPUMemSize { 0 };
        qint64 textureResidentGPUMemSize { 0 };
        qint64 textureFramebufferGPUMemSize { 0 };
        qint64 textureResourceGPUMemSize { 0 };
        qint64 textureExternalGPUMemSize { 0 };
        qint64 texturePendingGPUTransferSize { 0 };
        qint64 textureResourcePopulatedGPUMemSize { 0 };

        quint32 frameAPIDrawcallCount{ 0 };
        quint32 frameDrawcallCount{ 0 };
        quint32 frameDrawcallRate{ 0 };

        quint32 frameTriangleCount{ 0 };
        quint32 frameTriangleRate{ 0 };

        quint32 frameTextureCount{ 0 };
        quint32 frameTextureRate{ 0 };
        quint64 frameTextureMemoryUsage{ 0 };

        quint32 frameSetPipelineCount{ 0 };

        quint32 frameSetInputFormatCount{ 0 };
    };

    class EngineStats {
        gpu::ContextStats _gpuStats;
        QElapsedTimer _frameTimer;
    public:
        using Config = EngineStatsConfig;
        using JobModel = Job::Model<EngineStats, Config>;

        EngineStats() { _frameTimer.start(); }

        void configure(const Config& configuration) {}
        void run(const RenderContextPointer& renderContext);
    };
}

#endif
