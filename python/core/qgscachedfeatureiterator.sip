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
};

class QgsCachedFeatureWriterIterator : QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgscachedfeatureiterator.h>
%End
  public:
    /**
     * @brief
     * This constructor creates a feature iterator, which queries the backend and caches retrieved features.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     */
    QgsCachedFeatureWriterIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest );

    /**
     * @brief
     *
     * @param f
     * @return bool
     */
    virtual bool nextFeature( QgsFeature& f );

    /**
     * @brief
     *
     * @return bool
     */
    virtual bool rewind();

    /**
     * @brief
     *
     * @return bool
     */
    virtual bool close();
};
