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
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsidentifymenu.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrasterlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsrubberband.h"
#include "qgsunittypes.h"
#include "qgsstatusbar.h"
#include "qgsactionscoperegistry.h"

#include "qgssettings.h"
#include <QMouseEvent>
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
  QgsMapLayerAction *attrTableAction = new QgsMapLayerAction( tr( "Show attribute table" ), mIdentifyMenu, QgsMapLayer::VectorLayer, QgsMapLayerAction::MultipleFeatures );
  connect( attrTableAction, &QgsMapLayerAction::triggeredForFeatures, this, &QgsMapToolIdentifyAction::showAttributeTable );
  identifyMenu()->addCustomAction( attrTableAction );

  mFillColor = QColor( 254, 178, 76, 63 );
  mStrokeColor = QColor( 254, 58, 29, 100 );
  mSelectionMode = QgsMapToolIdentifyAction::SelectSimple;

  mSelectionHandler = new QgsMapToolSelectionHandler(canvas);
}

QgsMapToolIdentifyAction::~QgsMapToolIdentifyAction()
{
  if ( mResultsDialog )
  {
    mResultsDialog->done( 0 );
  }
}

QgsIdentifyResultsDialog *QgsMapToolIdentifyAction::resultsDialog()
{
  if ( !mResultsDialog )
  {
    mResultsDialog = new QgsIdentifyResultsDialog( mCanvas, mCanvas->window() );

    connect( mResultsDialog.data(), static_cast<void ( QgsIdentifyResultsDialog::* )( QgsRasterLayer * )>( &QgsIdentifyResultsDialog::formatChanged ), this, &QgsMapToolIdentify::formatChanged );
    connect( mResultsDialog.data(), &QgsIdentifyResultsDialog::copyToClipboard, this, &QgsMapToolIdentifyAction::handleCopyToClipboard );
  }

  return mResultsDialog;
}

void QgsMapToolIdentifyAction::showAttributeTable( QgsMapLayer *layer, const QList<QgsFeature> &featureList )
{
  resultsDialog()->clear();

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vl )
    return;

  QString filter = QStringLiteral( "$id IN (" );
  Q_FOREACH ( const QgsFeature &feature, featureList )
  {
    filter.append( QStringLiteral( "%1," ).arg( feature.id() ) );
  }
  filter = filter.replace( QRegExp( ",$" ), QStringLiteral( ")" ) );

  QgsAttributeTableDialog *tableDialog = new QgsAttributeTableDialog( vl, QgsAttributeTableFilterModel::ShowFilteredList );
  tableDialog->setFilterExpression( filter );
  tableDialog->show();
}

void QgsMapToolIdentifyAction::handleOnCanvasRelease( QgsMapMouseEvent *e )
{
  resultsDialog()->clear();
  connect( this, &QgsMapToolIdentifyAction::identifyProgress, QgisApp::instance(), &QgisApp::showProgress );
  connect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

  setClickContextScope( toMapCoordinates( e->pos() ) );

  identifyMenu()->setResultsIfExternalAction( false );

  // enable the right click for extended menu so it behaves as a contextual menu
  // this would be removed when a true contextual menu is brought in QGIS
  bool extendedMenu = e->modifiers() == Qt::ShiftModifier || e->button() == Qt::RightButton;
  extendedMenu = extendedMenu && !mJustFinishedSelection;
  identifyMenu()->setExecWithSingleResult( extendedMenu );
  identifyMenu()->setShowFeatureActions( extendedMenu );
  IdentifyMode mode = extendedMenu ? LayerSelection : DefaultQgsSetting;

  QList<IdentifyResult> results = QgsMapToolIdentify::identify( e->x(), e->y(), mode, AllLayers, mSelectionMode );

  disconnect( this, &QgsMapToolIdentifyAction::identifyProgress, QgisApp::instance(), &QgisApp::showProgress );
  disconnect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

  if ( mJustFinishedSelection ) mJustFinishedSelection = false;

  if ( results.isEmpty() )
  {
    resultsDialog()->clear();
    QgisApp::instance()->statusBarIface()->showMessage( tr( "No features at this position found." ) );
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

void QgsMapToolIdentifyAction::selectRadiusMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY radiusEdge = e->snapPoint();

  if ( !mSelectionActive )
    return;

  if ( !mSelectionRubberBand )
  {
    initRubberBand();
  }

  updateRadiusFromEdge( radiusEdge );
}

void QgsMapToolIdentifyAction::selectRadiusReleaseEvent( QgsMapMouseEvent *e )
{

  if ( e->button() == Qt::RightButton )
  {
    mSelectionActive = false;
    return;
  }

  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    mRadiusCenter = e->snapPoint();
  }
  else
  {
    if ( !mSelectionRubberBand )
    {
      initRubberBand();
    }
    mSelectionGeometry = mSelectionRubberBand->asGeometry();
    mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    mSelectionActive = false;
  }
}

void QgsMapToolIdentifyAction::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
  double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  const int RADIUS_SEGMENTS = 80;
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ),
                            mRadiusCenter.y() + radius * std::sin( theta ) );
    mSelectionRubberBand->addPoint( radiusPoint, false );
  }
  mSelectionRubberBand->closePoints( true );
}

void QgsMapToolIdentifyAction::canvasMoveEvent( QgsMapMouseEvent *e )
{

  switch ( mSelectionMode )
  {
    case QgsMapToolIdentifyAction::SelectSimple:
      mSelectionHandler->selectFeaturesMoveEvent( e );
      //selectFeaturesMoveEvent( e );
      break;
    case QgsMapToolIdentifyAction::SelectPolygon:
      mSelectionHandler->selectPolygonMoveEvent( e );
      //selectPolygonMoveEvent( e );
      break;
    case QgsMapToolIdentifyAction::SelectFreehand:
      mSelectionHandler->selectFreehandMoveEvent( e );
      //selectFreehandMoveEvent( e );
      break;
    case QgsMapToolIdentifyAction::SelectRadius:
      mSelectionHandler->selectRadiusMoveEvent( e );
      //selectRadiusMoveEvent( e );
      break;
  }
}

void QgsMapToolIdentifyAction::initRubberBand()
{
  mSelectionRubberBand = qgis::make_unique< QgsRubberBand>( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSelectionRubberBand->setFillColor( mFillColor );
  mSelectionRubberBand->setStrokeColor( mStrokeColor );
}

void QgsMapToolIdentifyAction::canvasPressEvent( QgsMapMouseEvent *e )
{
  mSelectionMode = mResultsDialog->selectionMode();

  switch ( mSelectionMode )
  {
    case QgsMapToolIdentifyAction::SelectSimple:
//      mSelectionRubberBand.reset();
//      initRubberBand();
//      mInitDragPos = e -> pos();
      mSelectionHandler->canvasPressEvent(e, QgsMapToolSelectionHandler::SelectSimple);
      break;
    case QgsMapToolIdentifyAction::SelectPolygon:
      break;
    case QgsMapToolIdentifyAction::SelectFreehand:
      break;
    case QgsMapToolIdentifyAction::SelectRadius:
      break;
  }
}

void QgsMapToolIdentifyAction::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( mResultsDialog )
  {
    mSelectionMode = mResultsDialog->selectionMode();
  }
  else
  {
    mSelectionMode = QgsMapToolIdentify::SelectSimple;
  }
  mSelectionHandler->canvasReleaseEvent( e );
  switch ( mSelectionMode )
  {
    case QgsMapToolIdentifyAction::SelectSimple:
      mSelectionHandler->selectFeaturesReleaseEvent( e );
      //selectFeaturesReleaseEvent( e );
      break;
    case QgsMapToolIdentifyAction::SelectPolygon:
      mSelectionHandler->selectPolygonReleaseEvent( e );
      //selectPolygonReleaseEvent( e );
      break;
    case QgsMapToolIdentifyAction::SelectFreehand:
      mSelectionHandler->selectFreehandReleaseEvent( e );
      //selectFreehandReleaseEvent( e );
      break;
    case QgsMapToolIdentifyAction::SelectRadius:
      mSelectionHandler->selectRadiusReleaseEvent( e );
      //selectRadiusReleaseEvent( e );
      break;
  }

  handleOnCanvasRelease( e );
}

void QgsMapToolIdentifyAction::handleChangedRasterResults( QList<IdentifyResult> &results )
{
  // Add new result after raster format change
  QgsDebugMsg( QString( "%1 raster results" ).arg( results.size() ) );
  QList<IdentifyResult>::const_iterator rresult;
  for ( rresult = results.constBegin(); rresult != results.constEnd(); ++rresult )
  {
    if ( rresult->mLayer->type() == QgsMapLayer::RasterLayer )
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
  QgsMapToolIdentify::deactivate();
}

QgsUnitTypes::DistanceUnit QgsMapToolIdentifyAction::displayDistanceUnits() const
{
  return QgsProject::instance()->distanceUnits();
}

QgsUnitTypes::AreaUnit QgsMapToolIdentifyAction::displayAreaUnits() const
{
  return QgsProject::instance()->areaUnits();
}

void QgsMapToolIdentifyAction::handleCopyToClipboard( QgsFeatureStore &featureStore )
{
  QgsDebugMsg( QString( "features count = %1" ).arg( featureStore.features().size() ) );
  emit copyToClipboard( featureStore );
}

void QgsMapToolIdentifyAction::setClickContextScope( const QgsPointXY &point )
{
  QgsExpressionContextScope clickScope;
  clickScope.addVariable( QgsExpressionContextScope::StaticVariable( QString( "click_x" ), point.x(), true ) );
  clickScope.addVariable( QgsExpressionContextScope::StaticVariable( QString( "click_y" ), point.y(), true ) );

  resultsDialog()->setExpressionContextScope( clickScope );

  if ( mIdentifyMenu )
  {
    mIdentifyMenu->setExpressionContextScope( clickScope );
  }
}

void QgsMapToolIdentifyAction::selectFeaturesMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  QRect rect;
  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    rect = QRect( e->pos(), e->pos() );
  }
  else
  {
    rect = QRect( e->pos(), mInitDragPos );
  }
  QgsMapToolSelectUtils::setRubberBand( mCanvas, rect, mSelectionRubberBand.get() );
}

void QgsMapToolIdentifyAction::selectFeaturesReleaseEvent( QgsMapMouseEvent *e )
{
  QPoint point = e->pos() - mInitDragPos;
  if ( !mSelectionActive || ( point.manhattanLength() < QApplication::startDragDistance() ) )
  {
    mSelectionGeometry = QgsGeometry::fromPointXY( toMapCoordinates( e ->pos() ) );
    mSelectionActive = false;
  }

  if ( mSelectionRubberBand && mSelectionActive )
  {
    mSelectionGeometry = mSelectionRubberBand->asGeometry();
    mSelectionRubberBand.reset();
  }

  mSelectionActive = false;
}

void QgsMapToolIdentifyAction::selectPolygonMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionRubberBand )
    return;

  if ( mSelectionRubberBand->numberOfVertices() > 0 )
  {
    mSelectionRubberBand->movePoint( toMapCoordinates( e->pos() ) );
  }
}

void QgsMapToolIdentifyAction::selectPolygonReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionRubberBand )
  {
    initRubberBand();
  }
  if ( e->button() == Qt::LeftButton )
  {
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
  }
  else
  {
    if ( mSelectionRubberBand->numberOfVertices() > 2 )
    {
      mSelectionGeometry = mSelectionRubberBand->asGeometry();
    }
    mSelectionRubberBand.reset();
    mJustFinishedSelection = true;
  }
}

void QgsMapToolIdentifyAction::selectFreehandMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive || !mSelectionRubberBand )
    return;

  mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
}

void QgsMapToolIdentifyAction::selectFreehandReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    if ( !mSelectionRubberBand )
    {
      initRubberBand();
    }
    else
    {
      mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    }
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( mSelectionRubberBand && mSelectionRubberBand->numberOfVertices() > 2 )
      {
        mSelectionGeometry = mSelectionRubberBand->asGeometry();
      }
    }

    mSelectionRubberBand.reset();
    mSelectionActive = false;
  }
}

void QgsMapToolIdentifyAction::keyReleaseEvent( QKeyEvent *e )
{
  if ( mSelectionActive && e->key() == Qt::Key_Escape )
  {
    mSelectionRubberBand.reset();
    mSelectionActive = false;
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}
