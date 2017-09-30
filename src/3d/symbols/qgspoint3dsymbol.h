#ifndef QGSPOINT3DSYMBOL_H
#define QGSPOINT3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"
#include "qgs3dutils.h"


/** \ingroup 3d
 * 3D symbol that draws point geometries as 3D objects using one of the predefined shapes.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPoint3DSymbol : public QgsAbstract3DSymbol
{
  public:
    QgsPoint3DSymbol();

    QString type() const override { return "point"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    //! Returns material used for shading of the symbol
    QgsPhongMaterialSettings material() const { return mMaterial; }
    //! Sets material used for shading of the symbol
    void setMaterial( const QgsPhongMaterialSettings &material ) { mMaterial = material; }

    //! Returns a key-value dictionary of point shape properties
    QVariantMap shapeProperties() const { return mShapeProperties; }
    //! Sets a key-value dictionary of point shape properties
    void setShapeProperties( const QVariantMap &properties ) { mShapeProperties = properties; }

    //! Returns transform for individual objects represented by the symbol
    QMatrix4x4 transform() const { return mTransform; }
    //! Sets transform for individual objects represented by the symbol
    void setTransform( const QMatrix4x4 &transform ) { mTransform = transform; }

  private:
    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
    QVariantMap mShapeProperties;  //!< What kind of shape to use and what
    QMatrix4x4 mTransform;  //!< Transform of individual instanced models
};


#endif // QGSPOINT3DSYMBOL_H
