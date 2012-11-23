class QgsCRSCache
{
%TypeHeaderCode
#include <qgscrscache.h>
%End

  public:
    static QgsCRSCache* instance();
    ~QgsCRSCache();
    /**Returns the CRS for authid, e.g. 'EPSG:4326' (or an invalid CRS in case of error)*/
    const QgsCoordinateReferenceSystem& crsByAuthId( const QString& authid );
    const QgsCoordinateReferenceSystem& crsByEpsgId( long epgs );

  protected:
    QgsCRSCache();
};
