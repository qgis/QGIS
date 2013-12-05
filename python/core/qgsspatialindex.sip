
class QgsSpatialIndex
{
%TypeHeaderCode
#include "qgsspatialindex.h"
%End

  public:

    /* creation of spatial index */

    /** constructor - creates R-tree */
    QgsSpatialIndex();

    /** copy constructor */
    QgsSpatialIndex( const QgsSpatialIndex& other );

    /** destructor finalizes work with spatial index */
    ~QgsSpatialIndex();

    /** implement assignment operator */
    //QgsSpatialIndex& operator=( const QgsSpatialIndex& other );

    /* operations */

    /** add feature to index */
    bool insertFeature( const QgsFeature& f );

    /** remove feature from index */
    bool deleteFeature( const QgsFeature& f );


    /* queries */

    /** returns features that intersect the specified rectangle */
    QList<qint64> intersects( QgsRectangle rect ) const;

    /** returns nearest neighbors (their count is specified by second parameter) */
    QList<qint64> nearestNeighbor( QgsPoint point, int neighbors ) const;


  protected:
    // static SpatialIndex::Region rectToRegion( QgsRectangle rect );
    // bool featureInfo( QgsFeature& f, SpatialIndex::Region& r, QgsFeatureId &id );


};
