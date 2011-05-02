/***************************************************************************
    qgsmaptooledit.cpp  -  base class for editing map tools
    ---------------------
    begin                : Juli 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsmaptooledit.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QKeyEvent>
#include <QSettings>

QgsMapToolEdit::QgsMapToolEdit( QgsMapCanvas* canvas ): QgsMapTool( canvas )
{
  mSnapper.setMapCanvas( canvas );
}


QgsMapToolEdit::~QgsMapToolEdit()
{

}

int QgsMapToolEdit::insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults, QgsVectorLayer* editedLayer )
{
  QgsPoint layerPoint;

  if ( !editedLayer || !editedLayer->isEditable() )
  {
    return 1;
  }

  //transform snaping coordinates to layer crs first
  QList<QgsSnappingResult> transformedSnapResults = snapResults;
  QList<QgsSnappingResult>::iterator it = transformedSnapResults.begin();
  for ( ; it != transformedSnapResults.constEnd(); ++it )
  {
    QgsPoint layerPoint = toLayerCoordinates( editedLayer, it->snappedVertex );
    it->snappedVertex = layerPoint;
  }

  return editedLayer->insertSegmentVerticesForSnap( transformedSnapResults );
}

QgsPoint QgsMapToolEdit::snapPointFromResults( const QList<QgsSnappingResult>& snapResults, const QPoint& screenCoords )
{
  if ( snapResults.size() < 1 )
  {
    return toMapCoordinates( screenCoords );
  }
  else
  {
    return snapResults.constBegin()->snappedVertex;
  }
}

QgsRubberBand* QgsMapToolEdit::createRubberBand( bool isPolygon )
{
  QSettings settings;
  QgsRubberBand* rb = new QgsRubberBand( mCanvas, isPolygon );
  QColor color( settings.value( "/qgis/digitizing/line_color_red", 255 ).toInt(),
                settings.value( "/qgis/digitizing/line_color_green", 0 ).toInt(),
                settings.value( "/qgis/digitizing/line_color_blue", 0 ).toInt() );
  rb->setColor( color );
  rb->setWidth( settings.value( "/qgis/digitizing/line_width", 1 ).toInt() );
  rb->show();
  return rb;
}

QgsVectorLayer* QgsMapToolEdit::currentVectorLayer()
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
  {
    return 0;
  }

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( !vlayer )
  {
    return 0;
  }
  return vlayer;
}


int QgsMapToolEdit::addTopologicalPoints( const QList<QgsPoint>& geom )
{
  if ( !mCanvas )
  {
    return 1;
  }

  //find out current vector layer
  QgsVectorLayer *vlayer = currentVectorLayer();

  if ( !vlayer )
  {
    return 2;
  }

  QList<QgsPoint>::const_iterator list_it = geom.constBegin();
  for ( ; list_it != geom.constEnd(); ++list_it )
  {
    vlayer->addTopologicalPoints( *list_it );
  }
  return 0;
}


