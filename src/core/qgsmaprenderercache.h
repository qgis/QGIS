/***************************************************************************
  qgsmaprenderercache.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPRENDERERCACHE_H
#define QGSMAPRENDERERCACHE_H

#include "qgis_core.h"
#include <QMap>
#include <QImage>
#include <QMutex>

#include "qgsrectangle.h"
#include "qgsmaplayer.h"


/**
 * \ingroup core
 * This class is responsible for keeping cache of rendered images resulting from
 * a map rendering job.
 *
 * Once a job has a rendered image stored in the cache (using setCacheImage(...)),
 * the cache listens to repaintRequested() signals from dependent layers.
 * If triggered, the cache removes the rendered image (and disconnects from the
 * layers).
 *
 * The class is thread-safe (multiple classes can access the same instance safely).
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsMapRendererCache : public QObject
{
    Q_OBJECT
  public:

    QgsMapRendererCache();

    /**
     * Invalidates the cache contents, clearing all cached images.
     * \see clearCacheImage()
     */
    void clear();

    /**
     * Initialize cache: set new parameters and clears the cache if any
     * parameters have changed since last initialization.
     * \returns flag whether the parameters are the same as last time
     */
    bool init( const QgsRectangle &extent, double scale );

    /**
     * Set the cached \a image for a particular \a cacheKey. The \a cacheKey usually
     * matches the QgsMapLayer::id() which the image is a render of.
     * A list of \a dependentLayers should be passed containing all layer
     * on which this cache image is dependent. If any of these layers triggers a
     * repaint then the cache image will be cleared.
     * \see cacheImage()
     */
    void setCacheImage( const QString &cacheKey, const QImage &image, const QList< QgsMapLayer * > &dependentLayers = QList< QgsMapLayer * >() );

    /**
     * Returns TRUE if the cache contains an image with the specified \a cacheKey.
     * \see cacheImage()
     * \since QGIS 3.0
     */
    bool hasCacheImage( const QString &cacheKey ) const;

    /**
     * Returns the cached image for the specified \a cacheKey. The \a cacheKey usually
     * matches the QgsMapLayer::id() which the image is a render of.
     * Returns a null image if it is not cached.
     * \see setCacheImage()
     * \see hasCacheImage()
     */
    QImage cacheImage( const QString &cacheKey ) const;

    /**
     * Returns a list of map layers on which an image in the cache depends.
     * \since QGIS 3.0
     */
    QList< QgsMapLayer * > dependentLayers( const QString &cacheKey ) const;

    /**
     * Removes an image from the cache with matching \a cacheKey.
     * \see clear()
     */
    void clearCacheImage( const QString &cacheKey );

  private slots:
    //! Remove layer (that emitted the signal) from the cache
    void layerRequestedRepaint();

  private:

    struct CacheParameters
    {
      QImage cachedImage;
      QgsWeakMapLayerPointerList dependentLayers;
    };

    //! Invalidate cache contents (without locking)
    void clearInternal();

    //! Disconnects from layers we no longer care about
    void dropUnusedConnections();

    QSet< QgsWeakMapLayerPointer > dependentLayers() const;

    mutable QMutex mMutex;
    QgsRectangle mExtent;
    double mScale = 0;

    //! Map of cache key to cache parameters
    QMap<QString, CacheParameters> mCachedImages;
    //! List of all layers on which this cache is currently connected
    QSet< QgsWeakMapLayerPointer > mConnectedLayers;
};


#endif // QGSMAPRENDERERCACHE_H
