/***************************************************************************
    qgsvectorlayerundopassthroughcommand.cpp
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Vincent Mora
    email                : vincent dot mora at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerundopassthroughcommand.h"

#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"

#include "qgslogger.h"
#include "qgstransaction.h"

#include <QUuid>

// TODO use setObsolete instead of mHasError when upgrading qt version, this will allow auto removal of the command
// for the moment a errored command is left on the stack

QgsVectorLayerUndoPassthroughCommand::QgsVectorLayerUndoPassthroughCommand( QgsVectorLayerEditBuffer *buffer, const QString &text, bool autocreate )
  : QgsVectorLayerUndoCommand( buffer )
  , mSavePointId( ( mBuffer->L->isEditCommandActive() && !mBuffer->L->dataProvider()->transaction()->savePoints().isEmpty() )
                  || !autocreate
                  ? mBuffer->L->dataProvider()->transaction()->savePoints().last()
                  : mBuffer->L->dataProvider()->transaction()->createSavepoint( mError ) )
  , mHasError( !mError.isEmpty() )
  , mRecreateSavePoint( mBuffer->L->isEditCommandActive()
                        ? !mBuffer->L->dataProvider()->transaction()->lastSavePointIsDirty()
                        : true )
{
  // the first command in the undo stack macro will have a clean save point
  // the first command is responsible to re-create the savepoint after undo
  setText( text );
}


void QgsVectorLayerUndoPassthroughCommand::setError()
{
  if ( !mHasError )
  {
    setText( text() + " " + QObject::tr( "failed" ) );
    mHasError = true;
  }
}

void QgsVectorLayerUndoPassthroughCommand::setErrorMessage( const QString &errorMessage )
{
  mError = errorMessage;
}

QString QgsVectorLayerUndoPassthroughCommand::errorMessage() const
{
  return mError;
}

bool QgsVectorLayerUndoPassthroughCommand::setSavePoint( const QString &savePointId )
{
  if ( !hasError() )
  {
    if ( savePointId.isEmpty() )
    {
      // re-create savepoint only if mRecreateSavePoint and rollBackToSavePoint as occurred
      if ( mRecreateSavePoint && mBuffer->L->dataProvider()->transaction()->savePoints().indexOf( mSavePointId ) == -1 )
      {
        mSavePointId = mBuffer->L->dataProvider()->transaction()->createSavepoint( mSavePointId, mError );
        if ( mSavePointId.isEmpty() )
        {
          setError();
        }
      }
    }
    else
    {
      mSavePointId = savePointId;
    }
  }
  return !hasError();
}

bool QgsVectorLayerUndoPassthroughCommand::rollBackToSavePoint()
{
  // rollback only occurs for the last command in undo macro
  if ( !hasError() && mBuffer->L->dataProvider()->transaction()->savePoints().indexOf( mSavePointId ) != -1 )
  {
    if ( !mBuffer->L->dataProvider()->transaction()->rollbackToSavepoint( mSavePointId, mError ) )
    {
      setError();
    }
  }
  return !hasError();
}


QgsVectorLayerUndoPassthroughCommandAddFeatures::QgsVectorLayerUndoPassthroughCommandAddFeatures( QgsVectorLayerEditBuffer *buffer, QgsFeatureList &features )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "add features" ) )
{
  static int sAddedIdLowWaterMark = -1;
  for ( const QgsFeature &f : std::as_const( features ) )
  {
    mInitialFeatures << f;
    //assign a temporary id to the feature (use negative numbers)
    sAddedIdLowWaterMark--;
    mInitialFeatures.last().setId( sAddedIdLowWaterMark );
  }
  mFeatures = mInitialFeatures;

}

void QgsVectorLayerUndoPassthroughCommandAddFeatures::undo()
{
  if ( rollBackToSavePoint() )
  {
    for ( const QgsFeature &f : std::as_const( mFeatures ) )
    {
      mBuffer->mAddedFeatures.remove( f.id() );
      emit mBuffer->featureDeleted( f.id() );
    }
    mFeatures = mInitialFeatures;
  }
}

void QgsVectorLayerUndoPassthroughCommandAddFeatures::redo()
{
  mFeatures = mInitialFeatures;
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->addFeatures( mFeatures ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    for ( const QgsFeature &f : std::as_const( mFeatures ) )
    {
      mBuffer->mAddedFeatures.insert( f.id(), f );
      emit mBuffer->featureAdded( f.id() );
    }
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandDeleteFeatures::QgsVectorLayerUndoPassthroughCommandDeleteFeatures( QgsVectorLayerEditBuffer *buffer, const QgsFeatureIds &fids )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "delete features" ) )
  , mFids( fids )
{
}

void QgsVectorLayerUndoPassthroughCommandDeleteFeatures::undo()
{
  if ( rollBackToSavePoint() )
  {
    for ( const QgsFeatureId &fid : mFids )
    {
      mBuffer->mDeletedFeatureIds.remove( fid );
      if ( mDeletedNewFeatures.contains( fid ) )
      {
        mBuffer->mAddedFeatures.insert( fid, mDeletedNewFeatures.value( fid ) );
      }
      emit mBuffer->featureAdded( fid );
    }
  }
}

void QgsVectorLayerUndoPassthroughCommandDeleteFeatures::redo()
{
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->deleteFeatures( mFids ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    mDeletedNewFeatures.clear();
    for ( const QgsFeatureId &fid : mFids )
    {
      if ( mBuffer->mAddedFeatures.contains( fid ) )
      {
        mDeletedNewFeatures.insert( fid, mBuffer->mAddedFeatures[ fid ] );
        mBuffer->mAddedFeatures.remove( fid );
      }
      else
      {
        mBuffer->mDeletedFeatureIds.insert( fid );
      }
      emit mBuffer->featureDeleted( fid );
    }
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandChangeGeometry::QgsVectorLayerUndoPassthroughCommandChangeGeometry( QgsVectorLayerEditBuffer *buffer, QgsFeatureId fid, const QgsGeometry &geom )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "change geometry" ) )
  , mFid( fid )
  , mNewGeom( geom )
  , mOldGeom( mBuffer->L->getFeature( mFid ).geometry() )
  , mFirstChange( true )
{
  if ( mBuffer->mAddedFeatures.contains( mFid ) )
  {
    mFirstChange = false;
  }
  else if ( mBuffer->mChangedGeometries.contains( mFid ) )
  {
    mFirstChange = false;
    mOldGeom = mBuffer->mChangedGeometries[mFid];
  }
}

void QgsVectorLayerUndoPassthroughCommandChangeGeometry::undo()
{
  if ( rollBackToSavePoint() )
  {
    if ( mBuffer->mAddedFeatures.contains( mFid ) )
    {
      mBuffer->mAddedFeatures[ mFid ].setGeometry( mOldGeom );
    }
    else if ( mFirstChange )
    {
      mBuffer->mChangedGeometries.remove( mFid );
    }
    else
    {
      mBuffer->mChangedGeometries[mFid] = mOldGeom;
    }
    emit mBuffer->geometryChanged( mFid,  mOldGeom );
  }
}

void QgsVectorLayerUndoPassthroughCommandChangeGeometry::redo()
{
  QgsGeometryMap geomMap;
  geomMap.insert( mFid, mNewGeom );
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->changeGeometryValues( geomMap ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    if ( mBuffer->mAddedFeatures.contains( mFid ) )
    {
      mBuffer->mAddedFeatures[ mFid ].setGeometry( mNewGeom );
    }
    else
    {
      mBuffer->mChangedGeometries[ mFid ] = mNewGeom;
    }
    emit mBuffer->geometryChanged( mFid, mNewGeom );
  }
  else
  {
    setError();
  }
}

bool QgsVectorLayerUndoPassthroughCommandChangeGeometry::mergeWith( const QUndoCommand *other )
{
  if ( other->id() != id() )
    return false;

  const QgsVectorLayerUndoPassthroughCommandChangeGeometry *merge = dynamic_cast<const QgsVectorLayerUndoPassthroughCommandChangeGeometry *>( other );
  if ( !merge )
    return false;

  if ( merge->mFid != mFid )
    return false;

  mNewGeom = merge->mNewGeom;
  merge->mNewGeom = QgsGeometry();

  return true;
}



QgsVectorLayerUndoPassthroughCommandChangeAttribute::QgsVectorLayerUndoPassthroughCommandChangeAttribute( QgsVectorLayerEditBuffer *buffer, QgsFeatureId fid, int field, const QVariant &newValue )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "change attribute value" ) )
  , mFid( fid )
  , mFieldIndex( field )
  , mNewValue( newValue )
  , mOldValue( mBuffer->L->getFeature( mFid ).attribute( field ) )
  , mFirstChange( true )
{

  if ( mBuffer->mAddedFeatures.contains( mFid ) )
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

void QgsVectorLayerUndoPassthroughCommandChangeAttribute::undo()
{
  if ( rollBackToSavePoint() )
  {
    QVariant original = mOldValue;

    if ( mBuffer->mAddedFeatures.contains( mFid ) )
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
        request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
        request.setSubsetOfAttributes( QgsAttributeList() << mFieldIndex );
        std::unique_ptr<QgsVectorLayer> layerClone( layer()->clone() );
        QgsFeatureIterator fi = layerClone->getFeatures( request );
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
}

void QgsVectorLayerUndoPassthroughCommandChangeAttribute::redo()
{
  QgsAttributeMap map;
  map.insert( mFieldIndex, mNewValue );
  QgsChangedAttributesMap attribMap;
  attribMap.insert( mFid, map );
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->changeAttributeValues( attribMap ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    // Update existing feature
    QgsFeatureMap::iterator it = mBuffer->mAddedFeatures.find( mFid );
    if ( it != mBuffer->mAddedFeatures.end() )
    {
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
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandAddAttribute::QgsVectorLayerUndoPassthroughCommandAddAttribute( QgsVectorLayerEditBuffer *buffer, const QgsField &field )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "add attribute" ) + " " + field.name() )
  , mField( field )
{
}

void QgsVectorLayerUndoPassthroughCommandAddAttribute::undo()
{
  // note that the deleteAttribute here is only necessary to inform the provider that
  // an attribute is removed after the rollBackToSavePoint
  const int attr = mBuffer->L->dataProvider()->fieldNameIndex( mField.name() );
  if ( rollBackToSavePoint() )
  {
    mBuffer->L->dataProvider()->deleteAttributes( QgsAttributeIds() << attr );
    mBuffer->mAddedAttributes.removeAll( mField );
    mBuffer->updateLayerFields();
    emit mBuffer->attributeDeleted( attr );
  }
  else
  {
    setError();
  }
}

void QgsVectorLayerUndoPassthroughCommandAddAttribute::redo()
{
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->addAttributes( QList<QgsField>() << mField ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    mBuffer->updateLayerFields();
    const int attr = mBuffer->L->dataProvider()->fieldNameIndex( mField.name() );
    mBuffer->mAddedAttributes.append( mField );
    emit mBuffer->attributeAdded( attr );
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandDeleteAttribute::QgsVectorLayerUndoPassthroughCommandDeleteAttribute( QgsVectorLayerEditBuffer *buffer, int attr )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "delete attribute" ) )
  , mField( mBuffer->L->fields()[ attr ] )
  , mOriginalFieldIndex( attr )
{
}

void QgsVectorLayerUndoPassthroughCommandDeleteAttribute::undo()
{
  // note that the addAttributes here is only necessary to inform the provider that
  // an attribute is added back after the rollBackToSavePoint
  mBuffer->L->dataProvider()->clearErrors();
  if ( mBuffer->L->dataProvider()->addAttributes( QList<QgsField>() << mField )  && rollBackToSavePoint() && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    mBuffer->mDeletedAttributeIds.removeOne( mOriginalFieldIndex );
    mBuffer->updateLayerFields();
    emit mBuffer->attributeAdded( mOriginalFieldIndex );
  }
  else
  {
    setError();
  }
}

void QgsVectorLayerUndoPassthroughCommandDeleteAttribute::redo()
{
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->deleteAttributes( QgsAttributeIds() << mOriginalFieldIndex ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    mBuffer->mDeletedAttributeIds.append( mOriginalFieldIndex );
    mBuffer->updateLayerFields();
    emit mBuffer->attributeDeleted( mOriginalFieldIndex );
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandRenameAttribute::QgsVectorLayerUndoPassthroughCommandRenameAttribute( QgsVectorLayerEditBuffer *buffer, int attr, const QString &newName )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "rename attribute" ) + " " + newName )
  , mAttr( attr )
  , mNewName( newName )
  , mOldName( mBuffer->L->fields()[ mAttr ].name() )
{
}

void QgsVectorLayerUndoPassthroughCommandRenameAttribute::undo()
{
  // note that the renameAttributes here is only necessary to inform the provider that
  // an attribute is renamed after the rollBackToSavePoint
  QgsFieldNameMap map;
  map[ mAttr ] = mOldName;
  mBuffer->L->dataProvider()->clearErrors();
  if ( mBuffer->L->dataProvider()->renameAttributes( map ) && rollBackToSavePoint() && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    mBuffer->updateLayerFields();
    emit mBuffer->attributeRenamed( mAttr, mOldName );
  }
  else
  {
    setError();
  }
}

void QgsVectorLayerUndoPassthroughCommandRenameAttribute::redo()
{
  QgsFieldNameMap map;
  map[ mAttr ] = mNewName;
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->renameAttributes( map ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    mBuffer->updateLayerFields();
    emit mBuffer->attributeRenamed( mAttr, mNewName );
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandUpdate::QgsVectorLayerUndoPassthroughCommandUpdate( QgsVectorLayerEditBuffer *buffer, QgsTransaction *transaction, const QString &sql, const QString &name )
  : QgsVectorLayerUndoPassthroughCommand( buffer, name.isEmpty() ? QObject::tr( "custom transaction" ) : name, false )
  , mTransaction( transaction )
  , mSql( sql )
{
}

void QgsVectorLayerUndoPassthroughCommandUpdate::undo()
{
  if ( rollBackToSavePoint() )
  {
    mUndone = true;
    emit mBuffer->L->layerModified();
  }
  else
  {
    setError();
  }
}

void QgsVectorLayerUndoPassthroughCommandUpdate::redo()
{
  // the first time that the sql query is execute is within QgsTransaction
  // itself. So the redo has to be executed only after an undo action.
  if ( mUndone )
  {
    QString errorMessage;

    QString savePointId = mTransaction->createSavepoint( errorMessage );

    if ( errorMessage.isEmpty() )
    {
      setSavePoint( savePointId );

      if ( mTransaction->executeSql( mSql, errorMessage ) )
      {
        mUndone = false;
      }
      else
      {
        setErrorMessage( errorMessage );
        setError();
      }
    }
    else
    {
      setErrorMessage( errorMessage );
      setError();
    }
  }
}

QgsVectorLayerUndoPassthroughCommandChangeAttributes::QgsVectorLayerUndoPassthroughCommandChangeAttributes( QgsVectorLayerEditBuffer *buffer, QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "change attribute value" ) )
  , mFid( fid )
  , mNewValues( newValues )
  , mOldValues( oldValues )
{
  if ( mOldValues.isEmpty() )
  {
    const auto oldAttrs( mBuffer->L->getFeature( mFid ).attributes() );
    for ( auto it = mNewValues.constBegin(); it != mNewValues.constEnd(); ++it )
    {
      mOldValues[ it.key() ] = oldAttrs[ it.key() ];
    }
  }
  const bool isAdded { mBuffer->mAddedFeatures.contains( mFid ) };
  for ( auto it = mNewValues.constBegin(); it != mNewValues.constEnd(); ++it )
  {
    if ( isAdded && mBuffer->mAddedFeatures[ mFid ].attribute( it.key() ).isValid() )
    {
      mFirstChanges[ it.key() ] = false;
    }
    else if ( mBuffer->mChangedAttributeValues.contains( mFid ) && mBuffer->mChangedAttributeValues[mFid].contains( it.key() ) )
    {
      mFirstChanges[ it.key() ] = false;
    }
    else
    {
      mFirstChanges[ it.key() ] = true;
    }
  }
}

void QgsVectorLayerUndoPassthroughCommandChangeAttributes::undo()
{
  if ( rollBackToSavePoint() )
  {
    QgsFeatureMap::iterator addedIt = mBuffer->mAddedFeatures.find( mFid );
    for ( auto it = mNewValues.constBegin(); it != mNewValues.constEnd(); ++it )
    {
      const auto fieldIndex { it.key() };
      if ( addedIt != mBuffer->mAddedFeatures.end() )
      {
        addedIt.value().setAttribute( fieldIndex, mOldValues[ it.key() ] );
      }
      else if ( mFirstChanges.contains( fieldIndex ) && mFirstChanges[ fieldIndex ] )
      {
        // existing feature
        mBuffer->mChangedAttributeValues[mFid].remove( fieldIndex );
      }
      else
      {
        // changed attribute of existing feature
        if ( !mBuffer->mChangedAttributeValues.contains( mFid ) )
        {
          mBuffer->mChangedAttributeValues.insert( mFid, QgsAttributeMap() );
        }
        mBuffer->mChangedAttributeValues[mFid].insert( fieldIndex, mOldValues[ it.key() ] );
      }
      emit mBuffer->attributeValueChanged( mFid, it.key(), mOldValues[ it.key() ] );
    }
    if ( mBuffer->mChangedAttributeValues[mFid].isEmpty() )
      mBuffer->mChangedAttributeValues.remove( mFid );
  }
}

void QgsVectorLayerUndoPassthroughCommandChangeAttributes::redo()
{
  QgsChangedAttributesMap attribMap;
  attribMap.insert( mFid, mNewValues );
  mBuffer->L->dataProvider()->clearErrors();
  if ( setSavePoint() && mBuffer->L->dataProvider()->changeAttributeValues( attribMap ) && ! mBuffer->L->dataProvider()->hasErrors() )
  {
    QgsFeatureMap::iterator addedIt = mBuffer->mAddedFeatures.find( mFid );
    for ( auto it = mNewValues.constBegin(); it != mNewValues.constEnd(); ++it )
    {
      const auto fieldIndex { it.key() };
      // Update existing feature
      if ( addedIt != mBuffer->mAddedFeatures.end() )
      {
        addedIt.value().setAttribute( fieldIndex, it.value() );
      }
      else
      {
        // changed attribute of existing feature
        if ( !mBuffer->mChangedAttributeValues.contains( mFid ) )
        {
          mBuffer->mChangedAttributeValues.insert( mFid, QgsAttributeMap() );
        }
        mBuffer->mChangedAttributeValues[mFid].insert( fieldIndex, it.value() );
      }
      emit mBuffer->attributeValueChanged( mFid, it.key(), it.value() );
    }
  }
}
