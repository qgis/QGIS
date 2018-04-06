/***************************************************************************
qgsmaptoolselectradius.cpp  -  map tool for selecting features by radius
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


#include <cmath>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

#include "qgisapp.h"
#include "qgsmaptoolselectradius.h"
#include "qgsmaptoolselectutils.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgis.h"
#include "qgslogger.h"
#include "qgssnapindicator.h"
#include "qgsmaptoolselectionhandler.h"

QgsMapToolSelectRadius::QgsMapToolSelectRadius( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator >( canvas ) )
{
  mCursor = Qt::ArrowCursor;
  mSelectionHandler = new QgsMapToolSelectionHandler( canvas );
  mSelectionHandler->setIface( reinterpret_cast<QgisInterface *>( QgisApp::instance()->getQgisInterface() ) );
}

QgsMapToolSelectRadius::~QgsMapToolSelectRadius()
{
  mSelectionHandler->deactivate();
  delete mSelectionHandler;
}

void QgsMapToolSelectRadius::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( e->mapPointMatch() );
  mSelectionHandler->selectRadiusMoveEvent( e );
}


void QgsMapToolSelectRadius::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionHandler->mQgisInterface )
  {
    mSelectionHandler->setIface( reinterpret_cast<QgisInterface *>( QgisApp::instance()->getQgisInterface() ) );
  }
  mSelectionHandler->selectRadiusReleaseEvent( e );
  if ( mSelectionHandler->mSelectFeatures )
  {
    QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, mSelectionHandler->selectedGeometry(), e->modifiers(), QgisApp::instance()->messageBar() );
    mSelectionHandler->mSelectFeatures = false;
  }
}

void QgsMapToolSelectRadius::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mSelectionHandler->deactivate();
  QgsMapTool::deactivate();
}

void QgsMapToolSelectRadius::keyReleaseEvent( QKeyEvent *e )
{
  if ( mSelectionHandler->escapeSelection( e ) )
  {
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}
