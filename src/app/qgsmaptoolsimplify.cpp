/***************************************************************************
    qgsmaptoolsimplify.cpp  - simplify vector layer features
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolsimplify.h"

#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"

#include <QMouseEvent>

#include <cmath>
#include <cfloat>

QgsSimplifyDialog::QgsSimplifyDialog( QgsMapToolSimplify* tool, QWidget* parent )
    : QDialog( parent )
    , mTool( tool )
{
  setupUi( this );

  spinTolerance->setValue( mTool->tolerance() );
  cboToleranceUnits->setCurrentIndex(( int ) mTool->toleranceUnits() );

  // communication with map tool
  connect( spinTolerance, SIGNAL( valueChanged( double ) ), mTool, SLOT( setTolerance( double ) ) );
  connect( cboToleranceUnits, SIGNAL( currentIndexChanged( int ) ), mTool, SLOT( setToleranceUnits( int ) ) );
  connect( okButton, SIGNAL( clicked() ), mTool, SLOT( storeSimplified() ) );
}

void QgsSimplifyDialog::updateStatusText()
{
  labelStatus->setText( mTool->statusText() );
}

void QgsSimplifyDialog::enableOkButton( bool enabled )
{
  okButton->setEnabled( enabled );
}


////////////////////////////////////////////////////////////////////////////


QgsMapToolSimplify::QgsMapToolSimplify( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mSelectionRubberBand( 0 )
    , mDragging( false )
    , mOriginalVertexCount( 0 )
    , mReducedVertexCount( 0 )
    , mReducedHasErrors( false )
{
  QSettings settings;
  mTolerance = settings.value( "/digitizing/simplify_tolerance", 1 ).toDouble();
  mToleranceUnits = ( QgsTolerance::UnitType ) settings.value( "/digitizing/simplify_tolerance_units", 0 ).toInt();

  mSimplifyDialog = new QgsSimplifyDialog( this, canvas->topLevelWidget() );
}

QgsMapToolSimplify::~QgsMapToolSimplify()
{
  clearSelection();
  delete mSimplifyDialog;
}


void QgsMapToolSimplify::setTolerance( double tolerance )
{
  mTolerance = tolerance;

  QSettings settings;
  settings.setValue( "/digitizing/simplify_tolerance", tolerance );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

void QgsMapToolSimplify::setToleranceUnits( int units )
{
  mToleranceUnits = ( QgsTolerance::UnitType ) units;

  QSettings settings;
  settings.setValue( "/digitizing/simplify_tolerance_units", units );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

void QgsMapToolSimplify::updateSimplificationPreview()
{
  QgsVectorLayer* vl = currentVectorLayer();

  double layerTolerance = QgsTolerance::toleranceInMapUnits( mTolerance, vl, mCanvas->mapSettings(), mToleranceUnits );
  mReducedHasErrors = false;
  mReducedVertexCount = 0;
  int i = 0;
  foreach ( const QgsFeature& fSel, mSelectedFeatures )
  {
    if ( QgsGeometry* g = fSel.constGeometry()->simplify( layerTolerance ) )
    {
      mReducedVertexCount += vertexCount( g );
      mRubberBands[i]->setToGeometry( g, vl );
      delete g;
    }
    else
      mReducedHasErrors = true;
    ++i;
  }

  mSimplifyDialog->updateStatusText();
  mSimplifyDialog->enableOkButton( !mReducedHasErrors );
}


int QgsMapToolSimplify::vertexCount( const QgsGeometry* g ) const
{
  switch ( g->type() )
  {
    case QGis::Line:
    {
      int count = 0;
      if ( g->isMultipart() )
      {
        foreach ( const QgsPolyline& polyline, g->asMultiPolyline() )
          count += polyline.count();
      }
      else
        count = g->asPolyline().count();
      return count;
    }
    case QGis::Polygon:
    {
      int count = 0;
      if ( g->isMultipart() )
      {
        foreach ( const QgsPolygon& polygon, g->asMultiPolygon() )
          foreach ( const QgsPolyline& ring, polygon )
            count += ring.count();
      }
      else
      {
        foreach ( const QgsPolyline& ring, g->asPolygon() )
          count += ring.count();
      }
      return count;
    }
    default:
      return 0;
  }
}


void QgsMapToolSimplify::storeSimplified()
{
  QgsVectorLayer * vlayer = currentVectorLayer();
  double layerTolerance = QgsTolerance::toleranceInMapUnits( mTolerance, vlayer, mCanvas->mapSettings(), mToleranceUnits );

  vlayer->beginEditCommand( tr( "Geometry simplified" ) );
  foreach ( const QgsFeature& feat, mSelectedFeatures )
  {
    if ( QgsGeometry* g = feat.constGeometry()->simplify( layerTolerance ) )
    {
      vlayer->changeGeometry( feat.id(), g );
      delete g;
    }
  }
  vlayer->endEditCommand();

  clearSelection();

  mCanvas->refresh();
}



void QgsMapToolSimplify::canvasPressEvent( QMouseEvent * e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( !currentVectorLayer() )
  {
    notifyNotVectorLayer();
    return;
  }

  // delete previous rubberband (if any)
  clearSelection();

  mSelectionRect.setRect( 0, 0, 0, 0 );
}


void QgsMapToolSimplify::canvasMoveEvent( QMouseEvent * e )
{
  if ( !( e->buttons() & Qt::LeftButton ) )
    return;

  if ( !mDragging )
  {
    mDragging = true;
    delete mSelectionRubberBand;
    mSelectionRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
    QColor color( Qt::blue );
    color.setAlpha( 63 );
    mSelectionRubberBand->setColor( color );
    mSelectionRect.setTopLeft( e->pos() );
  }
  mSelectionRect.setBottomRight( e->pos() );
  if ( mSelectionRubberBand )
  {
    mSelectionRubberBand->setToCanvasRectangle( mSelectionRect );
    mSelectionRubberBand->show();
  }
}


void QgsMapToolSimplify::canvasReleaseEvent( QMouseEvent * e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( !currentVectorLayer() )
    return;

  delete mSelectionRubberBand;
  mSelectionRubberBand = 0;

  if ( mDragging && ( mSelectionRect.topLeft() != mSelectionRect.bottomRight() ) )
  {
    mDragging = false;

    // store the rectangle
    mSelectionRect.setRight( e->pos().x() );
    mSelectionRect.setBottom( e->pos().y() );

    selectFeaturesInRect();
  }
  else
  {
    selectOneFeature( e->pos() );
  }

  mDragging = false;

  // count vertices, prepare rubber bands
  mOriginalVertexCount = 0;
  foreach ( const QgsFeature& f, mSelectedFeatures )
  {
    mOriginalVertexCount += vertexCount( f.constGeometry() );

    QgsRubberBand* rb = new QgsRubberBand( mCanvas );
    rb->setColor( QColor( 255, 0, 0, 65 ) );
    rb->setWidth( 2 );
    rb->show();
    mRubberBands << rb;
  }
  updateSimplificationPreview();

  // show dialog as a non-modal window
  mSimplifyDialog->show();
}


void QgsMapToolSimplify::selectOneFeature( const QPoint& canvasPoint )
{
  QgsVectorLayer * vlayer = currentVectorLayer();
  QgsPoint layerCoords = toLayerCoordinates( vlayer, canvasPoint );
  double r = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapSettings() );
  QgsRectangle selectRect = QgsRectangle( layerCoords.x() - r, layerCoords.y() - r,
                                          layerCoords.x() + r, layerCoords.y() + r );
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setSubsetOfAttributes( QgsAttributeList() ) );

  QgsGeometry* geometry = QgsGeometry::fromPoint( layerCoords );
  double minDistance = DBL_MAX;
  double currentDistance;
  QgsFeature minDistanceFeature;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    currentDistance = geometry->distance( *( f.constGeometry() ) );
    if ( currentDistance < minDistance )
    {
      minDistance = currentDistance;
      minDistanceFeature = f;
    }
  }
  delete geometry;

  if ( minDistanceFeature.isValid() )
  {
    mSelectedFeatures << minDistanceFeature;
  }
}


void QgsMapToolSimplify::selectFeaturesInRect()
{
  QgsVectorLayer * vlayer = currentVectorLayer();
  QgsPoint pt1 = toMapCoordinates( mSelectionRect.topLeft() );
  QgsPoint pt2 = toMapCoordinates( mSelectionRect.bottomRight() );
  QgsRectangle rect = toLayerCoordinates( vlayer, QgsRectangle( pt1, pt2 ) );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setFilterRect( rect );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  request.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator fit = vlayer->getFeatures( request );
  while ( fit.nextFeature( f ) )
    mSelectedFeatures << f;
}


void QgsMapToolSimplify::clearSelection()
{
  mSelectedFeatures.clear();

  qDeleteAll( mRubberBands );
  mRubberBands.clear();
}

void QgsMapToolSimplify::deactivate()
{
  delete mSelectionRubberBand;
  mSelectionRubberBand = 0;

  if ( mSimplifyDialog->isVisible() )
    mSimplifyDialog->close();
  clearSelection();
  QgsMapTool::deactivate();
}

QString QgsMapToolSimplify::statusText() const
{
  int percent = mOriginalVertexCount ? ( 100 * mReducedVertexCount / mOriginalVertexCount ) : 0;
  QString txt = tr( "%1 feature(s): %2 to %3 vertices (%4%)" )
                .arg( mSelectedFeatures.count() ).arg( mOriginalVertexCount ).arg( mReducedVertexCount ).arg( percent );
  if ( mReducedHasErrors )
    txt += "\n" + tr( "Simplification failed!" );
  return txt;
}
