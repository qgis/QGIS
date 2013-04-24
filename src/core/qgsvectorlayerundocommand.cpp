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
#ifdef QGISDEBUG
  QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.find( mFeature.id() );
  Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
#endif
  mBuffer->mAddedFeatures.remove( mFeature.id() );

  if ( mFeature.geometry() )
    cache()->removeGeometry( mFeature.id() );

  emit mBuffer->featureDeleted( mFeature.id() );
}

void QgsVectorLayerUndoCommandAddFeature::redo()
{
  mBuffer->mAddedFeatures.insert( mFeature.id(), mFeature );

  if ( mFeature.geometry() )
    cache()->cacheGeometry( mFeature.id(), *mFeature.geometry() );

  emit mBuffer->featureAdded( mFeature.id() );
}



QgsVectorLayerUndoCommandDeleteFeature::QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid )
    : QgsVectorLayerUndoCommand( buffer )
{
  mFid = fid;

  if ( FID_IS_NEW( mFid ) )
  {
    QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
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
    : QgsVectorLayerUndoCommand( buffer ),
    mFid( fid )
{

  if ( FID_IS_NEW( mFid ) )
  {
    QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
    mOldGeom = new QgsGeometry( *it.value().geometry() );
  }
  else
  {
    bool changedAlready = mBuffer->mChangedGeometries.contains( mFid );
    QgsGeometry geom;
    bool cachedGeom = cache()->geometry( mFid, geom );
    mOldGeom = ( changedAlready && cachedGeom ) ? new QgsGeometry( geom ) : 0;
  }

  mNewGeom = new QgsGeometry( *newGeom );
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
      if ( layer()->getFeatures( QgsFeatureRequest().setFilterFid( mFid ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f ) && f.geometry() )
      {
        cache()->cacheGeometry( mFid, *f.geometry() );
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





QgsVectorLayerUndoCommandChangeAttribute::QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid, int fieldIndex, const QVariant& newValue )
    : QgsVectorLayerUndoCommand( buffer ),
    mFid( fid ),
    mFieldIndex( fieldIndex ),
    mNewValue( newValue ),
    mFirstChange( true )
{

  if ( FID_IS_NEW( mFid ) )
  {
    // work with added feature
    QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.find( mFid );
    Q_ASSERT( it != mBuffer->mAddedFeatures.end() );
    if ( it.value().attribute( mFieldIndex ).isValid() )
    {
      mOldValue = it.value().attribute( mFieldIndex );
      mFirstChange = false;
    }
  }
  else
  {
    if ( mBuffer->mChangedAttributeValues.contains( mFid ) && mBuffer->mChangedAttributeValues[mFid].contains( mFieldIndex ) )
    {
      mOldValue = mBuffer->mChangedAttributeValues[mFid][mFieldIndex];
      mFirstChange = false;
    }
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
  else
  {
    // existing feature
    if ( mFirstChange )
    {
      mBuffer->mChangedAttributeValues[mFid].remove( mFieldIndex );
      if ( mBuffer->mChangedAttributeValues[mFid].isEmpty() )
        mBuffer->mChangedAttributeValues.remove( mFid );

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
    else
    {
      mBuffer->mChangedAttributeValues[mFid][mFieldIndex] = mOldValue;
    }
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
    : QgsVectorLayerUndoCommand( buffer ),
    mField( field )
{
  mFieldIndex = layer()->pendingFields().count();
}

void QgsVectorLayerUndoCommandAddAttribute::undo()
{
  int index = layer()->pendingFields().fieldOriginIndex( mFieldIndex );

  mBuffer->mAddedAttributes.removeAt( index );
  mBuffer->updateLayerFields();
  mBuffer->handleAttributeDeleted( mFieldIndex );

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
    : QgsVectorLayerUndoCommand( buffer ),
    mFieldIndex( fieldIndex )
{
  const QgsFields& fields = layer()->pendingFields();
  QgsFields::FieldOrigin origin = fields.fieldOrigin( mFieldIndex );
  mOriginIndex = fields.fieldOriginIndex( mFieldIndex );
  mProviderField = ( origin == QgsFields::OriginProvider );

  if ( !mProviderField )
  {
    // need to store the field definition
    mOldField = mBuffer->mAddedAttributes[mOriginIndex];
  }

  // save values of new features
  for ( QgsFeatureMap::const_iterator it = mBuffer->mAddedFeatures.begin(); it != mBuffer->mAddedFeatures.end(); ++it )
  {
    const QgsFeature& f = it.value();
    mDeletedValues.insert( f.id(), f.attribute( mFieldIndex ) );
  }

  // save changed values
  for ( QgsChangedAttributesMap::const_iterator it = mBuffer->mChangedAttributeValues.begin(); it != mBuffer->mChangedAttributeValues.end(); ++it )
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

  // set previously used attributes of new features
  for ( QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.begin(); it != mBuffer->mAddedFeatures.end(); ++it )
  {
    QgsFeature& f = it.value();
    f.setAttribute( mFieldIndex, mDeletedValues.value( f.id() ) );
  }
  // set previously used changed attributes
  for ( QMap<QgsFeatureId, QVariant>::const_iterator it = mDeletedValues.begin(); it != mDeletedValues.end(); ++it )
  {
    if ( !FID_IS_NEW( it.key() ) )
    {
      QgsAttributeMap& attrs = mBuffer->mChangedAttributeValues[it.key()]; // also adds record if nonexistant
      attrs.insert( mFieldIndex, it.value() );
    }
  }

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

  mBuffer->updateLayerFields();
  mBuffer->handleAttributeDeleted( mFieldIndex ); // update changed attributes + new features
  emit mBuffer->attributeDeleted( mFieldIndex );
}
