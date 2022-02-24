/***************************************************************************
  qgsline3dsymbol.h
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

#ifndef QGSLINE3DSYMBOL_H
#define QGSLINE3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgs3dtypes.h"

class QgsAbstractMaterialSettings;

/**
 * \ingroup 3d
 * \brief 3D symbol that draws linestring geometries as planar polygons (created from lines using a buffer with given thickness).
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsLine3DSymbol : public QgsAbstract3DSymbol SIP_NODEFAULTCTORS
{
  public:
    //! Constructor for QgsLine3DSymbol
    QgsLine3DSymbol();
    ~QgsLine3DSymbol() override;

    /**
     * Creates a new QgsLine3DSymbol.
     *
     * Caller takes ownership of the returned symbol.
     */
    static QgsAbstract3DSymbol *create() SIP_FACTORY;

    QString type() const override { return "line"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    QList< QgsWkbTypes::GeometryType > compatibleGeometryTypes() const override;
    void setDefaultPropertiesFromLayer( const QgsVectorLayer *layer ) override;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgis::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgis::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns method that determines how altitude is bound to individual vertices
    Qgis::AltitudeBinding altitudeBinding() const { return mAltBinding; }
    //! Sets method that determines how altitude is bound to individual vertices
    void setAltitudeBinding( Qgis::AltitudeBinding altBinding ) { mAltBinding = altBinding; }

    //! Returns width of the line symbol (in map units)
    float width() const { return mWidth; }
    //! Sets width of the line symbol (in map units)
    void setWidth( float width ) { mWidth = width; }

    //! Returns height (altitude) of the symbol (in map units)
    float height() const { return mHeight; }
    //! Sets height (altitude) of the symbol (in map units)
    void setHeight( float height ) { mHeight = height; }

    //! Returns extrusion height (in map units)
    float extrusionHeight() const { return mExtrusionHeight; }
    //! Sets extrusion height (in map units)
    void setExtrusionHeight( float extrusionHeight ) { mExtrusionHeight = extrusionHeight; }

    //! Returns whether the renderer will render data with simple lines (otherwise it uses buffer)
    bool renderAsSimpleLines() const { return mRenderAsSimpleLines; }
    //! Sets whether the renderer will render data with simple lines (otherwise it uses buffer)
    void setRenderAsSimpleLines( bool enabled ) { mRenderAsSimpleLines = enabled; }

    //! Returns material used for shading of the symbol
    QgsAbstractMaterialSettings *material() const;

    /**
     * Sets the \a material settings used for shading of the symbol.
     *
     * Ownership of \a material is transferred to the symbol.
     */
    void setMaterial( QgsAbstractMaterialSettings *material SIP_TRANSFER );

    /**
     * Exports the geometries contained within the hierarchy of entity.
     * Returns whether any objects were exported
     */
    bool exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const override SIP_SKIP;

  private:
    //! how to handle altitude of vector features
    Qgis::AltitudeClamping mAltClamping = Qgis::AltitudeClamping::Relative;
    //! how to handle clamping of vertices of individual features
    Qgis::AltitudeBinding mAltBinding = Qgis::AltitudeBinding::Centroid;

    float mWidth = 2.0f;            //!< Line width (horizontally)
    float mHeight = 0.0f;           //!< Base height of polygons
    float mExtrusionHeight = 0.0f;  //!< How much to extrude (0 means no walls)
    bool mRenderAsSimpleLines = false;   //!< Whether to render data with simple lines (otherwise it uses buffer)
    std::unique_ptr< QgsAbstractMaterialSettings > mMaterial;  //!< Defines appearance of objects
};


#endif // QGSLINE3DSYMBOL_H
