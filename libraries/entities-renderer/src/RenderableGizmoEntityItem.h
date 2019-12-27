//
//  Created by Sam Gondelman on 1/22/19
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_RenderableGizmoEntityItem_h
#define hifi_RenderableGizmoEntityItem_h

#include "RenderableEntityItem.h"

#include <GizmoEntityItem.h>

namespace render { namespace entities {

class GizmoEntityRenderer : public TypedEntityRenderer<GizmoEntityItem> {
    using Parent = TypedEntityRenderer<GizmoEntityItem>;
    using Pointer = std::shared_ptr<GizmoEntityRenderer>;
public:
    GizmoEntityRenderer(const EntityItemPointer& entity);
    ~GizmoEntityRenderer();

protected:
    Item::Bound getBound() override;
    ShapeKey getShapeKey() override;

    bool isTransparent() const override;

private:
    virtual void doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) override;
    virtual void doRender(RenderArgs* args) override;

    GizmoType _gizmoType { UNSET_GIZMO_TYPE };
    RingGizmoPropertyGroup _ringProperties;
    PrimitiveMode _prevPrimitiveMode;

    int _ringGeometryID { 0 };
    int _majorTicksGeometryID { 0 };
    int _minorTicksGeometryID { 0 };
    gpu::Primitive _solidPrimitive { gpu::TRIANGLE_FAN };

};

} }
#endif // hifi_RenderableGizmoEntityItem_h
