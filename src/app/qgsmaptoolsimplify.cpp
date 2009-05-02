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

#include <math.h>

QgsSimplifyDialog::QgsSimplifyDialog(QWidget* parent)
  : QDialog( parent )
{
   setupUi(this);
   connect( horizontalSlider, SIGNAL( valueChanged( int ) ),
           this, SLOT( valueChanged( int ) ) );
   connect( okButton, SIGNAL(clicked()), 
           this, SLOT(simplify()));

}

void QgsSimplifyDialog::valueChanged(int value)
{
  emit toleranceChanged(value);
}

void QgsSimplifyDialog::simplify()
{
  emit storeSimplified();
}

void QgsSimplifyDialog::setRange(int minValue, int maxValue)
{
  horizontalSlider->setMinimum(minValue);
  horizontalSlider->setMaximum(maxValue);

  // let's have 20 page steps
  horizontalSlider->setPageStep( (maxValue - minValue) / 20 );
}


QgsMapToolSimplify::QgsMapToolSimplify( QgsMapCanvas* canvas )
  : QgsMapToolEdit( canvas), mRubberBand( 0 )
{
  mSimplifyDialog = new QgsSimplifyDialog( canvas->topLevelWidget() );
  connect( mSimplifyDialog, SIGNAL( toleranceChanged( int ) ),
           this, SLOT( toleranceChanged( int ) ) );
  connect( mSimplifyDialog, SIGNAL( storeSimplified() ), 
           this, SLOT(storeSimplified()));
  connect( mSimplifyDialog, SIGNAL(finished(int)),
           this, SLOT(removeRubberBand()) );
}

QgsMapToolSimplify::~QgsMapToolSimplify()
{
  removeRubberBand();
  delete mSimplifyDialog;
}


void QgsMapToolSimplify::toleranceChanged(int tolerance)
{
  mTolerance = double(tolerance)/toleranceDivider;

  // create a copy of selected feature and do the simplification
  QgsFeature f = mSelectedFeature;
  QgsSimplifyFeature::simplifyLine(f, mTolerance);
  mRubberBand->setToGeometry(f.geometry(), false);
}


void QgsMapToolSimplify::storeSimplified()
{
  QgsVectorLayer * vlayer = currentVectorLayer();
  QgsSimplifyFeature::simplifyLine(mSelectedFeature, mTolerance);

  vlayer->changeGeometry( mSelectedFeature.id(), mSelectedFeature.geometry() );

  mCanvas->refresh();
}

int QgsMapToolSimplify::calculateDivider(double num)
{
  double tmp = num;
  int i = 1;
  while (tmp < 1) 
  {
    tmp = tmp*10;
    i = i *10;
  }
  return i;
}

void QgsMapToolSimplify::calculateSliderBoudaries()
{
  double minTolerance, maxTolerance;

  double tol = 0.0000001;
  bool found = false;
  bool isLine = mSelectedFeature.geometry()->type() == QGis::Line;
  QVector<QgsPoint> pts = getPointList(mSelectedFeature);
  int size = pts.size();
  if (size == 0 || (isLine && size < 2) || (!isLine && size < 3) )
  {
    return;
  }

  // calculate min
  while (!found)
  {
    if (QgsSimplifyFeature::simplifyPoints(pts, tol).size() < size)
    {
      found = true;
      minTolerance = tol/2;
    } else {
      tol = tol * 2;
    }
  }

  found = false;
  int requiredCnt = (isLine ? 2 : 3);
  // calculate max
  while (!found)
  {
    if (QgsSimplifyFeature::simplifyPoints(pts, tol).size() < requiredCnt + 1)
    {
//TODO: fix for polygon
      found = true;
      maxTolerance = tol;
    } else {
      tol = tol * 2;
    }
  }
  toleranceDivider = calculateDivider(minTolerance);
  // set min and max
  mSimplifyDialog->setRange( int(minTolerance * toleranceDivider),
                             int(maxTolerance * toleranceDivider) );
}


void QgsMapToolSimplify::canvasPressEvent( QMouseEvent * e )
{
  QgsVectorLayer * vlayer = currentVectorLayer();
  QgsPoint layerCoords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );

  double r = QgsTolerance::vertexSearchRadius(vlayer, mCanvas->mapRenderer());
  QgsRectangle selectRect = QgsRectangle( layerCoords.x() - r, layerCoords.y() - r,
                                          layerCoords.x() + r, layerCoords.y() + r);
  vlayer->select( QgsAttributeList(), selectRect, true );

  QgsGeometry* geometry = QgsGeometry::fromPoint( layerCoords );
  double minDistance = 10000000;
  double currentDistance;
  QgsFeature f;

  mSelectedFeature.setValid(FALSE);

  while (vlayer->nextFeature(f))
  {
    currentDistance = geometry->distance( *(f.geometry()) );
    if ( currentDistance < minDistance )
    {
      minDistance = currentDistance;
      mSelectedFeature = f;
    }
  }

  // delete previous rubberband (if any)
  removeRubberBand();

  if (mSelectedFeature.isValid())
  {
    mRubberBand = new QgsRubberBand(mCanvas);
    mRubberBand->setToGeometry(mSelectedFeature.geometry(), false);
    mRubberBand->setColor(Qt::red);
    mRubberBand->setWidth(2);
    mRubberBand->show();
    //calculate boudaries for slidebar
    calculateSliderBoudaries();

    // show dialog as a non-modal window
    mSimplifyDialog->show();
  }
}

void QgsMapToolSimplify::removeRubberBand()
{
  delete mRubberBand;
  mRubberBand = 0;
}

void QgsMapToolSimplify::deactivate()
{
  if (mSimplifyDialog->isVisible())
    mSimplifyDialog->close();
  removeRubberBand();
  QgsMapTool::deactivate();
}


QVector<QgsPoint> QgsMapToolSimplify::getPointList(QgsFeature& f)
{
  QgsGeometry* line = f.geometry();
  if ((line->type() != QGis::Line && line->type() != QGis::Polygon ) || line->isMultipart())
  {
    return QVector<QgsPoint>();
  }
  if ((line->type() == QGis::Line))
  {
    return line->asPolyline();
  } 
  else
  {
    if (line->asPolygon().size() > 1) 
    {
      return QVector<QgsPoint>();
    }
    return line->asPolygon()[0];
  }

}





bool QgsSimplifyFeature::simplifyLine(QgsFeature& lineFeature,  double tolerance)
{
  QgsGeometry* line = lineFeature.geometry();
  if (line->type() != QGis::Line)
  {
    return FALSE;
  }

  QVector<QgsPoint> resultPoints = simplifyPoints(line->asPolyline(), tolerance);
  lineFeature.setGeometry( QgsGeometry::fromPolyline( resultPoints ) );
  return TRUE;
}


//TODO: change to correct structure after
bool QgsSimplifyFeature::simplifyPartOfLine(QgsFeature& lineFeature, int fromVertexNr, int toVertexNr, double tolerance)
{
  QgsGeometry* line = lineFeature.geometry();
  if (line->type() != QGis::Line)
  {
    return FALSE;
  }
        
  QVector<QgsPoint> resultPoints = simplifyPoints(line->asPolyline(), tolerance);
  lineFeature.setGeometry( QgsGeometry::fromPolyline( resultPoints ) );
  return TRUE;
}



QVector<QgsPoint> QgsSimplifyFeature::simplifyPoints (const QVector<QgsPoint>& pts, double tolerance)
{
  // Douglas-Peucker simplification algorithm

  int anchor  = 0;
  int floater = pts.size() - 1;
  
  QList<StackEntry> stack;
  StackEntry temporary;
  StackEntry entry = {anchor, floater};
  stack.append(entry);

  QSet<int> keep;
  double anchorX;
  double anchorY;
  double seg_len;
  double max_dist;
  int farthest;
  double dist_to_seg;
  double vecX;
  double vecY;

  while (!stack.empty())
  {
     temporary = stack.takeLast();
     anchor = temporary.anchor;
     floater = temporary.floater;
     // initialize line segment
     if (pts[floater] != pts[anchor])
     {
       anchorX = pts[floater].x() - pts[anchor].x();
       anchorY = pts[floater].y() - pts[anchor].y();
       seg_len = sqrt(anchorX * anchorX + anchorY * anchorY);
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
      for (int i = anchor + 1; i < floater; i++)
      {
        dist_to_seg = 0.0;
        // compare to anchor
        vecX = pts[i].x() - pts[anchor].x();
        vecY = pts[i].y() - pts[anchor].y();
        seg_len = sqrt( vecX * vecX + vecY * vecY );
        // dot product:
        double proj = vecX * anchorX + vecY * anchorY;
        if (proj < 0.0)
        {
          dist_to_seg = seg_len;
        }
        else
        {
          // compare to floater
          vecX = pts[i].x() - pts[floater].x();
          vecY = pts[i].y() - pts[floater].y();
          seg_len = sqrt( vecX * vecX + vecY *vecY );
          // dot product:
          proj = vecX * (-anchorX) + vecY * (-anchorY);
          if (proj < 0.0)
          {
            dist_to_seg = seg_len;
          }
          else
          {  // calculate perpendicular distance to line (pythagorean theorem):
            dist_to_seg = sqrt(fabs(seg_len * seg_len - proj * proj));
          }
          if (max_dist < dist_to_seg)
          {
            max_dist = dist_to_seg;
            farthest = i;
          }
         }
      }
      if (max_dist <= tolerance)
      { // # use line segment
        keep.insert(anchor);
        keep.insert(floater);
      }
      else
      {
        StackEntry s = {anchor, farthest};
        stack.append(s);

        StackEntry r = {farthest, floater};
        stack.append(r);
      }
    }

    QList<int> keep2 = keep.toList();
    qSort(keep2);

    QVector<QgsPoint> result;
    int position;
    while (!keep2.empty())
    {
      position = keep2.takeFirst();
      result.append(pts[position]);
    }
    return result;
}
