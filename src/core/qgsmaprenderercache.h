#ifndef QGSMAPRENDERERCACHE_H
#define QGSMAPRENDERERCACHE_H

#include <QMap>
#include <QImage>
#include <QMutex>

#include "qgsrectangle.h"


/**
 * This class is responsible for keeping cache of rendered images of individual layers.
 *
 * Once a layer has rendered image stored in the cache (using setCacheImage(...)),
 * the cache listens to repaintRequested() signals from layer. If triggered, the cache
 * removes the rendered image (and disconnects from the layer).
 *
 * The class is thread-safe (multiple classes can access the same instance safely).
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsMapRendererCache : public QObject
{
    Q_OBJECT
  public:

    QgsMapRendererCache();

    //! invalidate the cache contents
    void clear();

    //! initialize cache: set new parameters and erase cache if parameters have changed
    //! @return flag whether the parameters are the same as last time
    bool init( QgsRectangle extent, double scale );

    //! set cached image for the specified layer ID
    void setCacheImage( QString layerId, const QImage& img );

    //! get cached image for the specified layer ID. Returns null image if it is not cached.
    QImage cacheImage( QString layerId );

    //! remove layer from the cache
    void clearCacheImage( QString layerId );

  protected slots:
    //! remove layer (that emitted the signal) from the cache
    void layerRequestedRepaint();

  protected:
    //! invalidate cache contents (without locking)
    void clearInternal();

  protected:
    QMutex mMutex;
    QgsRectangle mExtent;
    double mScale;
    QMap<QString, QImage> mCachedImages;
};


#endif // QGSMAPRENDERERCACHE_H
