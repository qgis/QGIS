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
#include <QMessageBox>

#include <cmath>
#include <cfloat>

QgsSimplifyDialog::QgsSimplifyDialog( QWidget* parent )
    : QDialog( parent )
{
  setupUi( this );
  connect( horizontalSlider, SIGNAL( valueChanged( int ) ),
           this, SLOT( valueChanged( int ) ) );
  connect( okButton, SIGNAL( clicked() ),
           this, SLOT( simplify() ) );

}

void QgsSimplifyDialog::valueChanged( int value )
{
  emit toleranceChanged( value );
}

void QgsSimplifyDialog::simplify()
{
  emit storeSimplified();
}

void QgsSimplifyDialog::setRange( int minValue, int maxValue )
{
  // let's have 20 page steps
  horizontalSlider->setPageStep(( maxValue - minValue ) / 20 );

  horizontalSlider->setMinimum(( minValue - 1 < 0 ? 0 : minValue - 1 ) );// -1 for count with minimum tolerance end caused by double imprecision
  horizontalSlider->setMaximum( maxValue );

}


QgsMapToolSimplify::QgsMapToolSimplify( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas ), mRubberBand( 0 )
{
  mSimplifyDialog = new QgsSimplifyDialog( canvas->topLevelWidget() );
  connect( mSimplifyDialog, SIGNAL( toleranceChanged( int ) ),
           this, SLOT( toleranceChanged( int ) ) );
  connect( mSimplifyDialog, SIGNAL( storeSimplified() ),
           this, SLOT( storeSimplified() ) );
  connect( mSimplifyDialog, SIGNAL( finished( int ) ),
           this, SLOT( removeRubberBand() ) );
}

QgsMapToolSimplify::~QgsMapToolSimplify()
{
  removeRubberBand();
  delete mSimplifyDialog;
}


void QgsMapToolSimplify::toleranceChanged( int tolerance )
{
  mTolerance = double( tolerance ) / toleranceDivider;

  // create a copy of selected feature and do the simplification
  QgsFeature f = mSelectedFeature;
  //QgsSimplifyFeature::simplifyLine(f, mTolerance);
  if ( mTolerance > 0 )
  {
    if ( mSelectedFeature.geometry()->type() == QGis::Line )
    {
      QgsSimplifyFeature::simplifyLine( f, mTolerance );
    }
    else
    {
      QgsSimplifyFeature::simplifyPolygon( f, mTolerance );
    }
  }
  mRubberBand->setToGeometry( f.geometry(), 0 );
}


void QgsMapToolSimplify::storeSimplified()
{
  QgsVectorLayer * vlayer = currentVectorLayer();
  if ( mSelectedFeature.geometry()->type() == QGis::Line )
  {
    QgsSimplifyFeature::simplifyLine( mSelectedFeature, mTolerance );
  }
  else
  {
    QgsSimplifyFeature::simplifyPolygon( mSelectedFeature, mTolerance );
  }

  vlayer->beginEditCommand( tr( "Geometry simplified" ) );
  vlayer->changeGeometry( mSelectedFeature.id(), mSelectedFeature.geometry() );
  vlayer->endEditCommand();

  mCanvas->refresh();
}

int QgsMapToolSimplify::calculateDivider( double minimum, double maximum )
{
  double tmp = minimum;
  long i = 1;
  if ( minimum == 0 )
  { //exception if min = 0 than divider must be counted from maximum
    tmp = maximum;
  }
  //count divider in such way so it can be used as whole number
  while ( tmp < 1 )
  {
    tmp = tmp * 10;
    i = i * 10;
  }
  if ( minimum == 0 )
  { //special case that minimum is 0 to have more than 1 step
    i = i * 100000;
  }
//taking care of problem when multiplication would overflow maxint
  while ( int( i * maximum ) < 0 )
  {
    i = i / 10;
  }
  return i;
}

bool QgsMapToolSimplify::calculateSliderBoudaries()
{
  double minTolerance = -1, maxTolerance = -1;

  double tol = 0.000001;
  bool found = false;
  bool isLine = mSelectedFeature.geometry()->type() == QGis::Line;
  QVector<QgsPoint> pts = getPointList( mSelectedFeature );
  int size = pts.size();
  if ( size == 0 || ( isLine && size <= 2 ) || ( !isLine && size <= 4 ) )
  {
    return false;
  }

  // calculate minimum tolerance where no vertex is excluded
  bool maximized = false;
  int count = 0;
  while ( !found )
  {
    count++;
    if ( count == 30 && !maximized )
    { //special case when tolerance is too low to be correct so it's near 0
      // else in some special cases this algorithm would create infinite loop
      found = true;
      minTolerance = 0;
    }

    if ( QgsSimplifyFeature::simplifyPoints( pts, tol ).size() < size )
    { //some vertexes were already excluded
      if ( maximized ) //if we were already in second direction end
      {
        found = true;
        minTolerance = tol / 2;
      }
      else //only lowering tolerance till it's low enough to have all vertexes
      {
        tol = tol / 2;
      }
    }
    else
    { // simplified feature has all vertexes therefore no need we need higher tolerance also ending flag set
      // when some tolerance will exclude some of vertexes
      maximized = true;
      tol = tol * 2;
    }
  }
  found = false;
  int requiredCnt = ( isLine ? 2 : 4 ); //4 for polygon is correct because first and last points are the same
  bool bottomFound = false;
  double highTol = DBL_MAX, lowTol = DBL_MIN;// two boundaries to be used when no directly correct solution is found
  // calculate minimum tolerance where minimum (requiredCnt) of vertexes are left in geometry
  while ( !found )
  {

    int foundVertexes = QgsSimplifyFeature::simplifyPoints( pts, tol ).size();
    if ( foundVertexes < requiredCnt + 1 )
    { //required or lower number of verticies found
      if ( foundVertexes == requiredCnt )
      {
        found = true;
        maxTolerance = tol;
      }
      else
      { //solving problem that polygon would have less than minimum alowed vertexes
        bottomFound = true;
        highTol = tol;
        tol = ( highTol + lowTol ) / 2;
        if ( doubleNear( highTol, lowTol, 0.0001 ) ) // without 0.0001 tolerance sometimes an endless loop in epsg:4326
        { //solving problem that two points are in same distance from  line, so they will be both excluded at same time
          //so some time more than required count of vertices can stay
          found = true;
          maxTolerance = lowTol;
        }
      }
    }
    else
    {
      if ( bottomFound )
      {
        lowTol = tol;
        tol = ( highTol + lowTol ) / 2;
        if ( doubleNear( highTol, lowTol, 0.0001 ) ) // without 0.0001 tolerance sometimes an endless loop in epsg:4326
        { //solving problem that two points are in same distance from  line, so they will be both excluded at same time
          //so some time more than required count of vertices can stay
          found = true;
          maxTolerance = lowTol;
        }
      }
      else
      { //still too much verticies left so we need to increase tolerance
        lowTol = tol;
        tol = tol * 2;
      }
    }
  }
  toleranceDivider = calculateDivider( minTolerance, maxTolerance );
  // set min and max
  mSimplifyDialog->setRange( int( minTolerance * toleranceDivider ),
                             int( maxTolerance * toleranceDivider ) );
  return true;
}


void QgsMapToolSimplify::canvasPressEvent( QMouseEvent * e )
{
  QgsVectorLayer * vlayer = currentVectorLayer();

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  QgsPoint layerCoords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );

  double r = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapRenderer() );
  QgsRectangle selectRect = QgsRectangle( layerCoords.x() - r, layerCoords.y() - r,
                                          layerCoords.x() + r, layerCoords.y() + r );
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setSubsetOfAttributes( QgsAttributeList() ) );

  QgsGeometry* geometry = QgsGeometry::fromPoint( layerCoords );
  double minDistance = DBL_MAX;
  double currentDistance;

  mSelectedFeature.setValid( false );

  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    currentDistance = geometry->distance( *( f.geometry() ) );
    if ( currentDistance < minDistance )
    {
      minDistance = currentDistance;
      mSelectedFeature = f;
    }
  }

  // delete previous rubberband (if any)
  removeRubberBand();

  if ( mSelectedFeature.isValid() )
  {
    if ( mSelectedFeature.geometry()->isMultipart() )
    {
      QMessageBox::critical( 0, tr( "Unsupported operation" ), tr( "Multipart features are not supported for simplification." ) );
      return;
    }

    mRubberBand = new QgsRubberBand( mCanvas );
    mRubberBand->setToGeometry( mSelectedFeature.geometry(), 0 );
    mRubberBand->setColor( Qt::red );
    mRubberBand->setWidth( 2 );
    mRubberBand->show();
    //calculate boudaries for slidebar
    if ( calculateSliderBoudaries() )
    {
      // show dialog as a non-modal window
      mSimplifyDialog->show();
    }
    else
    {
      QMessageBox::warning( 0, tr( "Unsupported operation" ), tr( "This feature cannot be simplified. Check if feature has enough vertices to be simplified." ) );
    }
  }
}

void QgsMapToolSimplify::removeRubberBand()
{
  delete mRubberBand;
  mRubberBand = 0;
}

void QgsMapToolSimplify::deactivate()
{
  if ( mSimplifyDialog->isVisible() )
    mSimplifyDialog->close();
  removeRubberBand();
  QgsMapTool::deactivate();
}


QVector<QgsPoint> QgsMapToolSimplify::getPointList( QgsFeature& f )
{
  QgsGeometry* line = f.geometry();
  if (( line->type() != QGis::Line && line->type() != QGis::Polygon ) || line->isMultipart() )
  {
    return QVector<QgsPoint>();
  }
  if (( line->type() == QGis::Line ) )
  {
    return line->asPolyline();
  }
  else
  {
    if ( line->asPolygon().size() > 1 )
    {
      return QVector<QgsPoint>();
    }
    return line->asPolygon()[0];
  }
}



bool QgsSimplifyFeature::simplifyLine( QgsFeature& lineFeature,  double tolerance )
{
  QgsGeometry* line = lineFeature.geometry();
  if ( line->type() != QGis::Line )
  {
    return false;
  }

  QVector<QgsPoint> resultPoints = simplifyPoints( line->asPolyline(), tolerance );
  lineFeature.setGeometry( QgsGeometry::fromPolyline( resultPoints ) );
  return true;
}

bool QgsSimplifyFeature::simplifyPolygon( QgsFeature& polygonFeature,  double tolerance )
{
  QgsGeometry* polygon = polygonFeature.geometry();
  if ( polygon->type() != QGis::Polygon )
  {
    return false;
  }

  QVector<QgsPoint> resultPoints = simplifyPoints( polygon->asPolygon()[0], tolerance );
  //resultPoints.push_back(resultPoints[0]);
  QVector<QgsPolyline> poly;
  poly.append( resultPoints );
  polygonFeature.setGeometry( QgsGeometry::fromPolygon( poly ) );
  return true;
}


QVector<QgsPoint> QgsSimplifyFeature::simplifyPoints( const QVector<QgsPoint>& pts, double tolerance )
{
  //just safety precaution
  if ( tolerance < 0 )
    return pts;
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

  QVector<QgsPoint> result;
  int position;
  while ( !keep2.empty() )
  {
    position = keep2.takeFirst();
    result.append( pts[position] );
  }
  return result;
}
