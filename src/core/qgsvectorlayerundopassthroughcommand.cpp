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

//@todo use setObsolete instead of mHasError when upgrading qt version, this will allow auto removal of the command
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
  for ( const QgsFeature &f : qgis::as_const( features ) )
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
    for ( const QgsFeature &f : qgis::as_const( mFeatures ) )
    {
      emit mBuffer->featureDeleted( f.id() );
    }
    mFeatures = mInitialFeatures;
  }
}

void QgsVectorLayerUndoPassthroughCommandAddFeatures::redo()
{
  mFeatures = mInitialFeatures;
  if ( setSavePoint() && mBuffer->L->dataProvider()->addFeatures( mFeatures ) )
  {
    for ( const QgsFeature &f : qgis::as_const( mFeatures ) )
    {
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
    for ( const QgsFeatureId &id : mFids )
    {
      emit mBuffer->featureAdded( id );
    }
  }
}

void QgsVectorLayerUndoPassthroughCommandDeleteFeatures::redo()
{
  if ( setSavePoint() && mBuffer->L->dataProvider()->deleteFeatures( mFids ) )
  {
    for ( const QgsFeatureId &id : mFids )
    {
      emit mBuffer->featureDeleted( id );
    }
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandChangeGeometry::QgsVectorLayerUndoPassthroughCommandChangeGeometry( QgsVectorLayerEditBuffer *buffer, const QgsFeatureId &fid, const QgsGeometry &geom )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "change geometry" ) )
  , mFid( fid )
  , mNewGeom( geom )
  , mOldGeom( mBuffer->L->getFeature( mFid ).geometry() )
{
}

void QgsVectorLayerUndoPassthroughCommandChangeGeometry::undo()
{
  if ( rollBackToSavePoint() )
  {
    emit mBuffer->geometryChanged( mFid, mOldGeom );
  }
}

void QgsVectorLayerUndoPassthroughCommandChangeGeometry::redo()
{
  QgsGeometryMap geomMap;
  geomMap.insert( mFid, mNewGeom );
  if ( setSavePoint() && mBuffer->L->dataProvider()->changeGeometryValues( geomMap ) )
  {
    emit mBuffer->geometryChanged( mFid, mNewGeom );
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandChangeAttribute::QgsVectorLayerUndoPassthroughCommandChangeAttribute( QgsVectorLayerEditBuffer *buffer, QgsFeatureId fid, int field, const QVariant &newValue )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "change attribute value" ) )
  , mFid( fid )
  , mField( field )
  , mNewValue( newValue )
  , mOldValue( mBuffer->L->getFeature( mFid ).attribute( field ) )
{
}

void QgsVectorLayerUndoPassthroughCommandChangeAttribute::undo()
{
  if ( rollBackToSavePoint() )
  {
    emit mBuffer->attributeValueChanged( mFid, mField, mOldValue );
  }
}

void QgsVectorLayerUndoPassthroughCommandChangeAttribute::redo()
{
  QgsAttributeMap map;
  map.insert( mField, mNewValue );
  QgsChangedAttributesMap attribMap;
  attribMap.insert( mFid, map );
  if ( setSavePoint() && mBuffer->L->dataProvider()->changeAttributeValues( attribMap ) )
  {
    emit mBuffer->attributeValueChanged( mFid, mField, mNewValue );
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
  if ( mBuffer->L->dataProvider()->deleteAttributes( QgsAttributeIds() << attr ) && rollBackToSavePoint() )
  {
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
  if ( setSavePoint() && mBuffer->L->dataProvider()->addAttributes( QList<QgsField>() << mField ) )
  {
    mBuffer->updateLayerFields();
    const int attr = mBuffer->L->dataProvider()->fieldNameIndex( mField.name() );
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
{
}

void QgsVectorLayerUndoPassthroughCommandDeleteAttribute::undo()
{
  // note that the addAttributes here is only necessary to inform the provider that
  // an attribute is added back after the rollBackToSavePoint
  if ( mBuffer->L->dataProvider()->addAttributes( QList<QgsField>() << mField )  && rollBackToSavePoint() )
  {
    mBuffer->updateLayerFields();
    const int attr = mBuffer->L->dataProvider()->fieldNameIndex( mField.name() );
    emit mBuffer->attributeAdded( attr );
  }
  else
  {
    setError();
  }
}

void QgsVectorLayerUndoPassthroughCommandDeleteAttribute::redo()
{
  const int attr = mBuffer->L->dataProvider()->fieldNameIndex( mField.name() );
  if ( setSavePoint() && mBuffer->L->dataProvider()->deleteAttributes( QgsAttributeIds() << attr ) )
  {
    mBuffer->updateLayerFields();
    emit mBuffer->attributeDeleted( attr );
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
  if ( mBuffer->L->dataProvider()->renameAttributes( map ) && rollBackToSavePoint() )
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
  if ( setSavePoint() && mBuffer->L->dataProvider()->renameAttributes( map ) )
  {
    mBuffer->updateLayerFields();
    emit mBuffer->attributeRenamed( mAttr, mNewName );
  }
  else
  {
    setError();
  }
}

QgsVectorLayerUndoPassthroughCommandUpdate::QgsVectorLayerUndoPassthroughCommandUpdate( QgsVectorLayerEditBuffer *buffer, QgsTransaction *transaction, const QString &sql )
  : QgsVectorLayerUndoPassthroughCommand( buffer, QObject::tr( "custom transaction" ), false )
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
