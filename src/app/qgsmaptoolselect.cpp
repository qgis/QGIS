/***************************************************************************
    qgsmaptoolselect.cpp  -  map tool for selecting features
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
/* $Id$ */

#include "qgsmaptoolselect.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include "qgscsexception.h"
#include "qgscursors.h"
#include "qgslogger.h"
#include "qgis.h"

#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRubberBand>
#include <QRect>


QgsMapToolSelect::QgsMapToolSelect( QgsMapCanvas* canvas )
    : QgsMapTool( canvas ), mDragging( false )
{
  QPixmap mySelectQPixmap = QPixmap(( const char ** ) select_cursor );
  mCursor = QCursor( mySelectQPixmap, 1, 1 );
  mRubberBand = 0;
}


void QgsMapToolSelect::canvasPressEvent( QMouseEvent * e )
{
  mSelectRect.setRect( 0, 0, 0, 0 );
}


void QgsMapToolSelect::canvasMoveEvent( QMouseEvent * e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  if ( !mDragging )
  {
    mDragging = true;
    mRubberBand = new QRubberBand( QRubberBand::Rectangle, mCanvas );
    mSelectRect.setTopLeft( e->pos() );
  }

  mSelectRect.setBottomRight( e->pos() );
  mRubberBand->setGeometry( mSelectRect.normalized() );
  mRubberBand->show();
}


void QgsMapToolSelect::canvasReleaseEvent( QMouseEvent * e )
{
  if ( !mCanvas->currentLayer() ||
       qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ) == NULL )
  {
    QMessageBox::warning( mCanvas, tr( "No active layer" ),
                          tr( "To select features, you must choose a "
                              "vector layer by clicking on its name in the legend"
                            ) );
    return;
  }
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  //if the user simply clicked without dragging a rect
  //we will fabricate a small 1x1 pix rect and then continue
  //as if they had dragged a rect
  if ( !mDragging )
  {
    int boxSize = 0;
    if ( vlayer->geometryType() != QGis::Polygon )
    {
      //if point or line use an artificial bounding box of 10x10 pixels
      //to aid the user to click on a feature accurately
      boxSize = 5;
    }
    else
    {
      //otherwise just use the click point for polys
      boxSize = 1;
    }
    mSelectRect.setLeft( e->pos().x() - boxSize );
    mSelectRect.setRight( e->pos().x() + boxSize );
    mSelectRect.setTop( e->pos().y() - boxSize );
    mSelectRect.setBottom( e->pos().y() + boxSize );
  }
  else
  {
    delete mRubberBand;
    mRubberBand = 0;

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

  mDragging = false;

  const QgsMapToPixel* transform = mCanvas->getCoordinateTransform();
  QgsPoint ll = transform->toMapCoordinates( mSelectRect.left(), mSelectRect.bottom() );
  QgsPoint ur = transform->toMapCoordinates( mSelectRect.right(), mSelectRect.top() );

  QgsRectangle search( ll.x(), ll.y(), ur.x(), ur.y() );

  // if Ctrl key is pressed, selected features will be flipped in selection
  // instead of removing old selection
  bool flip = ( e->modifiers() & Qt::ControlModifier );

  // toLayerCoordinates will throw an exception for an 'invalid' rectangle.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    search = toLayerCoordinates( vlayer, search );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' rectangle and leave existing selection unchanged
    QgsLogger::warning( "Caught CRS exception " + QString( __FILE__ ) + ": " + QString::number( __LINE__ ) );
    QMessageBox::warning( mCanvas, tr( "CRS Exception" ),
                          tr( "Selection extends beyond layer's coordinate system." ) );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );
  if ( flip )
  {
    vlayer->invertSelectionInRectangle( search );
  }
  else
  {
    vlayer->select( search, false );
  }
  QApplication::restoreOverrideCursor();
}
