/***************************************************************************
                              qgsinterpolationlayerbuilder.cpp   
                              --------------------------------
  begin                : July, 2008
  copyright            : (C) 2008 by Marco Hugentobler
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

#include "qgsinterpolationlayerbuilder.h"
#include "qgsmapserverlogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QTemporaryFile>

//for interpolation
#include "qgsgridfilewriter.h"
#include "qgsidwinterpolator.h"
#include "qgstininterpolator.h"

QgsInterpolationLayerBuilder::QgsInterpolationLayerBuilder(QgsVectorLayer* vl): mVectorLayer(vl)
{

}

QgsInterpolationLayerBuilder::QgsInterpolationLayerBuilder(): mVectorLayer(0)
{

}

QgsInterpolationLayerBuilder::~QgsInterpolationLayerBuilder()
{

}

QgsMapLayer* QgsInterpolationLayerBuilder::createMapLayer(const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching) const
{
  if(!mVectorLayer)
    {
      return 0;
    }

  QDomNodeList interpolationList = elem.elementsByTagName("Interpolation");
  if(interpolationList.size() < 1)
    {
      QgsMSDebugMsg("No Interpolation element found");
      return 0;
    }
  QDomElement interpolationElem = interpolationList.at(0).toElement();
  
  //create QgsInterpolator object from XML
  QDomNodeList tinList = interpolationElem.elementsByTagName("TINMethod");
  QDomNodeList idwList = interpolationElem.elementsByTagName("IDWMethod");
  
  QgsInterpolator* theInterpolator = 0;
  QList<QgsInterpolator::LayerData> layerDataList;
  QgsInterpolator::LayerData currentLayerData;
  currentLayerData.vectorLayer = mVectorLayer;
  QDomNodeList propertyNameList = interpolationElem.elementsByTagName("PropertyName");
  if(propertyNameList.size() < 1)
    {
      currentLayerData.zCoordInterpolation = true;
    }
  else
    {
      currentLayerData.zCoordInterpolation = false;

      //set attribute field interpolation or z-Coordinate interpolation
      QString attributeName = propertyNameList.at(0).toElement().text();
       QgsVectorDataProvider* provider = mVectorLayer->dataProvider();
       if(!provider)
        {
          return 0;
        }
      int attributeIndex = provider->fieldNameIndex(attributeName);
      if(attributeIndex == -1)
        {
          return 0; //attribute field not found
        }
      currentLayerData.interpolationAttribute = attributeIndex;
    }
  currentLayerData.mInputType = QgsInterpolator::POINTS;

  layerDataList.push_back(currentLayerData);

  if(idwList.size() > 0) //inverse distance interpolator
    {
      theInterpolator = new QgsIDWInterpolator(layerDataList);
      
      //todo: parse <DistanceWeightingCoefficient>
    }
  else //tin is default
    {
      theInterpolator = new QgsTINInterpolator(layerDataList);
      //todo: parse <InterpolationFunction>
    }  

  //Resolution
  int nCols, nRows;

  QDomNodeList resolutionNodeList = elem.elementsByTagName("Resolution");
  if(resolutionNodeList.size() < 1)
    {
      //use default values...
      nCols = 100;
      nRows = 100;
    }
  else
    {
      QDomElement resolutionElem = resolutionNodeList.at(0).toElement();
      nCols = resolutionElem.attribute("ncols").toInt();
      nRows = resolutionElem.attribute("nrows").toInt();
      if(nCols == 0 && nRows == 0)
	{
	  QgsMSDebugMsg("Reading of resolution failed");
	  return 0;
	}
    }

  QTemporaryFile* tmpFile = new QTemporaryFile();
  if(!tmpFile->open())
  {
    delete tmpFile;
  }

  QgsRectangle extent = mVectorLayer->extent();
  QgsGridFileWriter gridWriter(theInterpolator, tmpFile->fileName(), extent, nCols, nRows, extent.width() / nCols, extent.height() / nRows);
  if(gridWriter.writeFile(false) != 0)
    {
      QgsMSDebugMsg("Interpolation of raster failed");
      return 0;
    }

  filesToRemove.push_back(tmpFile); //store raster in temporary file and remove after request
  QgsRasterLayer* theRaster = new QgsRasterLayer(tmpFile->fileName());
  layersToRemove.push_back(theRaster);
  return theRaster;
}
