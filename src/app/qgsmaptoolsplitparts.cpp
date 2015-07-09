/***************************************************************************
    qgsmaptoolsplitparts.h
    ---------------------
    begin                : April 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsmaptoolsplitparts.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"

#include <QMouseEvent>

QgsMapToolSplitParts::QgsMapToolSplitParts( QgsMapCanvas* canvas )
    : QgsMapToolCapture( canvas, QgsMapToolCapture::CaptureLine )
{
  mToolName = tr( "Split parts" );
}

QgsMapToolSplitParts::~QgsMapToolSplitParts()
{

}

void QgsMapToolSplitParts::canvasMapReleaseEvent( QgsMapMouseEvent * e )
{
  //check if we operate on a vector layer
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  bool split = false;

  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    //If we snap the first point on a vertex of a line layer, we directly split the feature at this point
    if ( vlayer->geometryType() == QGis::Line && points().isEmpty() )
    {
      QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Vertex );
      if ( m.isValid() )
      {
        split = true;
      }
    }

    int error = addVertex( e->mapPoint() );
    if ( error == 1 )
    {
      //current layer is not a vector layer
      return;
    }
    else if ( error == 2 )
    {
      //problem with coordinate transformation
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Coordinate transform error" ),
        tr( "Cannot transform the point to the layers coordinate system" ),
        QgsMessageBar::INFO,
        QgisApp::instance()->messageTimeout() );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    split = true;
  }
  if ( split )
  {
    deleteTempRubberBand();

    //bring up dialog if a split was not possible (polygon) or only done once (line)
    int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
    vlayer->beginEditCommand( tr( "Parts split" ) );
    int returnCode = vlayer->splitParts( points(), topologicalEditing );
    vlayer->endEditCommand();
    if ( returnCode == 4 )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "No parts were split" ),
        tr( "If there are selected parts, the split tool only applies to those. If you would like to split all parts under the split line, clear the selection." ),
        QgsMessageBar::WARNING,
        QgisApp::instance()->messageTimeout() );
    }
    else if ( returnCode == 3 )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "No part split done" ),
        tr( "Cut edges detected. Make sure the line splits parts into multiple parts." ),
        QgsMessageBar::WARNING,
        QgisApp::instance()->messageTimeout() );
    }
    else if ( returnCode == 7 )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "No part split done" ),
        tr( "The geometry is invalid. Please repair before trying to split it." ),
        QgsMessageBar::WARNING,
        QgisApp::instance()->messageTimeout() );
    }
    else if ( returnCode != 0 )
    {
      //several intersections but only one split (most likely line)
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Split error" ),
        tr( "An error occured during splitting." ),
        QgsMessageBar::WARNING,
        QgisApp::instance()->messageTimeout() );
    }

    stopCapturing();
  }
}
