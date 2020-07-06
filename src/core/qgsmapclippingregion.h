/***************************************************************************
  qgsmapclippingregion.h
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

#ifndef QGSMAPCLIPPINGREGION_H
#define QGSMAPCLIPPINGREGION_H

#include "qgis_core.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"

/**
 * \class QgsMapClippingRegion
 * \ingroup core
 *
 * A map clipping region (in map coordinates and CRS).
 *
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsMapClippingRegion
{
  public:

    /**
     * Feature clipping behavior, which controls how features from vector layers
     * will be clipped.
     */
    enum class FeatureClippingType : int
    {
      ClipToIntersection, //!< Clip the geometry of these features to the region prior to rendering (i.e. feature boundaries will follow the clip region)
      ClipPainterOnly, //!< Applying clipping on the painter only (i.e. feature boundaries will be unchanged, but may be invisible where the feature falls outside the clipping region)
      NoClipping, //!< Only render features which intersect the clipping region, but do not clip these features to the region
    };

    /**
     * Constructor for a map clipping region, with the specified \a geometry in the destination map CRS.
     */
    explicit QgsMapClippingRegion( const QgsGeometry &geometry )
      : mGeometry( geometry )
    {}

    /**
     * Returns the geometry of the clipping region (in the destination map CRS).
     *
     * \see setGeometry().
     */
    QgsGeometry geometry() const;

    /**
     * Sets the clipping region \a geometry (in the destination map CRS).
     *
     * \see geometry()
     */
    void setGeometry( const QgsGeometry &geometry );

    /**
     * Returns the feature clipping type.
     *
     * This setting is only used while rendering vector layers, for other layer types it is ignored.
     *
     * \see setFeatureClip()
     */
    FeatureClippingType featureClip() const
    {
      return mFeatureClip;
    }

    /**
     * Sets the feature clipping \a type.
     *
     * This setting is only used while rendering vector layers, for other layer types it is ignored.
     *
     * \see featureClip()
     */
    void setFeatureClip( FeatureClippingType type )
    {
      mFeatureClip = type;
    }

    /**
     * Sets a list of \a layers to restrict the clipping region effects to.
     *
     * By default the clipping region applies to all layers.
     *
     * \see restrictedLayers()
     */
    void setRestrictedLayers( const QList< QgsMapLayer * > &layers );


    /**
     * Returns the list of layers to restrict the clipping region effects to.
     *
     * If the list is empty then the clipping will be applied to all layers.
     *
     * \see setRestrictedLayers()
     */
    QList< QgsMapLayer * > restrictedLayers() const;

    /**
     * Returns TRUE if the clipping region should be applied to the specified map \a layer.
     */
    bool appliesToLayer( const QgsMapLayer *layer ) const;

  private:

    //! Geometry of clipping region (in destination map coordinates and CRS)
    QgsGeometry mGeometry;

    QgsWeakMapLayerPointerList mRestrictToLayersList;

    FeatureClippingType mFeatureClip = FeatureClippingType::ClipToIntersection;

};

#endif // QGSMAPCLIPPINGREGION_H
