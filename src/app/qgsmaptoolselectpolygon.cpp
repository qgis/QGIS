/***************************************************************************
qgsmaptoolselectpolygon.cpp  -  map tool for selecting features by polygon
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaptoolselectpolygon.h"
#include "qgsmaptoolselectutils.h"
#include "qgsgeometry.h"
#include "qgsmaptoolselectionhandler.h"
#include "qgsmapcanvas.h"
#include "qgis.h"

#include <QMouseEvent>

class QgsMapToolSelectionHandler;


QgsMapToolSelectPolygon::QgsMapToolSelectPolygon( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mCursor = Qt::ArrowCursor;
  mSelectionHandler = new QgsMapToolSelectionHandler( canvas );
}

QgsMapToolSelectPolygon::~QgsMapToolSelectPolygon()
{
  delete mSelectionHandler;
}

void QgsMapToolSelectPolygon::canvasPressEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->selectPolygonReleaseEvent( e );
  if ( mSelectionHandler->mSelectFeatures )
  {
    QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, mSelectionHandler->selectedGeometry(), e->modifiers() );
  }
}

void QgsMapToolSelectPolygon::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->selectPolygonMoveEvent( e );
}

