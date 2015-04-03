
class QgsSpatialIndex
{
%TypeHeaderCode
#include "qgsspatialindex.h"
%End

  public:

    /* creation of spatial index */

    /** constructor - creates R-tree */
    QgsSpatialIndex();

    /** constructor - creates R-tree and bulk loads it with features from the iterator.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * @note added in 2.8
     */
    explicit QgsSpatialIndex( const QgsFeatureIterator& fi );

    /** copy constructor */
    QgsSpatialIndex( const QgsSpatialIndex& other );

    /** destructor finalizes work with spatial index */
    ~QgsSpatialIndex();

    /** implement assignment operator */
    // QgsSpatialIndex& operator=( const QgsSpatialIndex& other );

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

    /* debugging */

    //! get reference count - just for debugging!
    int refs() const;

  protected:
    // @note not available in python bindings
    // static SpatialIndex::Region rectToRegion( QgsRectangle rect );
    // @note not available in python bindings
    // bool featureInfo( const QgsFeature& f, SpatialIndex::Region& r, QgsFeatureId &id );


};
