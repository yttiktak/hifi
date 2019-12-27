//
//  RenderPipelines.h
//  render-utils/src/
//
//  Created by Sam Gondelman on 2/15/18
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_RenderPipelines_h
#define hifi_RenderPipelines_h

#include <graphics/Material.h>
#include <render/Args.h>

class RenderPipelines {
public:
    static void updateMultiMaterial(graphics::MultiMaterial& multiMaterial);
    static bool bindMaterial(graphics::MaterialPointer& material, gpu::Batch& batch, render::Args::RenderMode renderMode, bool enableTextures);
    static bool bindMaterials(graphics::MultiMaterial& multiMaterial, gpu::Batch& batch, render::Args::RenderMode renderMode, bool enableTextures);
};


#endif // hifi_RenderPipelines_h
