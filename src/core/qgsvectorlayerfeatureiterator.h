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

#include "qgsfeatureiterator.h"

#include <QSet>

typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsExpressionFieldBuffer;
class QgsVectorLayer;
class QgsVectorLayerEditBuffer;
class QgsVectorLayerJoinBuffer;
struct QgsVectorJoinInfo;

class QgsVectorLayerFeatureIterator;

/** Partial snapshot of vector layer's state (only the members necessary for access to features) */
class QgsVectorLayerFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsVectorLayerFeatureSource( QgsVectorLayer* layer );
    ~QgsVectorLayerFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

    friend class QgsVectorLayerFeatureIterator;

  protected:

    QgsAbstractFeatureSource* mProviderFeatureSource;

    QgsVectorLayerJoinBuffer* mJoinBuffer;

    QgsExpressionFieldBuffer* mExpressionFieldBuffer;

    QgsFields mFields;

    bool mHasEditBuffer;

    bool mCanBeSimplified;

    // A deep-copy is only performed, if the original maps change
    // see here https://github.com/qgis/Quantum-GIS/pull/673
    // for explanation
    QgsFeatureMap mAddedFeatures;
    QgsGeometryMap mChangedGeometries;
    QgsFeatureIds mDeletedFeatureIds;
    QList<QgsField> mAddedAttributes;
    QgsChangedAttributesMap mChangedAttributeValues;
    QgsAttributeList mDeletedAttributeIds;
};


class CORE_EXPORT QgsVectorLayerFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsVectorLayerFeatureSource>
{
  public:
    QgsVectorLayerFeatureIterator( QgsVectorLayerFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

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
    inline virtual bool nextFeatureFilterExpression( QgsFeature &f ) { return fetchFeature( f ); }

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod& simplifyMethod );


    QgsFeatureRequest mProviderRequest;
    QgsFeatureIterator mProviderIterator;
    QgsFeatureRequest mChangedFeaturesRequest;
    QgsFeatureIterator mChangedFeaturesIterator;


    // only related to editing
    QSet<QgsFeatureId> mFetchConsidered;
    QgsGeometryMap::ConstIterator mFetchChangedGeomIt;
    QgsFeatureMap::ConstIterator mFetchAddedFeaturesIt;

    bool mFetchedFid; // when iterating by FID: indicator whether it has been fetched yet or not

    void rewindEditBuffer();
    void prepareJoins();
    void prepareExpressions();
    bool fetchNextAddedFeature( QgsFeature& f );
    bool fetchNextChangedGeomFeature( QgsFeature& f );
    bool fetchNextChangedAttributeFeature( QgsFeature& f );
    void useAddedFeature( const QgsFeature& src, QgsFeature& f );
    void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f );
    bool nextFeatureFid( QgsFeature& f );
    void addJoinedAttributes( QgsFeature &f );
    /**
     * Adds attributes that don't source from the provider but are added inside QGIS
     * Includes
     *  - Joined fields
     *  - Expression fields
     *
     * @param f The feature will be modified
     */
    void addVirtualAttributes( QgsFeature &f );

    /** Update feature with uncommited attribute updates */
    void updateChangedAttributes( QgsFeature& f );

    /** Update feature with uncommited geometry updates */
    void updateFeatureGeometry( QgsFeature& f );

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

    /** information about joins used in the current select() statement.
      Allows faster mapping of attribute ids compared to mVectorJoins */
    QMap<QgsVectorLayer*, FetchJoinInfo> mFetchJoinInfo;

    QMap<int, QgsExpression*> mExpressionFieldInfo;

    bool mHasVirtualAttributes;

  private:
    //! optional object to locally simplify edited (changed or added) geometries fetched by this feature iterator
    QgsAbstractGeometrySimplifier* mEditGeometrySimplifier;

    //! returns whether the iterator supports simplify geometries on provider side
    virtual bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const;
};

#endif // QGSVECTORLAYERFEATUREITERATOR_H
