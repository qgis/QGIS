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
  connect( this, SIGNAL( finished( int ) ), mTool, SLOT( removeRubberBand() ) );

}

void QgsSimplifyDialog::updateStatusText()
{
  labelStatus->setText( mTool->statusText() );
}


////////////////////////////////////////////////////////////////////////////


QgsMapToolSimplify::QgsMapToolSimplify( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mSelectionRubberBand( 0 )
    , mDragging( false )
    , mOriginalVertexCount( 0 )
    , mReducedVertexCount( 0 )
{
  QSettings settings;
  mTolerance = settings.value( "/digitizing/simplify_tolerance", 1 ).toDouble();
  mToleranceUnits = ( ToleranceUnits ) settings.value( "/digitizing/simplify_tolerance_units", 0 ).toInt();

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
  mToleranceUnits = ( ToleranceUnits ) units;

  QSettings settings;
  settings.setValue( "/digitizing/simplify_tolerance_units", units );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

void QgsMapToolSimplify::updateSimplificationPreview()
{
  QgsVectorLayer* vl = currentVectorLayer();

  mReducedVertexCount = 0;
  int i = 0;
  foreach ( const QgsFeature& fSel, mSelectedFeatures )
  {
    // create a copy of selected feature and do the simplification
    QgsFeature f = fSel;
    QgsSimplifyFeature::simplify( f, mTolerance, mToleranceUnits, mCanvas->mapSettings().layerTransform( vl ) );
    mReducedVertexCount += vertexCount( f.geometry() );
    mRubberBands[i]->setToGeometry( f.geometry(), vl );
    ++i;
  }

  mSimplifyDialog->updateStatusText();
}


int QgsMapToolSimplify::vertexCount( QgsGeometry* g ) const
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

  vlayer->beginEditCommand( tr( "Geometry simplified" ) );
  foreach ( const QgsFeature& feat, mSelectedFeatures )
  {
    QgsFeature f = feat;
    QgsSimplifyFeature::simplify( f, mTolerance, mToleranceUnits, mCanvas->mapSettings().layerTransform( vlayer ) );
    vlayer->changeGeometry( f.id(), f.geometry() );
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
    mOriginalVertexCount += vertexCount( f.geometry() );

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
    currentDistance = geometry->distance( *( f.geometry() ) );
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
  return tr( "%1 feature(s): %2 to %3 vertices (%4%)" )
         .arg( mSelectedFeatures.count() ).arg( mOriginalVertexCount ).arg( mReducedVertexCount ).arg( percent );
}


////////////////////////////////////////////////////////////////////////////


bool QgsSimplifyFeature::simplify( QgsFeature& feature, double tolerance, QgsMapToolSimplify::ToleranceUnits units, const QgsCoordinateTransform* ctLayerToMap )
{
  if ( tolerance <= 0 )
    return false;

  QgsGeometry* g = feature.geometry();
  if ( g->type() == QGis::Line )
  {
    if ( g->isMultipart() )
    {
      QgsMultiPolyline poly;
      foreach ( const QgsPolyline& ring, g->asMultiPolyline() )
        poly << simplifyPoints( ring, tolerance, units, ctLayerToMap );
      feature.setGeometry( QgsGeometry::fromMultiPolyline( poly ) );
    }
    else
    {
      QgsPolyline resultPoints = simplifyPoints( g->asPolyline(), tolerance, units, ctLayerToMap );
      feature.setGeometry( QgsGeometry::fromPolyline( resultPoints ) );
    }
    return true;
  }
  else if ( g->type() == QGis::Polygon )
  {
    if ( g->isMultipart() )
    {
      QgsMultiPolygon mpoly;
      foreach ( const QgsPolygon& polygon, g->asMultiPolygon() )
      {
        QgsPolygon poly;
        foreach ( const QgsPolyline& ring, polygon )
          poly << simplifyPoints( ring, tolerance, units, ctLayerToMap );
        mpoly << poly;
      }
      feature.setGeometry( QgsGeometry::fromMultiPolygon( mpoly ) );
    }
    else
    {
      QgsPolygon poly;
      foreach ( const QgsPolyline& ring, g->asPolygon() )
        poly << simplifyPoints( ring, tolerance, units, ctLayerToMap );
      feature.setGeometry( QgsGeometry::fromPolygon( poly ) );
    }
    return true;
  }
  else
    return false;
}


QVector<QgsPoint> QgsSimplifyFeature::simplifyPoints( const QVector<QgsPoint>& pts, double tolerance, QgsMapToolSimplify::ToleranceUnits units, const QgsCoordinateTransform* ctLayerToMap )
{
  if ( tolerance < 0 )
    return pts;

  // if using map units, we transform coordinates to map units and, run simplification on transformed points
  // and use indices with original coordinates. This will ensure that we do not modify coordinate values
  // by transforming them back and forth.
  QVector<QgsPoint> ptsForAlg;
  bool transform = ( units == QgsMapToolSimplify::MapUnits && ctLayerToMap );
  if ( transform )
  {
    foreach ( const QgsPoint& pt, pts )
      ptsForAlg << ctLayerToMap->transform( pt );
  }

  QList<int> indices = QgsSimplifyFeature::simplifyPointsIndices( transform ? ptsForAlg : pts, tolerance );

  QVector<QgsPoint> result;
  foreach ( int index, indices )
    result.append( pts[index] );
  return result;
}


QList<int> QgsSimplifyFeature::simplifyPointsIndices( const QVector<QgsPoint>& pts, double tolerance )
{
  // Douglas-Peucker simplification algorithm

  int anchor  = 0;
  int floater = pts.size() - 1;

  QList<StackEntry> stack;
  StackEntry temporary;
  StackEntry entry = {anchor, floater};
  stack.append( entry );

  QSet<int> keep;
  double anchorX;
  double anchorY;
  double seg_len;
  double max_dist;
  int farthest;
  double dist_to_seg;
  double vecX;
  double vecY;

  while ( !stack.empty() )
  {
    temporary = stack.takeLast();
    anchor = temporary.anchor;
    floater = temporary.floater;
    // initialize line segment
    if ( pts[floater] != pts[anchor] )
    {
      anchorX = pts[floater].x() - pts[anchor].x();
      anchorY = pts[floater].y() - pts[anchor].y();
      seg_len = sqrt( anchorX * anchorX + anchorY * anchorY );
      // get the unit vector
      anchorX /= seg_len;
      anchorY /= seg_len;
    }
    else
    {
      anchorX = anchorY = seg_len = 0.0;
    }
    // inner loop:
    max_dist = 0.0;
    farthest = anchor + 1;
    for ( int i = anchor + 1; i < floater; i++ )
    {
      dist_to_seg = 0.0;
      // compare to anchor
      vecX = pts[i].x() - pts[anchor].x();
      vecY = pts[i].y() - pts[anchor].y();
      seg_len = sqrt( vecX * vecX + vecY * vecY );
      // dot product:
      double proj = vecX * anchorX + vecY * anchorY;
      if ( proj < 0.0 )
      {
        dist_to_seg = seg_len;
      }
      else
      {
        // compare to floater
        vecX = pts[i].x() - pts[floater].x();
        vecY = pts[i].y() - pts[floater].y();
        seg_len = sqrt( vecX * vecX + vecY * vecY );
        // dot product:
        proj = vecX * ( -anchorX ) + vecY * ( -anchorY );
        if ( proj < 0.0 )
        {
          dist_to_seg = seg_len;
        }
        else
        {  // calculate perpendicular distance to line (pythagorean theorem):
          dist_to_seg = sqrt( qAbs( seg_len * seg_len - proj * proj ) );
        }
        if ( max_dist < dist_to_seg )
        {
          max_dist = dist_to_seg;
          farthest = i;
        }
      }
    }
    if ( max_dist <= tolerance )
    { // # use line segment
      keep.insert( anchor );
      keep.insert( floater );
    }
    else
    {
      StackEntry s = {anchor, farthest};
      stack.append( s );

      StackEntry r = {farthest, floater};
      stack.append( r );
    }
  }

  QList<int> keep2 = keep.toList();
  qSort( keep2 );
  return keep2;
}

