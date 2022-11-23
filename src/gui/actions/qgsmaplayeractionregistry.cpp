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
#include "qgsmaplayeraction.h"

QgsMapLayerActionRegistry::QgsMapLayerActionRegistry( QObject *parent ) : QObject( parent )
{

}

void QgsMapLayerActionRegistry::addMapLayerAction( QgsMapLayerAction *action )
{
  mMapLayerActionList.append( action );
  emit changed();
}

QList< QgsMapLayerAction * > QgsMapLayerActionRegistry::mapLayerActions( QgsMapLayer *layer, Qgis::MapLayerActionTargets targets, const QgsMapLayerActionContext &context )
{
  QList< QgsMapLayerAction * > validActions;

  for ( QgsMapLayerAction *action : std::as_const( mMapLayerActionList ) )
  {
    bool canRun = false;
    Q_NOWARN_DEPRECATED_PUSH
    canRun = action->canRunUsingLayer( layer );
    Q_NOWARN_DEPRECATED_POP
    if ( !canRun )
      canRun = action->canRunUsingLayer( layer, context );

    if ( canRun && ( targets & action->targets() ) )
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

