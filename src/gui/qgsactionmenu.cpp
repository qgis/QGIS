/***************************************************************************
    qgsactionmenu.cpp
     --------------------------------------
    Date                 : 11.8.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

#include <QMenuItem>

QgsActionMenu::QgsActionMenu( QgsVectorLayer* layer, const QgsFeature* feature, QWidget*  parent )
    : QMenu( parent )
    , mLayer( layer )
    , mAttributeActionSignalMapper( 0 )
    , mMapLayerActionSignalMapper( 0 )
    , mActions( 0 )
    , mFeature( feature )
    , mFeatureId( feature->id() )
    , mOwnsFeature( false )
{
  init();
}

QgsActionMenu::QgsActionMenu( QgsVectorLayer* layer, const QgsFeatureId fid, QWidget*  parent )
  : QMenu( parent )
  , mLayer( layer )
  , mAttributeActionSignalMapper( 0 )
  , mMapLayerActionSignalMapper( 0 )
  , mActions( 0 )
  , mFeature( 0 )
  , mFeatureId( fid )
  , mOwnsFeature( false )
{
  init();
}

void QgsActionMenu::init()
{
  setTitle( tr( "&Actions" ) );

  connect( QgsMapLayerActionRegistry::instance(), SIGNAL(changed()), this, SLOT(reloadActions()) );

  reloadActions();
}

const QgsFeature* QgsActionMenu::feature()
{
  if ( !mFeature )
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

void QgsActionMenu::triggerAttributeAction( int index )
{
  if ( feature() )
  {
    mActions->doAction( index, *feature() );
  }
  else
  {
    QgsDebugMsg( QString( "Trying to run an action on a non-existing feature with fid %1" ).arg( mFeatureId ) );
  }
}

void QgsActionMenu::triggerMapLayerAction( int index )
{
  if ( feature() )
  {
    QgsMapLayerAction* action = QgsMapLayerActionRegistry::instance()->mapLayerActions( mLayer ).at( index );

    action->triggerForFeature( mLayer, feature() );
  }
}

void QgsActionMenu::reloadActions()
{
  delete mAttributeActionSignalMapper;
  mAttributeActionSignalMapper = new QSignalMapper( this );
  delete mMapLayerActionSignalMapper;
  mMapLayerActionSignalMapper = new QSignalMapper( this );

  connect( mAttributeActionSignalMapper, SIGNAL(mapped(int)), this, SLOT(triggerAttributeAction(int)) );
  connect( mMapLayerActionSignalMapper, SIGNAL(mapped(int)), this, SLOT(triggerMapLayerAction(int)) );

  delete mActions;
  mActions = new QgsAttributeAction( *mLayer->actions() );

  for( int idx = 0; idx < mActions->size(); ++idx )
  {
    const QgsAction& qaction( mActions->at( idx ) );

    QAction* action = new QAction( qaction.icon(), qaction.name(), this );
    action->setData( QVariant::fromValue<ActionData>( ActionData( idx, mFeatureId, mLayer ) ) );

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

    mAttributeActionSignalMapper->setMapping( action, idx );

    connect( action, SIGNAL(triggered()), mAttributeActionSignalMapper, SLOT(map()) );

    addAction( action );
  }

  QList<QgsMapLayerAction*> mapLayerActions = QgsMapLayerActionRegistry::instance()->mapLayerActions( mLayer );

  if ( mapLayerActions.size() > 0 )
  {
    //add a separator between user defined and standard actions
    addSeparator();

    for ( int i = 0; i < mapLayerActions.size(); ++i )
    {
      QgsMapLayerAction* qaction = mapLayerActions.at( i );
      QAction* action = new QAction( qaction->text(), this );
      action->setData( QVariant::fromValue<ActionData>( ActionData( MapLayerAction, mFeatureId, mLayer ) ) );
      mMapLayerActionSignalMapper->setMapping( action, i );
      addAction( action );
      connect( action, SIGNAL(triggered()), mMapLayerActionSignalMapper, SLOT(map()) );
    }
  }

  emit reinit();
}

