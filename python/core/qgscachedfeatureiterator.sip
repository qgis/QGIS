class QgsCachedFeatureIterator : QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgscachedfeatureiterator.h>
%End
  public:
    /**
     * This constructor creates a feature iterator, that delivers only cached information, based on the
     * @link QgsFeatureIds @endlink. No request is made to the backend.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     * @param featureIds       The feature ids to return
     *
     * @deprecated Use QgsCachedFeatureIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest )
     *             instead
     */
    QgsCachedFeatureIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest, QgsFeatureIds featureIds );

    /**
     * This constructor creates a feature iterator, that delivers all cached features. No request is made to the backend.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     */
    QgsCachedFeatureIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest );

    /**
     * Rewind to the beginning of the iterator
     *
     * @return bool true if the operation was ok
     */
    virtual bool rewind();

    /**
     * Close this iterator. No further features will be available.
     *
     * @return true if successful
     */
    virtual bool close();

    // QgsAbstractFeatureIterator interface
  protected:
    /**
     * Implementation for fetching a feature.
     *
     * @param f      Will write to this feature
     * @return bool  true if the operation was ok
     *
     * @see bool getFeature( QgsFeature& f )
     */
    virtual bool fetchFeature( QgsFeature& f );

    /**
     * We have a local special iterator for FilterFids, no need to run the generic.
     *
     * @param f      Will write to this feature
     * @return bool  true if the operation was ok
     */
    virtual bool nextFeatureFilterFids( QgsFeature& f );

};

/**
 * @brief
 * Uses another iterator as backend and writes features to the cache
 *
 */
class QgsCachedFeatureWriterIterator : QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgscachedfeatureiterator.h>
%End
  public:
    /**
     * This constructor creates a feature iterator, which queries the backend and caches retrieved features.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     */
    QgsCachedFeatureWriterIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest );

    /**
     * Rewind to the beginning of the iterator
     *
     * @return bool true if the operation was ok
     */
    virtual bool rewind();

    /**
     * Close this iterator. No further features will be available.
     *
     * @return true if successful
     */
    virtual bool close();

  protected:

    /**
     * Implementation for fetching a feature.
     *
     * @param f      Will write to this feature
     * @return bool  true if the operation was ok
     *
     * @see bool getFeature( QgsFeature& f )
     */
    virtual bool fetchFeature( QgsFeature& f );
};
