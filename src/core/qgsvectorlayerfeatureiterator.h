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

class QgsVectorLayer;
class QgsVectorLayerEditBuffer;
struct QgsVectorJoinInfo;

class CORE_EXPORT QgsVectorLayerFeatureIterator : public QgsAbstractFeatureIterator
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

#if 0
    // general stuff
    bool mFetching;
    QgsRectangle mFetchRect;
    QgsAttributeList mFetchAttributes;
    QgsAttributeList mFetchProvAttributes;
    bool mFetchGeometry;
#endif

    // only related to editing
    QSet<QgsFeatureId> mFetchConsidered;
    QgsGeometryMap::ConstIterator mFetchChangedGeomIt;
    QgsFeatureMap::ConstIterator mFetchAddedFeaturesIt;

    bool mFetchedFid; // when iterating by FID: indicator whether it has been fetched yet or not

    void rewindEditBuffer();
    void prepareJoins();
    bool fetchNextAddedFeature( QgsFeature& f );
    bool fetchNextChangedGeomFeature( QgsFeature& f );
    void useAddedFeature( const QgsFeature& src, QgsFeature& f );
    void useChangedAttributeFeature( QgsFeatureId fid, const QgsGeometry& geom, QgsFeature& f );
    bool nextFeatureFid( QgsFeature& f );
    void addJoinedAttributes( QgsFeature &f );

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

    // A deep-copy is only performed, if the original maps change
    // see here https://github.com/qgis/Quantum-GIS/pull/673
    // for explanation
    QgsFeatureMap mAddedFeatures;
    QgsGeometryMap mChangedGeometries;
    QgsFeatureIds mDeletedFeatureIds;
    QList<QgsField> mAddedAttributes;
    QgsChangedAttributesMap mChangedAttributeValues;
    QgsAttributeList mDeletedAttributeIds;

    /** Informations about joins used in the current select() statement.
      Allows faster mapping of attribute ids compared to mVectorJoins */
    QMap<QgsVectorLayer*, FetchJoinInfo> mFetchJoinInfo;
};

#endif // QGSVECTORLAYERFEATUREITERATOR_H
