class QgsMapRendererCache : QObject
{
%TypeHeaderCode
#include <qgsmaprenderercache.h>
%End

  public:

    QgsMapRendererCache();

    void clear();

    bool init( const QgsRectangle& extent, double scale );

    void setCacheImage( const QString& cacheKey, const QImage& image, const QStringList& dependentLayerIds = QStringList() );

    bool hasCacheImage( const QString& cacheKey ) const;

    QImage cacheImage( const QString& cacheKey ) const;

    void clearCacheImage( const QString& cacheKey );

};
