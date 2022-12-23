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
#include "qgshighlight.h"
#include "qgsmapcanvas.h"
#include "qgsactionmenu.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsgui.h"
#include "qgsexpressioncontextutils.h"
#include "qgsiconutils.h"
#include "qgsmapmouseevent.h"
#include "qgsmaplayeraction.h"
#include "qgsmaplayeractionregistry.h"

//TODO 4.0 add explicitly qobject parent to constructor
QgsIdentifyMenu::QgsIdentifyMenu( QgsMapCanvas *canvas )
  : QMenu( canvas )
  , mCanvas( canvas )
  , mAllowMultipleReturn( true )
  , mExecWithSingleResult( false )
  , mShowFeatureActions( false )
  , mResultsIfExternalAction( false )
  , mMaxLayerDisplay( 10 )
  , mMaxFeatureDisplay( 10 )
  , mDefaultActionName( tr( "Identify" ) )
{
}

QgsIdentifyMenu::~QgsIdentifyMenu()
{
  deleteRubberBands();
}

QList<QgsMapToolIdentify::IdentifyResult> QgsIdentifyMenu::findFeaturesOnCanvas( QgsMapMouseEvent *event, QgsMapCanvas *canvas, const QList<QgsWkbTypes::GeometryType> &geometryTypes )
{
  QList<QgsMapToolIdentify::IdentifyResult> results;
  const QMap< QString, QString > derivedAttributes;

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->toMapCoordinates( event->pos() ) ;
  const double x = mapPoint.x();
  const double y = mapPoint.y();
  const double sr = QgsMapTool::searchRadiusMU( canvas );

  const QList<QgsMapLayer *> layers = canvas->layers( true );
  for ( QgsMapLayer *layer : layers )
  {
    if ( layer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );

      bool typeIsSelectable = false;
      for ( QgsWkbTypes::GeometryType type : geometryTypes )
      {
        if ( vectorLayer->geometryType() == type )
        {
          typeIsSelectable = true;
          break;
        }
      }
      if ( typeIsSelectable )
      {
        QgsRectangle rect( x - sr, y - sr, x + sr, y + sr );
        QgsCoordinateTransform transform = canvas->mapSettings().layerTransform( vectorLayer );
        transform.setBallparkTransformsAreAppropriate( true );

        try
        {
          rect = transform.transformBoundingBox( rect, Qgis::TransformDirection::Reverse );
        }
        catch ( QgsCsException & )
        {
          QgsDebugMsg( QStringLiteral( "Could not transform geometry to layer CRS" ) );
        }

        QgsFeatureIterator fit = vectorLayer->getFeatures( QgsFeatureRequest()
                                 .setFilterRect( rect )
                                 .setFlags( QgsFeatureRequest::ExactIntersect ) );
        QgsFeature f;
        while ( fit.nextFeature( f ) )
        {
          results << QgsMapToolIdentify::IdentifyResult( vectorLayer, f, derivedAttributes );
        }
      }
    }
  }

  return results;
}

void QgsIdentifyMenu::setMaxLayerDisplay( int maxLayerDisplay )
{
  if ( maxLayerDisplay < 0 )
  {
    QgsDebugMsg( QStringLiteral( "invalid value for number of layers displayed." ) );
  }
  mMaxLayerDisplay = maxLayerDisplay;
}


void QgsIdentifyMenu::setMaxFeatureDisplay( int maxFeatureDisplay )
{
  if ( maxFeatureDisplay < 0 )
  {
    QgsDebugMsg( QStringLiteral( "invalid value for number of layers displayed." ) );
  }
  mMaxFeatureDisplay = maxFeatureDisplay;
}


QList<QgsMapToolIdentify::IdentifyResult> QgsIdentifyMenu::exec( const QList<QgsMapToolIdentify::IdentifyResult> &idResults, QPoint pos )
{
  clear();
  mLayerIdResults.clear();

  QList<QgsMapToolIdentify::IdentifyResult> returnResults = QList<QgsMapToolIdentify::IdentifyResult>();

  if ( idResults.isEmpty() )
  {
    return returnResults;
  }
  if ( idResults.count() == 1 && !mExecWithSingleResult )
  {
    returnResults << idResults[0];
    return returnResults;
  }

  // sort results by layer
  const auto constIdResults = idResults;
  for ( const QgsMapToolIdentify::IdentifyResult &result : constIdResults )
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
  const bool singleLayer = mLayerIdResults.count() == 1;
  int count = 0;
  QMapIterator< QgsMapLayer *, QList<QgsMapToolIdentify::IdentifyResult> > it( mLayerIdResults );
  while ( it.hasNext() )
  {
    if ( mMaxLayerDisplay != 0 && count > mMaxLayerDisplay )
      break;
    ++count;
    it.next();
    QgsMapLayer *layer = it.key();
    switch ( layer->type() )
    {
      case QgsMapLayerType::RasterLayer:
      {
        addRasterLayer( layer );
        break;
      }
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
        if ( !vl )
          continue;
        addVectorLayer( vl, it.value(), singleLayer );
        break;
      }

      case QgsMapLayerType::VectorTileLayer:
        // TODO: add support
        break;

      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }

  // add an "identify all" action on the top level
  if ( !singleLayer && mAllowMultipleReturn && idResults.count() > 1 )
  {
    addSeparator();
    QAction *allAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIdentify.svg" ) ), tr( "%1 All (%2)" ).arg( mDefaultActionName ).arg( idResults.count() ), this );
    allAction->setData( QVariant::fromValue<ActionData>( ActionData( nullptr ) ) );
    connect( allAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
    addAction( allAction );
  }

  // exec
  QAction *selectedAction = QMenu::exec( pos );
  bool externalAction;
  returnResults = results( selectedAction, externalAction );

  // delete actions
  clear();
  // also remove the QgsActionMenu
  qDeleteAll( findChildren<QgsActionMenu *>() );

  if ( externalAction && !mResultsIfExternalAction )
  {
    return QList<QgsMapToolIdentify::IdentifyResult>();
  }
  else
  {
    return returnResults;
  }
}

void QgsIdentifyMenu::closeEvent( QCloseEvent *e )
{
  deleteRubberBands();
  QMenu::closeEvent( e );
}

void QgsIdentifyMenu::addRasterLayer( QgsMapLayer *layer )
{
  QAction *layerAction = nullptr;
  QMenu *layerMenu = nullptr;

  QList<QgsMapLayerAction *> separators = QList<QgsMapLayerAction *>();
  QgsMapLayerActionContext context;
  QList<QgsMapLayerAction *> layerActions = mCustomActionRegistry.mapLayerActions( layer, Qgis::MapLayerActionTarget::Layer, context );
  const int nCustomActions = layerActions.count();
  if ( nCustomActions )
  {
    separators.append( layerActions[0] );
  }
  if ( mShowFeatureActions )
  {
    layerActions.append( QgsGui::mapLayerActionRegistry()->mapLayerActions( layer, Qgis::MapLayerActionTarget::Layer, context ) );
    if ( layerActions.count() > nCustomActions )
    {
      separators.append( layerActions[nCustomActions] );
    }
  }

  // use a menu only if actions will be listed
  if ( layerActions.isEmpty() )
  {
    layerAction = new QAction( layer->name(), this );
  }
  else
  {
    layerMenu = new QMenu( layer->name(), this );
    layerAction = layerMenu->menuAction();
  }

  // add layer action to the top menu
  layerAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconRasterLayer.svg" ) ) );
  layerAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
  connect( layerAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
  addAction( layerAction );

  // no need to go further if there is no menu
  if ( !layerMenu )
    return;

  // add default identify action
  QAction *identifyFeatureAction = new QAction( mDefaultActionName, layerMenu );
  connect( identifyFeatureAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
  identifyFeatureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
  layerMenu->addAction( identifyFeatureAction );

  // add custom/layer actions
  const auto constLayerActions = layerActions;
  for ( QgsMapLayerAction *mapLayerAction : constLayerActions )
  {
    QAction *action = new QAction( mapLayerAction->icon(), mapLayerAction->text(), layerMenu );
    action->setData( QVariant::fromValue<ActionData>( ActionData( layer, true ) ) );
    connect( action, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
    connect( action, &QAction::triggered, this, &QgsIdentifyMenu::triggerMapLayerAction );
    layerMenu->addAction( action );
    if ( separators.contains( mapLayerAction ) )
    {
      layerMenu->insertSeparator( action );
    }
  }
}

void QgsIdentifyMenu::addVectorLayer( QgsVectorLayer *layer, const QList<QgsMapToolIdentify::IdentifyResult> &results, bool singleLayer )
{
  QAction *layerAction = nullptr;
  QMenu *layerMenu = nullptr;

  // do not add actions with MultipleFeatures as target if only 1 feature is found for this layer
  // targets defines which actions will be shown
  const Qgis::MapLayerActionTargets targets = results.count() > 1 ? ( Qgis::MapLayerActionTarget::Layer | Qgis::MapLayerActionTarget::MultipleFeatures ) : Qgis::MapLayerActionTarget::Layer;

  QList<QgsMapLayerAction *> separators = QList<QgsMapLayerAction *>();
  QgsMapLayerActionContext actionContext;
  QList<QgsMapLayerAction *> layerActions = mCustomActionRegistry.mapLayerActions( layer, targets, actionContext );
  const int nCustomActions = layerActions.count();
  if ( nCustomActions )
  {
    separators << layerActions[0];
  }
  if ( mShowFeatureActions )
  {
    layerActions << QgsGui::mapLayerActionRegistry()->mapLayerActions( layer, targets, actionContext );

    if ( layerActions.count() > nCustomActions )
    {
      separators << layerActions[nCustomActions];
    }
  }

  // determines if a menu should be created or not. Following cases:
  // 1. only one result and no feature action to be shown => just create an action
  // 2. several features (2a) or display feature actions (2b) => create a menu
  // 3. case 2 but only one layer (singeLayer) => do not create a menu, but give the top menu instead

  bool createMenu = results.count() > 1 || !layerActions.isEmpty();

  // case 2b: still create a menu for layer, if there is a sub-level for features
  // i.e custom actions or map layer actions at feature level
  if ( !createMenu )
  {
    createMenu = !mCustomActionRegistry.mapLayerActions( layer, Qgis::MapLayerActionTarget::SingleFeature, actionContext ).isEmpty();
    if ( !createMenu && mShowFeatureActions )
    {
      QgsActionMenu *featureActionMenu = new QgsActionMenu( layer, results[0].mFeature, QStringLiteral( "Feature" ), this );
      featureActionMenu->setMode( QgsAttributeEditorContext::IdentifyMode );
      createMenu  = !featureActionMenu->actions().isEmpty();
      delete featureActionMenu;
    }
  }

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  QgsExpression exp( layer->displayExpression() );
  exp.prepare( &context );
  context.setFeature( results[0].mFeature );
  // use a menu only if actions will be listed
  if ( !createMenu )
  {
    // case 1
    QString featureTitle = exp.evaluate( &context ).toString();
    if ( featureTitle.isEmpty() )
      featureTitle = QString::number( results[0].mFeature.id() );
    layerAction = new QAction( QStringLiteral( "%1 (%2)" ).arg( layer->name(), featureTitle ), this );
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
      // case 2a
      if ( results.count() > 1 )
      {
        layerMenu = new QMenu( layer->name(), this );
      }
      // case 2b
      else
      {
        QString featureTitle = exp.evaluate( &context ).toString();
        if ( featureTitle.isEmpty() )
          featureTitle = QString::number( results[0].mFeature.id() );
        layerMenu = new QMenu( QStringLiteral( "%1 (%2)" ).arg( layer->name(), featureTitle ), this );
      }
      layerAction = layerMenu->menuAction();
    }
  }

  // case 1 or 2
  if ( layerAction )
  {
    layerAction->setIcon( QgsIconUtils::iconForWkbType( layer->wkbType() ) );

    // add layer action to the top menu
    layerAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
    connect( layerAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
    addAction( layerAction );
  }

  // case 1. no need to go further
  if ( !layerMenu )
    return;

  // add results to the menu
  int count = 0;
  const auto constResults = results;
  for ( const QgsMapToolIdentify::IdentifyResult &result : constResults )
  {
    if ( mMaxFeatureDisplay != 0 && count > mMaxFeatureDisplay )
      break;
    ++count;

    QAction *featureAction = nullptr;
    QMenu *featureMenu = nullptr;
    QgsActionMenu *featureActionMenu = nullptr;

    const QList<QgsMapLayerAction *> customFeatureActions = mCustomActionRegistry.mapLayerActions( layer, Qgis::MapLayerActionTarget::SingleFeature, actionContext );
    if ( mShowFeatureActions )
    {
      featureActionMenu = new QgsActionMenu( layer, result.mFeature, QStringLiteral( "Feature" ), layerMenu );
      featureActionMenu->setMode( QgsAttributeEditorContext::IdentifyMode );
      featureActionMenu->setExpressionContextScope( mExpressionContextScope );
    }

    // feature title
    context.setFeature( result.mFeature );
    QString featureTitle = exp.evaluate( &context ).toString();
    if ( featureTitle.isEmpty() )
      featureTitle = QString::number( result.mFeature.id() );

    if ( customFeatureActions.isEmpty() && ( !featureActionMenu || featureActionMenu->actions().isEmpty() ) )
    {
      featureAction = new QAction( featureTitle, layerMenu );
      // add the feature action (or menu) to the layer menu
      featureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id() ) ) );
      connect( featureAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
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
      connect( featureAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
      layerMenu->addAction( featureAction );
    }

    // if no feature menu, no need to go further
    if ( !featureMenu )
      continue;

    // add default identify action
    QAction *identifyFeatureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIdentify.svg" ) ), mDefaultActionName, featureMenu );
    connect( identifyFeatureAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
    identifyFeatureAction->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id() ) ) );
    featureMenu->addAction( identifyFeatureAction );
    featureMenu->addSeparator();

    // custom action at feature level
    const auto constCustomFeatureActions = customFeatureActions;
    for ( QgsMapLayerAction *mapLayerAction : constCustomFeatureActions )
    {
      QAction *action = new QAction( mapLayerAction->icon(), mapLayerAction->text(), featureMenu );
      action->setData( QVariant::fromValue<ActionData>( ActionData( layer, result.mFeature.id(), mapLayerAction ) ) );
      connect( action, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
      connect( action, &QAction::triggered, this, &QgsIdentifyMenu::triggerMapLayerAction );
      featureMenu->addAction( action );
    }
    // use QgsActionMenu for feature actions
    if ( featureActionMenu )
    {
      const auto constActions = featureActionMenu->actions();
      for ( QAction *action : constActions )
      {
        connect( action, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
        featureMenu->addAction( action );
      }
    }
  }

  // back to layer level

  // identify all action
  if ( mAllowMultipleReturn && results.count() > 1 )
  {
    layerMenu->addSeparator();
    QAction *allAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionIdentify.svg" ) ), tr( "%1 All (%2)" ).arg( mDefaultActionName ).arg( results.count() ), layerMenu );
    allAction->setData( QVariant::fromValue<ActionData>( ActionData( layer ) ) );
    connect( allAction, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
    layerMenu->addAction( allAction );
  }

  // add custom/layer actions
  const auto constLayerActions = layerActions;
  for ( QgsMapLayerAction *mapLayerAction : constLayerActions )
  {
    QString title = mapLayerAction->text();
    if ( mapLayerAction->targets().testFlag( Qgis::MapLayerActionTarget::MultipleFeatures ) )
      title.append( QStringLiteral( " (%1)" ).arg( results.count() ) );
    QAction *action = new QAction( mapLayerAction->icon(), title, layerMenu );
    action->setData( QVariant::fromValue<ActionData>( ActionData( layer, mapLayerAction ) ) );
    connect( action, &QAction::hovered, this, &QgsIdentifyMenu::handleMenuHover );
    connect( action, &QAction::triggered, this, &QgsIdentifyMenu::triggerMapLayerAction );
    layerMenu->addAction( action );
    if ( separators.contains( mapLayerAction ) )
    {
      layerMenu->insertSeparator( action );
    }
  }
}

void QgsIdentifyMenu::triggerMapLayerAction()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;
  const QVariant varData = action->data();
  if ( !varData.isValid() || !varData.canConvert<ActionData>() )
    return;

  ActionData actData = action->data().value<ActionData>();

  if ( actData.mIsValid && actData.mMapLayerAction )
  {
    QgsMapLayerActionContext context;

    // layer
    if ( actData.mMapLayerAction->targets().testFlag( Qgis::MapLayerActionTarget::Layer ) )
    {
      Q_NOWARN_DEPRECATED_PUSH
      actData.mMapLayerAction->triggerForLayer( actData.mLayer );
      Q_NOWARN_DEPRECATED_POP
      actData.mMapLayerAction->triggerForLayer( actData.mLayer, context );
    }

    // multiples features
    if ( actData.mMapLayerAction->targets().testFlag( Qgis::MapLayerActionTarget::MultipleFeatures ) )
    {
      QList<QgsFeature> featureList;
      const auto results { mLayerIdResults[actData.mLayer] };
      for ( const QgsMapToolIdentify::IdentifyResult &result : results )
      {
        featureList << result.mFeature;
      }
      Q_NOWARN_DEPRECATED_PUSH
      actData.mMapLayerAction->triggerForFeatures( actData.mLayer, featureList );
      Q_NOWARN_DEPRECATED_POP
      actData.mMapLayerAction->triggerForFeatures( actData.mLayer, featureList, context );
    }

    // single feature
    if ( actData.mMapLayerAction->targets().testFlag( Qgis::MapLayerActionTarget::SingleFeature ) )
    {
      const auto results { mLayerIdResults[actData.mLayer] };
      for ( const QgsMapToolIdentify::IdentifyResult &result : results )
      {
        if ( result.mFeature.id() == actData.mFeatureId )
        {
          Q_NOWARN_DEPRECATED_PUSH
          actData.mMapLayerAction->triggerForFeature( actData.mLayer, result.mFeature );
          Q_NOWARN_DEPRECATED_POP
          actData.mMapLayerAction->triggerForFeature( actData.mLayer, result.mFeature, context );
          return;
        }
      }
      QgsDebugMsg( QStringLiteral( "Identify menu: could not retrieve feature for action %1" ).arg( action->text() ) );
    }
  }
}


QList<QgsMapToolIdentify::IdentifyResult> QgsIdentifyMenu::results( QAction *action, bool &externalAction )
{
  QList<QgsMapToolIdentify::IdentifyResult> idResults = QList<QgsMapToolIdentify::IdentifyResult>();

  externalAction = false;

  ActionData actData;
  bool hasData = false;

  if ( !action )
    return idResults;

  const QVariant varData = action->data();
  if ( !varData.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Identify menu: could not retrieve results from menu entry (invalid data)" ) );
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
    const QgsActionMenu::ActionData dataSrc = action->data().value<QgsActionMenu::ActionData>();
    if ( dataSrc.actionType != Qgis::ActionType::Invalid )
    {
      externalAction = true;
      actData = ActionData( dataSrc.mapLayer, dataSrc.featureId );
      hasData = true;
    }
  }

  if ( !hasData )
  {
    QgsDebugMsg( QStringLiteral( "Identify menu: could not retrieve results from menu entry (no data found)" ) );
    return idResults;
  }

  // return all results
  if ( actData.mAllResults )
  {
    // this means "All" action was triggered
    QMapIterator< QgsMapLayer *, QList<QgsMapToolIdentify::IdentifyResult> > it( mLayerIdResults );
    while ( it.hasNext() )
    {
      it.next();
      idResults << it.value();
    }
    return idResults;
  }

  if ( !mLayerIdResults.contains( actData.mLayer ) )
  {
    QgsDebugMsg( QStringLiteral( "Identify menu: could not retrieve results from menu entry (layer not found)" ) );
    return idResults;
  }

  if ( actData.mLevel == LayerLevel )
  {
    return mLayerIdResults[actData.mLayer];
  }

  if ( actData.mLevel == FeatureLevel )
  {
    const auto results {mLayerIdResults[actData.mLayer]};
    for ( const QgsMapToolIdentify::IdentifyResult &res : results )
    {
      if ( res.mFeature.id() == actData.mFeatureId )
      {
        idResults << res;
        return idResults;
      }
    }
  }

  QgsDebugMsg( QStringLiteral( "Identify menu: could not retrieve results from menu entry (don't know what happened')" ) );
  return idResults;
}

void QgsIdentifyMenu::handleMenuHover()
{
  if ( !mCanvas )
    return;

  deleteRubberBands();

  QAction *senderAction = qobject_cast<QAction *>( sender() );
  if ( !senderAction )
    return;

  bool externalAction;
  const QList<QgsMapToolIdentify::IdentifyResult> idResults = results( senderAction, externalAction );

  const auto constIdResults = idResults;
  for ( const QgsMapToolIdentify::IdentifyResult &result : constIdResults )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( result.mLayer );
    if ( !vl )
      continue;

    QgsHighlight *hl = new QgsHighlight( mCanvas, result.mFeature.geometry(), vl );
    hl->applyDefaultStyle();
    mRubberBands.append( hl );
    connect( vl, &QObject::destroyed, this, &QgsIdentifyMenu::layerDestroyed );
  }
}

void QgsIdentifyMenu::styleHighlight( QgsHighlight *highlight )
{
  highlight->applyDefaultStyle();
}

void QgsIdentifyMenu::deleteRubberBands()
{
  QList<QgsHighlight *>::const_iterator it = mRubberBands.constBegin();
  for ( ; it != mRubberBands.constEnd(); ++it )
    delete *it;
  mRubberBands.clear();
}

void QgsIdentifyMenu::layerDestroyed()
{
  QList<QgsHighlight *>::iterator it = mRubberBands.begin();
  while ( it != mRubberBands.end() )
  {
    if ( ( *it )->layer() == sender() )
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

void QgsIdentifyMenu::setExpressionContextScope( const QgsExpressionContextScope &scope )
{
  mExpressionContextScope = scope;
}

QgsExpressionContextScope QgsIdentifyMenu::expressionContextScope() const
{
  return mExpressionContextScope;
}
