/***************************************************************************
                              qgsmapcanvassnapper.cpp    
                              -----------------------
  begin                : June 21, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapcanvassnapper.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include <QSettings>

QgsMapCanvasSnapper::QgsMapCanvasSnapper(QgsMapCanvas* canvas): mMapCanvas(canvas), mSnapper(0)
{
  if(canvas)
    {
      QgsMapRender* canvasRender = canvas->mapRender();
      if(canvasRender)
	{
	  mSnapper = new QgsSnapper(canvasRender);
	}
    }
}

QgsMapCanvasSnapper::QgsMapCanvasSnapper(): mMapCanvas(0), mSnapper(0)
{

}

QgsMapCanvasSnapper::~QgsMapCanvasSnapper()
{
  delete mSnapper;
}

void QgsMapCanvasSnapper::setMapCanvas(QgsMapCanvas* canvas)
{
  mMapCanvas = canvas;
  if(mMapCanvas)
    {
      mSnapper = new QgsSnapper(canvas->mapRender());
    }
  else
    {
      mSnapper = 0;
    }
}

int QgsMapCanvasSnapper::snapToCurrentLayer(const QPoint& p, QList<QgsSnappingResult>& results, QgsSnapper::SNAP_TO snap_to, double snappingTol)
{
  results.clear();
  
  if(mSnapper && mMapCanvas)
    {

      //topological editing on?
      int topologicalEditing = QgsProject::instance()->readNumEntry("Digitizing", "/TopologicalEditing", 0);
      if(topologicalEditing == 0)
	{
	  mSnapper->setSnapMode(QgsSnapper::ONE_RESULT);
	}
      else
	{
	  mSnapper->setSnapMode(QgsSnapper::SEVERAL_RESULTS_SAME_POSITION);
	}

      //current vector layer
      QgsMapLayer* currentLayer = mMapCanvas->currentLayer();
      if(!currentLayer)
	{
	  return 2;
	}
      QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>(currentLayer);
      if(!vlayer)
	{
	  return 3;
	}

      QList<QgsVectorLayer*> layerList;
      QList<double> toleranceList;
      QList<QgsSnapper::SNAP_TO> snapToList;

      layerList.push_back(vlayer);
      snapToList.push_back(snap_to);

      QSettings settings;

      if(snappingTol < 0)
	{
	  //use search tolerance for vertex editing
	  toleranceList.push_back(settings.value("/qgis/digitizing/search_radius_vertex_edit", 50).toDouble());
	}
      else
	{
	  toleranceList.push_back(snappingTol);
	}
      

      mSnapper->setLayersToSnap(layerList);
      mSnapper->setTolerances(toleranceList);
      mSnapper->setSnapToList(snapToList);

      if(mSnapper->snapPoint(p, results) != 0)
	{
	  return 4;
	}
      
      return 0;
    }
  else
    {
      return 1;
    }
}

int QgsMapCanvasSnapper::snapToBackgroundLayers(const QPoint& p, QList<QgsSnappingResult>& results)
{
  results.clear();

  if(mSnapper)
    {
      //topological editing on?
      int topologicalEditing = QgsProject::instance()->readNumEntry("Digitizing", "/TopologicalEditing", 0);
      if(topologicalEditing == 0)
	{
	  mSnapper->setSnapMode(QgsSnapper::ONE_RESULT);
	}
      else
	{
	  mSnapper->setSnapMode(QgsSnapper::SEVERAL_RESULTS_SAME_POSITION);
	}

      //read snapping settings from project
      bool ok; //todo: take the default snapping tolerance for all vector layers if snapping not defined in project
      bool snappingDefinedInProject = true;
      QStringList layerIdList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnappingList", &ok);
      if(!ok)
	{
	  snappingDefinedInProject = false;
	}
      QStringList enabledList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnappingEnabledList", &ok);
      QStringList toleranceList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnappingToleranceList", &ok); 
      QStringList snapToList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnapToList", &ok);

      if(!(layerIdList.size() == enabledList.size() && layerIdList.size() == toleranceList.size() && layerIdList.size() == snapToList.size()))
	{
	  return 1; //lists must have the same size, otherwise something is wrong
	}

      QList<QgsVectorLayer*> vectorLayerList;
      QList<double> toleranceDoubleList;
      QList<QgsSnapper::SNAP_TO> snapTo;

      //Use snapping information from the project
      if(snappingDefinedInProject)
	{
	  //set layers, tolerances, snap to segment/vertex to QgsSnapper
	  QgsMapLayer* layer = 0;
	  QgsVectorLayer* vlayer = 0;
	  
	  QStringList::const_iterator layerIt = layerIdList.constBegin();
	  QStringList::const_iterator tolIt = toleranceList.constBegin();
	  QStringList::const_iterator snapIt = snapToList.constBegin();
	  QStringList::const_iterator enabledIt = enabledList.constBegin();
	  
	  for(; layerIt != layerIdList.constEnd(); ++layerIt, ++tolIt, ++snapIt, ++enabledIt)
	    {
	      if( (*enabledIt) != "enabled") //skip layer if snapping is not enabled
		{
		  continue;
		}
	      
	      //layer
	      layer = QgsMapLayerRegistry::instance()->mapLayer(*layerIt);
	      if(layer)
		{
		  vlayer = dynamic_cast<QgsVectorLayer*>(layer);
		  if(vlayer)
		    {
		      vectorLayerList.push_back(vlayer);
		    }
		}
	      
	      //tolerance
	      toleranceDoubleList.push_back(tolIt->toDouble());
	      
	      //segment or vertex
	      if( (*snapIt) == "to_vertex" )
		{
		  snapTo.push_back(QgsSnapper::SNAP_TO_VERTEX);
		}
	      else if( (*snapIt) == "to_segment")
		{
		  snapTo.push_back(QgsSnapper::SNAP_TO_SEGMENT);
		}
	      else //to vertex and segment
		{
		  snapTo.push_back(QgsSnapper::SNAP_TO_VERTEX_AND_SEGMENT);
		}

	    }
	}
      else //nothing in project. Use default snapping tolerance to vertex of current layer
	{
	  QgsMapLayer* currentLayer = mMapCanvas->currentLayer();
	  if(!currentLayer)
	    {
	      return 2;
	    }
	      
	  QgsVectorLayer* currentVectorLayer = dynamic_cast<QgsVectorLayer*>(currentLayer);
	  if(!currentVectorLayer)
	    {
	      return 3;
	    }
	  
	  vectorLayerList.push_back(currentVectorLayer);
	  QSettings settings;

	  //default snap mode
	  QString defaultSnapString = settings.value("/qgis/digitizing/default_snap_mode", "to vertex").toString();
	  if(defaultSnapString == "to segment")
	    {
	      snapTo.push_back(QgsSnapper::SNAP_TO_SEGMENT);
	    }
	  else if(defaultSnapString == "to vertex and segment")
	    {
	      snapTo.push_back(QgsSnapper::SNAP_TO_VERTEX_AND_SEGMENT);
	    }
	  else
	    {
	      snapTo.push_back(QgsSnapper::SNAP_TO_VERTEX);
	    }
	  
	  //default snapping tolerance
	  toleranceDoubleList.push_back(settings.value("/qgis/digitizing/default_snapping_tolerance", 0).toDouble());
	}

      mSnapper->setLayersToSnap(vectorLayerList);
      mSnapper->setTolerances(toleranceDoubleList);
      mSnapper->setSnapToList(snapTo);

      if(mSnapper->snapPoint(p, results) != 0)
	{
	  return 4;
	}
      return 0;
    }
  else
    {
      return 5;
    }
}
