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
/* $Id$ */


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


QgsMapToolSelect::QgsMapToolSelect( QgsMapCanvas* canvas )
  : QgsMapTool( canvas )
{
  mCursor = Qt::ArrowCursor;
}

void QgsMapToolSelect::canvasReleaseEvent( QMouseEvent * e )
{
  QgsVectorLayer* vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( mCanvas );
  if( vlayer == NULL )
  {
    return;
  }
  QgsRubberBand rubberBand( mCanvas, true );
  QRect selectRect( 0, 0, 0, 0 );
  QgsMapToolSelectUtils::expandSelectRectangle( selectRect, vlayer, e->pos() );
  QgsMapToolSelectUtils::setRubberBand( mCanvas, selectRect, &rubberBand );
  QgsGeometry* selectGeom = rubberBand.asGeometry();
  bool doDifference = e->modifiers() & Qt::ControlModifier ? true : false;
  QgsMapToolSelectUtils::setSelectFeatures( mCanvas, selectGeom, false, doDifference, true );
  delete selectGeom;
  rubberBand.reset( true );
}
