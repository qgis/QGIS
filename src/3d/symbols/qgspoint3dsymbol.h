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
#include "qgs3dtypes.h"

#include <QMatrix4x4>

class QgsAbstractMaterialSettings;
class QgsMarkerSymbol;

/**
 * \ingroup 3d
 * \brief 3D symbol that draws point geometries as 3D objects using one of the predefined shapes.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPoint3DSymbol : public QgsAbstract3DSymbol SIP_NODEFAULTCTORS
{
  public:
    //! Constructor for QgsPoint3DSymbol with default QgsMarkerSymbol as the billboardSymbol
    QgsPoint3DSymbol();

    //! Copy Constructor for QgsPoint3DSymbol
    QgsPoint3DSymbol( const QgsPoint3DSymbol &other );

    ~QgsPoint3DSymbol() override;

    /**
     * Creates a new QgsPoint3DSymbol.
     *
     * Caller takes ownership of the returned symbol.
     */
    static QgsAbstract3DSymbol *create() SIP_FACTORY;

    QString type() const override { return "point"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    QList< QgsWkbTypes::GeometryType > compatibleGeometryTypes() const override;
    void setDefaultPropertiesFromLayer( const QgsVectorLayer *layer ) override;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgis::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgis::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns material used for shading of the symbol
    QgsAbstractMaterialSettings *material() const;

    /**
     * Sets the \a material settings used for shading of the symbol.
     *
     * Ownership of \a material is transferred to the symbol.
     */
    void setMaterial( QgsAbstractMaterialSettings *material SIP_TRANSFER );

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
      Billboard,
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

    //! Returns a symbol for billboard
    QgsMarkerSymbol *billboardSymbol() const;
    //! Set symbol for billboard and the ownership is transferred
    void setBillboardSymbol( QgsMarkerSymbol *symbol );

    //! Returns transform for individual objects represented by the symbol
    QMatrix4x4 transform() const { return mTransform; }
    //! Sets transform for individual objects represented by the symbol
    void setTransform( const QMatrix4x4 &transform ) { mTransform = transform; }

    //! Returns transform for billboards
    QMatrix4x4 billboardTransform() const;

    /**
     * Exports the geometries contained within the hierarchy of entity.
     * Returns whether any objects were exported
     */
    bool exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const override SIP_SKIP;
  private:
    //! how to handle altitude of vector features
    Qgis::AltitudeClamping mAltClamping = Qgis::AltitudeClamping::Relative;

    std::unique_ptr< QgsAbstractMaterialSettings> mMaterial;  //!< Defines appearance of objects
    Shape mShape = Cylinder;  //!< What kind of shape to use
    QVariantMap mShapeProperties;  //!< Key-value dictionary of shape's properties (different keys for each shape)
    QMatrix4x4 mTransform;  //!< Transform of individual instanced models
    std::unique_ptr<QgsMarkerSymbol> mBillboardSymbol;
#ifdef SIP_RUN
    QgsPoint3DSymbol &operator=( const QgsPoint3DSymbol & );
#else
    QgsPoint3DSymbol &operator=( const QgsPoint3DSymbol & ) = delete;
#endif
};


#endif // QGSPOINT3DSYMBOL_H
