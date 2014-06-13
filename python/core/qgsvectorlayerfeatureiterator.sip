
// abstract feature iterator implementations are not part of public API


class QgsVectorLayerFeatureIterator : QgsAbstractFeatureIterator
{
%TypeHeaderCode
#include <qgsvectorlayerfeatureiterator.h>
%End
  public:
    ~QgsVectorLayerFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature );

    //! Overrides default method as we only need to filter features in the edit buffer
    //! while for others filtering is left to the provider implementation.
    virtual bool nextFeatureFilterExpression( QgsFeature &f );

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod& simplifyMethod );
    void rewindEditBuffer();
    void prepareJoins();
    bool fetchNextAddedFeature( QgsFeature& f );
    bool fetchNextChangedGeomFeature( QgsFeature& f );
    bool fetchNextChangedAttributeFeature( QgsFeature& f );
    void useAddedFeature( const QgsFeature& src, QgsFeature& f );
    void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f );
    bool nextFeatureFid( QgsFeature& f );
    void addJoinedAttributes( QgsFeature &f );

    /** Update feature with uncommited attribute updates */
    void updateChangedAttributes( QgsFeature& f );

    /** Update feature with uncommited geometry updates */
    void updateFeatureGeometry( QgsFeature& f );

  private:
    QgsVectorLayerFeatureIterator();
};
