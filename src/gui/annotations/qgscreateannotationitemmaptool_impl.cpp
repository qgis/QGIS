/***************************************************************************
                             qgscreateannotationitemmaptool_impl.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscreateannotationitemmaptool_impl.h"
#include "qgsmapmouseevent.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationlayer.h"
#include "qgsstyle.h"
#include "qgsmapcanvas.h"
#include "qgsmarkersymbol.h"

///@cond PRIVATE

//
// QgsCreatePointTextItemMapTool
//

QgsCreatePointTextItemMapTool::QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mHandler( new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget ) )
{

}

QgsCreatePointTextItemMapTool::~QgsCreatePointTextItemMapTool() = default;

void QgsCreatePointTextItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( mHandler->targetLayer(), event->mapPoint() );

  std::unique_ptr< QgsAnnotationPointTextItem > createdItem = std::make_unique< QgsAnnotationPointTextItem >( tr( "Text" ), layerPoint );
  createdItem->setAlignment( Qt::AlignLeft );
  createdItem->setFormat( QgsStyle::defaultStyle()->defaultTextFormat( QgsStyle::TextFormatContext::Labeling ) );
  // newly created point text items default to using symbology reference scale at the current map scale
  createdItem->setUseSymbologyReferenceScale( true );
  createdItem->setSymbologyReferenceScale( canvas()->scale() );
  mHandler->pushCreatedItem( createdItem.release() );
}

QgsCreateAnnotationItemMapToolHandler *QgsCreatePointTextItemMapTool::handler()
{
  return mHandler;
}

QgsMapTool *QgsCreatePointTextItemMapTool::mapTool()
{
  return this;
}



//
// QgsCreateMarkerMapTool
//

QgsCreateMarkerItemMapTool::QgsCreateMarkerItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mHandler( new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget ) )
{

}

QgsCreateMarkerItemMapTool::~QgsCreateMarkerItemMapTool() = default;

void QgsCreateMarkerItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( mHandler->targetLayer(), event->mapPoint() );

  std::unique_ptr< QgsAnnotationMarkerItem > createdItem = std::make_unique< QgsAnnotationMarkerItem >( QgsPoint( layerPoint ) );
  createdItem->setSymbol( qgis::down_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );
  // set reference scale to match canvas scale, but don't enable it by default for marker items
  createdItem->setSymbologyReferenceScale( canvas()->scale() );

  mHandler->pushCreatedItem( createdItem.release() );
}

QgsCreateAnnotationItemMapToolHandler *QgsCreateMarkerItemMapTool::handler()
{
  return mHandler;
}

QgsMapTool *QgsCreateMarkerItemMapTool::mapTool()
{
  return this;
}

///@endcond PRIVATE
