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
 * \ingroup 3d
 * \brief This class defines configuration of how a vector layer gets tiled for 3D rendering.
 *
 * Zoom levels count tells how deep will be the quadtree and thus how many tiles will
 * be generated ( 4 ^ (count-1) ). So for example, for count=1 there will be just
 * a single tile for the whole layer, for count=3 there will be 16 tiles.
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
     */
    int zoomLevelsCount() const { return mZoomLevelsCount; }

    /**
     * Sets number of zoom levels. See zoomLevelsCount() documentation for more details.
     */
    void setZoomLevelsCount( int count ) { mZoomLevelsCount = count; }

    //! Sets whether to display bounding boxes of entity's tiles (for debugging)
    void setShowBoundingBoxes( bool enabled ) { mShowBoundingBoxes = enabled; }
    //! Returns whether to display bounding boxes of entity's tiles (for debugging)
    bool showBoundingBoxes() const { return mShowBoundingBoxes; }

    //! Writes content of the object to XML
    void writeXml( QDomElement &elem ) const;
    //! Reads content of the object from XML
    void readXml( const QDomElement &elem );

  private:
    int mZoomLevelsCount = 3;
    bool mShowBoundingBoxes = false;
};


/**
 * \ingroup 3d
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
