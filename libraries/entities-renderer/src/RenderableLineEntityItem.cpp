//
//  RenderableLineEntityItem.cpp
//  libraries/entities-renderer/src/
//
//  Created by Seth Alves on 5/11/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableLineEntityItem.h"

#include <gpu/Batch.h>
#include <PerfStat.h>

using namespace render;
using namespace render::entities;

void LineEntityRenderer::onRemoveFromSceneTyped(const TypedEntityPointer& entity) {
    if (_lineVerticesID != GeometryCache::UNKNOWN_ID) {
        auto geometryCache = DependencyManager::get<GeometryCache>();
        if (geometryCache) {
            geometryCache->releaseID(_lineVerticesID);
        }
    }
}

void LineEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    _linePoints = entity->getLinePoints();
    auto geometryCache = DependencyManager::get<GeometryCache>();
    if (_lineVerticesID == GeometryCache::UNKNOWN_ID) {
        _lineVerticesID = geometryCache->allocateID();
    }
    glm::vec4 lineColor(toGlm(entity->getColor()), 1.0f);
    geometryCache->updateVertices(_lineVerticesID, _linePoints, lineColor);
}

void LineEntityRenderer::doRender(RenderArgs* args) {
    if (_lineVerticesID == GeometryCache::UNKNOWN_ID) {
        return;
    }

    PerformanceTimer perfTimer("RenderableLineEntityItem::render");
    Q_ASSERT(args->_batch);
    gpu::Batch& batch = *args->_batch;
    const auto& modelTransform = getModelTransform();
    Transform transform = Transform();
    transform.setTranslation(modelTransform.getTranslation());
    transform.setRotation(modelTransform.getRotation());
    batch.setModelTransform(transform);
    if (_linePoints.size() > 1) {
        DependencyManager::get<GeometryCache>()->bindSimpleProgram(batch, false, false, false, false, true,
            _renderLayer != RenderLayer::WORLD || args->_renderMethod == Args::RenderMethod::FORWARD);
        DependencyManager::get<GeometryCache>()->renderVertices(batch, gpu::LINE_STRIP, _lineVerticesID);
    }
}
