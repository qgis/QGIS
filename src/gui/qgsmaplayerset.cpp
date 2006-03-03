/***************************************************************************
    qgsmaplayerset.cpp  -  holds a set of layers
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


#include "qgsmaplayerset.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"


void QgsMapLayerSet::setLayerSet(const std::deque<QString>& layers)
{
  mLayerSet = layers;
  updateFullExtent();
}


void QgsMapLayerSet::updateFullExtent()
{
#ifdef QGISDEBUG
  std::cout << "QgsMapLayerSet::updateFullExtent() called !" << std::endl;
#endif
    
  QgsMapLayerRegistry* registry = QgsMapLayerRegistry::instance();
  bool projectionsEnabled = (QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0)!=0);
  
  // reset the map canvas extent since the extent may now be smaller
  // We can't use a constructor since QgsRect normalizes the rectangle upon construction
  mFullExtent.setMinimal();
  
  // iterate through the map layers and test each layers extent
  // against the current min and max values
  std::deque<QString>::iterator it = mLayerSet.begin();
  while(it != mLayerSet.end())
  {
    QgsMapLayer * lyr = registry->mapLayer(*it);
    if (lyr == NULL)
    {
      std::cout << "WARNING: layer '" << (*it).toLocal8Bit().data()
          << "' not found in map layer registry!" << std::endl;
    }
    else
    {
    
#ifdef QGISDEBUG 
      std::cout << "Updating extent using " << lyr->name().toLocal8Bit().data() << std::endl;
      std::cout << "Input extent: " << lyr->extent().stringRep().toLocal8Bit().data() << std::endl;
#endif 
      // Layer extents are stored in the coordinate system (CS) of the
      // layer. The extent must be projected to the canvas CS prior to passing
      // on to the updateFullExtent function
      if (projectionsEnabled)
      {
        try
        {
          if ( ! lyr->coordinateTransform() )
            throw QgsCsException( string("NO COORDINATE TRANSFORM FOUND FOR LAYER") );
              
          mFullExtent.unionRect(lyr->coordinateTransform()->transformBoundingBox(lyr->extent()));
        }
        catch (QgsCsException &cse)
        {
          qDebug( "Transform error caught in %s line %d:\n%s", __FILE__, __LINE__, cse.what());
        }
      }
      else
      {
        mFullExtent.unionRect(lyr->extent());
      }
      
    }
    it++;
  } 
}
