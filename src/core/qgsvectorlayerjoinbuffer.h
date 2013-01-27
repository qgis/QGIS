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
    void addJoin( const QgsVectorJoinInfo& joinInfo );

    /**Removes  a vector layer join*/
    void removeJoin( const QString& joinLayerId );

    /**Updates field map with joined attributes
      @param fields map to append joined attributes
     */
    void updateFields( QgsFields& fields );

    /**Calls cacheJoinLayer() for all vector joins*/
    void createJoinCaches();

    /**Saves mVectorJoins to xml under the layer node*/
    void writeXml( QDomNode& layer_node, QDomDocument& document ) const;

    /**Reads joins from project file*/
    void readXml( const QDomNode& layer_node );

    /**Quick way to test if there is any join at all*/
    bool containsJoins() const { return !mVectorJoins.isEmpty(); }

    const QList< QgsVectorJoinInfo >& vectorJoins() const { return mVectorJoins; }

    /**Finds the vector join for a layer field index.
      @param index this layers attribute index
      @param fields fields of the vector layer (including joined fields)
      @param sourceFieldIndex Output: field's index in source layer */
    const QgsVectorJoinInfo* joinForFieldIndex( int index, const QgsFields& fields, int& sourceFieldIndex ) const;

  private:

    /**Joined vector layers*/
    QList< QgsVectorJoinInfo > mVectorJoins;

    /**Caches attributes of join layer in memory if QgsVectorJoinInfo.memoryCache is true (and the cache is not already there)*/
    void cacheJoinLayer( QgsVectorJoinInfo& joinInfo );
};

#endif // QGSVECTORLAYERJOINBUFFER_H
