/***************************************************************************
    qgsidentifymenu.cpp  -  menu to be used in identify map tool
    ---------------------
    begin                : August 2014
    copyright            : (C) 2014 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMouseEvent>

#include "qgsidentifymenu.h"

#include "qgsapplication.h"
#include "qgsattributeaction.h"
#include "qgshighlight.h"

QgsIdentifyMenu::CustomActionRegistry::CustomActionRegistry( QObject* parent )
    : QgsMapLayerActionRegistry( parent )
{
}


QgsIdentifyMenu::QgsIdentifyMenu( QgsMapCanvas* canvas )
    : QMenu( canvas )
    , mCanvas( canvas )
    , mAllowMultipleReturn( true )
    , mExecWithSingleResult( false )
    , mResultsIfExternalAction( false )
    , mMaxLayerDisplay( 10 )
    , mMaxFeatureDisplay( 10 )
    , mDefaultActionName( tr( "Identify" ) )
    , mCustomActionRegistry( CustomActionRegistry::instance() )
{
}

QgsIdentifyMenu::~QgsIdentifyMenu()
{
  deleteRubberBands();
}


void QgsIdentifyMenu::setMaxLayerDisplay( int maxLayerDisplay )
{
  if ( maxLayerDisplay < 0 )
  {
    QgsDebugMsg( "invalid value for number of layers displayed." );
  }
  mMaxLayerDisplay = maxLayerDisplay;
}


void QgsIdentifyMenu::setMaxFeatureDisplay( int maxFeatureDisplay )
{
  if ( maxFeatureDisplay < 0 )
  {
    QgsDebugMsg( "invalid value for number of layers displayed." );
  }
  mMaxFeatureDisplay = maxFeatureDisplay;
}


QList<QgsMapToolIdentify::IdentifyResult> QgsIdentifyMenu::exec( const QList<QgsMapToolIdentify::IdentifyResult> idResults, QPoint pos )
{
  clear();
  mLayerIdResults.clear();

  QList<QgsMapToolIdentify::IdentifyResult> returnResults = QList<QgsMapToolIdentify::IdentifyResult>();

  if ( idResults.count() == 0 )
  {
    return returnResults;
  }
  if ( idResults.count() == 1 && !mExecWithSingleResult )
  {
    returnResults << idResults[0];
    return returnResults;
  }

  // sort results by layer
  Q_FOREACH ( const QgsMapToolIdentify::IdentifyResult result, idResults )
  {
    QgsMapLayer *layer = result.mLayer;
    if ( mLayerIdResults.contains( layer ) )
    {
      mLayerIdResults[layer].append( result );
    }
    else
    {
      mLayerIdResults.insert( layer, QList<QgsMapToolIdentify::IdentifyResult>() << result );
    }
  }

  // add results to the menu
  bool singleLayer = mLayerIdResults.count() == 1;
  int count = 0;
  QMapIterator< QgsMapLayer*, QList<QgsMapToolIdentify::IdentifyResult> > it( mLayerIdResults );
  while ( it.hasNext() )
  {
    if ( mMaxLayerDisplay != 0 && count > mMaxLayerDisplay )
      break;
    ++count;
    it.next();
    QgsMapLayer* layer = it.key();
    if ( layer->type() == QgsMapLayer::RasterLayer )
    {
      addRasterLayer( layer );
    }
    else if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer );
      if ( !vl )
        continue;
      addVectorLayer( vl, it.value(), singleLayer );
    }
  }

  // add an "identify all" action on the top level
  if ( !singleLayer && mAllowMultipleReturn && idResults.count() > 1 )
  {
    addSeparator();
    QAction* allAction = new QAction( QgsApplication::getThemeIcon( "/mActionIdentify.svg" ), tr( "%1 all (%2)" ).arg( mDefaultActionName ).arg( idResults.count() ), this );
    allAction->setData( QVariant::fromValue<ActionData>( ActionData( 0 ) ) );
    connect( allAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
    addAction( allAction );
  }

  // exec
  QAction* selectedAction = QMenu::exec( pos );
  bool externalAction;
  returnResults = results( selectedAction, externalAction );

  // delete actions
  clear();
  // also remove the QgsActionMenu
  qDeleteAll( findChildren<QgsActionMenu*>() );

  if ( externalAction && !mResultsIfExternalAction )
  {
    return QList<QgsMapToolIdentify::IdentifyResult>();
  }
  else
  {
    return returnResults;
  }
}

void QgsIdentifyMenu::closeEvent( QCloseEvent* e )
{
  deleteRubberBands();
  QMenu::closeEvent( e );
}

void QgsIdentifyMenu::addRasterLayer( QgsMapLayer* layer )
{
  QAction* layerAction;
  QMenu* layerMenu = 0;

  QList<QgsMapLayerAction*> separators = QList<QgsMapLayerAction*>();
  QList<QgsMapLayerAction*> layerActions = mCustomActionRegistry.mapLayerActions( layer, QgsMapLayerAction::Layer );
  int nCustomActions = layerActions.count();
  if ( nCustomActions )
  {
    separators.append( layerActions[0] );
  }
  if ( mShowFeatureActions )
  {
    layerActions.append( QgsMapLayerActionRegistry::instance()->mapLayerActions( layer, QgsMapLayerAction::Layer ) );
    if ( layerActions.count() > nCustomActions )
    {
      separators.append( layerActions[nCustomActions] );
    }
  }

  // use a menu only if actions will be listed
  if ( !layerActions.count() )
  {
    layerAction = new QAction( layer->name(), this );
  }
  else
  {
    layerMenu = new QMenu( layer->name(), this );
    layerAction = layerMenu->menuAction();
  }

  // add layer action to the top menu
  layerAction->setIcon( QgsApplication::getThemeIcon( "/mIconRasterLayer.png" ) );
  layerAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
  connect( layerAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
  addAction( layerAction );

  // no need to go further if there is no menu
  if ( !layerMenu )
    return;

  // add default identify action
  QAction* identifyFeatureAction = new QAction( mDefaultActionName, layerMenu );
  connect( identifyFeatureAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
  identifyFeatureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
  layerMenu->addAction( identifyFeatureAction );

  // add custom/layer actions
  Q_FOREACH ( QgsMapLayerAction* mapLayerAction, layerActions )
  {
    QAction* action = new QAction( mapLayerAction->icon(), mapLayerAction->text(), layerMenu );
    action->setData( QVariant::fromValue<ActionData>( ActionData( layer, true ) ) );
    connect( action, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( triggerMapLayerAction() ) );
    layerMenu->addAction( action );
    if ( separators.contains( mapLayerAction ) )
    {
      layerMenu->insertSeparator( action );
    }
  }
}

void QgsIdentifyMenu::addVectorLayer( QgsVectorLayer* layer, const QList<QgsMapToolIdentify::IdentifyResult> results, bool singleLayer )
{
  QAction* layerAction = 0;
  QMenu* layerMenu = 0;

  // do not add actions with MultipleFeatures as target if only 1 feature is found for this layer
  // targets defines which actions will be shown
  QgsMapLayerAction::Targets targets = results.count() > 1 ? QgsMapLayerAction::Layer | QgsMapLayerAction::MultipleFeatures : QgsMapLayerAction::Layer;

  QList<QgsMapLayerAction*> separators = QList<QgsMapLayerAction*>();
  QList<QgsMapLayerAction*> layerActions = mCustomActionRegistry.mapLayerActions( layer, targets );
  int nCustomActions = layerActions.count();
  if ( nCustomActions )
  {
    separators << layerActions[0];
  }
  if ( mShowFeatureActions )
  {
    layerActions << QgsMapLayerActionRegistry::instance()->mapLayerActions( layer, targets );

    if ( layerActions.count() > nCustomActions )
    {
      separators << layerActions[nCustomActions];
    }
  }

  // determines if a menu should be created or not. Following cases:
  // 1. only one result and no feature action to be shown => just create an action
  // 2. several features (2a) or display feature actions (2b) => create a menu
  // 3. case 2 but only one layer (singeLayer) => do not create a menu, but give the top menu instead

  bool createMenu = results.count() > 1 || layerActions.count() > 0;

  // case 2b: still create a menu for layer, if there is a sub-level for features
  // i.e custom actions or map layer actions at feature level
  if ( !createMenu )
  {
    createMenu = mCustomActionRegistry.mapLayerActions( layer, QgsMapLayerAction::SingleFeature ).count() > 0;
    if ( !createMenu && mShowFeatureActions )
    {
      QgsActionMenu* featureActionMenu = new QgsActionMenu( layer, &( results[0].mFeature ), this );
      createMenu  = featureActionMenu->actions().count() > 0;
      delete featureActionMenu;
    }
  }

  // use a menu only if actions will be listed
  if ( !createMenu )
  {
    // case 1
    layerAction = new QAction( layer->name(), this );
  }
  else
  {
    if ( singleLayer )
    {
      // case 3
      layerMenu = this;
    }
    else
    {
      // case 2
      layerMenu = new QMenu( layer->name(), this );
      layerAction = layerMenu->menuAction();
    }
  }

  // case 1 or 2
  if ( layerAction )
  {
    // icons
    switch ( layer->geometryType() )
    {
      case QGis::Point:
        layerAction->setIcon( QgsApplication::getThemeIcon( "/mIconPointLayer.png" ) );
        break;
      case QGis::Line:
        layerAction->setIcon( QgsApplication::getThemeIcon( "/mIconLineLayer.png" ) );
        break;
      case QGis::Polygon:
        layerAction->setIcon( QgsApplication::getThemeIcon( "/mIconPolygonLayer.png" ) );
        break;
      default:
        break;
    }

    // add layer action to the top menu
    layerAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
    connect( layerAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
    addAction( layerAction );
  }

  // case 1. no need to go further
  if ( !layerMenu )
    return;

  // add results to the menu
  int count = 0;
  Q_FOREACH ( const QgsMapToolIdentify::IdentifyResult result, results )
  {
    if ( mMaxFeatureDisplay != 0 && count > mMaxFeatureDisplay )
      break;
    ++count;

    QAction* featureAction = 0;
    QMenu* featureMenu = 0;
    QgsActionMenu* featureActionMenu = 0;

    QList<QgsMapLayerAction*> customFeatureActions = mCustomActionRegistry.mapLayerActions( layer, QgsMapLayerAction::SingleFeature );
    if ( mShowFeatureActions )
    {
      featureActionMenu = new QgsActionMenu( layer, result.mFeature.id(), layerMenu );
    }

    // feature title
    QString featureTitle = result.mFeature.attribute( layer->displayField() ).toString();
    if ( featureTitle.isEmpty() )
      featureTitle = QString( "%1" ).arg( result.mFeature.id() );

    if ( !customFeatureActions.count() && ( !featureActionMenu || !featureActionMenu->actions().count() ) )
    {
      featureAction = new QAction( featureTitle, layerMenu );
      // add the feature action (or menu) to the layer menu
      featureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id() ) ) );
      connect( featureAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
      layerMenu->addAction( featureAction );
    }
    else if ( results.count() == 1 )
    {
      // if we are here with only one results, this means there is a sub-feature level (for actions)
      // => skip the feature level since there would be only a single entry
      // => give the layer menu as pointer instead of a new feature menu
      featureMenu = layerMenu;
    }
    else
    {
      featureMenu = new QMenu( featureTitle, layerMenu );

      // get the action from the menu
      featureAction = featureMenu->menuAction();
      // add the feature action (or menu) to the layer menu
      featureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id() ) ) );
      connect( featureAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
      layerMenu->addAction( featureAction );
    }

    // if no feature menu, no need to go further
    if ( !featureMenu )
      continue;

    // add default identify action
    QAction* identifyFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionIdentify.svg" ), mDefaultActionName, featureMenu );
    connect( identifyFeatureAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
    identifyFeatureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id() ) ) );
    featureMenu->addAction( identifyFeatureAction );
    featureMenu->addSeparator();

    // custom action at feature level
    Q_FOREACH ( QgsMapLayerAction* mapLayerAction, customFeatureActions )
    {
      QAction* action = new QAction( mapLayerAction->icon(), mapLayerAction->text(), featureMenu );
      action->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id(), mapLayerAction ) ) );
      connect( action, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
      connect( action, SIGNAL( triggered() ), this, SLOT( triggerMapLayerAction() ) );
      featureMenu->addAction( action );
    }
    // use QgsActionMenu for feature actions
    if ( featureActionMenu )
    {
      Q_FOREACH ( QAction* action, featureActionMenu->actions() )
      {
        connect( action, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
        featureMenu->addAction( action );
      }
    }
  }

  // back to layer level

  // identify all action
  if ( mAllowMultipleReturn && results.count() > 1 )
  {
    layerMenu->addSeparator();
    QAction* allAction = new QAction( QgsApplication::getThemeIcon( "/mActionIdentify.svg" ), tr( "%1 all (%2)" ).arg( mDefaultActionName ).arg( results.count() ), layerMenu );
    allAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
    connect( allAction, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
    layerMenu->addAction( allAction );
  }

  // add custom/layer actions
  Q_FOREACH ( QgsMapLayerAction* mapLayerAction, layerActions )
  {
    QString title = mapLayerAction->text();
    if ( mapLayerAction->targets().testFlag( QgsMapLayerAction::MultipleFeatures ) )
      title.append( QString( " (%1)" ).arg( results.count() ) );
    QAction* action = new QAction( mapLayerAction->icon(), title, layerMenu );
    action->setData( QVariant::fromValue<ActionData>( ActionData( layer, mapLayerAction ) ) );
    connect( action, SIGNAL( hovered() ), this, SLOT( handleMenuHover() ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( triggerMapLayerAction() ) );
    layerMenu->addAction( action );
    if ( separators.contains( mapLayerAction ) )
    {
      layerMenu->insertSeparator( action );
    }
  }
}

void QgsIdentifyMenu::triggerMapLayerAction()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  if ( !action )
    return;
  QVariant varData = action->data();
  if ( !varData.isValid() || !varData.canConvert<ActionData>() )
    return;

  ActionData actData = action->data().value<ActionData>();

  if ( actData.mIsValid && actData.mMapLayerAction )
  {
    // layer
    if ( actData.mMapLayerAction->targets().testFlag( QgsMapLayerAction::Layer ) )
    {
      actData.mMapLayerAction->triggerForLayer( actData.mLayer );
    }

    // multiples features
    if ( actData.mMapLayerAction->targets().testFlag( QgsMapLayerAction::MultipleFeatures ) )
    {
      QList<QgsFeature> featureList;
      Q_FOREACH ( QgsMapToolIdentify::IdentifyResult result, mLayerIdResults[actData.mLayer] )
      {
        featureList << result.mFeature;
      }
      actData.mMapLayerAction->triggerForFeatures( actData.mLayer, featureList );
    }

    // single feature
    if ( actData.mMapLayerAction->targets().testFlag( QgsMapLayerAction::SingleFeature ) )
    {
      Q_FOREACH ( QgsMapToolIdentify::IdentifyResult result, mLayerIdResults[actData.mLayer] )
      {
        if ( result.mFeature.id() == actData.mFeatureId )
        {
          actData.mMapLayerAction->triggerForFeature( actData.mLayer, new QgsFeature( result.mFeature ) );
          return;
        }
      }
      QgsDebugMsg( QString( "Identify menu: could not retrieve feature for action %1" ).arg( action->text() ) );
    }
  }
}


QList<QgsMapToolIdentify::IdentifyResult> QgsIdentifyMenu::results( QAction* action, bool &externalAction )
{
  QList<QgsMapToolIdentify::IdentifyResult> idResults = QList<QgsMapToolIdentify::IdentifyResult>();

  externalAction = false;

  ActionData actData;
  bool hasData = false;

  if ( !action )
    return idResults;

  QVariant varData = action->data();
  if ( !varData.isValid() )
  {
    QgsDebugMsg( "Identify menu: could not retrieve results from menu entry (invalid data)" );
    return idResults;
  }

  if ( varData.canConvert<ActionData>() )
  {
    actData = action->data().value<ActionData>();
    if ( actData.mIsValid )
    {
      externalAction = actData.mIsExternalAction;
      hasData = true;
    }
  }

  if ( !hasData && varData.canConvert<QgsActionMenu::ActionData>() )
  {
    QgsActionMenu::ActionData dataSrc = action->data().value<QgsActionMenu::ActionData>();
    if ( dataSrc.actionType != QgsActionMenu::Invalid )
    {
      externalAction = true;
      actData = ActionData( dataSrc.mapLayer, dataSrc.featureId );
      hasData = true;
    }
  }

  if ( !hasData )
  {
    QgsDebugMsg( "Identify menu: could not retrieve results from menu entry (no data found)" );
    return idResults;
  }

  // return all results
  if ( actData.mAllResults )
  {
    // this means "All" action was triggered
    QMapIterator< QgsMapLayer*, QList<QgsMapToolIdentify::IdentifyResult> > it( mLayerIdResults );
    while ( it.hasNext() )
    {
      it.next();
      idResults << it.value();
    }
    return idResults;
  }

  if ( !mLayerIdResults.contains( actData.mLayer ) )
  {
    QgsDebugMsg( "Identify menu: could not retrieve results from menu entry (layer not found)" );
    return idResults;
  }

  if ( actData.mLevel == LayerLevel )
  {
    return mLayerIdResults[actData.mLayer];
  }

  if ( actData.mLevel == FeatureLevel )
  {
    Q_FOREACH ( QgsMapToolIdentify::IdentifyResult res, mLayerIdResults[actData.mLayer] )
    {
      if ( res.mFeature.id() == actData.mFeatureId )
      {
        idResults << res;
        return idResults;
      }
    }
  }

  QgsDebugMsg( "Identify menu: could not retrieve results from menu entry (don't know what happened')" );
  return idResults;
}

void QgsIdentifyMenu::handleMenuHover()
{
  if ( !mCanvas )
    return;

  deleteRubberBands();

  QAction* senderAction = qobject_cast<QAction*>( sender() );
  if ( !senderAction )
    return;

  bool externalAction;
  QList<QgsMapToolIdentify::IdentifyResult> idResults = results( senderAction, externalAction );

  Q_FOREACH ( const QgsMapToolIdentify::IdentifyResult result, idResults )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( result.mLayer );
    if ( !vl )
      continue;
    QgsHighlight *hl = new QgsHighlight( mCanvas, result.mFeature.geometry(), vl );
    hl->setColor( QColor( 255, 0, 0 ) );
    hl->setWidth( 2 );
    mRubberBands.append( hl );
    connect( vl, SIGNAL( destroyed() ), this, SLOT( layerDestroyed() ) );
  }
}

void QgsIdentifyMenu::deleteRubberBands()
{
  QList<QgsHighlight*>::const_iterator it = mRubberBands.constBegin();
  for ( ; it != mRubberBands.constEnd(); ++it )
    delete *it;
  mRubberBands.clear();
}

void QgsIdentifyMenu::layerDestroyed()
{
  QList<QgsHighlight*>::iterator it = mRubberBands.begin();
  while ( it != mRubberBands.end() )
  {
    if (( *it )->layer() == sender() )
    {
      delete *it;
      it = mRubberBands.erase( it );
    }
    else
    {
      ++it;
    }
  }
}

void QgsIdentifyMenu::removeCustomActions()
{
  mCustomActionRegistry.clear();

}
