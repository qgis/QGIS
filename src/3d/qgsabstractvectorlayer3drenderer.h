/***************************************************************************
  qgsabstractvectorlayer3drenderer.h
  --------------------------------------
  Date                 : January 2020
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

#ifndef QGSABSTRACTVECTORLAYER3DRENDERER_H
#define QGSABSTRACTVECTORLAYER3DRENDERER_H

#include "qgis_3d.h"
#include "qgis_sip.h"
#include "qgsabstract3drenderer.h"
#include "qgsmaplayerref.h"

class QgsVectorLayer;

/**
 * \ingroup qgis_3d
 * \brief Defines configuration of how a vector layer gets tiled for 3D rendering.
 *
 * Root node has the layer's extent and geometry error is set as a fraction of the extent (see maximumLeafExtent())
 * Child nodes are created using quadtree approach - a node gets four children, each having their width/height (2D extent)
 * and geometry error halved.
 *
 * For example a root tile that is 1'000 meters wide will get geometry error set as 10 meters.
 *
 * When loading data for each tile, we fetch up to maximumChunkFeatures() features. If the limit is hit, we will recursively
 * create children, so that user gets to see more features when zoomed in.
 * If the geographical extent is very large (see maximumLeafExtent()), we would still recursively create children, even if
 * the limit of features is not hit, to avoid jitter due to numerical precision issues.
 *
 * \since QGIS 3.12
 */
class _3D_EXPORT QgsVectorLayer3DTilingSettings
{
  public:
    /**
     * Returns number of zoom levels. One zoom level means there will be one tile.
     * Every extra zoom level multiplies number of tiles by four. For example, three
     * zoom levels will produce 16 tiles at the highest zoom level. It is therefore
     * recommended to keep the number of zoom levels low to prevent excessive number
     * of tiles.
     * \deprecated QGIS 4.0. Tiling is now done based on maximumScreenError() and maximumChunkFeatures().
     */
    Q_DECL_DEPRECATED int zoomLevelsCount() const SIP_DEPRECATED { return mZoomLevelsCount; }

    /**
     * Sets number of zoom levels. See zoomLevelsCount() documentation for more details.
     * \deprecated QGIS 4.0. Tiling is now done based on maximumScreenError() and maximumChunkFeatures().
     */
    Q_DECL_DEPRECATED void setZoomLevelsCount( int count ) SIP_DEPRECATED { mZoomLevelsCount = count; }

    //! Sets whether to display bounding boxes of entity's tiles (for debugging)
    void setShowBoundingBoxes( bool enabled ) { mShowBoundingBoxes = enabled; }
    //! Returns whether to display bounding boxes of entity's tiles (for debugging)
    bool showBoundingBoxes() const { return mShowBoundingBoxes; }

    /**
     * Returns the maximum number of features that will be fetched in any chunk.
     * If more features than the maximum belong to the chunk, further features will become available in child nodes.
     * This is meant as a control for performance - fewer features mean faster load times, but user needs to zoom in more to see everything.
     * \see setMaximumChunkFeatures()
     * \since QGIS 4.0
     */
    int maximumChunkFeatures() const { return mMaxChunkFeatures; }

    /**
     * Sets the maximum number of features that will be fetched in any chunk.
     * If more features than the maximum belong to the chunk, further features will become available in child nodes.
     * This is meant as a control for performance - fewer features mean faster load times, but user needs to zoom in more to see everything.
     * \see maximumChunkFeatures()
     * \since QGIS 4.0
     */
    void setMaximumChunkFeatures( int value ) { mMaxChunkFeatures = value; }

    //! Writes content of the object to XML
    void writeXml( QDomElement &elem ) const;
    //! Reads content of the object from XML
    void readXml( const QDomElement &elem );

    /**
     * This is the ratio of tile's largest size to geometry error and is used when setting the root tile's error.
     * The choice of 1/100 was relatively arbitrary and based on test data.
     * \note not available in Python bindings
     * \since QGIS 4.0
     */
    static double tileGeometryErrorRatio() SIP_SKIP { return 0.01; }

    /**
     * This is the maximum width or height a tile can have and still be considered a leaf node.
     * Children will be generated for tiles larger than this, even if their chunks did not reach the maximumChunkFeatures limit.
     * We want to avoid having huge leaf nodes so we don't have float precision issues
     * \note not available in Python bindings
     * \since QGIS 4.0
     */
    static double maximumLeafExtent() SIP_SKIP { return 500'000; }

  private:
    int mZoomLevelsCount = 3;
    bool mShowBoundingBoxes = false;
    int mMaxChunkFeatures = 1000;
};


/**
 * \ingroup qgis_3d
 * \brief Base class for 3D renderers that are based on vector layers.
 *
 * \since QGIS 3.12
 */
class _3D_EXPORT QgsAbstractVectorLayer3DRenderer : public QgsAbstract3DRenderer SIP_ABSTRACT
{
  public:
    QgsAbstractVectorLayer3DRenderer();

    //! Sets vector layer associated with the renderer
    void setLayer( QgsVectorLayer *layer );
    //! Returns vector layer associated with the renderer
    QgsVectorLayer *layer() const;

    //! Sets tiling settings of the renderer
    void setTilingSettings( const QgsVectorLayer3DTilingSettings &settings ) { mTilingSettings = settings; }
    //! Returns tiling settings of the renderer
    QgsVectorLayer3DTilingSettings tilingSettings() const { return mTilingSettings; }

    void resolveReferences( const QgsProject &project ) override;

  protected:
    //! Copies common properties of this object to another object
    void copyBaseProperties( QgsAbstractVectorLayer3DRenderer *r ) const;
    //! Writes common properties of this object to DOM element
    void writeXmlBaseProperties( QDomElement &elem, const QgsReadWriteContext &context ) const;
    //! Reads common properties of this object from DOM element
    void readXmlBaseProperties( const QDomElement &elem, const QgsReadWriteContext &context );

  private:
    QgsMapLayerRef mLayerRef;                       //!< Layer used to extract polygons from
    QgsVectorLayer3DTilingSettings mTilingSettings; //!< How is layer tiled into chunks
};

#endif // QGSABSTRACTVECTORLAYER3DRENDERER_H
