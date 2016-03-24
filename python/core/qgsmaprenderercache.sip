
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
class QgsMapRendererCache : QObject
{
%TypeHeaderCode
#include <qgsmaprenderercache.h>
%End

  public:

    QgsMapRendererCache();

    //! invalidate the cache contents
    void clear();

    //! initialize cache: set new parameters and erase cache if parameters have changed
    //! @return flag whether the parameters are the same as last time
    bool init( const QgsRectangle& extent, double scale );

    //! set cached image for the specified layer ID
    void setCacheImage( const QString& layerId, const QImage& img );

    //! get cached image for the specified layer ID. Returns null image if it is not cached.
    QImage cacheImage( const QString& layerId );

    //! remove layer from the cache
    void clearCacheImage( const QString& layerId );

  protected slots:
    //! remove layer (that emitted the signal) from the cache
    void layerRequestedRepaint();

  protected:
    //! invalidate cache contents (without locking)
    void clearInternal();
};
