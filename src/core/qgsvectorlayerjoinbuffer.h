/***************************************************************************
                          qgsvectorlayerjoinbuffer.h
                          ---------------------------
    begin                : Feb 09, 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERJOINBUFFER_H
#define QGSVECTORLAYERJOINBUFFER_H

#include "qgsfeature.h"
#include "qgsvectorlayer.h"

#include <QHash>
#include <QString>

/**Manages joined fields for a vector layer*/
class CORE_EXPORT QgsVectorLayerJoinBuffer
{
  public:
    QgsVectorLayerJoinBuffer();
    ~QgsVectorLayerJoinBuffer();

    /**Joins another vector layer to this layer
      @param joinInfo join object containing join layer id, target and source field */
    void addJoin( QgsVectorJoinInfo joinInfo );

    /**Removes  a vector layer join*/
    void removeJoin( const QString& joinLayerId );

    /**Creates QgsVectorLayerJoinBuffer for the joins containing attributes to fetch*/
    void select( const QgsAttributeList& fetchAttributes,
                 QgsAttributeList& sourceJoinFields, int maxProviderIndex );

    /**Updates field map with joined attributes
      @param fields map to append joined attributes
      @param maxIndex in/out: maximum attribute index*/
    void updateFieldMap( QgsFieldMap& fields, int& maxIndex );

    /**Update feature with uncommited attribute updates and joined attributes*/
    void updateFeatureAttributes( QgsFeature &f, int maxProviderIndex, bool all = false );

    /**Calls cacheJoinLayer() for all vector joins*/
    void createJoinCaches();

    /**Saves mVectorJoins to xml under the layer node*/
    void writeXml( QDomNode& layer_node, QDomDocument& document ) const;

    /**Reads joins from project file*/
    void readXml( const QDomNode& layer_node );

    /**Quick way to test if there is any join at all*/
    bool containsJoins() const { return ( mVectorJoins.size() > 0 ); }
    /**Quick way to test if there is a join to be fetched*/
    bool containsFetchJoins() const { return ( mFetchJoinInfos.size() > 0 ); }

    const QList< QgsVectorJoinInfo >& vectorJoins() const { return mVectorJoins; }

    /**Finds the vector join for a layer field index.
      @param index this layers attribute index
      @param maxProviderIndex maximum attribute index of the vectorlayer provider
      @param indexOffset out: offset between layer index and original provider index
       @return pointer to the join if the index belongs to a joined field, otherwise 0 (possibily provider field or added field)*/
    const QgsVectorJoinInfo* joinForFieldIndex( int index, int maxProviderIndex, int& indexOffset ) const;

    /** Helper function to find out the maximum index of a field map
        @return true in case of success, otherwise false (e.g. empty map)*/
    static bool maximumIndex( const QgsFieldMap& fMap, int& index );

  private:

    /**Joined vector layers*/
    QList< QgsVectorJoinInfo > mVectorJoins;

    /**Informations about joins used in the current select() statement.
      Allows faster mapping of attribute ids compared to mVectorJoins*/
    QMap<QgsVectorLayer*, QgsFetchJoinInfo> mFetchJoinInfos;

    /**Caches attributes of join layer in memory if QgsVectorJoinInfo.memoryCache is true (and the cache is not already there)*/
    void cacheJoinLayer( QgsVectorJoinInfo& joinInfo );

    /**Adds joined attributes to a feature
      @param f the feature to add the attributes
      @param joinInfo vector join
      @param joinFieldName name of the (source) join Field
      @param joinValue lookup value for join
      @param attributes (join layer) attribute indices to add
      @param attributeIndexOffset index offset to get from join layer attribute index to layer index*/
    void addJoinedFeatureAttributes( QgsFeature& f, const QgsVectorJoinInfo& joinInfo, const QString& joinFieldName, const QVariant& joinValue,
                                     const QgsAttributeList& attributes, int attributeIndexOffset );
};

#endif // QGSVECTORLAYERJOINBUFFER_H
