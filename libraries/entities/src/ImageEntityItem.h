//
//  Created by Sam Gondelman on 11/29/18
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_ImageEntityItem_h
#define hifi_ImageEntityItem_h

#include "EntityItem.h"

#include "PulsePropertyGroup.h"

class ImageEntityItem : public EntityItem {
    using Pointer = std::shared_ptr<ImageEntityItem>;
public:
    static EntityItemPointer factory(const EntityItemID& entityID, const EntityItemProperties& properties);

    ImageEntityItem(const EntityItemID& entityItemID);

    ALLOW_INSTANTIATION // This class can be instantiated

    virtual void setUnscaledDimensions(const glm::vec3& value) override;

    // methods for getting/setting all properties of an entity
    EntityItemProperties getProperties(const EntityPropertyFlags& desiredProperties, bool allowEmptyDesiredProperties) const override;
    bool setProperties(const EntityItemProperties& properties) override;

    EntityPropertyFlags getEntityProperties(EncodeBitstreamParams& params) const override;

    void appendSubclassData(OctreePacketData* packetData, EncodeBitstreamParams& params,
                            EntityTreeElementExtraEncodeDataPointer entityTreeElementExtraEncodeData,
                            EntityPropertyFlags& requestedProperties,
                            EntityPropertyFlags& propertyFlags,
                            EntityPropertyFlags& propertiesDidntFit,
                            int& propertyCount,
                            OctreeElement::AppendState& appendState) const override;

    int readEntitySubclassDataFromBuffer(const unsigned char* data, int bytesLeftToRead,
                                         ReadBitstreamToTreeParams& args,
                                         EntityPropertyFlags& propertyFlags, bool overwriteLocalData,
                                         bool& somethingChanged) override;

    glm::vec3 getRaycastDimensions() const override;
    virtual bool supportsDetailedIntersection() const override { return true; }
    virtual bool findDetailedRayIntersection(const glm::vec3& origin, const glm::vec3& direction,
                         OctreeElementPointer& element, float& distance,
                         BoxFace& face, glm::vec3& surfaceNormal,
                         QVariantMap& extraInfo, bool precisionPicking) const override;
    virtual bool findDetailedParabolaIntersection(const glm::vec3& origin, const glm::vec3& velocity,
                         const glm::vec3& acceleration, OctreeElementPointer& element, float& parabolicDistance,
                         BoxFace& face, glm::vec3& surfaceNormal,
                         QVariantMap& extraInfo, bool precisionPicking) const override;

    void setImageURL(const QString& imageUrl);
    QString getImageURL() const;

    void setEmissive(bool emissive);
    bool getEmissive() const;

    void setKeepAspectRatio(bool keepAspectRatio);
    bool getKeepAspectRatio() const;

    void setBillboardMode(BillboardMode value);
    BillboardMode getBillboardMode() const;

    void setSubImage(const QRect& subImage);
    QRect getSubImage() const;

    void setColor(const glm::u8vec3& color);
    glm::u8vec3 getColor() const;

    void setAlpha(float alpha);
    float getAlpha() const;

    PulsePropertyGroup getPulseProperties() const;

protected:
    glm::u8vec3 _color;
    float _alpha;
    PulsePropertyGroup _pulseProperties;
    BillboardMode _billboardMode;

    QString _imageURL;
    bool _emissive { false };
    bool _keepAspectRatio { true };
    QRect _subImage;
};

#endif // hifi_ImageEntityItem_h
