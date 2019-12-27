//
//  LightingModel.h
//  libraries/render-utils/src/
//
//  Created by Sam Gateau 7/1/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_LightingModel_h
#define hifi_LightingModel_h

#include <gpu/Resource.h>

#include <render/Forward.h>
#include <render/DrawTask.h>

// LightingModel is  a helper class gathering in one place the flags to enable the lighting contributions
class LightingModel {
public:
    using UniformBufferView = gpu::BufferView;

    LightingModel();


    void setUnlit(bool enable);
    bool isUnlitEnabled() const;

    void setEmissive(bool enable);
    bool isEmissiveEnabled() const;
    void setLightmap(bool enable);
    bool isLightmapEnabled() const;

    void setBackground(bool enable);
    bool isBackgroundEnabled() const;

    void setHaze(bool enable);
    bool isHazeEnabled() const;
    void setBloom(bool enable);
    bool isBloomEnabled() const;

    void setObscurance(bool enable);
    bool isObscuranceEnabled() const;

    void setScattering(bool enable);
    bool isScatteringEnabled() const;
    void setDiffuse(bool enable);
    bool isDiffuseEnabled() const;
    void setSpecular(bool enable);
    bool isSpecularEnabled() const;

    void setAlbedo(bool enable);
    bool isAlbedoEnabled() const;

    void setMaterialTexturing(bool enable);
    bool isMaterialTexturingEnabled() const;

    void setAmbientLight(bool enable);
    bool isAmbientLightEnabled() const;
    void setDirectionalLight(bool enable);
    bool isDirectionalLightEnabled() const;
    void setPointLight(bool enable);
    bool isPointLightEnabled() const;
    void setSpotLight(bool enable);
    bool isSpotLightEnabled() const;

    void setShowLightContour(bool enable);
    bool isShowLightContourEnabled() const;

    void setWireframe(bool enable);
    bool isWireframeEnabled() const;
    void setSkinning(bool enable);
    bool isSkinningEnabled() const;
    void setBlendshape(bool enable);
    bool isBlendshapeEnabled() const;


    void setAmbientOcclusion(bool enable);
    bool isAmbientOcclusionEnabled() const;
    void setShadow(bool enable);
    bool isShadowEnabled() const;

    UniformBufferView getParametersBuffer() const { return _parametersBuffer; }
    gpu::TexturePointer getAmbientFresnelLUT() const { return _ambientFresnelLUT; }

protected:


    // Class describing the uniform buffer with the transform info common to the AO shaders
    // It s changing every frame
    class Parameters {
    public:
        float enableUnlit{ 1.0f };
        float enableEmissive{ 1.0f };
        float enableLightmap{ 1.0f };
        float enableBackground{ 1.0f };

        float enableScattering{ 1.0f };
        float enableDiffuse{ 1.0f };
        float enableSpecular{ 1.0f };
        float enableAlbedo{ 1.0f };

        float enableAmbientLight{ 1.0f };
        float enableDirectionalLight{ 1.0f };
        float enablePointLight{ 1.0f };
        float enableSpotLight{ 1.0f };

        float showLightContour { 0.0f }; // false by default

        float enableObscurance{ 1.0f };

        float enableMaterialTexturing { 1.0f };
        float enableWireframe { 0.0f }; // false by default

        float enableHaze{ 1.0f };
        float enableBloom{ 1.0f };
        float enableSkinning{ 1.0f };
        float enableBlendshape{ 1.0f };

        float enableAmbientOcclusion{ 0.0f }; // false by default
        float enableShadow{ 1.0f };
        float spare1{ 1.0f };
        float spare2{ 1.0f };

        Parameters() {}
    };
    UniformBufferView _parametersBuffer;
    static gpu::TexturePointer _ambientFresnelLUT;
};

using LightingModelPointer = std::shared_ptr<LightingModel>;




class MakeLightingModelConfig : public render::Job::Config {
    Q_OBJECT

    Q_PROPERTY(bool enableUnlit MEMBER enableUnlit NOTIFY dirty)
    Q_PROPERTY(bool enableEmissive MEMBER enableEmissive NOTIFY dirty)
    Q_PROPERTY(bool enableLightmap MEMBER enableLightmap NOTIFY dirty)
    Q_PROPERTY(bool enableBackground MEMBER enableBackground NOTIFY dirty)
    Q_PROPERTY(bool enableHaze MEMBER enableHaze NOTIFY dirty)

    Q_PROPERTY(bool enableObscurance MEMBER enableObscurance NOTIFY dirty)

    Q_PROPERTY(bool enableScattering MEMBER enableScattering NOTIFY dirty)
    Q_PROPERTY(bool enableDiffuse MEMBER enableDiffuse NOTIFY dirty)
    Q_PROPERTY(bool enableSpecular MEMBER enableSpecular NOTIFY dirty)
    Q_PROPERTY(bool enableAlbedo MEMBER enableAlbedo NOTIFY dirty)

    Q_PROPERTY(bool enableMaterialTexturing MEMBER enableMaterialTexturing NOTIFY dirty)

    Q_PROPERTY(bool enableAmbientLight MEMBER enableAmbientLight NOTIFY dirty)
    Q_PROPERTY(bool enableDirectionalLight MEMBER enableDirectionalLight NOTIFY dirty)
    Q_PROPERTY(bool enablePointLight MEMBER enablePointLight NOTIFY dirty)
    Q_PROPERTY(bool enableSpotLight MEMBER enableSpotLight NOTIFY dirty)

    Q_PROPERTY(bool enableWireframe MEMBER enableWireframe NOTIFY dirty)
    Q_PROPERTY(bool showLightContour MEMBER showLightContour NOTIFY dirty)

    Q_PROPERTY(bool enableBloom MEMBER enableBloom NOTIFY dirty)
    Q_PROPERTY(bool enableSkinning MEMBER enableSkinning NOTIFY dirty)
    Q_PROPERTY(bool enableBlendshape MEMBER enableBlendshape NOTIFY dirty)

    Q_PROPERTY(bool enableAmbientOcclusion READ isAmbientOcclusionEnabled WRITE setAmbientOcclusion NOTIFY dirty)
    Q_PROPERTY(bool enableShadow READ isShadowEnabled WRITE setShadow NOTIFY dirty)


public:
    MakeLightingModelConfig() : render::Job::Config() {} // Make Lighting Model is always on

    bool enableUnlit{ true };
    bool enableEmissive{ true };
    bool enableLightmap{ true };
    bool enableBackground{ true };
    bool enableObscurance{ true };

    bool enableScattering{ true };
    bool enableDiffuse{ true };
    bool enableSpecular{ true };

    bool enableAlbedo{ true };
    bool enableMaterialTexturing { true };

    bool enableAmbientLight{ true };
    bool enableDirectionalLight{ true };
    bool enablePointLight{ true };
    bool enableSpotLight{ true };

    bool showLightContour { false }; // false by default

    bool enableWireframe { false }; // false by default
    bool enableHaze{ true };
    bool enableBloom{ true };
    bool enableSkinning{ true };
    bool enableBlendshape{ true };

    bool enableAmbientOcclusion{ false }; // false by default
    bool enableShadow{ true };


    void setAmbientOcclusion(bool enable) { enableAmbientOcclusion = enable; emit dirty();}
    bool isAmbientOcclusionEnabled() const { return enableAmbientOcclusion; }
    void setShadow(bool enable) { enableShadow = enable; emit dirty(); }
    bool isShadowEnabled() const { return enableShadow; }

signals:
    void dirty();
};

class MakeLightingModel {
public:
    using Config = MakeLightingModelConfig;
    using JobModel = render::Job::ModelO<MakeLightingModel, LightingModelPointer, Config>;

    MakeLightingModel();

    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, LightingModelPointer& lightingModel);

private:
    LightingModelPointer _lightingModel;
};

#endif // hifi_SurfaceGeometryPass_h
