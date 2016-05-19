/***************************************************************************
    qgsmaptoolselectrectangle.cpp  -  map tool for selecting features by
                                   rectangle
    ----------------------
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

#include "qgsmaptoolselectrectangle.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include "qgscursors.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include "qgis.h"

#include <QMouseEvent>
#include <QRect>


QgsMapToolSelectFeatures::QgsMapToolSelectFeatures( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mDragging( false )
{
  mToolName = tr( "Select features" );
  QPixmap mySelectQPixmap = QPixmap(( const char ** ) select_cursor );
  mCursor = QCursor( mySelectQPixmap, 1, 1 );
  mRubberBand = nullptr;
  mFillColor = QColor( 254, 178, 76, 63 );
  mBorderColour = QColor( 254, 58, 29, 100 );
}


void QgsMapToolSelectFeatures::canvasPressEvent( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );
  mSelectRect.setRect( 0, 0, 0, 0 );
  delete mRubberBand;
  mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
  mRubberBand->setFillColor( mFillColor );
  mRubberBand->setBorderColor( mBorderColour );
}


void QgsMapToolSelectFeatures::canvasMoveEvent( QgsMapMouseEvent* e )
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


void QgsMapToolSelectFeatures::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  QgsVectorLayer* vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( mCanvas );
  if ( !vlayer )
  {
    delete mRubberBand;
    mRubberBand = nullptr;
    mDragging = false;
    return;
  }

  //if the user simply clicked without dragging a rect
  //we will fabricate a small 1x1 pix rect and then continue
  //as if they had dragged a rect
  if ( !mDragging )
  {
    QgsMapToolSelectUtils::expandSelectRectangle( mSelectRect, vlayer, e->pos() );
  }
  else
  {
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

  if ( mRubberBand )
  {
    QgsMapToolSelectUtils::setRubberBand( mCanvas, mSelectRect, mRubberBand );

    QgsGeometry* selectGeom = mRubberBand->asGeometry();
    if ( !mDragging )
    {
      QgsMapToolSelectUtils::selectSingleFeature( mCanvas, selectGeom, e );
    }
    else
      QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, selectGeom, e );

    delete selectGeom;

    delete mRubberBand;
    mRubberBand = nullptr;
  }

  mDragging = false;
}
