/***************************************************************************
  qgspolygon3dsymbol.h
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

#ifndef QGSPOLYGON3DSYMBOL_H
#define QGSPOLYGON3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"
#include "qgs3dtypes.h"

#include <Qt3DRender/QCullFace>

class QgsAbstractMaterialSettings;

/**
 * \ingroup 3d
 * \brief 3D symbol that draws polygon geometries as planar polygons, optionally extruded (with added walls).
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPolygon3DSymbol : public QgsAbstract3DSymbol SIP_NODEFAULTCTORS
{
  public:
    //! Constructor for QgsPolygon3DSymbol
    QgsPolygon3DSymbol();
    ~QgsPolygon3DSymbol() override;

    QString type() const override { return "polygon"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    QList< QgsWkbTypes::GeometryType > compatibleGeometryTypes() const override;
    void setDefaultPropertiesFromLayer( const QgsVectorLayer *layer ) override;

    /**
     * Creates a new QgsPolygon3DSymbol.
     *
     * Caller takes ownership of the returned symbol.
     */
    static QgsAbstract3DSymbol *create() SIP_FACTORY;

    //! Returns method that determines altitude (whether to clamp to feature to terrain)
    Qgis::AltitudeClamping altitudeClamping() const { return mAltClamping; }
    //! Sets method that determines altitude (whether to clamp to feature to terrain)
    void setAltitudeClamping( Qgis::AltitudeClamping altClamping ) { mAltClamping = altClamping; }

    //! Returns method that determines how altitude is bound to individual vertices
    Qgis::AltitudeBinding altitudeBinding() const { return mAltBinding; }
    //! Sets method that determines how altitude is bound to individual vertices
    void setAltitudeBinding( Qgis::AltitudeBinding altBinding ) { mAltBinding = altBinding; }

    //! Returns height (altitude) of the symbol (in map units)
    float height() const { return mHeight; }
    //! Sets height (altitude) of the symbol (in map units)
    void setHeight( float height ) { mHeight = height; }

    //! Returns extrusion height (in map units)
    float extrusionHeight() const { return mExtrusionHeight; }
    //! Sets extrusion height (in map units)
    void setExtrusionHeight( float extrusionHeight ) { mExtrusionHeight = extrusionHeight; }

    //! Returns material used for shading of the symbol
    QgsAbstractMaterialSettings *material() const;

    /**
     * Sets the \a material settings used for shading of the symbol.
     *
     * Ownership of \a material is transferred to the symbol.
     */
    void setMaterial( QgsAbstractMaterialSettings *material SIP_TRANSFER );

    //! Returns front/back culling mode
    Qgs3DTypes::CullingMode cullingMode() const { return mCullingMode; }
    //! Sets front/back culling mode
    void setCullingMode( Qgs3DTypes::CullingMode mode ) { mCullingMode = mode; }

    //! Returns whether the normals of triangles will be inverted (useful for fixing clockwise / counter-clockwise face vertex orders)
    bool invertNormals() const { return mInvertNormals; }
    //! Sets whether the normals of triangles will be inverted (useful for fixing clockwise / counter-clockwise face vertex orders)
    void setInvertNormals( bool invert ) { mInvertNormals = invert; }

    /**
     * Returns whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     * \since QGIS 3.2
     */
    bool addBackFaces() const { return mAddBackFaces; }

    /**
     * Sets whether also triangles facing the other side will be created. Useful if input data have inconsistent order of vertices
     * \since QGIS 3.2
     */
    void setAddBackFaces( bool add ) { mAddBackFaces = add; }

    /**
     * Returns whether edge highlighting is enabled
     * \since QGIS 3.8
     */
    bool edgesEnabled() const { return mEdgesEnabled; }

    /**
     * Sets whether edge highlighting is enabled
     * \since QGIS 3.8
     */
    void setEdgesEnabled( bool enabled ) { mEdgesEnabled = enabled; }

    /**
     * Returns width of edge lines (in pixels)
     * \since QGIS 3.8
     */
    float edgeWidth() const { return mEdgeWidth; }

    /**
     * Sets width of edge lines (in pixels)
     * \since QGIS 3.8
     */
    void setEdgeWidth( float width ) { mEdgeWidth = width; }

    /**
     * Returns edge lines color
     * \since QGIS 3.8
     */
    QColor edgeColor() const { return mEdgeColor; }

    /**
     * Sets edge lines color
     * \since QGIS 3.8
     */
    void setEdgeColor( const QColor &color ) { mEdgeColor = color; }

    /**
     * Sets which facade of the buildings is rendered (0 for None, 1 for Walls, 2 for Roofs, 3 for WallsAndRoofs)
     * \since QGIS 3.16
     */
    void setRenderedFacade( int side ) { mRenderedFacade = side; }

    /**
     * Returns which facade of the buildings is rendered (0 for None, 1 for Walls, 2 for Roofs, 3 for WallsAndRoofs)
     * \since QGIS 3.16
     */
    int renderedFacade() const { return mRenderedFacade; }

    /**
     * Exports the geometries contained within the hierarchy of entity.
     * Returns whether any objects were exported
     * \since QGIS 3.16
     */
    bool exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const override SIP_SKIP;

  private:
    //! how to handle altitude of vector features
    Qgis::AltitudeClamping mAltClamping = Qgis::AltitudeClamping::Relative;
    //! how to handle clamping of vertices of individual features
    Qgis::AltitudeBinding mAltBinding = Qgis::AltitudeBinding::Centroid;

    float mHeight = 0.0f;           //!< Base height of polygons
    float mExtrusionHeight = 0.0f;  //!< How much to extrude (0 means no walls)
    std::unique_ptr< QgsAbstractMaterialSettings > mMaterial; //!< Defines appearance of objects
    Qgs3DTypes::CullingMode mCullingMode = Qgs3DTypes::NoCulling;  //!< Front/back culling mode
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
    int mRenderedFacade = 3;

    bool mEdgesEnabled = false;  //!< Whether to highlight edges
    float mEdgeWidth = 1.f;  //!< Width of edges in pixels
    QColor mEdgeColor = Qt::black; //!< Color of edge lines
};


#endif // QGSPOLYGON3DSYMBOL_H
