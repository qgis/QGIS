/***************************************************************************
    qgsvectorlayereditbuffer.cpp
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
#include "qgsvectorlayereditbuffer.h"

#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvectorlayereditbuffergroup.h"
#include "qgsvectorlayerundocommand.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsmessagelog.h"


//! populate two lists (ks, vs) from map - in reverse order
template <class Key, class T> void mapToReversedLists( const QMap< Key, T > &map, QList<Key> &ks, QList<T> &vs )
{
  ks.reserve( map.size() );
  vs.reserve( map.size() );
  typename QMap<Key, T>::const_iterator i = map.constEnd();
  while ( i-- != map.constBegin() )
  {
    ks.append( i.key() );
    vs.append( i.value() );
  }
}


QgsVectorLayerEditBuffer::QgsVectorLayerEditBuffer( QgsVectorLayer *layer )
  : L( layer )
{
  connect( L->undoStack(), &QUndoStack::indexChanged, this, &QgsVectorLayerEditBuffer::undoIndexChanged ); // TODO[MD]: queued?
}

bool QgsVectorLayerEditBuffer::isModified() const
{
  return !L->undoStack()->isClean();
}


void QgsVectorLayerEditBuffer::undoIndexChanged( int index )
{
  QgsDebugMsgLevel( QStringLiteral( "undo index changed %1" ).arg( index ), 4 );
  Q_UNUSED( index )
  emit layerModified();
}


void QgsVectorLayerEditBuffer::updateFields( QgsFields &fields )
{
  // delete attributes from the higher indices to lower indices
  for ( int i = mDeletedAttributeIds.count() - 1; i >= 0; --i )
  {
    fields.remove( mDeletedAttributeIds.at( i ) );
  }

  // rename fields
  QgsFieldNameMap::const_iterator renameIt = mRenamedAttributes.constBegin();
  for ( ; renameIt != mRenamedAttributes.constEnd(); ++renameIt )
  {
    fields.rename( renameIt.key(), renameIt.value() );
  }

  // add new fields
  for ( int i = 0; i < mAddedAttributes.count(); ++i )
  {
    fields.append( mAddedAttributes.at( i ), QgsFields::OriginEdit, i );
  }
}

QgsVectorLayerEditBufferGroup *QgsVectorLayerEditBuffer::editBufferGroup() const
{
  return mEditBufferGroup;
}

void QgsVectorLayerEditBuffer::setEditBufferGroup( QgsVectorLayerEditBufferGroup *editBufferGroup )
{
  mEditBufferGroup = editBufferGroup;
}

void QgsVectorLayerEditBuffer::updateFeatureGeometry( QgsFeature &f )
{
  if ( mChangedGeometries.contains( f.id() ) )
    f.setGeometry( mChangedGeometries[f.id()] );
}


void QgsVectorLayerEditBuffer::updateChangedAttributes( QgsFeature &f )
{
  QgsAttributes attrs = f.attributes();

  // remove all attributes that will disappear - from higher indices to lower
  for ( int idx = mDeletedAttributeIds.count() - 1; idx >= 0; --idx )
  {
    attrs.remove( mDeletedAttributeIds[idx] );
  }

  // adjust size to accommodate added attributes
  attrs.resize( attrs.count() + mAddedAttributes.count() );

  // update changed attributes
  if ( mChangedAttributeValues.contains( f.id() ) )
  {
    const QgsAttributeMap &map = mChangedAttributeValues[f.id()];
    for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); ++it )
      attrs[it.key()] = it.value();
  }

  f.setAttributes( attrs );
}




bool QgsVectorLayerEditBuffer::addFeature( QgsFeature &f )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    return false;
  }
  if ( L->mFields.count() != f.attributes().count() )
  {
    QgsMessageLog::logMessage( tr( "cannot add feature, wrong field count: layer: %1 feature: %2:" ).arg( L->mFields.count() ).arg( f.attributes().count() ) );
    return false;
  }

  // TODO: check correct geometry type

  L->undoStack()->push( new QgsVectorLayerUndoCommandAddFeature( this, f ) );
  return true;
}


bool QgsVectorLayerEditBuffer::addFeatures( QgsFeatureList &features )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures ) )
    return false;

  bool result = true;
  for ( QgsFeatureList::iterator iter = features.begin(); iter != features.end(); ++iter )
  {
    result = result && addFeature( *iter );
  }

  L->updateExtents();
  return result;
}



bool QgsVectorLayerEditBuffer::deleteFeature( QgsFeatureId fid )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    QgsDebugMsg( QStringLiteral( "Cannot delete features (missing DeleteFeature capability)" ) );
    return false;
  }

  if ( FID_IS_NEW( fid ) )
  {
    if ( !mAddedFeatures.contains( fid ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot delete features (in the list of added features)" ) );
      return false;
    }
  }
  else // existing feature
  {
    if ( mDeletedFeatureIds.contains( fid ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot delete features (in the list of deleted features)" ) );
      return false;
    }
  }

  L->undoStack()->push( new QgsVectorLayerUndoCommandDeleteFeature( this, fid ) );
  return true;
}

bool QgsVectorLayerEditBuffer::deleteFeatures( const QgsFeatureIds &fids )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
  {
    QgsDebugMsg( QStringLiteral( "Cannot delete features (missing DeleteFeatures capability)" ) );
    return false;
  }

  bool ok = true;
  const auto constFids = fids;
  for ( QgsFeatureId fid : constFids )
    ok = deleteFeature( fid ) && ok;

  return ok;
}


bool QgsVectorLayerEditBuffer::changeGeometry( QgsFeatureId fid, const QgsGeometry &geom )
{
  if ( !L->isSpatial() )
  {
    return false;
  }

  if ( FID_IS_NEW( fid ) )
  {
    if ( !mAddedFeatures.contains( fid ) )
      return false;
  }
  else if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries ) )
    return false;

  // TODO: check compatible geometry

  L->undoStack()->push( new QgsVectorLayerUndoCommandChangeGeometry( this, fid, geom ) );
  return true;
}

bool QgsVectorLayerEditBuffer::changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues )
{
  bool success = true;
  for ( auto it = newValues.constBegin() ; it != newValues.constEnd(); ++it )
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

bool QgsVectorLayerEditBuffer::changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue )
{
  if ( FID_IS_NEW( fid ) )
  {
    if ( !mAddedFeatures.contains( fid ) )
      return false;
  }
  else if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) )
  {
    return false;
  }

  if ( field < 0 || field >= L->fields().count() ||
       L->fields().fieldOrigin( field ) == QgsFields::OriginJoin ||
       L->fields().fieldOrigin( field ) == QgsFields::OriginExpression )
    return false;

  L->undoStack()->push( new QgsVectorLayerUndoCommandChangeAttribute( this, fid, field, newValue, oldValue ) );
  return true;
}


bool QgsVectorLayerEditBuffer::addAttribute( const QgsField &field )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes ) )
    return false;

  if ( field.name().isEmpty() )
    return false;

  const QgsFields fields = L->fields();
  for ( const QgsField &updatedField : fields )
  {
    if ( updatedField.name() == field.name() )
      return false;
  }

  if ( !L->dataProvider()->supportedType( field ) )
    return false;

  L->undoStack()->push( new QgsVectorLayerUndoCommandAddAttribute( this, field ) );
  return true;
}


bool QgsVectorLayerEditBuffer::deleteAttribute( int index )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes ) )
    return false;

  if ( index < 0 || index >= L->fields().count() )
    return false;

  // find out source of the field
  QgsFields::FieldOrigin origin = L->fields().fieldOrigin( index );
  int originIndex = L->fields().fieldOriginIndex( index );

  if ( origin == QgsFields::OriginProvider && mDeletedAttributeIds.contains( originIndex ) )
    return false;

  if ( origin == QgsFields::OriginJoin )
    return false;

  L->undoStack()->push( new QgsVectorLayerUndoCommandDeleteAttribute( this, index ) );
  return true;
}

bool QgsVectorLayerEditBuffer::renameAttribute( int index, const QString &newName )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) )
    return false;

  if ( newName.isEmpty() )
    return false;

  if ( index < 0 || index >= L->fields().count() )
    return false;

  const QgsFields fields = L->fields();
  for ( const QgsField &updatedField : fields )
  {
    if ( updatedField.name() == newName )
      return false;
  }

  L->undoStack()->push( new QgsVectorLayerUndoCommandRenameAttribute( this, index, newName ) );
  return true;
}


bool QgsVectorLayerEditBuffer::commitChanges( QStringList &commitErrors )
{
  commitErrors.clear();

  bool success = true;

  // geometry updates   attribute updates
  // yes                no                    => changeGeometryValues
  // no                 yes                   => changeAttributeValues
  // yes                yes                   => changeFeatures

  // Check geometry types
  // to fix https://github.com/qgis/QGIS/issues/23663
  if ( !mAddedFeatures.isEmpty() )
    success &= commitChangesCheckGeometryTypeCompatibility( commitErrors );

  const QgsFields oldFields = L->fields();

  //
  // delete attributes
  //
  bool attributesChanged = false;
  if ( success && !mDeletedAttributeIds.isEmpty() )
  {
    bool attributesDeleted = false;
    success &= commitChangesDeleteAttributes( attributesDeleted, commitErrors );
    attributesChanged |= attributesDeleted;
  }

  // rename attributes
  if ( success && !mRenamedAttributes.isEmpty() )
  {
    bool attributesRenamed = false;
    success &= commitChangesRenameAttributes( attributesRenamed, commitErrors );
    attributesChanged |= attributesRenamed;
  }

  //
  // add attributes
  //
  if ( success && !mAddedAttributes.isEmpty() )
  {
    bool attributesAdded = false;
    success &= commitChangesAddAttributes( attributesAdded, commitErrors );
    attributesChanged |= attributesAdded;
  }

  //
  // check that addition/removal went as expected
  //
  if ( success && attributesChanged )
    success &= commitChangesCheckAttributesModifications( oldFields, commitErrors );

  //
  // change attributes
  //
  if ( success && ( !mChangedAttributeValues.isEmpty() || !mChangedGeometries.isEmpty() ) )
  {
    bool attributesChanged;
    success &= commitChangesChangeAttributes( attributesChanged, commitErrors );
  }

  //
  // delete features
  //
  if ( success && !mDeletedFeatureIds.isEmpty() )
  {
    bool featuresDeleted;
    success &= commitChangesDeleteFeatures( featuresDeleted, commitErrors );
  }

  //
  //  add features
  //
  if ( success && !mAddedFeatures.isEmpty() )
  {
    bool featuresAdded;
    success &= commitChangesAddFeatures( featuresAdded, commitErrors );
  }

  QgsVectorDataProvider *provider = L->dataProvider();
  if ( !success && provider->hasErrors() )
  {
    commitErrors << tr( "\n  Provider errors:" );
    const auto constErrors = provider->errors();
    for ( QString e : constErrors )
    {
      commitErrors << "    " + e.replace( '\n', QLatin1String( "\n    " ) );
    }
    provider->clearErrors();
  }

  return success;
}


void QgsVectorLayerEditBuffer::rollBack()
{
  if ( !isModified() )
    return;

  // limit canvas redraws to one by jumping to beginning of stack
  // see QgsUndoWidget::indexChanged
  L->undoStack()->setIndex( 0 );

  Q_ASSERT( mAddedAttributes.isEmpty() );
  Q_ASSERT( mDeletedAttributeIds.isEmpty() );
  Q_ASSERT( mChangedAttributeValues.isEmpty() );
  Q_ASSERT( mChangedGeometries.isEmpty() );
  Q_ASSERT( mAddedFeatures.isEmpty() );
}

QgsFeatureIds QgsVectorLayerEditBuffer::allAddedOrEditedFeatures() const
{
  return qgis::listToSet( mAddedFeatures.keys() ).unite( qgis::listToSet( mChangedAttributeValues.keys() ) ).unite( qgis::listToSet( mChangedGeometries.keys() ) );
}

#if 0
QString QgsVectorLayerEditBuffer::dumpEditBuffer()
{
  QString msg;
  if ( !mChangedGeometries.isEmpty() )
  {
    msg += "CHANGED GEOMETRIES:\n";
    for ( QgsGeometryMap::const_iterator it = mChangedGeometries.begin(); it != mChangedGeometries.end(); ++it )
    {
      // QgsFeatureId, QgsGeometry
      msg += QString( "- FID %1: %2" ).arg( it.key() ).arg( it.value().to );
    }
  }
  return msg;
}
#endif

void QgsVectorLayerEditBuffer::handleAttributeAdded( int index )
{
  // go through the changed attributes map and adapt indices
  QgsChangedAttributesMap::iterator it = mChangedAttributeValues.begin();
  for ( ; it != mChangedAttributeValues.end(); ++it )
  {
    updateAttributeMapIndex( it.value(), index, + 1 );
  }

  // go through added features and adapt attributes
  QgsFeatureMap::iterator featureIt = mAddedFeatures.begin();
  for ( ; featureIt != mAddedFeatures.end(); ++featureIt )
  {
    QgsAttributes attrs = featureIt->attributes();
    attrs.insert( index, QVariant() );
    featureIt->setAttributes( attrs );
  }

  // go through renamed attributes and adapt
  QList< int > sortedRenamedIndices = mRenamedAttributes.keys();
  //sort keys
  std::sort( sortedRenamedIndices.begin(), sortedRenamedIndices.end(), std::greater< int >() );
  const auto constSortedRenamedIndices = sortedRenamedIndices;
  for ( int renameIndex : constSortedRenamedIndices )
  {
    if ( renameIndex >= index )
    {
      mRenamedAttributes[ renameIndex + 1 ] = mRenamedAttributes.value( renameIndex );
    }
  }
  //remove last
  mRenamedAttributes.remove( index );
}

void QgsVectorLayerEditBuffer::handleAttributeDeleted( int index )
{
  // go through the changed attributes map and adapt indices
  QgsChangedAttributesMap::iterator it = mChangedAttributeValues.begin();
  for ( ; it != mChangedAttributeValues.end(); ++it )
  {
    QgsAttributeMap &attrMap = it.value();
    // remove the attribute
    if ( attrMap.contains( index ) )
      attrMap.remove( index );

    // update attribute indices
    updateAttributeMapIndex( attrMap, index, -1 );
  }

  // go through added features and adapt attributes
  QgsFeatureMap::iterator featureIt = mAddedFeatures.begin();
  for ( ; featureIt != mAddedFeatures.end(); ++featureIt )
  {
    QgsAttributes attrs = featureIt->attributes();
    attrs.remove( index );
    featureIt->setAttributes( attrs );
  }

  // go through rename attributes and adapt
  QList< int > sortedRenamedIndices = mRenamedAttributes.keys();
  //sort keys
  std::sort( sortedRenamedIndices.begin(), sortedRenamedIndices.end() );
  int last = -1;
  mRenamedAttributes.remove( index );
  const auto constSortedRenamedIndices = sortedRenamedIndices;
  for ( int renameIndex : constSortedRenamedIndices )
  {
    if ( renameIndex > index )
    {
      mRenamedAttributes.insert( renameIndex - 1, mRenamedAttributes.value( renameIndex ) );
      last = renameIndex;
    }
  }
  //remove last
  if ( last > -1 )
    mRenamedAttributes.remove( last );
}



void QgsVectorLayerEditBuffer::updateAttributeMapIndex( QgsAttributeMap &map, int index, int offset ) const
{
  QgsAttributeMap updatedMap;
  for ( QgsAttributeMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
  {
    int attrIndex = it.key();
    updatedMap.insert( attrIndex < index ? attrIndex : attrIndex + offset, it.value() );
  }
  map = updatedMap;
}



void QgsVectorLayerEditBuffer::updateLayerFields()
{
  L->updateFields();
}

bool QgsVectorLayerEditBuffer::commitChangesCheckGeometryTypeCompatibility( QStringList &commitErrors )
{
  if ( mAddedFeatures.isEmpty() )
    return true;

  if ( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures )
  {
    if ( L->dataProvider()->doesStrictFeatureTypeCheck() )
    {
      for ( const QgsFeature &f : std::as_const( mAddedFeatures ) )
      {
        if ( ( ! f.hasGeometry() ) ||
             ( f.geometry().wkbType() == L->dataProvider()->wkbType() ) )
          continue;

        if ( L->dataProvider()->convertToProviderType( f.geometry() ).isNull() )
        {
          commitErrors << tr( "ERROR: %n feature(s) not added - geometry type is not compatible with the current layer.", "not added features count", mAddedFeatures.size() );
          return false;
        }
      }
    }
  }
  else
  {
    commitErrors << tr( "ERROR: %n feature(s) not added - provider doesn't support adding features.", "not added features count", mAddedFeatures.size() );
    return false;
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesDeleteAttributes( bool &attributesDeleted, QStringList &commitErrors )
{
  attributesDeleted = false;

  if ( mDeletedAttributeIds.isEmpty() )
    return true;

  if ( ( L->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes ) && L->dataProvider()->deleteAttributes( qgis::listToSet( mDeletedAttributeIds ) ) )
  {
    commitErrors << tr( "SUCCESS: %n attribute(s) deleted.", "deleted attributes count", mDeletedAttributeIds.size() );

    emit committedAttributesDeleted( L->id(), mDeletedAttributeIds );

    mDeletedAttributeIds.clear();
    attributesDeleted = true;
  }
  else
  {
    commitErrors << tr( "ERROR: %n attribute(s) not deleted.", "not deleted attributes count", mDeletedAttributeIds.size() );
#if 0
    QString list = "ERROR: Pending attribute deletes:";
    const auto constMDeletedAttributeIds = mDeletedAttributeIds;
    for ( int idx : constMDeletedAttributeIds )
    {
      list.append( ' ' + L->fields().at( idx ).name() );
    }
    commitErrors << list;
#endif
    return false;
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesRenameAttributes( bool &attributesRenamed, QStringList &commitErrors )
{
  attributesRenamed = false;

  if ( mRenamedAttributes.isEmpty() )
    return true;

  if ( ( L->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) && L->dataProvider()->renameAttributes( mRenamedAttributes ) )
  {
    commitErrors << tr( "SUCCESS: %n attribute(s) renamed.", "renamed attributes count", mRenamedAttributes.size() );

    emit committedAttributesRenamed( L->id(), mRenamedAttributes );

    mRenamedAttributes.clear();
    attributesRenamed = true;
  }
  else
  {
    commitErrors << tr( "ERROR: %n attribute(s) not renamed", "not renamed attributes count", mRenamedAttributes.size() );
    return false;
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesAddAttributes( bool &attributesAdded, QStringList &commitErrors )
{
  attributesAdded = false;

  if ( mAddedAttributes.isEmpty() )
    return true;

  if ( ( L->dataProvider()->capabilities()  & QgsVectorDataProvider::AddAttributes ) && L->dataProvider()->addAttributes( mAddedAttributes ) )
  {
    commitErrors << tr( "SUCCESS: %n attribute(s) added.", "added attributes count", mAddedAttributes.size() );
    emit committedAttributesAdded( L->id(), mAddedAttributes );
    mAddedAttributes.clear();
    attributesAdded = true;
  }
  else
  {
    commitErrors << tr( "ERROR: %n new attribute(s) not added", "not added attributes count", mAddedAttributes.size() );
#if 0
    QString list = "ERROR: Pending adds:";
    const auto constMAddedAttributes = mAddedAttributes;
    for ( QgsField f : constMAddedAttributes )
    {
      list.append( ' ' + f.name() );
    }
    commitErrors << list;
#endif
    return false;
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesCheckAttributesModifications( const QgsFields oldFields, QStringList &commitErrors )
{
  L->updateFields();
  QgsFields newFields = L->fields();

  if ( oldFields.count() != newFields.count() )
  {
    commitErrors << tr( "ERROR: the count of fields is incorrect after addition/removal of fields!" );
    return false;   // don't try attribute updates - they'll fail.
  }

  for ( int i = 0; i < std::min( oldFields.count(), newFields.count() ); ++i )
  {
    QgsField oldField = oldFields.at( i );
    QgsField newField = newFields.at( i );
    if ( oldField != newField )
    {
      commitErrors
          << tr( "ERROR: field with index %1 is not the same!" ).arg( i )
          << tr( "Provider: %1" ).arg( L->providerType() )
          << tr( "Storage: %1" ).arg( L->storageType() )
          << QStringLiteral( "%1: name=%2 type=%3 typeName=%4 len=%5 precision=%6" )
          .arg( tr( "expected field" ),
                oldField.name(),
                QVariant::typeToName( oldField.type() ),
                oldField.typeName() )
          .arg( oldField.length() )
          .arg( oldField.precision() )
          << QStringLiteral( "%1: name=%2 type=%3 typeName=%4 len=%5 precision=%6" )
          .arg( tr( "retrieved field" ),
                newField.name(),
                QVariant::typeToName( newField.type() ),
                newField.typeName() )
          .arg( newField.length() )
          .arg( newField.precision() );
      return false;   // don't try attribute updates - they'll fail.
    }
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesChangeAttributes( bool &attributesChanged, QStringList &commitErrors )
{
  attributesChanged = false;

  if ( L->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeFeatures && !mChangedGeometries.isEmpty() && !mChangedAttributeValues.isEmpty() )
  {
    Q_ASSERT( ( L->dataProvider()->capabilities() & ( QgsVectorDataProvider::ChangeAttributeValues | QgsVectorDataProvider::ChangeGeometries ) ) == ( QgsVectorDataProvider::ChangeAttributeValues | QgsVectorDataProvider::ChangeGeometries ) );

    if ( L->dataProvider()->changeFeatures( mChangedAttributeValues, mChangedGeometries ) )
    {
      commitErrors << tr( "SUCCESS: %1 attribute value(s) and %2 geometries changed." ).arg( mChangedAttributeValues.size(), mChangedGeometries.size() );
      attributesChanged = true;
      emit committedAttributeValuesChanges( L->id(), mChangedAttributeValues );
      mChangedAttributeValues.clear();

      emit committedGeometriesChanges( L->id(), mChangedGeometries );
      mChangedGeometries.clear();
    }
    else
    {
      commitErrors << tr( "ERROR: %1 attributes and %2 geometries not changed.", "not changed attributes and geometries count" ).arg( mChangedAttributeValues.size() ).arg( mChangedGeometries.size() );
      return false;
    }
  }

  if ( !mChangedGeometries.isEmpty() )
  {
    if ( ! L->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::ChangeFeatures )
         && ! L->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::ChangeGeometries ) )
    {
      commitErrors << tr( "ERROR: %1 geometries not changed. Data provider '%2' does not have ChangeFeatures or ChangeGeometries capabilities", "not changed geometries count" )
                   .arg( mChangedGeometries.size() )
                   .arg( L->dataProvider()->name() );
      return false;
    }

    if ( L->dataProvider()->changeGeometryValues( mChangedGeometries ) )
    {
      commitErrors << tr( "SUCCESS: %n geometries were changed.", "changed geometries count", mChangedGeometries.size() );
      attributesChanged = true;
      emit committedGeometriesChanges( L->id(), mChangedGeometries );
      mChangedGeometries.clear();
    }
    else
    {
      commitErrors << tr( "ERROR: %n geometries not changed.", "not changed geometries count", mChangedGeometries.size() );
      return false;
    }
  }

  if ( !mChangedAttributeValues.isEmpty() )
  {
    if ( ! L->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::ChangeFeatures )
         && ! L->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::ChangeAttributeValues ) )
    {
      commitErrors << tr( "ERROR: %1 attribute value change(s) not applied. Data provider '%2' does not have ChangeFeatures or ChangeAttributeValues capabilities", "not changed attribute values count" )
                   .arg( mChangedAttributeValues.size() )
                   .arg( L->dataProvider()->name() );
      return false;
    }

    if ( L->dataProvider()->changeAttributeValues( mChangedAttributeValues ) )
    {
      commitErrors << tr( "SUCCESS: %n attribute value(s) changed.", "changed attribute values count", mChangedAttributeValues.size() );
      attributesChanged = true;
      emit committedAttributeValuesChanges( L->id(), mChangedAttributeValues );
      mChangedAttributeValues.clear();
    }
    else
    {
      commitErrors << tr( "ERROR: %n attribute value change(s) not applied.", "not changed attribute values count", mChangedAttributeValues.size() );
#if 0
      QString list = "ERROR: pending changes:";
      const auto constKeys = mChangedAttributeValues.keys();
      for ( QgsFeatureId id : constKeys )
      {
        list.append( "\n  " + FID_TO_STRING( id ) + '[' );
        const auto constKeys = mChangedAttributeValues[ id ].keys();
        for ( int idx : constKeys )
        {
          list.append( QString( " %1:%2" ).arg( L->fields().at( idx ).name() ).arg( mChangedAttributeValues[id][idx].toString() ) );
        }
        list.append( " ]" );
      }
      commitErrors << list;
#endif
      return false;
    }
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesDeleteFeatures( bool &featuresDeleted, QStringList &commitErrors )
{
  featuresDeleted = false;

  if ( mDeletedFeatureIds.isEmpty() )
    return true;

  if ( ( L->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) && L->dataProvider()->deleteFeatures( mDeletedFeatureIds ) )
  {
    commitErrors << tr( "SUCCESS: %n feature(s) deleted.", "deleted features count", mDeletedFeatureIds.size() );
    featuresDeleted = true;
    // TODO[MD]: we should not need this here
    for ( QgsFeatureId id : std::as_const( mDeletedFeatureIds ) )
    {
      mChangedAttributeValues.remove( id );
      mChangedGeometries.remove( id );
    }

    emit committedFeaturesRemoved( L->id(), mDeletedFeatureIds );

    mDeletedFeatureIds.clear();
  }
  else
  {
    commitErrors << tr( "ERROR: %n feature(s) not deleted.", "not deleted features count", mDeletedFeatureIds.size() );
#if 0
    QString list = "ERROR: pending deletes:";
    const auto constMDeletedFeatureIds = mDeletedFeatureIds;
    for ( QgsFeatureId id : constMDeletedFeatureIds )
    {
      list.append( ' ' + FID_TO_STRING( id ) );
    }
    commitErrors << list;
#endif
    return false;
  }

  return true;
}

bool QgsVectorLayerEditBuffer::commitChangesAddFeatures( bool &featuresAdded, QStringList &commitErrors )
{
  featuresAdded = false;

  if ( mAddedFeatures.isEmpty() )
    return true;

  if ( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures )
  {
    QList<QgsFeatureId> ids;
    QgsFeatureList featuresToAdd;
    // get the list of added features in reversed order
    // this will preserve the order how they have been added e.g. (-1, -2, -3) while in the map they are ordered (-3, -2, -1)
    mapToReversedLists( mAddedFeatures, ids, featuresToAdd );

    // we need to strip any extra attributes here -- e.g. virtual fields, which should
    // not be sent to the data provider. Refs #18784
    for ( int i = 0; i < featuresToAdd.count(); ++i )
    {
      QgsVectorLayerUtils::matchAttributesToFields( featuresToAdd[i], L->dataProvider()->fields() );
    }

    if ( L->dataProvider()->addFeatures( featuresToAdd, QgsFeatureSink::Flag::RollBackOnErrors ) )
    {
      commitErrors << tr( "SUCCESS: %n feature(s) added.", "added features count", featuresToAdd.size() );
      featuresAdded = true;
      emit committedFeaturesAdded( L->id(), featuresToAdd );

      // notify everyone that the features with temporary ids were updated with permanent ids
      for ( int i = 0; i < featuresToAdd.count(); ++i )
      {
        if ( featuresToAdd[i].id() != ids[i] )
        {
          //update selection
          if ( L->mSelectedFeatureIds.contains( ids[i] ) )
          {
            L->mSelectedFeatureIds.remove( ids[i] );
            L->mSelectedFeatureIds.insert( featuresToAdd[i].id() );
          }
          emit featureDeleted( ids[i] );
          emit featureAdded( featuresToAdd[i].id() );
        }
      }

      mAddedFeatures.clear();
    }
    else
    {
      commitErrors << tr( "ERROR: %n feature(s) not added.", "not added features count", mAddedFeatures.size() );
#if 0
      QString list = "ERROR: pending adds:";
      const auto constMAddedFeatures = mAddedFeatures;
      for ( QgsFeature f : constMAddedFeatures )
      {
        list.append( ' ' + FID_TO_STRING( f.id() ) + '[' );
        for ( int i = 0; i < L->fields().size(); i++ )
        {
          list.append( QString( " %1:%2" ).arg( L->fields().at( i ).name() ).arg( f.attributes()[i].toString() ) );
        }
        list.append( " ]" );
      }
      commitErrors << list;
#endif
      return false;
    }
  }
  else
  {
    commitErrors << tr( "ERROR: %n feature(s) not added - provider doesn't support adding features.", "not added features count", mAddedFeatures.size() );
    return false;
  }

  return true;
}
