/***************************************************************************
    qgsmaptoolselect.cpp  -  map tool for selecting features by single click
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


#include "qgsmaptoolselect.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include "qgis.h"

#include <QMouseEvent>
#include <QRect>
#include <QColor>
#include <QScopedPointer>


QgsMapToolSelect::QgsMapToolSelect( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
  mToolName = tr( "Select" );
  mCursor = Qt::ArrowCursor;
  mFillColor = QColor( 254, 178, 76, 63 );
  mBorderColour = QColor( 254, 58, 29, 100 );
}

void QgsMapToolSelect::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  QgsVectorLayer* vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( mCanvas );
  if ( !vlayer )
    return;

  QgsRubberBand rubberBand( mCanvas, QGis::Polygon );
  rubberBand.setFillColor( mFillColor );
  rubberBand.setBorderColor( mBorderColour );
  QRect selectRect( 0, 0, 0, 0 );
  QgsMapToolSelectUtils::expandSelectRectangle( selectRect, vlayer, e->pos() );
  QgsMapToolSelectUtils::setRubberBand( mCanvas, selectRect, &rubberBand );
  QScopedPointer< QgsGeometry > selectGeom( rubberBand.asGeometry() );
  QgsMapToolSelectUtils::selectSingleFeature( mCanvas, selectGeom.data(), e );
  rubberBand.reset( QGis::Polygon );
}
