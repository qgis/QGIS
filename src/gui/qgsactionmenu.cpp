/***************************************************************************
    qgsactionmenu.cpp
     --------------------------------------
    Date                 : 11.8.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsactionmenu.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsactionmanager.h"
#include "qgsfeatureiterator.h"
#include "qgsgui.h"

QgsActionMenu::QgsActionMenu( QgsVectorLayer *layer, const QgsFeature &feature, const QString &actionScope, QWidget  *parent )
  : QMenu( parent )
  , mLayer( layer )
  , mFeature( feature )
  , mFeatureId( feature.id() )
  , mActionScope( actionScope )
{
  init();
}

QgsActionMenu::QgsActionMenu( QgsVectorLayer *layer, const QgsFeatureId fid, const QString &actionScope, QWidget  *parent )
  : QMenu( parent )
  , mLayer( layer )
  , mFeatureId( fid )
  , mActionScope( actionScope )
{
  init();
}

void QgsActionMenu::init()
{
  setTitle( tr( "&Actions" ) );

  connect( QgsGui::mapLayerActionRegistry(), &QgsMapLayerActionRegistry::changed, this, &QgsActionMenu::reloadActions );
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsActionMenu::reloadActions );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsActionMenu::reloadActions );
  connect( mLayer, &QgsVectorLayer::readOnlyChanged, this, &QgsActionMenu::reloadActions );
  connect( mLayer, &QgsMapLayer::willBeDeleted, this, &QgsActionMenu::layerWillBeDeleted );

  reloadActions();
}

QgsFeature QgsActionMenu::feature()
{
  if ( !mFeature.isValid() )
  {
    mLayer->getFeatures( QgsFeatureRequest( mFeatureId ) ).nextFeature( mFeature );
  }

  return mFeature;
}

void QgsActionMenu::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

void QgsActionMenu::setMode( const QgsAttributeEditorContext::Mode mode )
{
  mMode = mode;
  reloadActions();
}

void QgsActionMenu::triggerAction()
{
  if ( !feature().isValid() )
    return;

  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  if ( !action->data().isValid() || !action->data().canConvert<ActionData>() )
    return;

  const ActionData data = action->data().value<ActionData>();

  if ( data.actionType == Invalid )
    return;

  if ( data.actionType == MapLayerAction )
  {
    QgsMapLayerAction *mapLayerAction = data.actionData.value<QgsMapLayerAction *>();
    mapLayerAction->triggerForFeature( data.mapLayer, mFeature );
  }
  else if ( data.actionType == AttributeAction )
  {
    // define custom substitutions: layer id and clicked coords
    QgsExpressionContext context = mLayer->createExpressionContext();
    context.setFeature( mFeature );

    QgsExpressionContextScope *actionScope = new QgsExpressionContextScope();
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "action_scope" ), mActionScope, true ) );
    context << actionScope;
    const QgsAction act = data.actionData.value<QgsAction>();
    act.run( context );
  }
}

void QgsActionMenu::reloadActions()
{
  clear();

  mActions = mLayer->actions()->actions( mActionScope );

  const auto constMActions = mActions;
  for ( const QgsAction &action : constMActions )
  {
    if ( !mLayer->isEditable() && action.isEnabledOnlyWhenEditable() )
      continue;

    if ( action.isEnabledOnlyWhenEditable() && ( mMode == QgsAttributeEditorContext::AddFeatureMode || mMode == QgsAttributeEditorContext::IdentifyMode ) )
      continue;

    QgsAction act( action );
    act.setExpressionContextScope( mExpressionContextScope );

    QAction *qAction = new QAction( action.icon(), action.name(), this );
    qAction->setData( QVariant::fromValue<ActionData>( ActionData( act, mFeatureId, mLayer ) ) );
    qAction->setIcon( action.icon() );

    // Only enable items on supported platforms
    if ( !action.runable() )
    {
      qAction->setEnabled( false );
      qAction->setToolTip( tr( "Not supported on your platform" ) );
    }
    else
    {
      qAction->setToolTip( action.command() );
    }
    connect( qAction, &QAction::triggered, this, &QgsActionMenu::triggerAction );
    addAction( qAction );
  }

  const QList<QgsMapLayerAction *> mapLayerActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( mLayer, QgsMapLayerAction::SingleFeature );

  if ( !mapLayerActions.isEmpty() )
  {
    //add a separator between user defined and standard actions
    addSeparator();

    for ( int i = 0; i < mapLayerActions.size(); ++i )
    {
      QgsMapLayerAction *qaction = mapLayerActions.at( i );

      if ( qaction->isEnabledOnlyWhenEditable() && ( mMode == QgsAttributeEditorContext::AddFeatureMode || mMode == QgsAttributeEditorContext::IdentifyMode ) )
        continue;

      QAction *qAction = new QAction( qaction->icon(), qaction->text(), this );
      qAction->setData( QVariant::fromValue<ActionData>( ActionData( qaction, mFeatureId, mLayer ) ) );
      addAction( qAction );
      connect( qAction, &QAction::triggered, this, &QgsActionMenu::triggerAction );
    }
  }

  emit reinit();
}

void QgsActionMenu::layerWillBeDeleted()
{
  // here we are just making sure that we are not going to have reloadActions() called again
  // with a dangling pointer to a layer when actions get removed on QGIS exit
  clear();
  mLayer = nullptr;
  disconnect( QgsGui::mapLayerActionRegistry(), &QgsMapLayerActionRegistry::changed, this, &QgsActionMenu::reloadActions );
}


QgsActionMenu::ActionData::ActionData( QgsMapLayerAction *action, QgsFeatureId featureId, QgsMapLayer *mapLayer )
  : actionType( MapLayerAction )
  , actionData( QVariant::fromValue<QgsMapLayerAction*>( action ) )
  , featureId( featureId )
  , mapLayer( mapLayer )
{}


QgsActionMenu::ActionData::ActionData( const QgsAction &action, QgsFeatureId featureId, QgsMapLayer *mapLayer )
  : actionType( AttributeAction )
  , actionData( QVariant::fromValue<QgsAction>( action ) )
  , featureId( featureId )
  , mapLayer( mapLayer )
{}


void QgsActionMenu::setExpressionContextScope( const QgsExpressionContextScope &scope )
{
  mExpressionContextScope = scope;
  reloadActions();
}

QgsExpressionContextScope QgsActionMenu::expressionContextScope() const
{
  return mExpressionContextScope;
}

QList<QgsAction> QgsActionMenu::menuActions()
{
  return mActions;
}
