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

void QgsVectorLayerJoinBuffer::addJoin( QgsVectorJoinInfo joinInfo )
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
      //remove corresponding fetch join info
      QgsVectorLayer* joinLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinLayerId ) );
      if ( joinLayer )
      {
        mFetchJoinInfos.remove( joinLayer );
      }
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
    joinInfo.cachedAttributes.clear();
    cacheLayer->select( cacheLayer->pendingAllAttributesList(), QgsRectangle(), false, false );
    QgsFeature f;
    while ( cacheLayer->nextFeature( f ) )
    {
      const QgsAttributeMap& map = f.attributeMap();
      joinInfo.cachedAttributes.insert( map.value( joinInfo.joinField ).toString(), map );
    }
  }
}

void QgsVectorLayerJoinBuffer::updateFieldMap( QgsFieldMap& fields, int& maxIndex )
{
  int currentMaxIndex = 0; //maximum index of the current join layer

  QList< QgsVectorJoinInfo>::const_iterator joinIt = mVectorJoins.constBegin();
  for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
  {
    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
    if ( !joinLayer )
    {
      continue;
    }

    const QgsFieldMap& joinFields = joinLayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = joinFields.constBegin();
    for ( ; fieldIt != joinFields.constEnd(); ++fieldIt )
    {
      fields.insert( maxIndex + 1 + fieldIt.key(), fieldIt.value() );
    }

    if ( maximumIndex( joinFields, currentMaxIndex ) )
    {
      maxIndex += ( currentMaxIndex + 1 ); //+1 because there are fields with index 0
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

void QgsVectorLayerJoinBuffer::select( const QgsAttributeList& fetchAttributes,
                                       QgsAttributeList& sourceJoinFields, int maxProviderIndex )
{
  mFetchJoinInfos.clear();
  sourceJoinFields.clear();

  QgsAttributeList::const_iterator attIt = fetchAttributes.constBegin();
  for ( ; attIt != fetchAttributes.constEnd(); ++attIt )
  {
    int indexOffset;
    const QgsVectorJoinInfo* joinInfo = joinForFieldIndex( *attIt, maxProviderIndex, indexOffset );
    if ( joinInfo )
    {
      QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinInfo->joinLayerId ) );
      if ( joinLayer )
      {
        mFetchJoinInfos[ joinLayer ].joinInfo = joinInfo;
        mFetchJoinInfos[ joinLayer].attributes.push_back( *attIt - indexOffset ); //store provider index
        mFetchJoinInfos[ joinLayer ].indexOffset = indexOffset;
        //for joined fields, we always need to request the targetField from the provider too
        if ( !fetchAttributes.contains( joinInfo->targetField ) )
        {
          sourceJoinFields << joinInfo->targetField;
        }
      }
    }
  }
}

void QgsVectorLayerJoinBuffer::updateFeatureAttributes( QgsFeature &f, int maxProviderIndex, bool all )
{
  if ( all )
  {
    int index = maxProviderIndex + 1;
    int currentMaxIndex;

    QList< QgsVectorJoinInfo >::const_iterator joinIt = mVectorJoins.constBegin();
    for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
    {
      QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
      if ( !joinLayer )
      {
        continue;
      }

      QString joinFieldName = joinLayer->pendingFields().value( joinIt->joinField ).name();
      if ( joinFieldName.isEmpty() )
      {
        continue;
      }

      QVariant targetFieldValue = f.attributeMap().value( joinIt->targetField );
      if ( !targetFieldValue.isValid() )
      {
        continue;
      }

      addJoinedFeatureAttributes( f, *joinIt, joinFieldName, targetFieldValue, joinLayer->pendingAllAttributesList(), index );

      maximumIndex( joinLayer->pendingFields(), currentMaxIndex );
      index += ( currentMaxIndex + 1 );
    }
  }
  else
  {
    QMap<QgsVectorLayer*, QgsFetchJoinInfo>::const_iterator joinIt = mFetchJoinInfos.constBegin();
    for ( ; joinIt != mFetchJoinInfos.constEnd(); ++joinIt )
    {
      QgsVectorLayer* joinLayer = joinIt.key();
      if ( !joinLayer )
      {
        continue;
      }

      QString joinFieldName = joinLayer->pendingFields().value( joinIt.value().joinInfo->joinField ).name();
      if ( joinFieldName.isEmpty() )
      {
        continue;
      }

      QVariant targetFieldValue = f.attributeMap().value( joinIt->joinInfo->targetField );
      if ( !targetFieldValue.isValid() )
      {
        continue;
      }

      addJoinedFeatureAttributes( f, *( joinIt.value().joinInfo ), joinFieldName, targetFieldValue, joinIt.value().attributes, joinIt.value().indexOffset );
    }
  }
}

void QgsVectorLayerJoinBuffer::addJoinedFeatureAttributes( QgsFeature& f, const QgsVectorJoinInfo& joinInfo, const QString& joinFieldName,
    const QVariant& joinValue, const QgsAttributeList& attributes, int attributeIndexOffset )
{
  const QHash< QString, QgsAttributeMap>& memoryCache = joinInfo.cachedAttributes;
  if ( !memoryCache.isEmpty() ) //use join memory cache
  {
    QgsAttributeMap featureAttributes = memoryCache.value( joinValue.toString() );
    bool found = !featureAttributes.isEmpty();
    QgsAttributeList::const_iterator attIt = attributes.constBegin();
    for ( ; attIt != attributes.constEnd(); ++attIt )
    {
      if ( found )
      {
        f.addAttribute( *attIt + attributeIndexOffset, featureAttributes.value( *attIt ) );
      }
      else
      {
        f.addAttribute( *attIt + attributeIndexOffset, QVariant() );
      }
    }
  }
  else //work with subset string
  {
    QgsVectorLayer* joinLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinInfo.joinLayerId ) );
    if ( !joinLayer )
    {
      return;
    }

    //no memory cache, query the joined values by setting substring
    QString subsetString = joinLayer->dataProvider()->subsetString(); //provider might already have a subset string
    QString bkSubsetString = subsetString;
    if ( !subsetString.isEmpty() )
    {
      subsetString.append( " AND " );
    }

    subsetString.append( "\"" + joinFieldName + "\"" + " = " + "\"" + joinValue.toString() + "\"" );
    joinLayer->dataProvider()->setSubsetString( subsetString, false );

    //select (no geometry)
    joinLayer->select( attributes, QgsRectangle(), false, false );

    //get first feature
    QgsFeature fet;
    if ( joinLayer->nextFeature( fet ) )
    {
      QgsAttributeMap attMap = fet.attributeMap();
      QgsAttributeMap::const_iterator attIt = attMap.constBegin();
      for ( ; attIt != attMap.constEnd(); ++attIt )
      {
        f.addAttribute( attIt.key() + attributeIndexOffset, attIt.value() );
      }
    }
    else //no suitable join feature found, insert invalid variants
    {
      QgsAttributeList::const_iterator attIt = attributes.constBegin();
      for ( ; attIt != attributes.constEnd(); ++attIt )
      {
        f.addAttribute( *attIt + attributeIndexOffset, QVariant() );
      }
    }

    joinLayer->dataProvider()->setSubsetString( bkSubsetString, false );
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
    joinElem.setAttribute( "targetField", joinIt->targetField );
    joinElem.setAttribute( "joinLayerId", joinIt->joinLayerId );
    joinElem.setAttribute( "joinField", joinIt->joinField );
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
      info.joinField = infoElem.attribute( "joinField" ).toInt();
      info.joinLayerId = infoElem.attribute( "joinLayerId" );
      info.targetField = infoElem.attribute( "targetField" ).toInt();
      info.memoryCache = infoElem.attribute( "memoryCache" ).toInt();
      addJoin( info );
    }
  }
}

const QgsVectorJoinInfo* QgsVectorLayerJoinBuffer::joinForFieldIndex( int index, int maxProviderIndex, int& indexOffset ) const
{
  int currentMaxIndex = 0;
  int totIndex = maxProviderIndex + 1;

  //go through all the joins to search the index
  QList< QgsVectorJoinInfo>::const_iterator joinIt = mVectorJoins.constBegin();
  for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
  {
    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
    if ( !joinLayer )
    {
      continue;
    }

    if ( joinLayer->pendingFields().contains( index - totIndex ) )
    {
      indexOffset = totIndex;
      return &( *joinIt );
    }

    maximumIndex( joinLayer->pendingFields(), currentMaxIndex );
    totIndex += ( currentMaxIndex + 1 );
  }

  //an added field or a provider field
  return 0;
}

bool QgsVectorLayerJoinBuffer::maximumIndex( const QgsFieldMap& fMap, int& index )
{
  if ( fMap.size() < 1 )
  {
    return false;
  }
  QgsFieldMap::const_iterator endIt = fMap.constEnd();
  --endIt;
  index = endIt.key();
  return true;
}
