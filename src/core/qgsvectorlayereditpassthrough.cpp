/***************************************************************************
    qgsvectorlayereditpassthrough.cpp
    ---------------------
    begin                : Jan 12 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayereditpassthrough.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayerundopassthroughcommand.h"
#include "qgstransaction.h"

QgsVectorLayerEditPassthrough::QgsVectorLayerEditPassthrough( QgsVectorLayer *layer )
  : mModified( false )
{
  L = layer;
}

bool QgsVectorLayerEditPassthrough::isModified() const
{
  return mModified;
}

bool QgsVectorLayerEditPassthrough::modify( QgsVectorLayerUndoPassthroughCommand *cmd )
{
  L->undoStack()->push( cmd ); // push takes owneship -> no need for cmd to be a smart ptr
  if ( cmd->hasError() )
    return false;

  if ( !mModified )
  {
    mModified = true;
    emit layerModified();
  }

  return true;
}

bool QgsVectorLayerEditPassthrough::addFeature( QgsFeature &f )
{
  QgsVectorLayerUndoPassthroughCommandAddFeatures *cmd = new QgsVectorLayerUndoPassthroughCommandAddFeatures( this, QgsFeatureList() << f );
  if ( !modify( cmd ) ) // modify takes owneship -> no need for cmd to be a smart ptr
    return false;

  const QgsFeatureList features = cmd->features();
  f = features.at( features.count() - 1 );
  return true;
}

bool QgsVectorLayerEditPassthrough::addFeatures( QgsFeatureList &features )
{
  QgsVectorLayerUndoPassthroughCommandAddFeatures *cmd = new QgsVectorLayerUndoPassthroughCommandAddFeatures( this, features );
  if ( !modify( cmd ) ) // modify takes owneship -> no need for cmd to be a smart ptr
    return false;

  features = cmd->features();
  return true;
}

bool QgsVectorLayerEditPassthrough::deleteFeature( QgsFeatureId fid )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandDeleteFeatures( this, QgsFeatureIds() << fid ) );
}

bool QgsVectorLayerEditPassthrough::deleteFeatures( const QgsFeatureIds &fids )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandDeleteFeatures( this, fids ) );
}

bool QgsVectorLayerEditPassthrough::changeGeometry( QgsFeatureId fid, const QgsGeometry &geom )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandChangeGeometry( this, fid, geom ) );
}

bool QgsVectorLayerEditPassthrough::changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &/*oldValue*/ )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandChangeAttribute( this, fid, field, newValue ) );
}

bool QgsVectorLayerEditPassthrough::changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandChangeAttributes( this, fid, newValues, oldValues ) );
}

bool QgsVectorLayerEditPassthrough::addAttribute( const QgsField &field )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandAddAttribute( this, field ) );
}

bool QgsVectorLayerEditPassthrough::deleteAttribute( int attr )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandDeleteAttribute( this, attr ) );
}

bool QgsVectorLayerEditPassthrough::renameAttribute( int attr, const QString &newName )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandRenameAttribute( this, attr, newName ) );
}

bool QgsVectorLayerEditPassthrough::commitChanges( QStringList & /*commitErrors*/ )
{
  mModified = false;
  return true;
}

void QgsVectorLayerEditPassthrough::rollBack()
{
  mModified = false;
}

bool QgsVectorLayerEditPassthrough::update( QgsTransaction *tr, const QString &sql, const QString &name )
{
  return modify( new QgsVectorLayerUndoPassthroughCommandUpdate( this, tr, sql, name ) );
}
