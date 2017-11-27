/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerfeatureiterator.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






%ModuleHeaderCode
#include "qgsfeatureiterator.h"
%End

class QgsVectorLayerFeatureSource : QgsAbstractFeatureSource
{
%Docstring
 Partial snapshot of vector layer's state (only the members necessary for access to features)
%End

%TypeHeaderCode
#include "qgsvectorlayerfeatureiterator.h"
%End
  public:

    explicit QgsVectorLayerFeatureSource( const QgsVectorLayer *layer );
%Docstring
 Constructor for QgsVectorLayerFeatureSource.
 \param layer source layer
%End

    ~QgsVectorLayerFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() );


    QgsFields fields() const;
%Docstring
 Returns the fields that will be available for features that are retrieved from
 this source.

.. versionadded:: 3.0
 :rtype: QgsFields
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
 Returns the coordinate reference system for features retrieved from this source.
.. versionadded:: 3.0
 :rtype: QgsCoordinateReferenceSystem
%End

  protected:








};


typedef QgsAbstractFeatureIteratorFromSource<QgsVectorLayerFeatureSource> QgsAbstractFeatureIteratorFromSourceQgsVectorLayerFeatureSourceBase;

class QgsVectorLayerFeatureIterator : QgsAbstractFeatureIteratorFromSourceQgsVectorLayerFeatureSourceBase
{

%TypeHeaderCode
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsfeatureiterator.h"
typedef QgsAbstractFeatureIteratorFromSource<QgsVectorLayerFeatureSource> QgsAbstractFeatureIteratorFromSourceQgsVectorLayerFeatureSourceBase;
%End
  public:
    QgsVectorLayerFeatureIterator( QgsVectorLayerFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsVectorLayerFeatureIterator();

    virtual bool rewind();
%Docstring
reset the iterator to the starting position
 :rtype: bool
%End

    virtual bool close();
%Docstring
end of iterating: free the resources / lock
 :rtype: bool
%End


    struct FetchJoinInfo
    {
      const QgsVectorLayerJoinInfo *joinInfo;//!< Canonical source of information about the join
      QgsAttributeList attributes;      //!< Attributes to fetch
      int indexOffset;                  //!< At what position the joined fields start
      QgsVectorLayer *joinLayer;        //!< Resolved pointer to the joined layer
      int targetField;                  //!< Index of field (of this layer) that drives the join
      int joinField;                    //!< Index of field (of the joined layer) must have equal value

      void addJoinedAttributesCached( QgsFeature &f, const QVariant &joinValue ) const;
      void addJoinedAttributesDirect( QgsFeature &f, const QVariant &joinValue ) const;
    };


    virtual bool isValid() const;

  protected:
    virtual bool fetchFeature( QgsFeature &feature );
%Docstring
fetch next feature, return true on success
 :rtype: bool
%End

    virtual bool nextFeatureFilterExpression( QgsFeature &f );
%Docstring
 Overrides default method as we only need to filter features in the edit buffer
 while for others filtering is left to the provider implementation.
 :rtype: bool
%End

    virtual bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod );
%Docstring
Setup the simplification of geometries to fetch using the specified simplify method
 :rtype: bool
%End


















  private:
    QgsVectorLayerFeatureIterator( const QgsVectorLayerFeatureIterator &rhs );
};



class QgsVectorLayerSelectedFeatureSource : QgsFeatureSource
{
%Docstring
 QgsFeatureSource subclass for the selected features from a QgsVectorLayer.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsvectorlayerfeatureiterator.h"
%End
  public:

    QgsVectorLayerSelectedFeatureSource( QgsVectorLayer *layer );
%Docstring
 Constructor for QgsVectorLayerSelectedFeatureSource, for selected features from the specified ``layer``.
 The currently selected feature IDs are stored, so change to the layer selection after constructing
 the QgsVectorLayerSelectedFeatureSource will not be reflected.
%End

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const;

    virtual QgsCoordinateReferenceSystem sourceCrs() const;

    virtual QgsFields fields() const;

    virtual QgsWkbTypes::Type wkbType() const;

    virtual long featureCount() const;

    virtual QString sourceName() const;



};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerfeatureiterator.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
