#ifndef QGSPOLYGON3DSYMBOL_H
#define QGSPOLYGON3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "phongmaterialsettings.h"
#include "utils.h"


//! 3D symbol that draws polygon geometries as planar polygons, optionally extruded (with added walls).
class _3D_EXPORT QgsPolygon3DSymbol : public QgsAbstract3DSymbol
{
  public:
    QgsPolygon3DSymbol();

    QString type() const override { return "polygon"; }
    QgsAbstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects
};


#endif // QGSPOLYGON3DSYMBOL_H
