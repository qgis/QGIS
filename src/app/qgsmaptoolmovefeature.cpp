/***************************************************************************
    qgsmaptoolmovefeature.cpp  -  map tool for translating features by mouse drag
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

#include "qgsmaptoolmovefeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <limits>
QgsMapToolMoveFeature::QgsMapToolMoveFeature(QgsMapCanvas* canvas): QgsMapToolEdit(canvas), mRubberBand(0)
{
  
}

QgsMapToolMoveFeature::~QgsMapToolMoveFeature()
{
  delete mRubberBand;
}

void QgsMapToolMoveFeature::canvasMoveEvent(QMouseEvent * e)
{
  if(mRubberBand)
    {
      QgsPoint pointCanvasCoords = toMapCoords(e->pos());
      double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
      double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
      mRubberBand->setTranslationOffset(offsetX, offsetY);
      //QgsDebugMsg("move Rubber band by: " + QString::number(e->x() - mLastPointPixelCoords.x()) + " // " + QString::number(e->y() - mLastPointPixelCoords.y()));
      mRubberBand->updatePosition();
      mRubberBand->update();
    }
}

void QgsMapToolMoveFeature::canvasPressEvent(QMouseEvent * e)
{
  delete mRubberBand;
  mRubberBand = 0;

  QgsVectorLayer* vlayer = currentVectorLayer();
  if(!vlayer)
    {
      return;
    }

  if(!vlayer->isEditable())
    {
      QMessageBox::information(0, QObject::tr("Layer not editable"), \
			       QObject::tr("Cannot edit the vector layer. To make it editable, go to the file item " \
					   "of the layer, right click and check 'Allow Editing'."));
      return;
    }

  //find first geometry under mouse cursor and store iterator to it
  QgsPoint layerCoords = toLayerCoords((QgsMapLayer*)vlayer, e->pos());
  QSettings settings;
  int searchRadius = settings.value("/qgis/digitizing/search_radius_vertex_edit", 10).toInt();
  QgsRect selectRect(layerCoords.x()-searchRadius, layerCoords.y()-searchRadius, \
		     layerCoords.x()+searchRadius, layerCoords.y()+searchRadius);

  QList<QgsFeature> featureList;
  vlayer->featuresInRectangle(selectRect, featureList, true, false);
  
  if(featureList.size() > 0)
    {
      //find the closest feature
      QgsGeometry* pointGeometry = QgsGeometry::fromPoint(layerCoords);
      if(!pointGeometry)
	{
	  return;
	}

      QList<QgsFeature>::iterator closestFeatureIt;
      double minDistance = std::numeric_limits<double>::max();
      double currentDistance;
      
      QList<QgsFeature>::iterator it = featureList.begin();
      for(; it != featureList.end(); ++it)
	{
	  if(it->geometry())
	    {
	      currentDistance = pointGeometry->distance(*(it->geometry()));
	      if(currentDistance < minDistance)
		{
		  minDistance = currentDistance;
		  closestFeatureIt = it;
		}
	    }
	}
      
      mStartPointMapCoords = toMapCoords(e->pos());
      mMovedFeature = closestFeatureIt->featureId(); //todo: take the closest feature, not the first one...
      mRubberBand = createRubberBand();
      mRubberBand->setToGeometry(closestFeatureIt->geometry(), *vlayer);
      mRubberBand->setColor(Qt::red);
      mRubberBand->setWidth(2);
      mRubberBand->show();

      delete pointGeometry;
    }
}

void QgsMapToolMoveFeature::canvasReleaseEvent(QMouseEvent * e)
{
  //QgsDebugMsg("QgsMapToolMoveFeature::canvasReleaseEvent");
  if(!mRubberBand)
    {
      return;
    }

  QgsVectorLayer* vlayer = currentVectorLayer();
  if(!vlayer)
    {
      return;
    }

  QgsPoint startPointLayerCoords = toLayerCoords((QgsMapLayer*)vlayer, mStartPointMapCoords);
  QgsPoint stopPointLayerCoords = toLayerCoords((QgsMapLayer*)vlayer, e->pos());

  double dx = stopPointLayerCoords.x() - startPointLayerCoords.x();
  double dy = stopPointLayerCoords.y() - startPointLayerCoords.y();

  vlayer->translateFeature(mMovedFeature, dx, dy);

  delete mRubberBand;
  mRubberBand = 0;
  mCanvas->refresh();
}

//! called when map tool is being deactivated
void QgsMapToolMoveFeature::deactivate()
{
  //delete rubber band
  delete mRubberBand;
  mRubberBand = 0;
}
