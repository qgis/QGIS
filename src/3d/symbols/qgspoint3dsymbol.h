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
#include "qgis.h"

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
 */
class _3D_EXPORT QgsPoint3DSymbol : public QgsAbstract3DSymbol SIP_NODEFAULTCTORS
{
  public:
    //! Constructor for QgsPoint3DSymbol with default QgsMarkerSymbol as the billboardSymbol
    QgsPoint3DSymbol();

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
    QList<Qgis::GeometryType> compatibleGeometryTypes() const override;
    void setDefaultPropertiesFromLayer( const QgsVectorLayer *layer ) override;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgis::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgis::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns material settings used for shading of the symbol
    QgsAbstractMaterialSettings *materialSettings() const;

    /**
     * Sets the \a material settings used for shading of the symbol.
     *
     * Ownership of \a material is transferred to the symbol.
     */
    void setMaterialSettings( QgsAbstractMaterialSettings *materialSettings SIP_TRANSFER );

    //! Returns shape enum value from a string
    static Qgis::Point3DShape shapeFromString( const QString &shape );
    //! Returns string from a shape enum value
    static QString shapeToString( Qgis::Point3DShape shape );

    //! Returns 3D shape for points
    Qgis::Point3DShape shape() const { return mShape; }
    //! Sets 3D shape for points
    void setShape( Qgis::Point3DShape shape ) { mShape = shape; }

    /**
     * Returns a key-value dictionary of point shape properties.
     *
     * In most cases callers should use shapeProperty() instead, to
     * correctly handle default values when a property has not been
     * explicitly set.
     *
     * \see setShapeProperties() for a description of available properties.
     * \see shapeProperty()
     */
    QVariantMap shapeProperties() const { return mShapeProperties; }

    /**
     * Returns the value for a specific shape \a property.
     *
     * This method accounts for default property values for the symbol's shape(),
     * used when the property has not been explicitly set.
     *
     * \see setShapeProperties() for a description of available properties.
     * \since QGIS 3.36
     */
    QVariant shapeProperty( const QString &property ) const;

    /**
     * Sets a key-value dictionary of point shape \a properties.
     *
     * The available properties depend on the point shape().
     *
     * ### Cylinders (Qgis.Point3DShape.Cylinder)
     *
     * - \a radius
     * - \a length
     *
     * ### Spheres (Qgis.Point3DShape.Sphere)
     *
     * - \a radius
     *
     * ### Cones (Qgis.Point3DShape.Cone)
     *
     * - \a length
     * - \a topRadius
     * - \a bottomRadius
     *
     * ### Cubes (Qgis.Point3DShape.Cube)
     *
     * - \a size
     *
     * ### Torus (Qgis.Point3DShape.Torus)
     *
     * - \a radius
     * - \a minorRadius
     *
     * ### Flat Planes (Qgis.Point3DShape.Plane)
     *
     * - \a size
     *
     * ### Extruded Text (Qgis.Point3DShape.ExtrudedText)
     *
     * - \a depth
     * - \a text (string)
     *
     * ### Models (Qgis.Point3DShape.Model)
     *
     * - \a model (path to model file)
     *
     * \see shapeProperty()
     */
    void setShapeProperties( const QVariantMap &properties ) { mShapeProperties = properties; }

    //! Returns a symbol for billboard
    QgsMarkerSymbol *billboardSymbol() const;
    //! Set symbol for billboard and the ownership is transferred
    void setBillboardSymbol( QgsMarkerSymbol *symbol );

    //! Returns transform for individual objects represented by the symbol
    QMatrix4x4 transform() const { return mTransform; }
    //! Sets transform for individual objects represented by the symbol
    void setTransform( const QMatrix4x4 &transform ) { mTransform = transform; }

    //! Returns how much the billboard should be elevated upwards
    float billboardHeight() const;

    /**
     * Exports the geometries contained within the hierarchy of entity.
     * Returns whether any objects were exported
     */
    bool exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const override SIP_SKIP;

  private:
    //! how to handle altitude of vector features
    Qgis::AltitudeClamping mAltClamping = Qgis::AltitudeClamping::Relative;

    std::unique_ptr<QgsAbstractMaterialSettings> mMaterialSettings; //!< Defines appearance of objects
    Qgis::Point3DShape mShape = Qgis::Point3DShape::Cylinder;       //!< What kind of shape to use
    QVariantMap mShapeProperties;                                   //!< Key-value dictionary of shape's properties (different keys for each shape)
    QMatrix4x4 mTransform;                                          //!< Transform of individual instanced models
    std::unique_ptr<QgsMarkerSymbol> mBillboardSymbol;
#ifdef SIP_RUN
    QgsPoint3DSymbol &operator=( const QgsPoint3DSymbol & );
#else
    QgsPoint3DSymbol &operator=( const QgsPoint3DSymbol & ) = delete;
#endif
};


#endif // QGSPOINT3DSYMBOL_H
