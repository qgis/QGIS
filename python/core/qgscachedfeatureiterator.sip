class QgsCachedFeatureIterator : QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgscachedfeatureiterator.h>
%End
  public:
    /**
     * @brief
     * This constructor creates a feature iterator, that delivers only cached information, based on the
     * @link QgsFeatureIds @endlink. No request is made to the backend.
     *
     * @param vlCache          The vector layer cache to use
     * @param featureRequest   The feature request to answer
     * @param featureIds       The feature ids to return
     */
    QgsCachedFeatureIterator( QgsVectorLayerCache* vlCache, QgsFeatureRequest featureRequest, QgsFeatureIds featureIds );

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
