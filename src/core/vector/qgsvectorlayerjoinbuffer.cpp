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

#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsauxiliarystorage.h"

#include <QDomElement>

QgsVectorLayerJoinBuffer::QgsVectorLayerJoinBuffer( QgsVectorLayer *layer )
  : mLayer( layer )
{
}

static QList<QgsVectorLayer *> _outEdges( QgsVectorLayer *vl )
{
  QList<QgsVectorLayer *> lst;
  const auto constVectorJoins = vl->vectorJoins();
  for ( const QgsVectorLayerJoinInfo &info : constVectorJoins )
  {
    if ( QgsVectorLayer *joinVl = info.joinLayer() )
      lst << joinVl;
  }
  return lst;
}

static bool _hasCycleDFS( QgsVectorLayer *n, QHash<QgsVectorLayer *, int> &mark )
{
  if ( mark.value( n ) == 1 ) // temporary
    return true;
  if ( mark.value( n ) == 0 ) // not visited
  {
    mark[n] = 1; // temporary
    const auto outEdges { _outEdges( n ) };
    for ( QgsVectorLayer *m : outEdges )
    {
      if ( _hasCycleDFS( m, mark ) )
        return true;
    }
    mark[n] = 2; // permanent
  }
  return false;
}


bool QgsVectorLayerJoinBuffer::addJoin( const QgsVectorLayerJoinInfo &joinInfo )
{
  QMutexLocker locker( &mMutex );
  mVectorJoins.push_back( joinInfo );

  // run depth-first search to detect cycles in the graph of joins between layers.
  // any cycle would cause infinite recursion when updating fields
  QHash<QgsVectorLayer *, int> markDFS;
  if ( mLayer && _hasCycleDFS( mLayer, markDFS ) )
  {
    // we have to reject this one
    mVectorJoins.pop_back();
    return false;
  }

  // Wait for notifications about changed fields in joined layer to propagate them.
  // During project load the joined layers possibly do not exist yet so the connection will not be created,
  // but then QgsProject makes sure to call createJoinCaches() which will do the connection.
  // Unique connection makes sure we do not respond to one layer's update more times (in case of multiple join)
  if ( QgsVectorLayer *vl = joinInfo.joinLayer() )
  {
    connectJoinedLayer( vl );
  }

  locker.unlock();
  mLayer->updateFields();
  locker.relock();

  //cache joined layer to virtual memory if specified by user
  if ( joinInfo.isUsingMemoryCache() )
  {
    cacheJoinLayer( mVectorJoins.last() );
  }

  locker.unlock();

  return true;
}


bool QgsVectorLayerJoinBuffer::removeJoin( const QString &joinLayerId )
{
  bool res = false;
  {
    QMutexLocker locker( &mMutex );
    for ( int i = 0; i < mVectorJoins.size(); ++i )
    {
      if ( mVectorJoins.at( i ).joinLayerId() == joinLayerId )
      {
        if ( QgsVectorLayer *vl = mVectorJoins.at( i ).joinLayer() )
        {
          disconnect( vl, &QgsVectorLayer::updatedFields, this, &QgsVectorLayerJoinBuffer::joinedLayerUpdatedFields );
        }

        mVectorJoins.removeAt( i );
        res = true;
      }
    }
  }

  emit joinedFieldsChanged();
  return res;
}

void QgsVectorLayerJoinBuffer::cacheJoinLayer( QgsVectorLayerJoinInfo &joinInfo )
{
  //memory cache not required or already done
  if ( !joinInfo.isUsingMemoryCache() || !joinInfo.cacheDirty )
  {
    return;
  }

  QgsVectorLayer *cacheLayer = joinInfo.joinLayer();
  if ( cacheLayer )
  {
    int joinFieldIndex = cacheLayer->fields().indexFromName( joinInfo.joinFieldName() );

    if ( joinFieldIndex < 0 || joinFieldIndex >= cacheLayer->fields().count() )
      return;

    joinInfo.cachedAttributes.clear();

    QgsFeatureRequest request;
    request.setFlags( QgsFeatureRequest::NoGeometry );
    // maybe user requested just a subset of layer's attributes
    // so we do not have to cache everything
    QVector<int> subsetIndices;
    if ( joinInfo.hasSubset() )
    {
      const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( joinInfo );
      subsetIndices = joinSubsetIndices( cacheLayer, subsetNames );

      // we need just subset of attributes - but make sure to include join field name
      QgsAttributeList cacheLayerAttrs = subsetIndices.toList();
      if ( !cacheLayerAttrs.contains( joinFieldIndex ) )
        cacheLayerAttrs.append( joinFieldIndex );
      request.setSubsetOfAttributes( cacheLayerAttrs );
    }

    QgsFeatureIterator fit = cacheLayer->getFeatures( request );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      QgsAttributes attrs = f.attributes();
      QString key = attrs.at( joinFieldIndex ).toString();
      if ( joinInfo.hasSubset() )
      {
        QgsAttributes subsetAttrs( subsetIndices.count() );
        for ( int i = 0; i < subsetIndices.count(); ++i )
          subsetAttrs[i] = attrs.at( subsetIndices.at( i ) );
        joinInfo.cachedAttributes.insert( key, subsetAttrs );
      }
      else
      {
        QgsAttributes attributesCache;
        for ( int i = 0; i < attrs.size(); i++ )
        {
          if ( i == joinFieldIndex )
            continue;

          QString joinInfoPrefix = joinInfo.prefix();
          if ( joinInfoPrefix.isNull() ) // Default prefix 'layerName_' used
            joinInfoPrefix = QString( "%1_" ).arg( cacheLayer->name() );

          // Joined field name
          const QString joinFieldName = joinInfoPrefix + cacheLayer->fields().names().at( i );

          // Check for name collisions
          int fieldIndex = mLayer->fields().indexFromName( joinFieldName );
          if ( fieldIndex >= 0
               && mLayer->fields().fieldOrigin( fieldIndex ) != QgsFields::OriginJoin )
            continue;

          attributesCache.append( attrs.at( i ) );
        }
        joinInfo.cachedAttributes.insert( key, attributesCache );
      }
    }
    joinInfo.cacheDirty = false;
  }
}


QVector<int> QgsVectorLayerJoinBuffer::joinSubsetIndices( QgsVectorLayer *joinLayer, const QStringList &joinFieldsSubset )
{
  return joinSubsetIndices( joinLayer->fields(), joinFieldsSubset );
}

QVector<int> QgsVectorLayerJoinBuffer::joinSubsetIndices( const QgsFields &joinLayerFields, const QStringList &joinFieldsSubset )
{
  QVector<int> subsetIndices;
  for ( int i = 0; i < joinFieldsSubset.count(); ++i )
  {
    QString joinedFieldName = joinFieldsSubset.at( i );
    int index = joinLayerFields.lookupField( joinedFieldName );
    if ( index != -1 )
    {
      subsetIndices.append( index );
    }
    else
    {
      QgsDebugMsg( "Join layer subset field not found: " + joinedFieldName );
    }
  }

  return subsetIndices;
}

void QgsVectorLayerJoinBuffer::updateFields( QgsFields &fields )
{
  QString prefix;

  QList< QgsVectorLayerJoinInfo>::const_iterator joinIt = mVectorJoins.constBegin();
  for ( int joinIdx = 0; joinIt != mVectorJoins.constEnd(); ++joinIt, ++joinIdx )
  {
    QgsVectorLayer *joinLayer = joinIt->joinLayer();
    if ( !joinLayer )
    {
      continue;
    }

    const QgsFields &joinFields = joinLayer->fields();
    QString joinFieldName = joinIt->joinFieldName();

    QSet<QString> subset;
    if ( joinIt->hasSubset() )
    {
      const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( *joinIt );
      subset = qgis::listToSet( subsetNames );
    }

    if ( joinIt->prefix().isNull() )
    {
      prefix = joinLayer->name() + '_';
    }
    else
    {
      prefix = joinIt->prefix();
    }

    for ( int idx = 0; idx < joinFields.count(); ++idx )
    {
      // if using just a subset of fields, filter some of them out
      if ( joinIt->hasSubset() && !subset.contains( joinFields.at( idx ).name() ) )
        continue;

      //skip the join field to avoid double field names (fields often have the same name)
      // when using subset of field, use all the selected fields
      if ( joinIt->hasSubset() || joinFields.at( idx ).name() != joinFieldName )
      {
        QgsField f = joinFields.at( idx );
        f.setName( prefix + f.name() );
        fields.append( f, QgsFields::OriginJoin, idx + ( joinIdx * 1000 ) );
      }
    }
  }
}

void QgsVectorLayerJoinBuffer::createJoinCaches()
{
  QMutexLocker locker( &mMutex );
  QList< QgsVectorLayerJoinInfo >::iterator joinIt = mVectorJoins.begin();
  for ( ; joinIt != mVectorJoins.end(); ++joinIt )
  {
    if ( joinIt->isUsingMemoryCache() && joinIt->cacheDirty )
      cacheJoinLayer( *joinIt );
  }
}


void QgsVectorLayerJoinBuffer::writeXml( QDomNode &layer_node, QDomDocument &document ) const
{
  QDomElement vectorJoinsElem = document.createElement( QStringLiteral( "vectorjoins" ) );
  layer_node.appendChild( vectorJoinsElem );
  QList< QgsVectorLayerJoinInfo >::const_iterator joinIt = mVectorJoins.constBegin();
  for ( ; joinIt != mVectorJoins.constEnd(); ++joinIt )
  {
    if ( isAuxiliaryJoin( *joinIt ) )
      continue;

    QDomElement joinElem = document.createElement( QStringLiteral( "join" ) );

    joinElem.setAttribute( QStringLiteral( "targetFieldName" ), joinIt->targetFieldName() );

    joinElem.setAttribute( QStringLiteral( "joinLayerId" ), joinIt->joinLayerId() );
    joinElem.setAttribute( QStringLiteral( "joinFieldName" ), joinIt->joinFieldName() );

    joinElem.setAttribute( QStringLiteral( "memoryCache" ), joinIt->isUsingMemoryCache() );
    joinElem.setAttribute( QStringLiteral( "dynamicForm" ), joinIt->isDynamicFormEnabled() );
    joinElem.setAttribute( QStringLiteral( "editable" ), joinIt->isEditable() );
    joinElem.setAttribute( QStringLiteral( "upsertOnEdit" ), joinIt->hasUpsertOnEdit() );
    joinElem.setAttribute( QStringLiteral( "cascadedDelete" ), joinIt->hasCascadedDelete() );

    if ( joinIt->hasSubset() )
    {
      QDomElement subsetElem = document.createElement( QStringLiteral( "joinFieldsSubset" ) );
      const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( *joinIt );

      const auto constSubsetNames = subsetNames;
      for ( const QString &fieldName : constSubsetNames )
      {
        QDomElement fieldElem = document.createElement( QStringLiteral( "field" ) );
        fieldElem.setAttribute( QStringLiteral( "name" ), fieldName );
        subsetElem.appendChild( fieldElem );
      }

      joinElem.appendChild( subsetElem );
    }

    if ( !joinIt->prefix().isNull() )
    {
      joinElem.setAttribute( QStringLiteral( "customPrefix" ), joinIt->prefix() );
      joinElem.setAttribute( QStringLiteral( "hasCustomPrefix" ), 1 );
    }

    vectorJoinsElem.appendChild( joinElem );
  }
}

void QgsVectorLayerJoinBuffer::readXml( const QDomNode &layer_node )
{
  mVectorJoins.clear();
  QDomElement vectorJoinsElem = layer_node.firstChildElement( QStringLiteral( "vectorjoins" ) );
  if ( !vectorJoinsElem.isNull() )
  {
    QDomNodeList joinList = vectorJoinsElem.elementsByTagName( QStringLiteral( "join" ) );
    for ( int i = 0; i < joinList.size(); ++i )
    {
      QDomElement infoElem = joinList.at( i ).toElement();
      QgsVectorLayerJoinInfo info;
      info.setJoinFieldName( infoElem.attribute( QStringLiteral( "joinFieldName" ) ) );
      // read layer ID - to turn it into layer object, caller will need to call resolveReferences() later
      info.setJoinLayerId( infoElem.attribute( QStringLiteral( "joinLayerId" ) ) );
      info.setTargetFieldName( infoElem.attribute( QStringLiteral( "targetFieldName" ) ) );
      info.setUsingMemoryCache( infoElem.attribute( QStringLiteral( "memoryCache" ) ).toInt() );
      info.setDynamicFormEnabled( infoElem.attribute( QStringLiteral( "dynamicForm" ) ).toInt() );
      info.setEditable( infoElem.attribute( QStringLiteral( "editable" ) ).toInt() );
      info.setUpsertOnEdit( infoElem.attribute( QStringLiteral( "upsertOnEdit" ) ).toInt() );
      info.setCascadedDelete( infoElem.attribute( QStringLiteral( "cascadedDelete" ) ).toInt() );

      QDomElement subsetElem = infoElem.firstChildElement( QStringLiteral( "joinFieldsSubset" ) );
      if ( !subsetElem.isNull() )
      {
        QStringList *fieldNames = new QStringList;
        QDomNodeList fieldNodes = infoElem.elementsByTagName( QStringLiteral( "field" ) );
        fieldNames->reserve( fieldNodes.count() );
        for ( int i = 0; i < fieldNodes.count(); ++i )
          *fieldNames << fieldNodes.at( i ).toElement().attribute( QStringLiteral( "name" ) );
        info.setJoinFieldNamesSubset( fieldNames );
      }

      if ( infoElem.attribute( QStringLiteral( "hasCustomPrefix" ) ).toInt() )
        info.setPrefix( infoElem.attribute( QStringLiteral( "customPrefix" ) ) );
      else
        info.setPrefix( QString() );

      addJoin( info );
    }
  }
}

void QgsVectorLayerJoinBuffer::resolveReferences( QgsProject *project )
{
  bool resolved = false;
  for ( QgsVectorJoinList::iterator it = mVectorJoins.begin(); it != mVectorJoins.end(); ++it )
  {
    if ( it->joinLayer() )
      continue;  // already resolved

    if ( QgsVectorLayer *joinedLayer = qobject_cast<QgsVectorLayer *>( project->mapLayer( it->joinLayerId() ) ) )
    {
      it->setJoinLayer( joinedLayer );
      connectJoinedLayer( joinedLayer );
      resolved = true;
    }
  }

  if ( resolved )
    emit joinedFieldsChanged();
}

int QgsVectorLayerJoinBuffer::joinedFieldsOffset( const QgsVectorLayerJoinInfo *info, const QgsFields &fields )
{
  if ( !info )
    return -1;

  int joinIndex = mVectorJoins.indexOf( *info );
  if ( joinIndex == -1 )
    return -1;

  for ( int i = 0; i < fields.count(); ++i )
  {
    if ( fields.fieldOrigin( i ) != QgsFields::OriginJoin )
      continue;

    if ( fields.fieldOriginIndex( i ) / 1000 == joinIndex )
      return i;
  }
  return -1;
}

const QgsVectorLayerJoinInfo *QgsVectorLayerJoinBuffer::joinForFieldIndex( int index, const QgsFields &fields, int &sourceFieldIndex ) const
{
  if ( fields.fieldOrigin( index ) != QgsFields::OriginJoin )
    return nullptr;

  int originIndex = fields.fieldOriginIndex( index );
  int sourceJoinIndex = originIndex / 1000;
  sourceFieldIndex = originIndex % 1000;

  if ( sourceJoinIndex < 0 || sourceJoinIndex >= mVectorJoins.count() )
    return nullptr;

  return &( mVectorJoins[sourceJoinIndex] );
}

QList<const QgsVectorLayerJoinInfo *> QgsVectorLayerJoinBuffer::joinsWhereFieldIsId( const QgsField &field ) const
{
  QList<const QgsVectorLayerJoinInfo *> infos;

  const auto constMVectorJoins = mVectorJoins;
  for ( const QgsVectorLayerJoinInfo &info : constMVectorJoins )
  {
    if ( infos.contains( &info ) )
      continue;

    if ( info.targetFieldName() == field.name() )
      infos.append( &info );
  }

  return infos;
}

QgsFeature QgsVectorLayerJoinBuffer::joinedFeatureOf( const QgsVectorLayerJoinInfo *info, const QgsFeature &feature ) const
{
  QgsFeature joinedFeature;

  if ( info->joinLayer() )
  {
    joinedFeature.initAttributes( info->joinLayer()->fields().count() );
    joinedFeature.setFields( info->joinLayer()->fields() );

    QString joinFieldName = info->joinFieldName();
    const QVariant targetValue = feature.attribute( info->targetFieldName() );
    QString filter = QgsExpression::createFieldEqualityExpression( joinFieldName, targetValue );

    QgsFeatureRequest request;
    request.setFilterExpression( filter );
    request.setLimit( 1 );

    QgsFeatureIterator it = info->joinLayer()->getFeatures( request );
    it.nextFeature( joinedFeature );
  }

  return joinedFeature;
}

QgsFeature QgsVectorLayerJoinBuffer::targetedFeatureOf( const QgsVectorLayerJoinInfo *info, const QgsFeature &feature ) const
{
  QgsFeature targetedFeature;

  if ( info->joinLayer() )
  {
    const QVariant targetValue = feature.attribute( info->joinFieldName() );
    const QString filter = QgsExpression::createFieldEqualityExpression( info->targetFieldName(), targetValue );

    QgsFeatureRequest request;
    request.setFilterExpression( filter );
    request.setLimit( 1 );

    QgsFeatureIterator it = mLayer->getFeatures( request );
    it.nextFeature( targetedFeature );
  }

  return targetedFeature;
}

QgsVectorLayerJoinBuffer *QgsVectorLayerJoinBuffer::clone() const
{
  QgsVectorLayerJoinBuffer *cloned = new QgsVectorLayerJoinBuffer( mLayer );
  cloned->mVectorJoins = mVectorJoins;
  return cloned;
}

void QgsVectorLayerJoinBuffer::joinedLayerUpdatedFields()
{
  // TODO - check - this whole method is probably not needed anymore,
  // since the cache handling is covered by joinedLayerModified()

  QgsVectorLayer *joinedLayer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( joinedLayer );

  // recache the joined layer
  for ( QgsVectorJoinList::iterator it = mVectorJoins.begin(); it != mVectorJoins.end(); ++it )
  {
    if ( joinedLayer == it->joinLayer() )
    {
      it->cachedAttributes.clear();
      cacheJoinLayer( *it );
    }
  }

  emit joinedFieldsChanged();
}

void QgsVectorLayerJoinBuffer::joinedLayerModified()
{
  QgsVectorLayer *joinedLayer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( joinedLayer );

  // recache the joined layer
  for ( QgsVectorJoinList::iterator it = mVectorJoins.begin(); it != mVectorJoins.end(); ++it )
  {
    if ( joinedLayer == it->joinLayer() )
    {
      it->cacheDirty = true;
    }
  }
}

void QgsVectorLayerJoinBuffer::joinedLayerWillBeDeleted()
{
  QgsVectorLayer *joinedLayer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( joinedLayer );

  removeJoin( joinedLayer->id() );
}

void QgsVectorLayerJoinBuffer::connectJoinedLayer( QgsVectorLayer *vl )
{
  connect( vl, &QgsVectorLayer::updatedFields, this, &QgsVectorLayerJoinBuffer::joinedLayerUpdatedFields, Qt::UniqueConnection );
  connect( vl, &QgsVectorLayer::layerModified, this, &QgsVectorLayerJoinBuffer::joinedLayerModified, Qt::UniqueConnection );
  connect( vl, &QgsVectorLayer::willBeDeleted, this, &QgsVectorLayerJoinBuffer::joinedLayerWillBeDeleted, Qt::UniqueConnection );
}

bool QgsVectorLayerJoinBuffer::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags )
{
  if ( !containsJoins() )
    return false;

  // try to add/update a feature in each joined layer
  const QgsVectorJoinList joins = vectorJoins();
  for ( const QgsVectorLayerJoinInfo &info : joins )
  {
    QgsVectorLayer *joinLayer = info.joinLayer();

    if ( joinLayer && joinLayer->isEditable() && info.isEditable() && info.hasUpsertOnEdit() )
    {
      QgsFeatureList joinFeatures;

      for ( const QgsFeature &feature : std::as_const( features ) )
      {
        const QgsFeature joinFeature = info.extractJoinedFeature( feature );

        // we don't want to add a new feature in joined layer when the id
        // column value yet exist, we just want to update the existing one
        const QVariant idFieldValue = feature.attribute( info.targetFieldName() );
        const QString filter = QgsExpression::createFieldEqualityExpression( info.joinFieldName(), idFieldValue.toString() );

        QgsFeatureRequest request;
        request.setFlags( QgsFeatureRequest::NoGeometry );
        request.setNoAttributes();
        request.setFilterExpression( filter );
        request.setLimit( 1 );

        QgsFeatureIterator it = info.joinLayer()->getFeatures( request );
        QgsFeature existingFeature;
        it.nextFeature( existingFeature );

        if ( existingFeature.isValid() )
        {
          if ( info.hasSubset() )
          {
            const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( info );
            const auto constSubsetNames = subsetNames;
            for ( const QString &field : constSubsetNames )
            {
              QVariant newValue = joinFeature.attribute( field );
              int fieldIndex = joinLayer->fields().indexOf( field );
              joinLayer->changeAttributeValue( existingFeature.id(), fieldIndex, newValue );
            }
          }
          else
          {
            const QgsFields joinFields = joinFeature.fields();
            for ( const auto &field : joinFields )
            {
              QVariant newValue = joinFeature.attribute( field.name() );
              int fieldIndex = joinLayer->fields().indexOf( field.name() );
              joinLayer->changeAttributeValue( existingFeature.id(), fieldIndex, newValue );
            }
          }
        }
        else
        {
          // joined feature is added only if one of its field is not null
          bool notNullFields = false;
          const QgsFields joinFields = joinFeature.fields();
          for ( const auto &field : joinFields )
          {
            if ( field.name() == info.joinFieldName() )
              continue;

            if ( !joinFeature.attribute( field.name() ).isNull() )
            {
              notNullFields = true;
              break;
            }
          }

          if ( notNullFields )
            joinFeatures << joinFeature;
        }
      }

      joinLayer->addFeatures( joinFeatures );
    }
  }

  return true;
}

bool QgsVectorLayerJoinBuffer::changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue )
{
  if ( mLayer->fields().fieldOrigin( field ) != QgsFields::OriginJoin )
    return false;

  int srcFieldIndex;
  const QgsVectorLayerJoinInfo *info = joinForFieldIndex( field, mLayer->fields(), srcFieldIndex );
  if ( info && info->joinLayer() && info->isEditable() )
  {
    QgsFeature feature = mLayer->getFeature( fid );

    if ( !feature.isValid() )
      return false;

    const QgsFeature joinFeature = joinedFeatureOf( info, feature );

    if ( joinFeature.isValid() )
      return info->joinLayer()->changeAttributeValue( joinFeature.id(), srcFieldIndex, newValue, oldValue );
    else
    {
      feature.setAttribute( field, newValue );
      return addFeatures( QgsFeatureList() << feature );
    }
  }
  else
    return false;
}

bool QgsVectorLayerJoinBuffer::changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues )
{
  bool success = true;

  for ( auto it = newValues.constBegin(); it != newValues.constEnd(); ++it )
  {
    const int field = it.key();
    const QVariant newValue = it.value();
    QVariant oldValue;

    if ( oldValues.contains( field ) )
      oldValue = oldValues[field];

    success &= changeAttributeValue( fid, field, newValue, oldValue );
  }

  return success;
}

bool QgsVectorLayerJoinBuffer::deleteFeature( QgsFeatureId fid, QgsVectorLayer::DeleteContext *context ) const
{
  return deleteFeatures( QgsFeatureIds() << fid, context );
}

bool QgsVectorLayerJoinBuffer::deleteFeatures( const QgsFeatureIds &fids, QgsVectorLayer::DeleteContext *context ) const
{
  if ( !containsJoins() )
    return false;

  const auto constFids = fids;
  for ( const QgsFeatureId &fid : constFids )
  {
    const auto constVectorJoins = vectorJoins();
    for ( const QgsVectorLayerJoinInfo &info : constVectorJoins )
    {
      if ( info.isEditable() && info.hasCascadedDelete() )
      {
        const QgsFeature joinFeature = joinedFeatureOf( &info, mLayer->getFeature( fid ) );
        if ( joinFeature.isValid() )
          info.joinLayer()->deleteFeature( joinFeature.id(), context );
      }
    }
  }

  return true;
}

bool QgsVectorLayerJoinBuffer::isAuxiliaryJoin( const QgsVectorLayerJoinInfo &info ) const
{
  const QgsAuxiliaryLayer *al = mLayer->auxiliaryLayer();

  return al && al->id() == info.joinLayerId();
}
