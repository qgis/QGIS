/***************************************************************************
    qgsvectorlayerfeatureiterator.h
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYERFEATUREITERATOR_H
#define QGSVECTORLAYERFEATUREITERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeaturesource.h"
#include "qgsexpressioncontextscopegenerator.h"
#include "qgscoordinatetransform.h"

#include <QPointer>
#include <QSet>
#include <memory>

typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap SIP_SKIP;

class QgsExpressionFieldBuffer;
class QgsVectorLayer;
class QgsVectorLayerEditBuffer;
class QgsVectorLayerJoinBuffer;
class QgsVectorLayerJoinInfo;
class QgsExpressionContext;

class QgsVectorLayerFeatureIterator;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsfeatureiterator.h"
% End
#endif

/**
 * \ingroup core
 * \brief Partial snapshot of vector layer's state (only the members necessary for access to features)
*/
class CORE_EXPORT QgsVectorLayerFeatureSource : public QgsAbstractFeatureSource
{
  public:

    /**
     * Constructor for QgsVectorLayerFeatureSource.
     * \param layer source layer
     */
    explicit QgsVectorLayerFeatureSource( const QgsVectorLayer *layer );

    //! QgsVectorLayerFeatureSource cannot be copied
    QgsVectorLayerFeatureSource( const QgsVectorLayerFeatureSource &other ) = delete;
    //! QgsVectorLayerFeatureSource cannot be copied
    QgsVectorLayerFeatureSource &operator==( const QgsVectorLayerFeatureSource &other ) = delete;

    ~QgsVectorLayerFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) override;

    friend class QgsVectorLayerFeatureIterator SIP_SKIP;

    /**
     * Returns the fields that will be available for features that are retrieved from
     * this source.
     *
     * \since QGIS 3.0
     */
    QgsFields fields() const;

    /**
     * Returns the coordinate reference system for features retrieved from this source.
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns the layer id of the source layer.
     *
     * \since QGIS 3.4
     */
    QString id() const;

  protected:

    std::unique_ptr< QgsAbstractFeatureSource > mProviderFeatureSource;
    std::unique_ptr< QgsVectorLayerJoinBuffer > mJoinBuffer;
    std::unique_ptr< QgsExpressionFieldBuffer > mExpressionFieldBuffer;

    QgsFields mFields;

    QString mId;

    QgsExpressionContextScope mLayerScope;

    bool mHasEditBuffer;

    // A deep-copy is only performed, if the original maps change
    // see here https://github.com/qgis/Quantum-GIS/pull/673
    // for explanation
    QgsFeatureMap mAddedFeatures;
    QgsGeometryMap mChangedGeometries;
    QgsFeatureIds mDeletedFeatureIds;
    QList<QgsField> mAddedAttributes;
    QgsChangedAttributesMap mChangedAttributeValues;
    QgsAttributeList mDeletedAttributeIds;

    QgsCoordinateReferenceSystem mCrs;

  private:
#ifdef SIP_RUN
    QgsVectorLayerFeatureSource( const QgsVectorLayerFeatureSource &other );
#endif
};

/**
 * \ingroup core
 */
class CORE_EXPORT QgsVectorLayerFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsVectorLayerFeatureSource>
{
  public:
    QgsVectorLayerFeatureIterator( QgsVectorLayerFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsVectorLayerFeatureIterator() override;

    //! reset the iterator to the starting position
    bool rewind() override;

    //! end of iterating: free the resources / lock
    bool close() override;

    void setInterruptionChecker( QgsFeedback *interruptionChecker ) override SIP_SKIP;

    /**
     * Join information prepared for fast attribute id mapping in QgsVectorLayerJoinBuffer::updateFeatureAttributes().
     * Created in the select() method of QgsVectorLayerJoinBuffer for the joins that contain fetched attributes
     */
    struct CORE_EXPORT FetchJoinInfo
    {
      //! Canonical source of information about the join
      const QgsVectorLayerJoinInfo *joinInfo;
      //! Attributes to fetch
      QgsAttributeList attributes;
      //! Mapping from original attribute index to the joined layer index
      QMap<int, int> attributesSourceToDestLayerMap SIP_SKIP;
      //! At what position the joined fields start
      int indexOffset;

#ifndef SIP_RUN

      /**
       * Feature source for join
       *
       * \note Not available in Python bindings
       * \since QGIS 3.20
       */
      std::shared_ptr< QgsVectorLayerFeatureSource > joinSource;

      /**
       * Fields from joined layer.
       *
       * \note Not available in Python bindings
       * \since QGIS 3.20
       */
      QgsFields joinLayerFields;
#endif

      //! Index of field (of this layer) that drives the join
      int targetField;

      //!< Index of field (of the joined layer) must have equal value
      int joinField;

      void addJoinedAttributesCached( QgsFeature &f, const QVariant &joinValue ) const;
      void addJoinedAttributesDirect( QgsFeature &f, const QVariant &joinValue ) const;
    };

    bool isValid() const override;

  protected:
    //! fetch next feature, return TRUE on success
    bool fetchFeature( QgsFeature &feature ) override;

    /**
     * Overrides default method as we only need to filter features in the edit buffer
     * while for others filtering is left to the provider implementation.
     */
    bool nextFeatureFilterExpression( QgsFeature &f ) override { return fetchFeature( f ); }

    //! Setup the simplification of geometries to fetch using the specified simplify method
    bool prepareSimplification( const QgsSimplifyMethod &simplifyMethod ) override;

    //! \note not available in Python bindings
    void rewindEditBuffer() SIP_SKIP;

    //! \note not available in Python bindings
    void prepareJoin( int fieldIdx ) SIP_SKIP;

    //! \note not available in Python bindings
    void prepareExpression( int fieldIdx ) SIP_SKIP;

    //! \note not available in Python bindings
    void prepareFields() SIP_SKIP;

    //! \note not available in Python bindings
    void prepareField( int fieldIdx ) SIP_SKIP;

    //! \note not available in Python bindings
    bool fetchNextAddedFeature( QgsFeature &f ) SIP_SKIP;
    //! \note not available in Python bindings
    bool fetchNextChangedGeomFeature( QgsFeature &f ) SIP_SKIP;
    //! \note not available in Python bindings
    bool fetchNextChangedAttributeFeature( QgsFeature &f ) SIP_SKIP;
    //! \note not available in Python bindings
    void useAddedFeature( const QgsFeature &src, QgsFeature &f ) SIP_SKIP;
    //! \note not available in Python bindings
    void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry &geom, QgsFeature &f ) SIP_SKIP;
    //! \note not available in Python bindings
    bool nextFeatureFid( QgsFeature &f ) SIP_SKIP;
    //! \note not available in Python bindings
    void addJoinedAttributes( QgsFeature &f ) SIP_SKIP;

    /**
     * Adds attributes that don't source from the provider but are added inside QGIS
     * Includes
     *
     * - Joined fields
     * - Expression fields
     *
     * \param f The feature will be modified
     * \note not available in Python bindings
     */
    void addVirtualAttributes( QgsFeature &f ) SIP_SKIP;

    /**
     * Adds an expression based attribute to a feature
     * \param f feature
     * \param attrIndex attribute index
     * \note not available in Python bindings
     * \since QGIS 2.14
     */
    void addExpressionAttribute( QgsFeature &f, int attrIndex ) SIP_SKIP;

    /**
     * Update feature with uncommitted attribute updates.
     * \note not available in Python bindings
     */
    void updateChangedAttributes( QgsFeature &f ) SIP_SKIP;

    /**
     * Update feature with uncommitted geometry updates.
     * \note not available in Python bindings
     */
    void updateFeatureGeometry( QgsFeature &f ) SIP_SKIP;

    QgsFeatureRequest mProviderRequest;
    QgsFeatureIterator mProviderIterator;
    QgsFeatureRequest mChangedFeaturesRequest;
    QgsFeatureIterator mChangedFeaturesIterator;

    // filter bounding box constraint, in SOURCE CRS
    QgsRectangle mFilterRect;
    QgsCoordinateTransform mTransform;

    // distance within constraint reference geometry and distance IN DESTINATION CRS
    QgsGeometry mDistanceWithinGeom;
    std::shared_ptr< QgsGeometryEngine > mDistanceWithinEngine;
    double mDistanceWithin = 0;

    // only related to editing
    QSet<QgsFeatureId> mFetchConsidered;
    QgsGeometryMap::ConstIterator mFetchChangedGeomIt;
    QgsFeatureMap::ConstIterator mFetchAddedFeaturesIt;

    bool mFetchedFid; // when iterating by FID: indicator whether it has been fetched yet or not

    /**
     * Information about joins used in the current select() statement.
     * Allows faster mapping of attribute ids compared to mVectorJoins.
    */
    QMap<const QgsVectorLayerJoinInfo *, QgsVectorLayerFeatureIterator::FetchJoinInfo> mFetchJoinInfo;

    QMap<int, QgsExpression *> mExpressionFieldInfo;

    bool mHasVirtualAttributes;

  private:
#ifdef SIP_RUN
    QgsVectorLayerFeatureIterator( const QgsVectorLayerFeatureIterator &rhs );
#endif

    void createExpressionContext();
    std::unique_ptr<QgsExpressionContext> mExpressionContext;

    QgsFeedback *mInterruptionChecker = nullptr;

    QList< int > mPreparedFields;
    QList< int > mFieldsToPrepare;

    //! Join list sorted by dependency
    QList< FetchJoinInfo > mOrderedJoinInfoList;

    /**
     * Will always return TRUE. We assume that ordering has been done on provider level already.
     *
     */
    bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;

    //! returns whether the iterator supports simplify geometries on provider side
    bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const override;

    void createOrderedJoinList();

    /**
     * Performs any post-processing (such as transformation) and feature based validity checking, e.g. checking for geometry validity.
     */
    bool postProcessFeature( QgsFeature &feature );

    /**
     * Checks a feature's geometry for validity, if requested in feature request.
     */
    bool checkGeometryValidity( const QgsFeature &feature );

    bool mDelegatedOrderByToProvider = false;
};



/**
 * \class QgsVectorLayerSelectedFeatureSource
 * \ingroup core
 * \brief QgsFeatureSource subclass for the selected features from a QgsVectorLayer.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVectorLayerSelectedFeatureSource : public QgsFeatureSource, public QgsExpressionContextScopeGenerator
{
  public:

    /**
     * Constructor for QgsVectorLayerSelectedFeatureSource, for selected features from the specified \a layer.
     * The currently selected feature IDs are stored, so change to the layer selection after constructing
     * the QgsVectorLayerSelectedFeatureSource will not be reflected.
     */
    QgsVectorLayerSelectedFeatureSource( QgsVectorLayer *layer );

    //! QgsVectorLayerSelectedFeatureSource cannot be copied
    QgsVectorLayerSelectedFeatureSource( const QgsVectorLayerSelectedFeatureSource &other ) = delete;
    //! QgsVectorLayerSelectedFeatureSource cannot be copied
    QgsVectorLayerSelectedFeatureSource &operator==( const QgsVectorLayerSelectedFeatureSource &other ) = delete;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;
    QgsCoordinateReferenceSystem sourceCrs() const override;
    QgsFields fields() const override;
    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;
    QString sourceName() const override;
    QgsExpressionContextScope *createExpressionContextScope() const override;
    SpatialIndexPresence hasSpatialIndex() const override;

  private:

#ifdef SIP_RUN
    QgsVectorLayerSelectedFeatureSource( const QgsVectorLayerSelectedFeatureSource &other );
#endif

    // ideally this wouldn't be mutable, but QgsVectorLayerFeatureSource has non-const getFeatures()
    mutable QgsVectorLayerFeatureSource mSource;
    QgsFeatureIds mSelectedFeatureIds;
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    QString mName;
    QPointer< QgsVectorLayer > mLayer;

};

///@cond PRIVATE

#ifndef SIP_RUN
class QgsVectorLayerSelectedFeatureIterator : public QgsAbstractFeatureIterator
{
  public:

    QgsVectorLayerSelectedFeatureIterator( const QgsFeatureIds &selectedFeatureIds,
                                           const QgsFeatureRequest &request,
                                           QgsVectorLayerFeatureSource &source );

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &f ) override;

  private:
    QgsFeatureIds mSelectedFeatureIds;
    QgsFeatureIterator mIterator;

};

#endif

///@endcond

#endif // QGSVECTORLAYERFEATUREITERATOR_H
