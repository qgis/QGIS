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

  // TODO @vsklencar refactor with QgsMapToolSelectRectangle
  mRubberBand = nullptr;
  mFillColor = QColor( 254, 178, 76, 63 );
  mStrokeColor = QColor( 254, 58, 29, 100 );
  mSelectionMode = QgsMapToolIdentifyAction::SelectFeatures;
}

QgsMapToolIdentifyAction::~QgsMapToolIdentifyAction()
{
  if ( mResultsDialog )
  {
    mResultsDialog->done( 0 );
  }
  delete mRubberBand;
}

QgsIdentifyResultsDialog *QgsMapToolIdentifyAction::resultsDialog()
{
  if ( !mResultsDialog )
  {
    mResultsDialog = new QgsIdentifyResultsDialog( mCanvas, mCanvas->window() );

    connect( mResultsDialog.data(), static_cast<void ( QgsIdentifyResultsDialog::* )( QgsRasterLayer * )>( &QgsIdentifyResultsDialog::formatChanged ), this, &QgsMapToolIdentify::formatChanged );
    connect( mResultsDialog.data(), &QgsIdentifyResultsDialog::copyToClipboard, this, &QgsMapToolIdentifyAction::handleCopyToClipboard );
    // todo connect slectionMode change
    //connect( mResultsDialog->mActionSelectPolygon, &QAction::triggered, this, &QgsMapToolIdentifyAction::setSelectPolygonMode);
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

void QgsMapToolIdentifyAction::handleOnCanvasRelease(QgsMapMouseEvent *e)
{
    resultsDialog()->clear();
    connect( this, &QgsMapToolIdentifyAction::identifyProgress, QgisApp::instance(), &QgisApp::showProgress );
    connect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

    setClickContextScope( toMapCoordinates( e->pos() ) );

    identifyMenu()->setResultsIfExternalAction( false );

    // enable the right click for extended menu so it behaves as a contextual menu
    // this would be removed when a true contextual menu is brought in QGIS
    bool extendedMenu = e->modifiers() == Qt::ShiftModifier || e->button() == Qt::RightButton;
    // TODO @vsklencar
    extendedMenu = extendedMenu && !mJustFinishedSelection;
    identifyMenu()->setExecWithSingleResult( extendedMenu );
    identifyMenu()->setShowFeatureActions( extendedMenu );
    IdentifyMode mode = extendedMenu ? LayerSelection : DefaultQgsSetting;
    // TODO @vsklencar get selection mode before handling
    mSelectionMode = mResultsDialog->selectionMode();

    QList<IdentifyResult> results = QgsMapToolIdentify::identify( e->x(), e->y(), mode );

    disconnect( this, &QgsMapToolIdentifyAction::identifyProgress, QgisApp::instance(), &QgisApp::showProgress );
    disconnect( this, &QgsMapToolIdentifyAction::identifyMessage, QgisApp::instance(), &QgisApp::showStatusMessage );

    if (mJustFinishedSelection) mJustFinishedSelection = false;

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

void QgsMapToolIdentifyAction::selectRadiusMoveEvent(QgsMapMouseEvent *e)
{
    QgsPointXY radiusEdge = e->snapPoint();
    //mSnapIndicator->setMatch( e->mapPointMatch() );

    if ( !mActive )
      return;

    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setStrokeColor( mStrokeColor );
    }

    updateRadiusFromEdge( radiusEdge );
}

void QgsMapToolIdentifyAction::selectRadiusReleaseEvent(QgsMapMouseEvent *e)
{

    if ( e->button() == Qt::RightButton )
    {
      mActive = false;
      return;
    }

    if ( e->button() != Qt::LeftButton )
      return;

    if ( !mActive )
    {
      mActive = true;
      mRadiusCenter = e->snapPoint();
    }
    else
    {
      if ( !mRubberBand )
      {
        mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
        mRubberBand->setFillColor( mFillColor );
        mRubberBand->setStrokeColor( mStrokeColor );
      }
      QgsPointXY radiusEdge = e->snapPoint();
      updateRadiusFromEdge( radiusEdge );
      mSelectionGeometry = mRubberBand->asGeometry();
      mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
      mActive = false;
      mSelectRect = mSelectionGeometry.boundingBox().toRectF().toRect();
    }
}

void QgsMapToolIdentifyAction::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
    double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
    mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    const int RADIUS_SEGMENTS = 80;
    for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
    {
        double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
        QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ),
                                mRadiusCenter.y() + radius * std::sin( theta ) );
        mRubberBand->addPoint( radiusPoint, false );
    }
    mRubberBand->closePoints( true );
}

// TODO @vsklencar refactor with QgsMapToolSelectRectangle
void QgsMapToolIdentifyAction::canvasMoveEvent( QgsMapMouseEvent *e )
{

    switch ( mSelectionMode )
    {
      case QgsMapToolIdentifyAction::SelectFeatures:
        selectFeaturesMoveEvent(e);
        break;
      case QgsMapToolIdentifyAction::SelectPolygon:
        selectPolygonMoveEvent(e);
        break;
      case QgsMapToolIdentifyAction::SelectFreehand:
        selectFreehandMoveEvent(e);
        break;
      case QgsMapToolIdentifyAction::SelectRadius:
        selectRadiusMoveEvent(e);
        break;
    }
}

void QgsMapToolIdentifyAction::canvasPressEvent( QgsMapMouseEvent *e )
{
    mSelectionMode = mResultsDialog->selectionMode();
    switch ( mSelectionMode )
    {
      case QgsMapToolIdentifyAction::SelectFeatures:
        mSelectRect.setRect( 0, 0, 0, 0 );
        delete mRubberBand;
        mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
        mRubberBand->setFillColor( mFillColor );
        mRubberBand->setStrokeColor( mStrokeColor );
        mInitDraggPos = e -> pos();
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
    mSelectionMode = mResultsDialog->selectionMode();
    switch ( mSelectionMode )
    {
      case QgsMapToolIdentifyAction::SelectFeatures:
         selectFeaturesReleaseEvent(e);
        break;
      case QgsMapToolIdentifyAction::SelectPolygon:
        selectPolygonReleaseEvent(e);
        break;
      case QgsMapToolIdentifyAction::SelectFreehand:
        selectFreehandReleaseEvent(e);
        break;
      case QgsMapToolIdentifyAction::SelectRadius:
        selectRadiusReleaseEvent(e);
        break;
    }

    handleOnCanvasRelease(e);
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
  QgsMapTool::activate();
}

void QgsMapToolIdentifyAction::deactivate()
{
  resultsDialog()->deactivate();
  QgsMapTool::deactivate();
}

void QgsMapToolIdentifyAction::setSelectPolygonMode()
{
    mSelectionMode = SelectPolygon;
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

void QgsMapToolIdentifyAction::selectFeaturesMoveEvent(QgsMapMouseEvent *e)
{
    if ( e->buttons() != Qt::LeftButton )
        return;

    if ( !mDragging )
    {
        mDragging = true;
        mSelectRect.setTopLeft( e->pos() );
    }
    mSelectRect.setBottomRight( e->pos() );
    QgsMapToolSelectUtils::setRubberBand( mCanvas, mSelectRect, mRubberBand );
}

void QgsMapToolIdentifyAction::selectFeaturesReleaseEvent(QgsMapMouseEvent *e)
{
    QPoint point = e->pos() - mInitDraggPos;
    if ( !mDragging || (point.manhattanLength() < QApplication::startDragDistance()))
    {
        QPoint point = e ->pos();
        //QgsMapToolSelectUtils::expandSelectRectangle( mSelectRect, vlayer, e->pos() );
        int boxSize = QApplication::startDragDistance()/2;
        mSelectRect.setLeft( point.x() - boxSize);
        mSelectRect.setRight( point.x() + boxSize );
        mSelectRect.setTop( point.y() - boxSize );
        mSelectRect.setBottom( point.y() + boxSize );
        // convert point to map coordinates to get selecion
        point = toMapCoordinates (e ->pos()).toQPointF().toPoint();
        mSelectionGeometry = QgsGeometry::fromRect(QgsRectangle((double)(point.x() - boxSize), (double) (point.y() - boxSize), (double) (point.x() + boxSize), (double) (point.y() + boxSize)));
        mDragging = false;
    }
    else {
      // Set valid values for rectangle's width and height
      if ( mSelectRect.width() == 1 )
      {
        mSelectRect.setLeft( mSelectRect.left() + 1 );
      }
      if ( mSelectRect.height() == 1 )
      {
        mSelectRect.setBottom( mSelectRect.bottom() + 1 );
      }
    }

    if ( mRubberBand && mDragging)
    {
      mSelectionGeometry = mRubberBand->asGeometry();
      QgsMapToolSelectUtils::setRubberBand( mCanvas, mSelectRect, mRubberBand );
      delete mRubberBand;
      mRubberBand = nullptr;
    }

    mSelectRect = QRect(toMapCoordinates( mSelectRect.topLeft() ).toQPointF().toPoint(), toMapCoordinates(mSelectRect.bottomRight()).toQPointF().toPoint());
    mDragging = false;
}

void QgsMapToolIdentifyAction::selectPolygonMoveEvent(QgsMapMouseEvent *e)
{
    if ( !mRubberBand )
      return;

    if ( mRubberBand->numberOfVertices() > 0 )
    {
      mRubberBand->removeLastPoint( 0 );
      mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    }
}

void QgsMapToolIdentifyAction::selectPolygonReleaseEvent(QgsMapMouseEvent *e)
{
    if ( !mRubberBand )
    {
      mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
      mRubberBand->setFillColor( mFillColor );
      mRubberBand->setStrokeColor( mStrokeColor );
    }
    if ( e->button() == Qt::LeftButton )
    {
      mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    }
    else
    {
      if ( mRubberBand->numberOfVertices() > 2 )
      {
        mSelectionGeometry = mRubberBand->asGeometry();
        mSelectRect = mSelectionGeometry.boundingBox().toRectF().toRect();
      }
      mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
      delete mRubberBand;
      mRubberBand = nullptr;
      mJustFinishedSelection = true;
    }
}

void QgsMapToolIdentifyAction::selectFreehandMoveEvent(QgsMapMouseEvent *e)
{
    if ( !mActive || !mRubberBand )
      return;

    mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
}

void QgsMapToolIdentifyAction::selectFreehandReleaseEvent(QgsMapMouseEvent *e)
{
    if ( !mActive )
    {
      if ( e->button() != Qt::LeftButton )
        return;

      if ( !mRubberBand )
      {
        mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
        mRubberBand->setFillColor( mFillColor );
        mRubberBand->setStrokeColor( mStrokeColor );
      }
      else
      {
        mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
      }
      mRubberBand->addPoint( toMapCoordinates( e->pos() ) );
      mActive = true;
    }
    else
    {
      if ( e->button() == Qt::LeftButton )
      {
        if ( mRubberBand && mRubberBand->numberOfVertices() > 2 )
        {
          mSelectionGeometry = mRubberBand->asGeometry();
          mSelectRect = mSelectionGeometry.boundingBox().toRectF().toRect();
        }
      }

      mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
      delete mRubberBand;
      mRubberBand = nullptr;
      mActive = false;
    }
}

void QgsMapToolIdentifyAction::keyReleaseEvent( QKeyEvent *e )
{
  if ( mActive && e->key() == Qt::Key_Escape )
  {
      delete mRubberBand;
      mRubberBand = nullptr;
      mActive = false;
      mDragging = false;
      return;
  }
  QgsMapTool::keyReleaseEvent( e );
}
