#ifndef QGSVECTORLAYERFEATUREITERATOR_H
#define QGSVECTORLAYERFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QSet>

class QgsVectorLayer;
class QgsVectorJoinInfo;

class QgsVectorLayerFeatureIterator : public QgsAbstractFeatureIterator
{
public:
  QgsVectorLayerFeatureIterator( QgsVectorLayer* layer, const QgsFeatureRequest& request );

  ~QgsVectorLayerFeatureIterator();

  //! fetch next feature, return true on success
  virtual bool nextFeature( QgsFeature& feature );

  //! reset the iterator to the starting position
  virtual bool rewind();

  //! end of iterating: free the resources / lock
  virtual bool close();

protected:
  QgsVectorLayer* L;

  QgsFeatureRequest mProviderRequest;
  QgsFeatureIterator mProviderIterator;

  // general stuff
  //bool mFetching;
  //QgsRectangle mFetchRect;
  //QgsAttributeList mFetchAttributes;
  //QgsAttributeList mFetchProvAttributes;
  //bool mFetchGeometry;

  // only related to editing
  QSet<QgsFeatureId> mFetchConsidered;
  QgsGeometryMap::iterator mFetchChangedGeomIt;
  QgsFeatureList::iterator mFetchAddedFeaturesIt;

  bool mFetchedFid; // when iterating by FID: indicator whether it has been fetched yet or not

  void rewindEditBuffer();
  void prepareJoins();
  bool fetchNextAddedFeature( QgsFeature& f );
  bool fetchNextChangedGeomFeature( QgsFeature& f );
  void useAddedFeature( const QgsFeature& src, QgsFeature& f );
  void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f );
  bool nextFeatureFid( QgsFeature& f );
  void addJoinedAttributes( QgsFeature &f );

  /** Join information prepared for fast attribute id mapping in QgsVectorLayerJoinBuffer::updateFeatureAttributes().
    Created in the select() method of QgsVectorLayerJoinBuffer for the joins that contain fetched attributes
  */
  struct FetchJoinInfo
  {
    const QgsVectorJoinInfo* joinInfo;//!< cannonical source of information about the join
    QgsAttributeList attributes;      //!< attributes to fetch
    int indexOffset;                  //!< at what position the joined fields start
    QgsVectorLayer* joinLayer;        //!< resolved pointer to the joined layer
    int targetField;                  //!< index of field (of this layer) that drives the join
    int joinField;                    //!< index of field (of the joined layer) must have equal value

    void addJoinedAttributesCached( QgsFeature& f, const QVariant& joinValue ) const;
    void addJoinedAttributesDirect( QgsFeature& f, const QVariant& joinValue ) const;
  };


  /** Informations about joins used in the current select() statement.
    Allows faster mapping of attribute ids compared to mVectorJoins */
  QMap<QgsVectorLayer*, FetchJoinInfo> mFetchJoinInfo;

};

#endif // QGSVECTORLAYERFEATUREITERATOR_H
