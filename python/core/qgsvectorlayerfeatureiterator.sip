
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

    //! @note not available in Python bindings
    //void rewindEditBuffer();
    //! @note not available in Python bindings
    //void prepareJoins();
    //! @note not available in Python bindings
    //void prepareExpressions();
    //! @note not available in Python bindings
    //bool fetchNextAddedFeature( QgsFeature& f );
    //! @note not available in Python bindings
    //bool fetchNextChangedGeomFeature( QgsFeature& f );
    //! @note not available in Python bindings
    //bool fetchNextChangedAttributeFeature( QgsFeature& f );
    //! @note not available in Python bindings
    //void useAddedFeature( const QgsFeature& src, QgsFeature& f );
    //! @note not available in Python bindings
    //void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f );
    //! @note not available in Python bindings
    //bool nextFeatureFid( QgsFeature& f );
    //! @note not available in Python bindings
    //void addJoinedAttributes( QgsFeature &f );

    /**
     * Adds attributes that don't source from the provider but are added inside QGIS
     * Includes
     *  - Joined fields
     *  - Expression fields
     *
     * @param f The feature will be modified
     * @note not available in Python bindings
     */
    //void addVirtualAttributes( QgsFeature &f );

    /** Update feature with uncommited attribute updates.
     * @note not available in Python bindings
     */
    //void updateChangedAttributes( QgsFeature& f );

    /** Update feature with uncommited geometry updates.
     * @note not available in Python bindings
     */
    //void updateFeatureGeometry( QgsFeature& f );

  private:
    QgsVectorLayerFeatureIterator();
};
