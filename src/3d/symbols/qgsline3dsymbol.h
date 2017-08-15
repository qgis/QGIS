#ifndef QGSLINE3DSYMBOL_H
#define QGSLINE3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "phongmaterialsettings.h"
#include "utils.h"


//! 3D symbol that draws linestring geometries as planar polygons (created from lines using a buffer with given thickness).
class _3D_EXPORT QgsLine3DSymbol : public QgsAbstract3DSymbol
{
  public:
    QgsLine3DSymbol();

    QString type() const override { return "line"; }
    QgsAbstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    AltitudeClamping altitudeClamping() const { return mAltClamping; }
    void setAltitudeClamping( AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    AltitudeBinding altitudeBinding() const { return mAltBinding; }
    void setAltitudeBinding( AltitudeBinding altBinding ) { mAltBinding = altBinding; }

    float width() const { return mWidth; }
    void setWidth( float width ) { mWidth = width; }

    float height() const { return mHeight; }
    void setHeight( float height ) { mHeight = height; }

    float extrusionHeight() const { return mExtrusionHeight; }
    void setExtrusionHeight( float extrusionHeight ) { mExtrusionHeight = extrusionHeight; }

    PhongMaterialSettings material() const { return mMaterial; }
    void setMaterial( const PhongMaterialSettings &material ) { mMaterial = material; }

  private:
    AltitudeClamping mAltClamping;  //! how to handle altitude of vector features
    AltitudeBinding mAltBinding;    //! how to handle clamping of vertices of individual features

    float mWidth;            //!< Line width (horizontally)
    float mHeight;           //!< Base height of polygons
    float mExtrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings mMaterial;  //!< Defines appearance of objects
};


#endif // QGSLINE3DSYMBOL_H
