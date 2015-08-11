
class QgsSpatialIndex
{
%TypeHeaderCode
#include "qgsspatialindex.h"
%End

  public:

    /* creation of spatial index */

    /** Constructor - creates R-tree */
    QgsSpatialIndex();

    /** Constructor - creates R-tree and bulk loads it with features from the iterator.
     * This is much faster approach than creating an empty index and then inserting features one by one.
     *
     * @note added in 2.8
     */
    explicit QgsSpatialIndex( const QgsFeatureIterator& fi );

    /** Copy constructor */
    QgsSpatialIndex( const QgsSpatialIndex& other );

    /** Destructor finalizes work with spatial index */
    ~QgsSpatialIndex();

    /** Implement assignment operator */
    // QgsSpatialIndex& operator=( const QgsSpatialIndex& other );

    /* operations */

    /** Add feature to index */
    bool insertFeature( const QgsFeature& f );

    /** Remove feature from index */
    bool deleteFeature( const QgsFeature& f );


    /* queries */

    /** Returns features that intersect the specified rectangle */
    QList<qint64> intersects( QgsRectangle rect ) const;

    /** Returns nearest neighbors (their count is specified by second parameter) */
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
