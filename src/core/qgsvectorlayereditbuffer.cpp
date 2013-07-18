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
#include "qgsvectorlayerundocommand.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"


QgsVectorLayerEditBuffer::QgsVectorLayerEditBuffer( QgsVectorLayer* layer )
    : L( layer )
{
  connect( L->undoStack(), SIGNAL( indexChanged( int ) ), this, SLOT( undoIndexChanged( int ) ) ); // TODO[MD]: queued?
}

QgsVectorLayerEditBuffer::~QgsVectorLayerEditBuffer()
{
}


bool QgsVectorLayerEditBuffer::isModified() const
{
  return !L->undoStack()->isClean();
}


void QgsVectorLayerEditBuffer::undoIndexChanged( int index )
{
  qDebug( "undo index changed %d", index );
  Q_UNUSED( index );
  emit layerModified();
}


void QgsVectorLayerEditBuffer::updateFields( QgsFields& fields )
{
  // delete attributes from the higher indices to lower indices
  for ( int i = mDeletedAttributeIds.count() - 1; i >= 0; --i )
  {
    fields.remove( mDeletedAttributeIds[i] );
  }
  // add new fields
  for ( int i = 0; i < mAddedAttributes.count(); ++i )
  {
    fields.append( mAddedAttributes[i], QgsFields::OriginEdit, i );
  }
}


void QgsVectorLayerEditBuffer::updateFeatureGeometry( QgsFeature &f )
{
  if ( mChangedGeometries.contains( f.id() ) )
    f.setGeometry( mChangedGeometries[f.id()] );
}


void QgsVectorLayerEditBuffer::updateChangedAttributes( QgsFeature &f )
{
  QgsAttributes& attrs = f.attributes();

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
    for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); it++ )
      attrs[it.key()] = it.value();
  }
}




bool QgsVectorLayerEditBuffer::addFeature( QgsFeature& f )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures ) )
  {
    return false;
  }
  if ( L->mUpdatedFields.count() != f.attributes().count() )
    return false;

  // TODO: check correct geometry type

  L->undoStack()->push( new QgsVectorLayerUndoCommandAddFeature( this, f ) );
  return true;
}


bool QgsVectorLayerEditBuffer::addFeatures( QgsFeatureList& features )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures ) )
    return false;

  for ( QgsFeatureList::iterator iter = features.begin(); iter != features.end(); ++iter )
  {
    addFeature( *iter );
  }

  L->updateExtents();
  return true;
}



bool QgsVectorLayerEditBuffer::deleteFeature( QgsFeatureId fid )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
    return false;

  if ( FID_IS_NEW( fid ) )
  {
    if ( !mAddedFeatures.contains( fid ) )
      return false;
  }
  else // existing feature
  {
    if ( mDeletedFeatureIds.contains( fid ) )
      return false;
  }

  L->undoStack()->push( new QgsVectorLayerUndoCommandDeleteFeature( this, fid ) );
  return true;
}


bool QgsVectorLayerEditBuffer::changeGeometry( QgsFeatureId fid, QgsGeometry* geom )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries ) )
    return false;

  if ( !L->hasGeometryType() )
  {
    return false;
  }

  if ( FID_IS_NEW( fid ) )
  {
    if ( !mAddedFeatures.contains( fid ) )
      return false;
  }

  // TODO: check compatible geometry

  L->undoStack()->push( new QgsVectorLayerUndoCommandChangeGeometry( this, fid, geom ) );
  return true;
}


bool QgsVectorLayerEditBuffer::changeAttributeValue( QgsFeatureId fid, int field, QVariant value )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) )
    return false;

  if ( FID_IS_NEW( fid ) )
  {
    if ( !mAddedFeatures.contains( fid ) )
      return false;
  }

  if ( field < 0 || field >= L->pendingFields().count() ||
       L->pendingFields().fieldOrigin( field ) == QgsFields::OriginJoin )
    return false;

  L->undoStack()->push( new QgsVectorLayerUndoCommandChangeAttribute( this, fid, field, value ) );
  return true;
}


bool QgsVectorLayerEditBuffer::addAttribute( const QgsField &field )
{
  if ( !( L->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes ) )
    return false;

  if ( field.name().isEmpty() )
    return false;

  const QgsFields& updatedFields = L->pendingFields();
  for ( int idx = 0; idx < updatedFields.count(); ++idx )
  {
    if ( updatedFields[idx].name() == field.name() )
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

  if ( index < 0 || index >= L->pendingFields().count() )
    return false;

  // find out source of the field
  QgsFields::FieldOrigin origin = L->pendingFields().fieldOrigin( index );
  int originIndex = L->pendingFields().fieldOriginIndex( index );

  if ( origin == QgsFields::OriginProvider && mDeletedAttributeIds.contains( originIndex ) )
    return false;

  if ( origin == QgsFields::OriginJoin )
    return false;

  L->undoStack()->push( new QgsVectorLayerUndoCommandDeleteAttribute( this, index ) );
  return true;
}


bool QgsVectorLayerEditBuffer::commitChanges( QStringList& commitErrors )
{
  QgsVectorDataProvider* provider = L->dataProvider();
  commitErrors.clear();

  int cap = provider->capabilities();
  bool success = true;

  QgsFields oldFields = L->pendingFields();

  bool hadPendingDeletes = !mDeletedFeatureIds.isEmpty();

  //
  // delete attributes
  //
  bool attributesChanged = false;
  if ( !mDeletedAttributeIds.isEmpty() )
  {
    if (( cap & QgsVectorDataProvider::DeleteAttributes ) && provider->deleteAttributes( mDeletedAttributeIds.toSet() ) )
    {
      commitErrors << tr( "SUCCESS: %n attribute(s) deleted.", "deleted attributes count", mDeletedAttributeIds.size() );

      emit committedAttributesDeleted( L->id(), mDeletedAttributeIds );

      mDeletedAttributeIds.clear();
      attributesChanged = true;
    }
    else
    {
      commitErrors << tr( "ERROR: %n attribute(s) not deleted.", "not deleted attributes count", mDeletedAttributeIds.size() );
#if 0
      QString list = "ERROR: Pending attribute deletes:";
      foreach( int idx, mDeletedAttributeIds )
      {
        list.append( " " + L->pendingFields()[idx].name() );
      }
      commitErrors << list;
#endif
      success = false;
    }
  }

  //
  // add attributes
  //
  if ( !mAddedAttributes.isEmpty() )
  {
    if (( cap & QgsVectorDataProvider::AddAttributes ) && provider->addAttributes( mAddedAttributes ) )
    {
      commitErrors << tr( "SUCCESS: %n attribute(s) added.", "added attributes count", mAddedAttributes.size() );

      emit committedAttributesAdded( L->id(), mAddedAttributes );

      mAddedAttributes.clear();
      attributesChanged = true;
    }
    else
    {
      commitErrors << tr( "ERROR: %n new attribute(s) not added", "not added attributes count", mAddedAttributes.size() );
#if 0
      QString list = "ERROR: Pending adds:";
      foreach( QgsField f, mAddedAttributes )
      {
        list.append( " " + f.name() );
      }
      commitErrors << list;
#endif
      success = false;
    }
  }

  //
  // check that addition/removal went as expected
  //
  bool attributeChangesOk = true;
  if ( attributesChanged )
  {
    L->updateFields();
    QgsFields newFields = L->pendingFields();

    if ( oldFields.count() != newFields.count() )
    {
      commitErrors << tr( "ERROR: the count of fields is incorrect after addition/removal of fields!" );
      attributeChangesOk = false;   // don't try attribute updates - they'll fail.
    }

    for ( int i = 0; i < oldFields.count(); ++i )
    {
      const QgsField& oldField = oldFields[i];
      const QgsField& newField = newFields[i];
      if ( attributeChangesOk && oldField != newField )
      {
        commitErrors << tr( "ERROR: field with index %1 is not the same!" ).arg( i );
        attributeChangesOk = false;   // don't try attribute updates - they'll fail.
      }
    }
  }

  if ( attributeChangesOk )
  {
    //
    // change attributes
    //
    if ( !mChangedAttributeValues.isEmpty() )
    {
      if (( cap & QgsVectorDataProvider::ChangeAttributeValues ) && provider->changeAttributeValues( mChangedAttributeValues ) )
      {
        commitErrors << tr( "SUCCESS: %n attribute value(s) changed.", "changed attribute values count", mChangedAttributeValues.size() );

        emit committedAttributeValuesChanges( L->id(), mChangedAttributeValues );

        mChangedAttributeValues.clear();
      }
      else
      {
        commitErrors << tr( "ERROR: %n attribute value change(s) not applied.", "not changed attribute values count", mChangedAttributeValues.size() );
#if 0
        QString list = "ERROR: pending changes:";
        foreach( QgsFeatureId id, mChangedAttributeValues.keys() )
        {
	  list.append( "\n  " + FID_TO_STRING( id ) + "[" );
	  foreach( int idx, mChangedAttributeValues[ id ].keys() )
          {
            list.append( QString( " %1:%2" ).arg( L->pendingFields()[idx].name() ).arg( mChangedAttributeValues[id][idx].toString() ) );
          }
	  list.append( " ]" );
        }
        commitErrors << list;
#endif
        success = false;
      }
    }

    //
    // delete features
    //
    if ( success && !mDeletedFeatureIds.isEmpty() )
    {
      if (( cap & QgsVectorDataProvider::DeleteFeatures ) && provider->deleteFeatures( mDeletedFeatureIds ) )
      {
        commitErrors << tr( "SUCCESS: %n feature(s) deleted.", "deleted features count", mDeletedFeatureIds.size() );
        // TODO[MD]: we should not need this here
        for ( QgsFeatureIds::const_iterator it = mDeletedFeatureIds.begin(); it != mDeletedFeatureIds.end(); it++ )
        {
          mChangedAttributeValues.remove( *it );
          mChangedGeometries.remove( *it );
        }

        emit committedFeaturesRemoved( L->id(), mDeletedFeatureIds );

        mDeletedFeatureIds.clear();
      }
      else
      {
        commitErrors << tr( "ERROR: %n feature(s) not deleted.", "not deleted features count", mDeletedFeatureIds.size() );
#if 0
        QString list = "ERROR: pending deletes:";
        foreach( QgsFeatureId id, mDeletedFeatureIds )
        {
	  list.append( " " + FID_TO_STRING( id ) );
        }
        commitErrors << list;
#endif
        success = false;
      }
    }

    //
    //  add features
    //
    if ( success && !mAddedFeatures.isEmpty() )
    {
      if ( cap & QgsVectorDataProvider::AddFeatures )
      {
        QList<QgsFeatureId> ids = mAddedFeatures.keys();
        QgsFeatureList featuresToAdd = mAddedFeatures.values();

        if ( provider->addFeatures( featuresToAdd ) )
        {
          commitErrors << tr( "SUCCESS: %n feature(s) added.", "added features count", featuresToAdd.size() );

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
	  foreach( QgsFeature f, mAddedFeatures )
	  {
	    list.append( " " + FID_TO_STRING( f.id() ) + "[" );
	    for( int i = 0; i < L->pendingFields().size(); i++ )
	    {
	      list.append( QString( " %1:%2" ).arg( L->pendingFields()[i].name() ).arg( f.attributes()[i].toString() ) );
	    }
	    list.append( " ]" );
	  }
	  commitErrors << list;
#endif
          success = false;
        }
      }
      else
      {
        commitErrors << tr( "ERROR: %n feature(s) not added - provider doesn't support adding features.", "not added features count", mAddedFeatures.size() );
        success = false;
      }
    }
  }
  else
  {
    success = false;
  }

  //
  // update geometries
  //
  if ( success && !mChangedGeometries.isEmpty() )
  {
    if (( cap & QgsVectorDataProvider::ChangeGeometries ) && provider->changeGeometryValues( mChangedGeometries ) )
    {
      commitErrors << tr( "SUCCESS: %n geometries were changed.", "changed geometries count", mChangedGeometries.size() );

      emit committedGeometriesChanges( L->id(), mChangedGeometries );

      mChangedGeometries.clear();
    }
    else
    {
      commitErrors << tr( "ERROR: %n geometries not changed.", "not changed geometries count", mChangedGeometries.size() );
      success = false;
    }
  }

  // for shapes run a REPACK after each transaction
  // TODO: enhance provider interface to allow moving this there
  if ( success && hadPendingDeletes && L->providerType() == "ogr" && L->storageType() == "ESRI Shapefile" )
  {
    provider->createSpatialIndex();
  }

  if ( !success && provider->hasErrors() )
  {
    commitErrors << tr( "\n  Provider errors:" );
    foreach ( QString e, provider->errors() )
    {
      commitErrors << "    " + e.replace( "\n", "\n    " );
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
  for ( int i = 0; i < mChangedAttributeValues.size(); ++i )
  {
    updateAttributeMapIndex( mChangedAttributeValues[i], index, + 1 );
  }

  // go through added features and adapt attributes
  QgsFeatureMap::iterator featureIt = mAddedFeatures.begin();
  for ( ; featureIt != mAddedFeatures.end(); ++featureIt )
  {
    QgsAttributes& attrs = featureIt->attributes();
    attrs.insert( index, QVariant() );
  }
}

void QgsVectorLayerEditBuffer::handleAttributeDeleted( int index )
{
  // go through the changed attributes map and adapt indices
  for ( int i = 0; i < mChangedAttributeValues.size(); ++i )
  {
    QgsAttributeMap& attrMap = mChangedAttributeValues[i];
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
    QgsAttributes& attrs = featureIt->attributes();
    attrs.remove( index );
  }
}



void QgsVectorLayerEditBuffer::updateAttributeMapIndex( QgsAttributeMap& map, int index, int offset ) const
{
  QgsAttributeMap updatedMap;
  for ( QgsAttributeMap::const_iterator it = map.begin(); it != map.end(); ++it )
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
