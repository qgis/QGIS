/***************************************************************************
    qgsmaptoolidentify.cpp  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsattributetabledialog.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifycontext.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsidentifymenu.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmaptoolselectionhandler.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsstatusbar.h"
#include "qgssettings.h"
#include "qgsmapmouseevent.h"
#include "qgslayertreeview.h"
#include "qgsmaplayeraction.h"
#include "qgsunittypes.h"

#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>

QgsMapToolIdentifyAction::QgsMapToolIdentifyAction( QgsMapCanvas *canvas )
  : QgsMapToolIdentify( canvas )
{
  mToolName = tr( "Identify" );
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify ) );
  connect( this, &QgsMapToolIdentify::changedRasterResults, this, &QgsMapToolIdentifyAction::handleChangedRasterResults );
  mIdentifyMenu->setAllowMultipleReturn( true );
  QgsMapLayerAction *attrTableAction = new QgsMapLayerAction( tr( "Show Attribute Table" ), mIdentifyMenu, Qgis::LayerType::Vector, Qgis::MapLayerActionTarget::MultipleFeatures );
  connect( attrTableAction, &QgsMapLayerAction::triggeredForFeaturesV2, this, &QgsMapToolIdentifyAction::showAttributeTable );
  identifyMenu()->addCustomAction( attrTableAction );
  mSelectionHandler = new QgsMapToolSelectionHandler( canvas, QgsMapToolSelectionHandler::SelectSimple );
  connect( mSelectionHandler, &QgsMapToolSelectionHandler::geometryChanged, this, &QgsMapToolIdentifyAction::identifyFromGeometry );
}

QgsMapToolIdentifyAction::~QgsMapToolIdentifyAction()
{
  if ( mResultsDialog )
  {
    mResultsDialog->done( 0 );
  }
  delete mSelectionHandler;
}

QgsIdentifyResultsDialog *QgsMapToolIdentifyAction::resultsDialog()
{
  if ( !mResultsDialog )
  {
    mResultsDialog = new QgsIdentifyResultsDialog( mCanvas, mCanvas->window() );

    connect( mResultsDialog.data(), static_cast<void ( QgsIdentifyResultsDialog::* )( QgsRasterLayer * )>( &QgsIdentifyResultsDialog::formatChanged ), this, &QgsMapToolIdentify::formatChanged );
    connect( mResultsDialog.data(), &QgsIdentifyResultsDialog::copyToClipboard, this, &QgsMapToolIdentifyAction::handleCopyToClipboard );
    connect( mResultsDialog.data(), &QgsIdentifyResultsDialog::selectionModeChanged, this, [this]
    {
      mSelectionHandler->setSelectionMode( mResultsDialog->selectionMode() );
    } );
  }

  return mResultsDialog;
}

void QgsMapToolIdentifyAction::showAttributeTable( QgsMapLayer *layer, const QList<QgsFeature> &featureList, const QgsMapLayerActionContext & )
{
  resultsDialog()->clear();

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vl || !vl->dataProvider() )
    return;

  QStringList idList;
  idList.reserve( featureList.size() );
  for ( const QgsFeature &feature : featureList )
  {
    idList.append( FID_TO_STRING( feature.id() ) );
  }
  const QString filter = QStringLiteral( "$id IN (%1)" ).arg( idList.join( ',' ) );

  QgsAttributeTableDialog *tableDialog = new QgsAttributeTableDialog( vl, QgsAttributeTableFilterModel::ShowFilteredList );
  tableDialog->setFilterExpression( filter );
  tableDialog->show();
}

void QgsMapToolIdentifyAction::identifyFromGeometry()
{
  resultsDialog()->clear();
  connect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

  const QgsGeometry geometry = mSelectionHandler->selectedGeometry();
  const bool isSinglePoint = geometry.type() == Qgis::GeometryType::Point;

  if ( isSinglePoint )
    setClickContextScope( geometry.asPoint() );

  identifyMenu()->setResultsIfExternalAction( false );

  // enable the right click for extended menu so it behaves as a contextual menu
  // this would be removed when a true contextual menu is brought in QGIS
  const bool extendedMenu = isSinglePoint && mShowExtendedMenu;
  identifyMenu()->setExecWithSingleResult( extendedMenu );
  identifyMenu()->setShowFeatureActions( extendedMenu );
  IdentifyMode mode = extendedMenu ? LayerSelection : DefaultQgsSetting;
  if ( mode == DefaultQgsSetting )
    mode = QgsSettings().enumValue( QStringLiteral( "Map/identifyMode" ), ActiveLayer );
  QList<QgsMapLayer *> layerList;
  if ( mode == ActiveLayer )
  {
    layerList = QgisApp::instance()->layerTreeView()->selectedLayersRecursive();
  }
  QgsIdentifyContext identifyContext;
  if ( mCanvas->mapSettings().isTemporal() )
    identifyContext.setTemporalRange( mCanvas->temporalRange() );
  identifyContext.setZRange( mCanvas->zRange() );
  const QList<IdentifyResult> results = QgsMapToolIdentify::identify( geometry, mode, layerList, AllLayers, identifyContext );

  disconnect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

  if ( results.isEmpty() )
  {
    resultsDialog()->clear();
    QgisApp::instance()->statusBarIface()->showMessage( tr( "No features found at this position." ), 2000 );
  }
  else
  {
    // Show the dialog before items are inserted so that items can resize themselves
    // according to dialog size also the first time, see also #9377
    if ( results.size() != 1 || !QgsSettings().value( QStringLiteral( "/Map/identifyAutoFeatureForm" ), false ).toBool() )
      resultsDialog()->QDialog::show();

    QList<IdentifyResult>::const_iterator result;
    for ( result = results.constBegin(); result != results.constEnd(); ++result )
    {
      resultsDialog()->addFeature( *result );
    }

    // Call QgsIdentifyResultsDialog::show() to adjust with items
    resultsDialog()->show();
  }

  // update possible view modes
  resultsDialog()->updateViewModes();
}

void QgsMapToolIdentifyAction::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasMoveEvent( e );
}

void QgsMapToolIdentifyAction::canvasPressEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasPressEvent( e );
}

void QgsMapToolIdentifyAction::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  mShowExtendedMenu = e->modifiers() == Qt::ShiftModifier || e->button() == Qt::RightButton;
  mSelectionHandler->canvasReleaseEvent( e );
  mShowExtendedMenu = false;
}

void QgsMapToolIdentifyAction::handleChangedRasterResults( QList<IdentifyResult> &results )
{
  // Add new result after raster format change
  QgsDebugMsgLevel( QStringLiteral( "%1 raster results" ).arg( results.size() ), 2 );
  QList<IdentifyResult>::const_iterator rresult;
  for ( rresult = results.constBegin(); rresult != results.constEnd(); ++rresult )
  {
    if ( rresult->mLayer->type() == Qgis::LayerType::Raster )
    {
      resultsDialog()->addFeature( *rresult );
    }
  }
}

void QgsMapToolIdentifyAction::activate()
{
  resultsDialog()->activate();
  QgsMapToolIdentify::activate();
}

void QgsMapToolIdentifyAction::deactivate()
{
  resultsDialog()->deactivate();
  mSelectionHandler->deactivate();
  QgsMapToolIdentify::deactivate();
}

void QgsMapToolIdentifyAction::identifyAndShowResults( const QgsGeometry &geom, double searchRadiusMapUnits )
{
  setCanvasPropertiesOverrides( searchRadiusMapUnits );
  mSelectionHandler->setSelectedGeometry( geom );
  restoreCanvasPropertiesOverrides();
}

void QgsMapToolIdentifyAction::clearResults()
{
  resultsDialog()->clear();
}

void QgsMapToolIdentifyAction::showResultsForFeature( QgsVectorLayer *vlayer, QgsFeatureId fid, const QgsPoint &pt )
{
  const QgsFeature feature = vlayer->getFeature( fid );
  const QMap< QString, QString > derivedAttributes = derivedAttributesForPoint( pt );
  // TODO: private in QgsMapToolIdentify
  //derivedAttributes.unite( featureDerivedAttributes( feature, vlayer, QgsPointXY( pt ) ) );

  resultsDialog()->addFeature( IdentifyResult( vlayer, feature, derivedAttributes ) );
  resultsDialog()->show();
  // update possible view modes
  resultsDialog()->updateViewModes();
}

Qgis::DistanceUnit QgsMapToolIdentifyAction::displayDistanceUnits() const
{
  Qgis::DistanceUnit units = QgsProject::instance()->distanceUnits();
  // unknown units used as a placeholder for map units
  if ( units == Qgis::DistanceUnit::Unknown )
  {
    return QgsProject::instance()->crs().mapUnits();
  }
  return units;
}

Qgis::AreaUnit QgsMapToolIdentifyAction::displayAreaUnits() const
{
  Qgis::AreaUnit units = QgsProject::instance()->areaUnits();
  // unknown units used as a placeholder for map units
  if ( units == Qgis::AreaUnit::Unknown )
  {
    return QgsUnitTypes::distanceToAreaUnit( QgsProject::instance()->crs().mapUnits() );
  }
  return units;
}

void QgsMapToolIdentifyAction::handleCopyToClipboard( QgsFeatureStore &featureStore )
{
  QgsDebugMsgLevel( QStringLiteral( "features count = %1" ).arg( featureStore.features().size() ), 2 );
  emit copyToClipboard( featureStore );
}

void QgsMapToolIdentifyAction::setClickContextScope( const QgsPointXY &point )
{
  QgsExpressionContextScope clickScope;
  clickScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "click_x" ), point.x(), true ) );
  clickScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "click_y" ), point.y(), true ) );

  resultsDialog()->setExpressionContextScope( clickScope );

  if ( mIdentifyMenu )
  {
    mIdentifyMenu->setExpressionContextScope( clickScope );
  }
}

void QgsMapToolIdentifyAction::keyReleaseEvent( QKeyEvent *e )
{
  if ( mSelectionHandler->keyReleaseEvent( e ) )
    return;

  QgsMapTool::keyReleaseEvent( e );
}

void QgsMapToolIdentifyAction::showIdentifyResults( const QList<IdentifyResult> &identifyResults )
{
  for ( const IdentifyResult &res : identifyResults )
  {
    resultsDialog()->addFeature( res );
  }
  resultsDialog()->show();
  // update possible view modes
  resultsDialog()->updateViewModes();
}
