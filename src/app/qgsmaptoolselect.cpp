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
    mDragging = TRUE;
    mRubberBand = new QRubberBand( QRubberBand::Rectangle, mCanvas );
    mSelectRect.setTopLeft( e->pos() );
  }

  mSelectRect.setBottomRight( e->pos() );
  mRubberBand->setGeometry( mSelectRect.normalized() );
  mRubberBand->show();
}


void QgsMapToolSelect::canvasReleaseEvent( QMouseEvent * e )
{
  if ( !mDragging )
    return;

  mDragging = FALSE;

  delete mRubberBand;
  mRubberBand = 0;

  if ( !mCanvas->currentLayer() ||
       dynamic_cast<QgsVectorLayer*>( mCanvas->currentLayer() ) == NULL )
  {
    QMessageBox::warning( mCanvas, QObject::tr( "No active layer" ),
                          QObject::tr( "To select features, you must choose a vector layer by clicking on its name in the legend" ) );
    return;
  }

  // store the rectangle
  mSelectRect.setRight( e->pos().x() );
  mSelectRect.setBottom( e->pos().y() );

  const QgsMapToPixel* transform = mCanvas->getCoordinateTransform();
  QgsPoint ll = transform->toMapCoordinates( mSelectRect.left(), mSelectRect.bottom() );
  QgsPoint ur = transform->toMapCoordinates( mSelectRect.right(), mSelectRect.top() );

  QgsRect search( ll.x(), ll.y(), ur.x(), ur.y() );

  // if Ctrl key is pressed, selected features will be added to selection
  // instead of removing old selection
  bool lock = ( e->modifiers() & Qt::ControlModifier );

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( mCanvas->currentLayer() );
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
    QMessageBox::warning( mCanvas, QObject::tr( "CRS Exception" ),
                          QObject::tr( "Selection extends beyond layer's coordinate system." ) );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );
  vlayer->select( search, lock );
  QApplication::restoreOverrideCursor();
}
