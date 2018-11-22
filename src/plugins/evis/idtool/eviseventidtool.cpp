/*
 *
** File: eviseventidtool.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-19
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the The John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/


#include <QObject>
#include <QMessageBox>

#include "eviseventidtool.h"

#include "qgsmaptopixel.h"
#include "qgsmaptool.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfeatureiterator.h"
#include "qgsmapmouseevent.h"


/**
* Constructor for the id style tool, this tool inherits the QgsMapTool and requires a pointer to
* to the map canvas.
* \param canvas - Pointer to the QGIS map canvas
*/
eVisEventIdTool::eVisEventIdTool( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  //set cursor
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify ) );

  //set the current tool to this object
  if ( mCanvas )
  {
    mCanvas->setMapTool( this );
  }
}

/**
* Mouse release, i.e., select, event
* \param mouseEvent - Pointer to a QMouseEvent
*/
void eVisEventIdTool::canvasReleaseEvent( QgsMapMouseEvent *mouseEvent )
{
  if ( !mCanvas || !mouseEvent )
    return;

//Check to see if there is a layer selected
  if ( mCanvas->currentLayer() )
  {
    //Check to see if the current layer is a vector layer
    if ( QgsMapLayer::VectorLayer == mCanvas->currentLayer()->type() )
    {
      select( mCanvas->getCoordinateTransform()->toMapCoordinates( mouseEvent->x(), mouseEvent->y() ) );
    }
    else
    {
      QMessageBox::warning( mCanvas, QObject::tr( "eVis Event Id Tool" ), QObject::tr( "This tool only supports vector data." ) );
    }
  }
  else
  {
    QMessageBox::warning( mCanvas, QObject::tr( "eVis Event Id Tool" ), QObject::tr( "No active layers found." ) );
  }
}

/**
* Selection routine called by the mouse release event
* \param point = QgsPointXY representing the x, y coordinates of the mouse release event
*/
void eVisEventIdTool::select( const QgsPointXY &point )
{

  if ( !mCanvas )
    return;

  QgsVectorLayer *myLayer = ( QgsVectorLayer * )mCanvas->currentLayer();

  // create the search rectangle. this was modeled after the QgsMapIdentifyTool in core QGIS application
  double searchWidth = QgsMapTool::searchRadiusMU( mCanvas );

  QgsRectangle myRectangle;
  myRectangle.setXMinimum( point.x() - searchWidth );
  myRectangle.setXMaximum( point.x() + searchWidth );
  myRectangle.setYMinimum( point.y() - searchWidth );
  myRectangle.setYMaximum( point.y() + searchWidth );

  //Transform rectangle to map coordinates
  myRectangle = toLayerCoordinates( myLayer, myRectangle );

  //select features
  QgsFeatureIterator fit = myLayer->getFeatures( QgsFeatureRequest().setFilterRect( myRectangle ).setFlags( QgsFeatureRequest::ExactIntersect ).setNoAttributes() );

  QgsFeature f;
  QgsFeatureIds newSelectedFeatures;
  while ( fit.nextFeature( f ) )
  {
    newSelectedFeatures.insert( f.id() );
  }

  myLayer->selectByIds( newSelectedFeatures );

  //Launch a new event browser to view selected features
  mBrowser = new eVisGenericEventBrowserGui( mCanvas, mCanvas, nullptr );
  mBrowser->setAttribute( Qt::WA_DeleteOnClose );
}
