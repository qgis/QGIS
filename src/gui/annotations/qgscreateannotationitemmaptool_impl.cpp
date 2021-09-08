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
#include "qgsannotationlayer.h"
#include "qgsstyle.h"

///@cond PRIVATE

QgsCreatePointTextItemMapTool::QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsCreateAnnotationItemMapTool( canvas, cadDockWidget )
{

}

QgsCreatePointTextItemMapTool::~QgsCreatePointTextItemMapTool() = default;

void QgsCreatePointTextItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( targetLayer(), event->mapPoint() );

  mCreatedItem = std::make_unique< QgsAnnotationPointTextItem >( tr( "Text" ), layerPoint );
  mCreatedItem->setAlignment( Qt::AlignLeft );
  mCreatedItem->setFormat( QgsStyle::defaultStyle()->defaultTextFormat( QgsStyle::TextFormatContext::Labeling ) );
  emit itemCreated();
}

QgsAnnotationItem *QgsCreatePointTextItemMapTool::takeCreatedItem()
{
  return mCreatedItem.release();
}


///@endcond PRIVATE
