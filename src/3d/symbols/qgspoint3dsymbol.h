#ifndef QGSPOINT3DSYMBOL_H
#define QGSPOINT3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "phongmaterialsettings.h"
#include "utils.h"


//! 3D symbol that draws point geometries as 3D objects using one of the predefined shapes.
class _3D_EXPORT QgsPoint3DSymbol : public QgsAbstract3DSymbol
{
  public:
    QgsPoint3DSymbol();

    QString type() const override { return "point"; }
    QgsAbstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    PhongMaterialSettings material() const { return mMaterial; }
    void setMaterial( const PhongMaterialSettings &material ) { mMaterial = material; }

    QVariantMap shapeProperties() const { return mShapeProperties; }
    void setShapeProperties( const QVariantMap &properties ) { mShapeProperties = properties; }

    QMatrix4x4 transform() const { return mTransform; }
    void setTransform( const QMatrix4x4 &transform ) { mTransform = transform; }

  private:
    PhongMaterialSettings mMaterial;  //!< Defines appearance of objects
    QVariantMap mShapeProperties;  //!< What kind of shape to use and what
    QMatrix4x4 mTransform;  //!< Transform of individual instanced models
};


#endif // QGSPOINT3DSYMBOL_H
