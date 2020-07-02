/***************************************************************************
  qgsmapclippingutils.h
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCLIPPINGUTILS_H
#define QGSMAPCLIPPINGUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"

#include <QList>
#include <QPainterPath>

class QgsRenderContext;
class QgsMapLayer;
class QgsGeometry;
class QgsMapClippingRegion;

/**
 * \class QgsMapClippingUtils
 * \ingroup core
 *
 * Utility functions for use when clipping map renders.
 *
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsMapClippingUtils
{
  public:

    /**
     * Collects the list of map clipping regions from a \a context which apply to a map \a layer.
     */
    static QList< QgsMapClippingRegion > collectClippingRegionsForLayer( const QgsRenderContext &context, const QgsMapLayer *layer );

    /**
     * Returns the geometry representing the intersection of clipping \a regions from \a context.
     *
     * The returned geometry will be automatically reprojected into the same CRS as the source layer, ready for use for filtering
     * a feature request.
     *
     * \param regions list of clip regions which apply to the layer
     * \param context a render context
     * \param shouldFilter will be set to TRUE if layer's features should be filtered, i.e. one or more clipping regions applies to the layer
     *
     * \returns combined clipping region for use when filtering features to render
     */
    static QgsGeometry calculateFeatureRequestGeometry( const QList< QgsMapClippingRegion > &regions, const QgsRenderContext &context, bool &shouldFilter );

    /**
     * Returns the geometry representing the intersection of clipping \a regions from \a context which should be used to clip individual
     * feature geometries prior to rendering.
     *
     * The returned geometry will be automatically reprojected into the same CRS as the source layer, ready for use for clipping features.
     *
     * \param regions list of clip regions which apply to the layer
     * \param context a render context
     * \param shouldClip will be set to TRUE if layer's features should be filtered, i.e. one or more clipping regions applies to the layer
     *
     * \returns combined clipping region for use when rendering features
     */
    static QgsGeometry calculateFeatureIntersectionGeometry( const QList< QgsMapClippingRegion > &regions, const QgsRenderContext &context, bool &shouldClip );

    /**
     * Returns a QPainterPath representing the intersection of clipping \a regions from \a context which should be used to clip the painter
     * during rendering of a layer of the specified \a layerType.
     *
     * The returned coordinates are in painter coordinates for the destination \a context.
     *
     * \param regions list of clip regions which apply to the layer
     * \param context a render context
     * \param shouldClip will be set to TRUE if the clipping path should be applied
     *
     * \returns combined painter clipping region for use when rendering maps
     */
    static QPainterPath calculatePainterClipRegion( const QList< QgsMapClippingRegion > &regions, const QgsRenderContext &context, QgsMapLayerType layerType, bool &shouldClip );

    /**
     * Returns the geometry representing the intersection of clipping \a regions from \a context which should be used to clip individual
     * feature geometries while registering them with labeling engines.
     *
     * The returned geometry will be automatically reprojected into the same CRS as the source layer, ready for use for clipping features.
     *
     * \param regions list of clip regions which apply to the layer
     * \param context a render context
     * \param shouldClip will be set to TRUE if layer's features should be clipped for labeling, i.e. one or more clipping regions applies to the layer
     *
     * \returns combined clipping region for use when labeling features
     */
    static QgsGeometry calculateLabelIntersectionGeometry( const QList< QgsMapClippingRegion > &regions, const QgsRenderContext &context, bool &shouldClip );
};

#endif // QGSMAPCLIPPINGUTILS_H
