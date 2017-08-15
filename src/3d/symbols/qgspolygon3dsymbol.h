#ifndef QGSPOLYGON3DSYMBOL_H
#define QGSPOLYGON3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"
#include "qgs3dutils.h"


//! 3D symbol that draws polygon geometries as planar polygons, optionally extruded (with added walls).
class _3D_EXPORT QgsPolygon3DSymbol : public QgsAbstract3DSymbol
{
  public:
    QgsPolygon3DSymbol();

    QString type() const override { return "polygon"; }
    QgsAbstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    AltitudeClamping altitudeClamping() const { return mAltClamping; }
    void setAltitudeClamping( AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    AltitudeBinding altitudeBinding() const { return mAltBinding; }
    void setAltitudeBinding( AltitudeBinding altBinding ) { mAltBinding = altBinding; }

    float height() const { return mHeight; }
    void setHeight( float height ) { mHeight = height; }

    float extrusionHeight() const { return mExtrusionHeight; }
    void setExtrusionHeight( float extrusionHeight ) { mExtrusionHeight = extrusionHeight; }

    QgsPhongMaterialSettings material() const { return mMaterial; }
    void setMaterial( const QgsPhongMaterialSettings &material ) { mMaterial = material; }

  private:
    AltitudeClamping mAltClamping;  //! how to handle altitude of vector features
    AltitudeBinding mAltBinding;    //! how to handle clamping of vertices of individual features

    float mHeight;           //!< Base height of polygons
    float mExtrusionHeight;  //!< How much to extrude (0 means no walls)
    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
};


#endif // QGSPOLYGON3DSYMBOL_H
