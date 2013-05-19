/***************************************************************************
                          qgsvectorlayerjoinbuffer.cpp
                          ----------------------------
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

#include "qgsvectorlayerjoinbuffer.h"

#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"

#include <QDomElement>

QgsVectorLayerJoinBuffer::QgsVectorLayerJoinBuffer()
{
}

QgsVectorLayerJoinBuffer::~QgsVectorLayerJoinBuffer()
{
}

void QgsVectorLayerJoinBuffer::addJoin( const QgsVectorJoinInfo& joinInfo )
{
  mVectorJoins.push_back( joinInfo );

  //cache joined layer to virtual memory if specified by user
  if ( joinInfo.memoryCache )
  {
    cacheJoinLayer( mVectorJoins.last() );
  }
}

void QgsVectorLayerJoinBuffer::removeJoin( const QString& joinLayerId )
{
  for ( int i = 0; i < mVectorJoins.size(); ++i )
  {
    if ( mVectorJoins.at( i ).joinLayerId == joinLayerId )
    {
      mVectorJoins.removeAt( i );
    }
  }
}

void QgsVectorLayerJoinBuffer::cacheJoinLayer( QgsVectorJoinInfo& joinInfo )
{
  //memory cache not required or already done
  if ( !joinInfo.memoryCache || joinInfo.cachedAttributes.size() > 0 )
  {
    return;
  }

  QgsVectorLayer* cacheLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinInfo.joinLayerId ) );
  if ( cacheLayer )
  {
    int joinFieldIndex;
    if ( joinInfo.joinFieldName.isEmpty() )
      joinFieldIndex = joinInfo.joinFieldIndex;   //for compatibility with 1.x
    else
      joinFieldIndex = cacheLayer->pendingFields().indexFromName( joinInfo.joinFieldName );

    joinInfo.cachedAttributes.clear();

    QgsFeatureIterator fit = cacheLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      const QgsAttributes& attrs = f.attributes();
      joinInfo.cachedAttributes.insert( attrs[joinFieldIndex].toString(), attrs );
    }
  }
}

void QgsVectorLayerJoinBuffer::updateFields( QgsFields& fields )
{
  QList< QgsVectorJoinInfo>::const_iterator joinIt = mVectorJoins.constBegin();
  for ( int joinIdx = 0 ; joinIt != mVectorJoins.constEnd(); ++joinIt, ++joinIdx )
  {
    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
    if ( !joinLayer )
    {
      continue;
    }

    const QgsFields& joinFields = joinLayer->pendingFields();
    QString joinFieldName;
    if ( joinIt->joinFieldName.isEmpty() && joinIt->joinFieldIndex >= 0 && joinIt->joinFieldIndex < joinFields.count() )
      joinFieldName = joinFields.field( joinIt->joinFieldIndex ).name();  //for compatibility with 1.x
    else
      joinFieldName = joinIt->joinFieldName;

    for ( int idx = 0; idx < joinFields.count(); ++idx )
    {
      //skip the join field to avoid double field names (fields often have the same name)
      if ( joinFields[idx].name() != joinFieldName )
      {
        QgsField f = joinFields[idx];
        f.setName( joinLayer->name() + "_" + f.name() );
        fields.append( f, QgsFields::OriginJoin, idx + ( joinIdx*1000 ) );
      }
    }
  }
}

void QgsVectorLayerJoinBuffer::createJoinCaches()
{
  QList< QgsVectorJoinInfo >::iterator joinIt = mVectorJoins.begin();
  for ( ; joinIt != mVectorJoins.end(); ++joinIt )
  {
    cacheJoinLayer( *joinIt );
  }
}


void QgsVectorLayerJoinBuffer::writeXml( QDomNode& layer_node, QDomDocument& document ) const
{
  QDomElement vectorJoinsElem = document.createElement( "vectorjoins" );
  layer_node.appendChild( vectorJoinsElem );
  QList< QgsVectorJoinInfo >::const_iterator joinIt = mVectorJoins.constBegin();
  for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
  {
    QDomElement joinElem = document.createElement( "join" );

    if ( joinIt->targetFieldName.isEmpty() )
      joinElem.setAttribute( "targetField", joinIt->targetFieldIndex );   //for compatibility with 1.x
    else
      joinElem.setAttribute( "targetFieldName", joinIt->targetFieldName );

    joinElem.setAttribute( "joinLayerId", joinIt->joinLayerId );
    if ( joinIt->joinFieldName.isEmpty() )
      joinElem.setAttribute( "joinField", joinIt->joinFieldIndex );   //for compatibility with 1.x
    else
      joinElem.setAttribute( "joinFieldName", joinIt->joinFieldName );

    joinElem.setAttribute( "memoryCache", !joinIt->cachedAttributes.isEmpty() );
    vectorJoinsElem.appendChild( joinElem );
  }
}

void QgsVectorLayerJoinBuffer::readXml( const QDomNode& layer_node )
{
  mVectorJoins.clear();
  QDomElement vectorJoinsElem = layer_node.firstChildElement( "vectorjoins" );
  if ( !vectorJoinsElem.isNull() )
  {
    QDomNodeList joinList = vectorJoinsElem.elementsByTagName( "join" );
    for ( int i = 0; i < joinList.size(); ++i )
    {
      QDomElement infoElem = joinList.at( i ).toElement();
      QgsVectorJoinInfo info;
      info.joinFieldName = infoElem.attribute( "joinFieldName" );
      info.joinLayerId = infoElem.attribute( "joinLayerId" );
      info.targetFieldName = infoElem.attribute( "targetFieldName" );
      info.memoryCache = infoElem.attribute( "memoryCache" ).toInt();

      info.joinFieldIndex = infoElem.attribute( "joinField" ).toInt();   //for compatibility with 1.x
      info.targetFieldIndex = infoElem.attribute( "targetField" ).toInt();   //for compatibility with 1.x

      addJoin( info );
    }
  }
}

const QgsVectorJoinInfo* QgsVectorLayerJoinBuffer::joinForFieldIndex( int index, const QgsFields& fields, int& sourceFieldIndex ) const
{
  if ( fields.fieldOrigin( index ) != QgsFields::OriginJoin )
    return 0;

  int originIndex = fields.fieldOriginIndex( index );
  int sourceJoinIndex = originIndex / 1000;
  sourceFieldIndex = originIndex % 1000;

  if ( sourceJoinIndex < 0 || sourceJoinIndex >= mVectorJoins.count() )
    return 0;

  return &( mVectorJoins[sourceJoinIndex] );
}
