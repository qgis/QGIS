class QgsCoordinateTransformCache
{
  public:
    static QgsCoordinateTransformCache* instance();
    ~QgsCoordinateTransformCache();
    /**Returns coordinate transformation. Cache keeps ownership
        @param srcAuthId auth id string of source crs
        @param destAuthId auth id string of dest crs
        @param srcDatumTransform id of source's datum transform
        @param destDatumTransform id of destinations's datum transform
     */
    const QgsCoordinateTransform* transform( const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );
    /**Removes transformations where a changed crs is involved from the cache*/
    void invalidateCrs( const QString& crsAuthId );
};

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
    const QgsCoordinateReferenceSystem& crsByEpsgId( long epsg );

    void updateCRSCache( const QString &authid );

  protected:
    QgsCRSCache();
};
