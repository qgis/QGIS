class QgsCacheIndexFeatureId : QgsAbstractCacheIndex
{
%TypeHeaderCode
#include <qgscacheindexfeatureid.h>
%End
  public:
    QgsCacheIndexFeatureId( QgsVectorLayerCache* );

    virtual void flushFeature( const QgsFeatureId fid );
    virtual void flush();
    virtual void requestCompleted( const QgsFeatureRequest& featureRequest, const QgsFeatureIds& fids );
    virtual bool getCacheIterator( QgsFeatureIterator& featureIterator, const QgsFeatureRequest& featureRequest );
};
