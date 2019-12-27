//
//  Overlay2D.h
//  interface/src/ui/overlays
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_Overlay2D_h
#define hifi_Overlay2D_h

#include <QRect>

#include "Overlay.h"

class Overlay2D : public Overlay {
    Q_OBJECT

public:
    Overlay2D() {}
    Overlay2D(const Overlay2D* overlay2D);

    virtual AABox getBounds() const override;
    virtual uint32_t fetchMetaSubItems(render::ItemIDs& subItems) const override { subItems.push_back(getRenderItemID()); return 1; }

    // getters
    const QRect& getBoundingRect() const { return _bounds; }

    // setters
    void setX(int x) { _bounds.setX(x); }
    void setY(int y) { _bounds.setY(y);  }
    void setWidth(int width) { _bounds.setWidth(width); }
    void setHeight(int height) { _bounds.setHeight(height); }
    void setBounds(const QRect& bounds) { _bounds = bounds; }

    void setProperties(const QVariantMap& properties) override;

protected:
    QRect _bounds; // where on the screen to draw
};

 
#endif // hifi_Overlay2D_h
