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
      const QgsAttributes& attrs = f.attributes();
      joinInfo.cachedAttributes.insert( attrs[joinInfo.joinField].toString(), attrs );
    }
  }
}

void QgsVectorLayerJoinBuffer::updateFields( QgsFields& fields )
{
  QList< QgsVectorJoinInfo>::const_iterator joinIt = mVectorJoins.constBegin();
  for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
  {
    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
    if ( !joinLayer )
    {
      continue;
    }

    const QgsFields& joinFields = joinLayer->pendingFields();
    for ( int idx = 0; idx < joinFields.count(); ++idx )
    {
      //skip the join field to avoid double field names (fields often have the same name)
      if ( idx != joinIt->joinField )
      {
        QgsField f = joinFields[idx];
        f.setName( joinLayer->name() + "_" + f.name() );
        fields.append( f );
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

void QgsVectorLayerJoinBuffer::select( const QgsAttributeList& fetchAttributes,
                                       QgsAttributeList& sourceJoinFields, int providerFieldCount )
{
  mFetchJoinInfos.clear();
  sourceJoinFields.clear();

  QgsAttributeList::const_iterator attIt = fetchAttributes.constBegin();
  for ( ; attIt != fetchAttributes.constEnd(); ++attIt )
  {
    int indexOffset;
    const QgsVectorJoinInfo* joinInfo = joinForFieldIndex( *attIt, providerFieldCount, indexOffset );
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

void QgsVectorLayerJoinBuffer::updateFeatureAttributes( QgsFeature &f, int providerFieldCount, bool all )
{
  if ( all )
  {
    int index = providerFieldCount;

    QList< QgsVectorJoinInfo >::const_iterator joinIt = mVectorJoins.constBegin();
    for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
    {
      QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
      if ( !joinLayer )
      {
        continue;
      }

      const QgsFields& fields = joinLayer->pendingFields();
      if ( joinIt->joinField < 0 || joinIt->joinField >= fields.count() )
        continue;
      QString joinFieldName = fields[joinIt->joinField].name();
      if ( joinFieldName.isEmpty() )
      {
        continue;
      }

      QVariant targetFieldValue = f.attribute( joinIt->targetField );
      if ( !targetFieldValue.isValid() )
      {
        continue;
      }

      addJoinedFeatureAttributes( f, *joinIt, joinFieldName, targetFieldValue, joinLayer->pendingAllAttributesList(), index );

      index += joinLayer->pendingFields().count();
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

      int joinField = joinIt.value().joinInfo->joinField;
      const QgsFields& fields = joinLayer->pendingFields();
      if ( joinField < 0 || joinField < fields.count() )
        continue;
      QString joinFieldName = fields[joinField].name();
      if ( joinFieldName.isEmpty() )
      {
        continue;
      }

      QVariant targetFieldValue = f.attribute( joinIt->joinInfo->targetField );
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
  f.attributes().resize( attributeIndexOffset + attributes.count() ); // make sure we have enough space for newly added attributes
  const QHash< QString, QgsAttributes>& memoryCache = joinInfo.cachedAttributes;
  if ( !memoryCache.isEmpty() ) //use join memory cache
  {
    QgsAttributes featureAttributes = memoryCache.value( joinValue.toString() );
    bool found = !featureAttributes.isEmpty();
    for ( int i = 0; i < featureAttributes.count(); ++i )
    {
      //skip the join field to avoid double field names (fields often have the same name)
      if ( i == joinInfo.joinField )
      {
        continue;
      }

      if ( found )
      {
        f.setAttribute( i + attributeIndexOffset, featureAttributes[i] );
      }
      else
      {
        f.setAttribute( i + attributeIndexOffset, QVariant() );
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
      const QgsAttributes& attr = fet.attributes();
      for ( int i = 0; i < attr.count(); ++i )
      {
        f.setAttribute( i + attributeIndexOffset, attr[i] );
      }
    }
    else //no suitable join feature found, insert invalid variants
    {
      QgsAttributeList::const_iterator attIt = attributes.constBegin();
      for ( ; attIt != attributes.constEnd(); ++attIt )
      {
        f.setAttribute( *attIt + attributeIndexOffset, QVariant() );
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

const QgsVectorJoinInfo* QgsVectorLayerJoinBuffer::joinForFieldIndex( int index, int providerFieldCount, int& indexOffset ) const
{
  int totIndex = providerFieldCount;

  //go through all the joins to search the index
  QList< QgsVectorJoinInfo>::const_iterator joinIt = mVectorJoins.constBegin();
  for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
  {
    QgsVectorLayer* joinLayer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( joinIt->joinLayerId ) );
    if ( !joinLayer )
    {
      continue;
    }

    if ( joinLayer->pendingFields().count() > index - totIndex )
    {
      indexOffset = totIndex;
      return &( *joinIt );
    }

    totIndex += joinLayer->pendingFields().count();
  }

  //an added field or a provider field
  return 0;
}
