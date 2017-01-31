class QgsMapRendererCache : QObject
{
%TypeHeaderCode
#include <qgsmaprenderercache.h>
%End

  public:

    QgsMapRendererCache();

    void clear();

    bool init( const QgsRectangle& extent, double scale );

    void setCacheImage( const QString& layerId, const QImage& img );

    QImage cacheImage( const QString& layerId );

    void clearCacheImage( const QString& layerId );

};
