/***************************************************************************
                          qgszonalstatistics.cpp  -  description
                          ----------------------------
    begin                : August 29th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgszonalstatistics.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "gdal.h"
#include "cpl_string.h"
#include <QProgressDialog>

QgsZonalStatistics::QgsZonalStatistics(QgsVectorLayer* polygonLayer, const QString& rasterFile, const QString& attributePrefix, int rasterBand): \
    mRasterFilePath(rasterFile), mRasterBand(rasterBand), mPolygonLayer(polygonLayer), mAttributePrefix(attributePrefix), mInputNodataValue( -1 )
{

}

QgsZonalStatistics::QgsZonalStatistics(): mRasterBand(0), mPolygonLayer(0)
{

}

QgsZonalStatistics::~QgsZonalStatistics()
{

}

int QgsZonalStatistics::calculateStatistics(QProgressDialog* p)
{
  if(!mPolygonLayer || !mPolygonLayer->geometryType() == QGis::Polygon)
  {
    return 1;
  }

  QgsVectorDataProvider* vectorProvider = mPolygonLayer->dataProvider();
  if(!vectorProvider)
  {
    return 2;
  }

  //open the raster layer and the raster band
  GDALAllRegister();
  GDALDatasetH inputDataset = GDALOpen(mRasterFilePath.toLocal8Bit().data(), GA_ReadOnly );
  if ( inputDataset == NULL )
  {
    return 3;
  }

  if ( GDALGetRasterCount( inputDataset ) < (mRasterBand - 1) )
  {
      GDALClose( inputDataset );
      return 4;
  }

  GDALRasterBandH rasterBand = GDALGetRasterBand( inputDataset, mRasterBand );
  if ( rasterBand == NULL )
  {
    GDALClose( inputDataset );
    return 5;
  }
  mInputNodataValue = GDALGetRasterNoDataValue( rasterBand, NULL );

  //get geometry info about raster layer
  int nCellsX = GDALGetRasterXSize( inputDataset );
  int nCellsY = GDALGetRasterYSize( inputDataset );
  double geoTransform[6];
  if ( GDALGetGeoTransform( inputDataset, geoTransform ) != CE_None )
  {
    GDALClose( inputDataset );
    return 6;
  }
  double cellsizeX = geoTransform[1];
  if(cellsizeX < 0)
  {
    cellsizeX = -cellsizeX;
  }
  double cellsizeY = geoTransform[5];
  if(cellsizeY < 0)
  {
    cellsizeY = -cellsizeY;
  }
  QgsRectangle rasterBBox(geoTransform[0], geoTransform[3] - (nCellsY * cellsizeY), geoTransform[0] + (nCellsX * cellsizeX), geoTransform[3]);

  //add the new count, sum, mean fields to the provider
  QList<QgsField> newFieldList;
  QgsField countField(mAttributePrefix + "count", QVariant::Int);
  QgsField sumField(mAttributePrefix + "sum", QVariant::Double);
  QgsField meanField(mAttributePrefix + "mean", QVariant::Double);
  newFieldList.push_back(countField);
  newFieldList.push_back(sumField);
  newFieldList.push_back(meanField);
  if(!vectorProvider->addAttributes( newFieldList ))
  {
    return 7;
  }

  //index of the new fields
  int countIndex = vectorProvider->fieldNameIndex(mAttributePrefix + "count");
  int sumIndex = vectorProvider->fieldNameIndex(mAttributePrefix + "sum");
  int meanIndex = vectorProvider->fieldNameIndex(mAttributePrefix + "mean");

  if (countIndex == -1 || sumIndex == -1 || meanIndex == -1)
  {
    return 8;
  }

  //progress dialog
  long featureCount = vectorProvider->featureCount();
  if(p)
  {
    p->setMaximum(featureCount);
  }


  //iterate over each polygon
  vectorProvider->select(QgsAttributeList(), QgsRectangle(), true, false);
  vectorProvider->rewind();
  QgsFeature f;
  double count = 0;
  double sum = 0;
  double mean = 0;
  float* scanLine;
  int featureCounter = 0;
  //x- and y- coordinate of current cell
  double cellCenterX = 0;
  double cellCenterY = 0;
  QgsPoint currentCellCenter;

  while(vectorProvider->nextFeature(f))
  {
    if(p)
    {
      p->setValue(featureCounter);
    }

    if(p && p->wasCanceled())
    {
      break;
    }

    QgsGeometry* featureGeometry = f.geometry();
    if(!featureGeometry)
    {
      ++featureCounter;
      continue;
    }

    int offsetX, offsetY, nCellsX, nCellsY;
    if(cellInfoForBBox(rasterBBox, featureGeometry->boundingBox(), cellsizeX, cellsizeY, offsetX, offsetY, nCellsX, nCellsY) != 0)
    {
      ++featureCounter;
      continue;
    }

    scanLine = ( float * ) CPLMalloc( sizeof( float ) * nCellsX );
    cellCenterY = rasterBBox.yMaximum() - offsetY * cellsizeY - cellsizeY / 2;
    count = 0;
    sum = 0;
    float currentValue;

    for(int i = 0; i < nCellsY; ++i)
    {
      GDALRasterIO( rasterBand, GF_Read, offsetX, offsetY + i, nCellsX, 1, scanLine, nCellsX, 1, GDT_Float32, 0, 0 );
      cellCenterX = rasterBBox.xMinimum() + offsetX * cellsizeX + cellsizeX / 2;
      for(int j = 0; j < nCellsX; ++j)
      {
        currentCellCenter = QgsPoint(cellCenterX, cellCenterY);
        if(featureGeometry->contains(&currentCellCenter))
        {
          if(scanLine[j] != mInputNodataValue) //don't consider nodata values
          {
            sum += scanLine[j];
            ++count;
          }
        }
        cellCenterX += cellsizeX;
      }
      cellCenterY -= cellsizeY;
    }

    if(count == 0)
    {
      mean = 0;
    }
    else
    {
      mean = sum / count;
    }

    //write the new AEY value to the vector data provider
    QgsChangedAttributesMap changeMap;
    QgsAttributeMap changeAttributeMap;
    changeAttributeMap.insert(countIndex, QVariant(count));
    changeAttributeMap.insert(sumIndex, QVariant(sum));
    changeAttributeMap.insert(meanIndex, QVariant(mean));
    changeMap.insert(f.id(), changeAttributeMap);
    vectorProvider->changeAttributeValues(changeMap);

    CPLFree(scanLine);
    ++featureCounter;
  }

  if(p)
  {
    p->setValue(featureCount);
  }

  GDALClose( inputDataset );
  return 0;
}

int QgsZonalStatistics::cellInfoForBBox(const QgsRectangle& rasterBBox, const QgsRectangle& featureBBox, double cellSizeX, double cellSizeY, \
                                        int& offsetX, int& offsetY, int& nCellsX, int& nCellsY) const
{
  //get intersecting bbox
  QgsRectangle intersectBox = rasterBBox.intersect(&featureBBox);
  if(intersectBox.isEmpty())
  {
    nCellsX = 0; nCellsY = 0; offsetX = 0; offsetY = 0;
    return 0;
  }

  //get offset in pixels in x- and y- direction
  offsetX = (int)( (intersectBox.xMinimum() - rasterBBox.xMinimum()) / cellSizeX);
  offsetY = (int)( (rasterBBox.yMaximum() - intersectBox.yMaximum()) / cellSizeY);

  int maxColumn = (int) ( (intersectBox.xMaximum() - rasterBBox.xMinimum()) / cellSizeX) + 1;
  int maxRow = (int) ( (rasterBBox.yMaximum() - intersectBox.yMinimum()) / cellSizeY ) + 1;

  nCellsX = maxColumn - offsetX;
  nCellsY = maxRow - offsetY;

  return 0;
}




