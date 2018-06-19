/***************************************************************************
    qgsmaplayeractionregistry.cpp
    -----------------------------
    begin                : January 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayeractionregistry.h"
#include "qgsgui.h"
#include "qgsvectorlayer.h"

QgsMapLayerAction::QgsMapLayerAction( const QString &name, QObject *parent, Targets targets, const QIcon &icon, QgsMapLayerAction::Flags flags )
  : QAction( icon, name, parent )
  , mTargets( targets )
  , mFlags( flags )
{
}

QgsMapLayerAction::QgsMapLayerAction( const QString &name, QObject *parent, QgsMapLayer *layer, Targets targets, const QIcon &icon, QgsMapLayerAction::Flags flags )
  : QAction( icon, name, parent )
  , mSingleLayer( true )
  , mActionLayer( layer )
  , mTargets( targets )
  , mFlags( flags )
{
}

QgsMapLayerAction::QgsMapLayerAction( const QString &name, QObject *parent, QgsMapLayer::LayerType layerType, Targets targets, const QIcon &icon, QgsMapLayerAction::Flags flags )
  : QAction( icon, name, parent )
  , mSpecificLayerType( true )
  , mLayerType( layerType )
  , mTargets( targets )
  , mFlags( flags )
{
}

QgsMapLayerAction::~QgsMapLayerAction()
{
  //remove action from registry
  QgsGui::mapLayerActionRegistry()->removeMapLayerAction( this );
}

QgsMapLayerAction::Flags QgsMapLayerAction::flags() const
{
  return mFlags;
}

bool QgsMapLayerAction::canRunUsingLayer( QgsMapLayer *layer ) const
{
  if ( mFlags & EnabledOnlyWhenEditable )
  {
    // action is only enabled for editable layers
    if ( !layer )
      return false;
    if ( layer->type() != QgsMapLayer::VectorLayer )
      return false;
    if ( !qobject_cast<QgsVectorLayer *>( layer )->isEditable() )
      return false;
  }

  //check layer details
  if ( !mSingleLayer && !mSpecificLayerType )
  {
    //action is not a single layer of specific layer type action,
    //so return true
    return true;
  }
  if ( mSingleLayer && layer == mActionLayer )
  {
    //action is a single layer type and layer matches
    return true;
  }
  else if ( mSpecificLayerType && layer && layer->type() == mLayerType )
  {
    //action is for a layer type and layer type matches
    return true;
  }

  return false;
}

void QgsMapLayerAction::triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList )
{
  emit triggeredForFeatures( layer, featureList );
}

void QgsMapLayerAction::triggerForFeature( QgsMapLayer *layer, const QgsFeature *feature )
{
  emit triggeredForFeature( layer, *feature );
}

void QgsMapLayerAction::triggerForLayer( QgsMapLayer *layer )
{
  emit triggeredForLayer( layer );
}

bool QgsMapLayerAction::isEnabledOnlyWhenEditable() const
{
  return mFlags & EnabledOnlyWhenEditable;
}

//
// Main class begins now...
//

QgsMapLayerActionRegistry::QgsMapLayerActionRegistry( QObject *parent ) : QObject( parent )
{
  // constructor does nothing
}

void QgsMapLayerActionRegistry::addMapLayerAction( QgsMapLayerAction *action )
{
  mMapLayerActionList.append( action );
  emit changed();
}

QList< QgsMapLayerAction * > QgsMapLayerActionRegistry::mapLayerActions( QgsMapLayer *layer, QgsMapLayerAction::Targets targets )
{
  QList< QgsMapLayerAction * > validActions;

  Q_FOREACH ( QgsMapLayerAction *action, mMapLayerActionList )
  {
    if ( action->canRunUsingLayer( layer ) && ( targets & action->targets() ) )
    {
      validActions.append( action );
    }
  }
  return validActions;
}


bool QgsMapLayerActionRegistry::removeMapLayerAction( QgsMapLayerAction *action )
{
  if ( mMapLayerActionList.indexOf( action ) != -1 )
  {
    mMapLayerActionList.removeAll( action );

    //also remove this action from the default layer action map
    QMap<QgsMapLayer *, QgsMapLayerAction *>::iterator defaultIt;
    for ( defaultIt = mDefaultLayerActionMap.begin(); defaultIt != mDefaultLayerActionMap.end(); ++defaultIt )
    {
      if ( defaultIt.value() == action )
      {
        defaultIt.value() = nullptr;
      }
    }
    emit changed();
    return true;
  }
  //not found
  return false;
}

void QgsMapLayerActionRegistry::setDefaultActionForLayer( QgsMapLayer *layer, QgsMapLayerAction *action )
{
  mDefaultLayerActionMap[ layer ] = action;
}

QgsMapLayerAction *QgsMapLayerActionRegistry::defaultActionForLayer( QgsMapLayer *layer )
{
  if ( !mDefaultLayerActionMap.contains( layer ) )
  {
    return nullptr;
  }

  return mDefaultLayerActionMap[ layer ];
}
