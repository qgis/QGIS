class QgsVectorLayerFeatureIterator : QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgsvectorlayerfeatureiterator.h>
%End
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
    void rewindEditBuffer();
    void prepareJoins();
    bool fetchNextAddedFeature( QgsFeature& f );
    bool fetchNextChangedGeomFeature( QgsFeature& f );
    void useAddedFeature( const QgsFeature& src, QgsFeature& f );
    void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f );
    bool nextFeatureFid( QgsFeature& f );
    void addJoinedAttributes( QgsFeature &f );
};
