class QgsCoordinateTransformCache
{
%TypeHeaderCode
#include <qgscrscache.h>
%End
  public:
    static QgsCoordinateTransformCache* instance();

    ~QgsCoordinateTransformCache();

    /** Returns coordinate transformation. Cache keeps ownership
        @param srcAuthId auth id string of source crs
        @param destAuthId auth id string of dest crs
        @param srcDatumTransform id of source's datum transform
        @param destDatumTransform id of destinations's datum transform
        @returns matching transform, or an invalid transform if none could be created
     */
    QgsCoordinateTransform transform( const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );

    /** Removes transformations where a changed crs is involved from the cache*/
    void invalidateCrs( const QString& crsAuthId );

  private:

    QgsCoordinateTransformCache( const QgsCoordinateTransformCache& rh );
};

