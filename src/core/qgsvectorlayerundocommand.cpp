/***************************************************************************
    qgsvectorlayerundocommand.cpp
    ---------------------
    begin                : June 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerundocommand.h"

#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrycache.h"
#include "qgsvectorlayereditbuffer.h"

#include "qgslogger.h"


QgsVectorLayerUndoCommandAddFeature::QgsVectorLayerUndoCommandAddFeature( QgsVectorLayerEditBuffer* buffer, QgsFeature& f )
    : QgsVectorLayerUndoCommand( buffer )
{
  static int addedIdLowWaterMark = -1;

  //assign a temporary id to the feature (use negative numbers)
  addedIdLowWaterMark--;

  QgsDebugMsg( "Assigned feature id " + QString::number( addedIdLowWaterMark ) );

  // Force a feature ID (to keep other functions in QGIS happy,
  // providers will use their own new feature ID when we commit the new feature)
  // and add to the known added features.
  f.setFeatureId( addedIdLowWaterMark );

  mFeature = f;
}

void QgsVectorLayerUndoCommandAddFeature::undo()
{
#ifndef QT_NO_DEBUG
  QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.constFind( mFeature.id() );
  Q_ASSERT( it != mBuffer->mAddedFeatures.constEnd() );
#endif
  mBuffer->mAddedFeatures.remove( mFeature.id() );

  if ( mFeature.constGeometry() )
    cache()->removeGeometry( mFeature.id() );

  emit mBuffer->featureDeleted( mFeature.id() );
}

void QgsVectorLayerUndoCommandAddFeature::redo()
{
  mBuffer->mAddedFeatures.insert( mFeature.id(), mFeature );

  if ( mFeature.constGeometry() )
    cache()->cacheGeometry( mFeature.id(), *mFeature.constGeometry() );

  emit mBuffer->featureAdded( mFeature.id() );
}



QgsVectorLayerUndoCommandDeleteFeature::QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid )
    : QgsVectorLayerUndoCommand( buffer )
{
  mFid = fid;

  if ( FID_IS_NEW( mFid ) )
  {
    QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.constFind( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.constEnd() );
    mOldAddedFeature = it.value();
  }
}

void QgsVectorLayerUndoCommandDeleteFeature::undo()
{
  if ( FID_IS_NEW( mFid ) )
  {
    mBuffer->mAddedFeatures.insert( mOldAddedFeature.id(), mOldAddedFeature );
  }
  else
  {
    mBuffer->mDeletedFeatureIds.remove( mFid );
  }

  emit mBuffer->featureAdded( mFid );
}

void QgsVectorLayerUndoCommandDeleteFeature::redo()
{
  if ( FID_IS_NEW( mFid ) )
  {
    mBuffer->mAddedFeatures.remove( mFid );
  }
  else
  {
    mBuffer->mDeletedFeatureIds.insert( mFid );
  }

  emit mBuffer->featureDeleted( mFid );
}



QgsVectorLayerUndoCommandChangeGeometry::QgsVectorLayerUndoCommandChangeGeometry( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid, QgsGeometry* newGeom )
    : QgsVectorLayerUndoCommand( buffer )
    , mFid( fid )
{
  if ( FID_IS_NEW( mFid ) )
  {
    QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.constFind( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.constEnd() );
    mOldGeom = ( it.value().constGeometry() ? new QgsGeometry( *it.value().constGeometry() ) : nullptr );
  }
  else
  {
    bool changedAlready = mBuffer->mChangedGeometries.contains( mFid );
    QgsGeometry geom;
    bool cachedGeom = cache()->geometry( mFid, geom );
    mOldGeom = ( changedAlready && cachedGeom ) ? new QgsGeometry( geom ) : nullptr;
  }

  mNewGeom = new QgsGeometry( *newGeom );
}

int QgsVectorLayerUndoCommandChangeGeometry::id() const
{
  return 1;
}

bool QgsVectorLayerUndoCommandChangeGeometry::mergeWith( const QUndoCommand *other )
{
  if ( other->id() != id() )
    return false;

  const QgsVectorLayerUndoCommandChangeGeometry *merge = dynamic_cast<const QgsVectorLayerUndoCommandChangeGeometry *>( other );
  if ( !merge )
    return false;

  if ( merge->mFid != mFid )
    return false;

  delete mNewGeom;
  mNewGeom = merge->mNewGeom;
  merge->mNewGeom = nullptr;

  return true;
}

QgsVectorLayerUndoCommandChangeGeometry::~QgsVectorLayerUndoCommandChangeGeometry()
{
  delete mOldGeom;
  delete mNewGeom;
}

void QgsVectorLayerUndoCommandChangeGeometry::undo()
{
  if ( FID_IS_NEW( mFid ) )
  {
    // modify added features
    QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
    it.value().setGeometry( *mOldGeom );

    cache()->cacheGeometry( mFid, *mOldGeom );
    emit mBuffer->geometryChanged( mFid, *mOldGeom );
  }
  else
  {
    // existing feature

    if ( !mOldGeom )
    {
      mBuffer->mChangedGeometries.remove( mFid );

      QgsFeature f;
      if ( layer()->getFeatures( QgsFeatureRequest().setFilterFid( mFid ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) && f.constGeometry() )
      {
        cache()->cacheGeometry( mFid, *f.constGeometry() );
        emit mBuffer->geometryChanged( mFid, *f.geometry() );
      }
    }
    else
    {
      mBuffer->mChangedGeometries[mFid] = *mOldGeom;
      cache()->cacheGeometry( mFid, *mOldGeom );
      emit mBuffer->geometryChanged( mFid, *mOldGeom );
    }
  }

}

void QgsVectorLayerUndoCommandChangeGeometry::redo()
{
  if ( FID_IS_NEW( mFid ) )
  {
    // modify added features
    QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
    it.value().setGeometry( *mNewGeom );
  }
  else
  {
    mBuffer->mChangedGeometries[ mFid ] = *mNewGeom;
  }
  cache()->cacheGeometry( mFid, *mNewGeom );
  emit mBuffer->geometryChanged( mFid, *mNewGeom );
}


QgsVectorLayerUndoCommandChangeAttribute::QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid, int fieldIndex, const QVariant &newValue, const QVariant &oldValue )
    : QgsVectorLayerUndoCommand( buffer )
    , mFid( fid )
    , mFieldIndex( fieldIndex )
    , mOldValue( oldValue )
    , mNewValue( newValue )
    , mFirstChange( true )
{
  if ( FID_IS_NEW( mFid ) )
  {
    // work with added feature
    QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.constFind( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.constEnd() );
    if ( it.value().attribute( mFieldIndex ).isValid() )
    {
      mOldValue = it.value().attribute( mFieldIndex );
      mFirstChange = false;
    }
  }
  else if ( mBuffer->mChangedAttributeValues.contains( mFid ) && mBuffer->mChangedAttributeValues[mFid].contains( mFieldIndex ) )
  {
    mOldValue = mBuffer->mChangedAttributeValues[mFid][mFieldIndex];
    mFirstChange = false;
  }

}

void QgsVectorLayerUndoCommandChangeAttribute::undo()
{
  QVariant original = mOldValue;

  if ( FID_IS_NEW( mFid ) )
  {
    // added feature
    QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
    it.value().setAttribute( mFieldIndex, mOldValue );
  }
  else if ( mFirstChange )
  {
    // existing feature
    mBuffer->mChangedAttributeValues[mFid].remove( mFieldIndex );
    if ( mBuffer->mChangedAttributeValues[mFid].isEmpty() )
      mBuffer->mChangedAttributeValues.remove( mFid );

    if ( !mOldValue.isValid() )
    {
      // get old value from provider
      QgsFeature tmp;
      QgsFeatureRequest request;
      request.setFilterFid( mFid );
      request.setFlags( QgsFeatureRequest::NoGeometry );
      request.setSubsetOfAttributes( QgsAttributeList() << mFieldIndex );
      QgsFeatureIterator fi = layer()->getFeatures( request );
      if ( fi.nextFeature( tmp ) )
        original = tmp.attribute( mFieldIndex );
    }
  }
  else
  {
    mBuffer->mChangedAttributeValues[mFid][mFieldIndex] = mOldValue;
  }

  emit mBuffer->attributeValueChanged( mFid, mFieldIndex, original );
}

void QgsVectorLayerUndoCommandChangeAttribute::redo()
{
  if ( FID_IS_NEW( mFid ) )
  {
    // updated added feature
    QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
    it.value().setAttribute( mFieldIndex, mNewValue );
  }
  else
  {
    // changed attribute of existing feature
    if ( !mBuffer->mChangedAttributeValues.contains( mFid ) )
    {
      mBuffer->mChangedAttributeValues.insert( mFid, QgsAttributeMap() );
    }

    mBuffer->mChangedAttributeValues[mFid].insert( mFieldIndex, mNewValue );
  }

  emit mBuffer->attributeValueChanged( mFid, mFieldIndex, mNewValue );
}


QgsVectorLayerUndoCommandAddAttribute::QgsVectorLayerUndoCommandAddAttribute( QgsVectorLayerEditBuffer* buffer, const QgsField& field )
    : QgsVectorLayerUndoCommand( buffer )
    , mField( field )
{
  const QgsFields &fields = layer()->fields();
  int i;
  for ( i = 0; i < fields.count() && fields.fieldOrigin( i ) != QgsFields::OriginJoin; i++ )
    ;
  mFieldIndex = i;
}

void QgsVectorLayerUndoCommandAddAttribute::undo()
{
  int index = layer()->fields().fieldOriginIndex( mFieldIndex );

  mBuffer->mAddedAttributes.removeAt( index );
  mBuffer->handleAttributeDeleted( mFieldIndex );
  mBuffer->updateLayerFields();

  emit mBuffer->attributeDeleted( mFieldIndex );
}

void QgsVectorLayerUndoCommandAddAttribute::redo()
{
  mBuffer->mAddedAttributes.append( mField );
  mBuffer->updateLayerFields();
  mBuffer->handleAttributeAdded( mFieldIndex );

  emit mBuffer->attributeAdded( mFieldIndex );
}


QgsVectorLayerUndoCommandDeleteAttribute::QgsVectorLayerUndoCommandDeleteAttribute( QgsVectorLayerEditBuffer* buffer, int fieldIndex )
    : QgsVectorLayerUndoCommand( buffer )
    , mFieldIndex( fieldIndex )
{
  const QgsFields& fields = layer()->fields();
  QgsFields::FieldOrigin origin = fields.fieldOrigin( mFieldIndex );
  mOriginIndex = fields.fieldOriginIndex( mFieldIndex );
  mProviderField = ( origin == QgsFields::OriginProvider );
  mOldEditorWidgetConfig = mBuffer->L->editFormConfig()->widgetConfig( mFieldIndex );

  if ( !mProviderField )
  {
    // need to store the field definition
    mOldField = mBuffer->mAddedAttributes[mOriginIndex];
  }

  if ( mBuffer->mRenamedAttributes.contains( fieldIndex ) )
  {
    mOldName = mBuffer->mRenamedAttributes.value( fieldIndex );
  }

  // save values of new features
  for ( QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.constBegin(); it != mBuffer->mAddedFeatures.constEnd(); ++it )
  {
    const QgsFeature& f = it.value();
    mDeletedValues.insert( f.id(), f.attribute( mFieldIndex ) );
  }

  // save changed values
  for ( QgsChangedAttributesMap::const_iterator it = mBuffer->mChangedAttributeValues.constBegin(); it != mBuffer->mChangedAttributeValues.constEnd(); ++it )
  {
    const QgsAttributeMap& attrs = it.value();
    if ( attrs.contains( mFieldIndex ) )
      mDeletedValues.insert( it.key(), attrs[mFieldIndex] );
  }
}

void QgsVectorLayerUndoCommandDeleteAttribute::undo()
{
  if ( mProviderField )
  {
    mBuffer->mDeletedAttributeIds.removeOne( mOriginIndex );
  }
  else
  {
    // newly added attribute
    mBuffer->mAddedAttributes.insert( mOriginIndex, mOldField );
  }

  mBuffer->updateLayerFields();
  mBuffer->handleAttributeAdded( mFieldIndex ); // update changed attributes + new features

  if ( !mOldName.isEmpty() )
  {
    mBuffer->mRenamedAttributes[ mFieldIndex ] = mOldName;
    mBuffer->updateLayerFields();
  }

  // set previously used attributes of new features
  for ( QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.begin(); it != mBuffer->mAddedFeatures.end(); ++it )
  {
    QgsFeature& f = it.value();
    f.setAttribute( mFieldIndex, mDeletedValues.value( f.id() ) );
  }
  // set previously used changed attributes
  for ( QMap<QgsFeatureId, QVariant>::const_iterator it = mDeletedValues.constBegin(); it != mDeletedValues.constEnd(); ++it )
  {
    if ( !FID_IS_NEW( it.key() ) )
    {
      QgsAttributeMap& attrs = mBuffer->mChangedAttributeValues[it.key()]; // also adds record if nonexistant
      attrs.insert( mFieldIndex, it.value() );
    }
  }

  mBuffer->L->editFormConfig()->setWidgetConfig( mFieldIndex, mOldEditorWidgetConfig );

  emit mBuffer->attributeAdded( mFieldIndex );
}

void QgsVectorLayerUndoCommandDeleteAttribute::redo()
{
  if ( mProviderField )
  {
    mBuffer->mDeletedAttributeIds.append( mOriginIndex );
    qSort( mBuffer->mDeletedAttributeIds ); // keep it sorted
  }
  else
  {
    // newly added attribute
    mBuffer->mAddedAttributes.removeAt( mOriginIndex ); // removing temporary attribute
  }

  mBuffer->L->editFormConfig()->removeWidgetConfig( mFieldIndex );
  mBuffer->handleAttributeDeleted( mFieldIndex ); // update changed attributes + new features
  mBuffer->updateLayerFields();
  emit mBuffer->attributeDeleted( mFieldIndex );
}


QgsVectorLayerUndoCommandRenameAttribute::QgsVectorLayerUndoCommandRenameAttribute( QgsVectorLayerEditBuffer* buffer, int fieldIndex, const QString& newName )
    : QgsVectorLayerUndoCommand( buffer )
    , mFieldIndex( fieldIndex )
    , mOldName( layer()->fields().at( fieldIndex ).name() )
    , mNewName( newName )
{
}

void QgsVectorLayerUndoCommandRenameAttribute::undo()
{
  mBuffer->mRenamedAttributes[ mFieldIndex ] = mOldName;
  mBuffer->updateLayerFields();
  emit mBuffer->attributeRenamed( mFieldIndex, mOldName );
}

void QgsVectorLayerUndoCommandRenameAttribute::redo()
{
  mBuffer->mRenamedAttributes[ mFieldIndex ] = mNewName;
  mBuffer->updateLayerFields();
  emit mBuffer->attributeRenamed( mFieldIndex, mNewName );
}
