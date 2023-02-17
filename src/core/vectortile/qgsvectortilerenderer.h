/***************************************************************************
  qgsvectortilerenderer.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILERENDERER_H
#define QGSVECTORTILERENDERER_H

#include "qgis_core.h"

#include "qgsfeature.h"

#include "qgstiles.h"

class QgsRenderContext;
class QgsReadWriteContext;
class QgsProject;

//! Features of a vector tile, grouped by sub-layer names (key of the map)
typedef QMap<QString, QVector<QgsFeature> > QgsVectorTileFeatures SIP_SKIP;

/**
 * \ingroup core
 * \brief Contains decoded features of a single vector tile and any other data necessary
 * for rendering of it.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileRendererData
{
  public:
    //! Constructs the object
    explicit QgsVectorTileRendererData( QgsTileXYZ id )
      : mId( id )
    {}

    //! Returns coordinates of the tile
    QgsTileXYZ id() const { return mId; }

    //! Sets polygon of the tile
    void setTilePolygon( QPolygon polygon ) { mTilePolygon = polygon; }
    //! Returns polygon (made out of four corners of the tile) in screen coordinates calculated from render context
    QPolygon tilePolygon() const { return mTilePolygon; }

    //! Sets per-layer fields
    void setFields( const QMap<QString, QgsFields> &fields ) { mFields = fields; }
    //! Returns per-layer fields
    QMap<QString, QgsFields> fields() const { return mFields; }

    //! Sets features of the tile
    void setFeatures( const QgsVectorTileFeatures &features ) SIP_SKIP { mFeatures = features; }
    //! Returns features of the tile grouped by sub-layer names
    QgsVectorTileFeatures features() const SIP_SKIP { return mFeatures; }
    //! Returns list of layer names present in the tile
    QStringList layers() const { return mFeatures.keys(); }
    //! Returns list of all features within a single sub-layer
    QVector<QgsFeature> layerFeatures( const QString &layerName ) const { return mFeatures[layerName]; }

  private:
    //! Position of the tile in the tile matrix set
    QgsTileXYZ mId;
    //! Per-layer fields
    QMap<QString, QgsFields> mFields;
    //! Features of the tile grouped into sub-layers
    QgsVectorTileFeatures mFeatures;
    //! Polygon (made out of four corners of the tile) in screen coordinates calculated from render context
    QPolygon mTilePolygon;
};

/**
 * \ingroup core
 * \brief Abstract base class for all vector tile renderer implementations.
 *
 * For rendering it is expected that client code calls:
 *
 * # startRender() to prepare renderer
 * # renderTile() for each tile
 * # stopRender() to clean up renderer and free resources
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileRenderer
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE

    const QString type = sipCpp->type();

    if ( type == QLatin1String( "basic" ) )
      sipType = sipType_QgsVectorTileBasicRenderer;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    virtual ~QgsVectorTileRenderer() = default;

    //! Returns unique type name of the renderer implementation
    virtual QString type() const = 0;

    //! Returns a clone of the renderer
    virtual QgsVectorTileRenderer *clone() const = 0 SIP_FACTORY;

    //! Initializes rendering. It should be paired with a stopRender() call.
    virtual void startRender( QgsRenderContext &context, int tileZoom, const QgsTileRange &tileRange ) = 0;

    //! Returns field names of sub-layers that will be used for rendering. Must be called between startRender/stopRender.
    virtual QMap<QString, QSet<QString> > usedAttributes( const QgsRenderContext & ) SIP_SKIP { return QMap<QString, QSet<QString> >(); }

    //TODO QGIS 4.0 -- make pure virtual

    /**
     * Returns a list of the layers required for rendering.
     *
     * Only layers which are visible at the specified \a tileZoom should be included in this list.
     *
     * An empty string present in the list indicates that all layer in the tiles are required.
     *
     * \since QGIS 3.16
     */
    virtual QSet< QString > requiredLayers( QgsRenderContext &context, int tileZoom ) const { Q_UNUSED( context ); Q_UNUSED( tileZoom ); return QSet< QString >() << QString(); }

    //! Finishes rendering and cleans up any resources
    virtual void stopRender( QgsRenderContext &context ) = 0;

    //! Renders given vector tile. Must be called between startRender/stopRender.
    virtual void renderTile( const QgsVectorTileRendererData &tile, QgsRenderContext &context ) = 0;

    /**
     * Returns TRUE if the specified \a feature will be rendered in the given render \a context.
     *
     * \since QGIS 3.28
     */
    virtual bool willRenderFeature( const QgsFeature &feature, int tileZoom, const QString &layerName, QgsRenderContext &context ) = 0;

    /**
     * Renders the specified features in a selected state.
     *
     * This will be called after rendering the tiles, so that the selected features are always visible on the top of the layer.
     *
     * \since QGIS 3.28
     */
    virtual void renderSelectedFeatures( const QList< QgsFeature > &selection, QgsRenderContext &context ) = 0;

    //! Writes renderer's properties to given XML element
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    //! Reads renderer's properties from given XML element
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
    //! Resolves references to other objects - second phase of loading - after readXml()
    virtual void resolveReferences( const QgsProject &project ) { Q_UNUSED( project ) }

};

#endif // QGSVECTORTILERENDERER_H
