/***************************************************************************
  qgspoint3dsymbol.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINT3DSYMBOL_H
#define QGSPOINT3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgsphongmaterialsettings.h"
#include "qgs3dtypes.h"

#include <QMatrix4x4>

/**
 * \ingroup 3d
 * 3D symbol that draws point geometries as 3D objects using one of the predefined shapes.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPoint3DSymbol : public QgsAbstract3DSymbol
{
  public:
    //! Constructor for QgsPoint3DSymbol.
    QgsPoint3DSymbol() = default;

    QString type() const override { return "point"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgs3DTypes::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgs3DTypes::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns material used for shading of the symbol
    QgsPhongMaterialSettings material() const { return mMaterial; }
    //! Sets material used for shading of the symbol
    void setMaterial( const QgsPhongMaterialSettings &material ) { mMaterial = material; }

    //! 3D shape types supported by the symbol
    enum Shape
    {
      Cylinder,
      Sphere,
      Cone,
      Cube,
      Torus,
      Plane,
      ExtrudedText,  //!< Supported in Qt 5.9+
      Model,
    };

    //! Returns shape enum value from a string
    static Shape shapeFromString( const QString &shape );
    //! Returns string from a shape enum value
    static QString shapeToString( Shape shape );

    //! Returns 3D shape for points
    Shape shape() const { return mShape; }
    //! Sets 3D shape for points
    void setShape( Shape shape ) { mShape = shape; }

    //! Returns a key-value dictionary of point shape properties
    QVariantMap shapeProperties() const { return mShapeProperties; }
    //! Sets a key-value dictionary of point shape properties
    void setShapeProperties( const QVariantMap &properties ) { mShapeProperties = properties; }

    //! Returns transform for individual objects represented by the symbol
    QMatrix4x4 transform() const { return mTransform; }
    //! Sets transform for individual objects represented by the symbol
    void setTransform( const QMatrix4x4 &transform ) { mTransform = transform; }

  private:
    //! how to handle altitude of vector features
    Qgs3DTypes::AltitudeClamping mAltClamping = Qgs3DTypes::AltClampRelative;

    QgsPhongMaterialSettings mMaterial;  //!< Defines appearance of objects
    Shape mShape = Cylinder;  //!< What kind of shape to use
    QVariantMap mShapeProperties;  //!< Key-value dictionary of shape's properties (different keys for each shape)
    QMatrix4x4 mTransform;  //!< Transform of individual instanced models
};


#endif // QGSPOINT3DSYMBOL_H
