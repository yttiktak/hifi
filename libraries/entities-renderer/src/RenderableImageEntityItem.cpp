//
//  Created by Sam Gondelman on 11/29/18
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableImageEntityItem.h"

#include <DependencyManager.h>
#include <GeometryCache.h>

using namespace render;
using namespace render::entities;

ImageEntityRenderer::ImageEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    _geometryId = DependencyManager::get<GeometryCache>()->allocateID();
}

ImageEntityRenderer::~ImageEntityRenderer() {
    auto geometryCache = DependencyManager::get<GeometryCache>();
    if (geometryCache) {
        geometryCache->releaseID(_geometryId);
    }
}

bool ImageEntityRenderer::isTransparent() const {
    return Parent::isTransparent() || (_textureIsLoaded && _texture->getGPUTexture() && _texture->getGPUTexture()->getUsage().isAlpha()) || _alpha < 1.0f || _pulseProperties.getAlphaMode() != PulseMode::NONE;
}

bool ImageEntityRenderer::needsRenderUpdate() const {
    bool textureLoadedChanged = resultWithReadLock<bool>([&] {
        return (!_textureIsLoaded && _texture && _texture->isLoaded());
    });

    if (textureLoadedChanged) {
        return true;
    }

    return Parent::needsRenderUpdate();
}

void ImageEntityRenderer::doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) {
    withWriteLock([&] {
        auto imageURL = entity->getImageURL();
        if (_imageURL != imageURL) {
            _imageURL = imageURL;
            if (imageURL.isEmpty()) {
                _texture.reset();
            } else {
                _texture = DependencyManager::get<TextureCache>()->getTexture(_imageURL);
            }
            _textureIsLoaded = false;
        }

        _emissive = entity->getEmissive();
        _keepAspectRatio = entity->getKeepAspectRatio();
        _subImage = entity->getSubImage();

        _color = entity->getColor();
        _alpha = entity->getAlpha();
        _pulseProperties = entity->getPulseProperties();
        _billboardMode = entity->getBillboardMode();

        if (!_textureIsLoaded && _texture && _texture->isLoaded()) {
            _textureIsLoaded = true;
        }
    });

    void* key = (void*)this;
    AbstractViewStateInterface::instance()->pushPostUpdateLambda(key, [this, entity]() {
        withWriteLock([&] {
            updateModelTransformAndBound();
            _renderTransform = getModelTransform();
            _renderTransform.postScale(entity->getScaledDimensions());
        });
    });
}

Item::Bound ImageEntityRenderer::getBound() {
    auto bound = Parent::getBound();
    if (_billboardMode != BillboardMode::NONE) {
        glm::vec3 dimensions = bound.getScale();
        float max = glm::max(dimensions.x, glm::max(dimensions.y, dimensions.z));
        const float SQRT_2 = 1.41421356237f;
        bound.setScaleStayCentered(glm::vec3(SQRT_2 * max));
    }
    return bound;
}

ShapeKey ImageEntityRenderer::getShapeKey() {
    auto builder = render::ShapeKey::Builder().withoutCullFace().withDepthBias();
    if (isTransparent()) {
        builder.withTranslucent();
    }

    withReadLock([&] {
        if (_emissive) {
            builder.withUnlit();
        }

        if (_primitiveMode == PrimitiveMode::LINES) {
            builder.withWireframe();
        }
    });

    return builder.build();
}

void ImageEntityRenderer::doRender(RenderArgs* args) {
    NetworkTexturePointer texture;
    QRect subImage;
    glm::vec4 color;
    Transform transform;
    withReadLock([&] {
        texture = _texture;
        subImage = _subImage;
        color = glm::vec4(toGlm(_color), _alpha);
        color = EntityRenderer::calculatePulseColor(color, _pulseProperties, _created);
        transform = _renderTransform;
    });

    if (!_visible || !texture || !texture->isLoaded() || color.a == 0.0f) {
        return;
    }

    Q_ASSERT(args->_batch);
    gpu::Batch* batch = args->_batch;

    transform.setRotation(EntityItem::getBillboardRotation(transform.getTranslation(), transform.getRotation(), _billboardMode, args->getViewFrustum().getPosition()));

    batch->setModelTransform(transform);
    batch->setResourceTexture(0, texture->getGPUTexture());

    float imageWidth = texture->getWidth();
    float imageHeight = texture->getHeight();

    QRect fromImage;
    if (subImage.width() <= 0) {
        fromImage.setX(0);
        fromImage.setWidth(imageWidth);
    } else {
        float scaleX = imageWidth / texture->getOriginalWidth();
        fromImage.setX(scaleX * subImage.x());
        fromImage.setWidth(scaleX * subImage.width());
    }

    if (subImage.height() <= 0) {
        fromImage.setY(0);
        fromImage.setHeight(imageHeight);
    } else {
        float scaleY = imageHeight / texture->getOriginalHeight();
        fromImage.setY(scaleY * subImage.y());
        fromImage.setHeight(scaleY * subImage.height());
    }

    float maxSize = glm::max(fromImage.width(), fromImage.height());
    float x = _keepAspectRatio ? fromImage.width() / (2.0f * maxSize) : 0.5f;
    float y = _keepAspectRatio ? fromImage.height() / (2.0f * maxSize) : 0.5f;

    glm::vec2 texCoordBottomLeft((fromImage.x() + 0.5f) / imageWidth, (fromImage.y() + fromImage.height() - 0.5f) / imageHeight);
    glm::vec2 texCoordTopRight((fromImage.x() + fromImage.width() - 0.5f) / imageWidth, (fromImage.y() + 0.5f) / imageHeight);

    DependencyManager::get<GeometryCache>()->renderQuad(
        *batch, glm::vec2(-x, -y), glm::vec2(x, y), texCoordBottomLeft, texCoordTopRight,
        color, _geometryId
    );

    batch->setResourceTexture(0, nullptr);
}
