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


QgsMapLayerAction::QgsMapLayerAction( QString name, QObject* parent, Targets targets, QIcon icon )
    : QAction( icon, name, parent )
    , mSingleLayer( false )
    , mActionLayer( 0 )
    , mSpecificLayerType( false )
    , mTargets( targets )
{
}

/**Creates a map layer action which can run only on a specific layer*/
QgsMapLayerAction::QgsMapLayerAction( QString name, QObject* parent, QgsMapLayer* layer , Targets targets, QIcon icon )
    : QAction( icon, name, parent )
    , mSingleLayer( true )
    , mActionLayer( layer )
    , mSpecificLayerType( false )
    , mTargets( targets )
{
}

/**Creates a map layer action which can run on a specific type of layer*/
QgsMapLayerAction::QgsMapLayerAction( QString name, QObject* parent, QgsMapLayer::LayerType layerType, Targets targets, QIcon icon )
    : QAction( icon, name, parent )
    , mSingleLayer( false )
    , mActionLayer( 0 )
    , mSpecificLayerType( true )
    , mLayerType( layerType )
    , mTargets( targets )
{
}

QgsMapLayerAction::~QgsMapLayerAction()
{
  //remove action from registry
  QgsMapLayerActionRegistry::instance()->removeMapLayerAction( this );
}

bool QgsMapLayerAction::canRunUsingLayer( QgsMapLayer* layer ) const
{
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
  else if ( mSpecificLayerType && layer->type() == mLayerType )
  {
    //action is for a layer type and layer type matches
    return true;
  }

  return false;
}

void QgsMapLayerAction::triggerForFeatures( QgsMapLayer* layer, const QList<QgsFeature> featureList )
{
  emit triggeredForFeatures( layer, featureList );
}

void QgsMapLayerAction::triggerForFeature( QgsMapLayer* layer, const QgsFeature* feature )
{
  emit triggeredForFeature( layer, *feature );
}

void QgsMapLayerAction::triggerForLayer( QgsMapLayer* layer )
{
  emit triggeredForLayer( layer );
}

//
// Static calls to enforce singleton behaviour
//
QgsMapLayerActionRegistry *QgsMapLayerActionRegistry::mInstance = 0;
QgsMapLayerActionRegistry *QgsMapLayerActionRegistry::instance()
{
  if ( mInstance == 0 )
  {
    mInstance = new QgsMapLayerActionRegistry();
  }
  return mInstance;
}

//
// Main class begins now...
//

QgsMapLayerActionRegistry::QgsMapLayerActionRegistry( QObject *parent ) : QObject( parent )
{
  // constructor does nothing
}

QgsMapLayerActionRegistry::~QgsMapLayerActionRegistry()
{

}

void QgsMapLayerActionRegistry::addMapLayerAction( QgsMapLayerAction * action )
{
  mMapLayerActionList.append( action );
  emit changed();
}

QList< QgsMapLayerAction* > QgsMapLayerActionRegistry::mapLayerActions( QgsMapLayer* layer, QgsMapLayerAction::Targets targets )
{
  QList< QgsMapLayerAction* > validActions;
  QList<QgsMapLayerAction*>::iterator actionIt;
  for ( actionIt = mMapLayerActionList.begin(); actionIt != mMapLayerActionList.end(); ++actionIt )
  {
    if (( *actionIt )->canRunUsingLayer( layer ) && ( targets & ( *actionIt )->targets() ) )
    {
      validActions.append(( *actionIt ) );
    }
  }
  return validActions;
}


bool QgsMapLayerActionRegistry::removeMapLayerAction( QgsMapLayerAction* action )
{
  if ( mMapLayerActionList.indexOf( action ) != -1 )
  {
    mMapLayerActionList.removeAll( action );

    //also remove this action from the default layer action map
    QMap<QgsMapLayer*, QgsMapLayerAction*>::iterator defaultIt;
    for ( defaultIt = mDefaultLayerActionMap.begin(); defaultIt != mDefaultLayerActionMap.end(); ++defaultIt )
    {
      if ( defaultIt.value() == action )
      {
        defaultIt.value() = 0;
      }
    }
    emit changed();
    return true;
  }
  //not found
  return false;
}

void QgsMapLayerActionRegistry::setDefaultActionForLayer( QgsMapLayer* layer, QgsMapLayerAction* action )
{
  mDefaultLayerActionMap[ layer ] = action;
}

QgsMapLayerAction * QgsMapLayerActionRegistry::defaultActionForLayer( QgsMapLayer* layer )
{
  if ( !mDefaultLayerActionMap.contains( layer ) )
  {
    return 0;
  }

  return mDefaultLayerActionMap[ layer ];
}
