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

QgsActionMenu::QgsActionMenu( QgsVectorLayer* layer, const QgsFeature* feature, QWidget*  parent )
    : QMenu( parent )
    , mLayer( layer )
    , mActions( nullptr )
    , mFeature( feature )
    , mFeatureId( feature->id() )
    , mOwnsFeature( false )
{
  init();
}

QgsActionMenu::QgsActionMenu( QgsVectorLayer* layer, const QgsFeatureId fid, QWidget*  parent )
    : QMenu( parent )
    , mLayer( layer )
    , mActions( nullptr )
    , mFeature( nullptr )
    , mFeatureId( fid )
    , mOwnsFeature( false )
{
  init();
}

void QgsActionMenu::init()
{
  setTitle( tr( "&Actions" ) );

  connect( QgsMapLayerActionRegistry::instance(), SIGNAL( changed() ), this, SLOT( reloadActions() ) );

  reloadActions();
}

const QgsFeature* QgsActionMenu::feature()
{
  if ( !mFeature || !mFeature->isValid() )
  {
    QgsFeature* feat = new QgsFeature();
    if ( mActions->layer()->getFeatures( QgsFeatureRequest( mFeatureId ) ).nextFeature( *feat ) )
    {
      mFeature = feat;
      mOwnsFeature = true;
    }
    else
    {
      delete feat;
    }
  }

  return mFeature;
}

QgsActionMenu::~QgsActionMenu()
{
  delete mActions;

  if ( mOwnsFeature )
    delete mFeature;
}

void QgsActionMenu::setFeature( QgsFeature* feature )
{
  if ( mOwnsFeature )
    delete mFeature;
  mOwnsFeature = false;
  mFeature = feature;
}

void QgsActionMenu::triggerAction()
{
  if ( !feature() )
    return;

  QAction* action = qobject_cast<QAction*>( sender() );
  if ( !action )
    return;

  if ( !action->data().isValid() || !action->data().canConvert<ActionData>() )
    return;

  ActionData data = action->data().value<ActionData>();

  if ( data.actionType == Invalid )
    return;

  if ( data.actionType == MapLayerAction )
  {
    QgsMapLayerAction* mapLayerAction = data.actionId.action;
    mapLayerAction->triggerForFeature( data.mapLayer, feature() );
  }
  else if ( data.actionType == AttributeAction )
  {
    mActions->doAction( data.actionId.id, *feature() );
  }
}

void QgsActionMenu::reloadActions()
{
  clear();

  delete mActions;
  mActions = new QgsActionManager( *mLayer->actions() );

  for ( int idx = 0; idx < mActions->size(); ++idx )
  {
    const QgsAction& qaction( mActions->at( idx ) );

    QAction* action = new QAction( qaction.icon(), qaction.name(), this );
    action->setData( QVariant::fromValue<ActionData>( ActionData( idx, mFeatureId, mLayer ) ) );
    action->setIcon( qaction.icon() );

    // Only enable items on supported platforms
    if ( !qaction.runable() )
    {
      action->setEnabled( false );
      action->setToolTip( tr( "Not supported on your platform" ) );
    }
    else
    {
      action->setToolTip( qaction.action() );
    }
    connect( action, SIGNAL( triggered() ), this, SLOT( triggerAction() ) );
    addAction( action );
  }

  QList<QgsMapLayerAction*> mapLayerActions = QgsMapLayerActionRegistry::instance()->mapLayerActions( mLayer, QgsMapLayerAction::SingleFeature );

  if ( !mapLayerActions.isEmpty() )
  {
    //add a separator between user defined and standard actions
    addSeparator();

    for ( int i = 0; i < mapLayerActions.size(); ++i )
    {
      QgsMapLayerAction* qaction = mapLayerActions.at( i );
      QAction* action = new QAction( qaction->icon(), qaction->text(), this );
      action->setData( QVariant::fromValue<ActionData>( ActionData( qaction, mFeatureId, mLayer ) ) );
      addAction( action );
      connect( action, SIGNAL( triggered() ), this, SLOT( triggerAction() ) );
    }
  }

  emit reinit();
}

