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

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x) (x).toUtf8().constData()
#else
#define TO8(x) (x).toLocal8Bit().constData()
#endif

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer* polygonLayer, const QString& rasterFile, const QString& attributePrefix, int rasterBand )
    : mRasterFilePath( rasterFile )
    , mRasterBand( rasterBand )
    , mPolygonLayer( polygonLayer )
    , mAttributePrefix( attributePrefix )
    , mInputNodataValue( -1 )
{

}

QgsZonalStatistics::QgsZonalStatistics()
    : mRasterBand( 0 )
    , mPolygonLayer( 0 )
{

}

QgsZonalStatistics::~QgsZonalStatistics()
{

}

int QgsZonalStatistics::calculateStatistics( QProgressDialog* p )
{
  if ( !mPolygonLayer || mPolygonLayer->geometryType() != QGis::Polygon )
  {
    return 1;
  }

  QgsVectorDataProvider* vectorProvider = mPolygonLayer->dataProvider();
  if ( !vectorProvider )
  {
    return 2;
  }

  //open the raster layer and the raster band
  GDALAllRegister();
  GDALDatasetH inputDataset = GDALOpen( TO8( mRasterFilePath ), GA_ReadOnly );
  if ( inputDataset == NULL )
  {
    return 3;
  }

  if ( GDALGetRasterCount( inputDataset ) < ( mRasterBand - 1 ) )
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
  if ( cellsizeX < 0 )
  {
    cellsizeX = -cellsizeX;
  }
  double cellsizeY = geoTransform[5];
  if ( cellsizeY < 0 )
  {
    cellsizeY = -cellsizeY;
  }
  QgsRectangle rasterBBox( geoTransform[0], geoTransform[3] - ( nCellsY * cellsizeY ), geoTransform[0] + ( nCellsX * cellsizeX ), geoTransform[3] );

  //add the new count, sum, mean fields to the provider
  QList<QgsField> newFieldList;
  QgsField countField( mAttributePrefix + "count", QVariant::Double );
  QgsField sumField( mAttributePrefix + "sum", QVariant::Double );
  QgsField meanField( mAttributePrefix + "mean", QVariant::Double );
  newFieldList.push_back( countField );
  newFieldList.push_back( sumField );
  newFieldList.push_back( meanField );
  if ( !vectorProvider->addAttributes( newFieldList ) )
  {
    return 7;
  }

  //index of the new fields
  int countIndex = vectorProvider->fieldNameIndex( mAttributePrefix + "count" );
  int sumIndex = vectorProvider->fieldNameIndex( mAttributePrefix + "sum" );
  int meanIndex = vectorProvider->fieldNameIndex( mAttributePrefix + "mean" );

  if ( countIndex == -1 || sumIndex == -1 || meanIndex == -1 )
  {
    return 8;
  }

  //progress dialog
  long featureCount = vectorProvider->featureCount();
  if ( p )
  {
    p->setMaximum( featureCount );
  }


  //iterate over each polygon
  vectorProvider->select( QgsAttributeList(), QgsRectangle(), true, false );
  vectorProvider->rewind();
  QgsFeature f;
  double count = 0;
  double sum = 0;
  double mean = 0;
  int featureCounter = 0;

  while ( vectorProvider->nextFeature( f ) )
  {
    qWarning( "%d", featureCounter );
    if ( p )
    {
      p->setValue( featureCounter );
    }

    if ( p && p->wasCanceled() )
    {
      break;
    }

    QgsGeometry* featureGeometry = f.geometry();
    if ( !featureGeometry )
    {
      ++featureCounter;
      continue;
    }

    int offsetX, offsetY, nCellsX, nCellsY;
    if ( cellInfoForBBox( rasterBBox, featureGeometry->boundingBox(), cellsizeX, cellsizeY, offsetX, offsetY, nCellsX, nCellsY ) != 0 )
    {
      ++featureCounter;
      continue;
    }

    statisticsFromMiddlePointTest_improved( rasterBand, featureGeometry, offsetX, offsetY, nCellsX, nCellsY, cellsizeX, cellsizeY,
                                            rasterBBox, sum, count );

    if ( count <= 1 )
    {
      //the cell resolution is probably larger than the polygon area. We switch to precise pixel - polygon intersection in this case
      statisticsFromPreciseIntersection( rasterBand, featureGeometry, offsetX, offsetY, nCellsX, nCellsY, cellsizeX, cellsizeY,
                                         rasterBBox, sum, count );
    }


    if ( count == 0 )
    {
      mean = 0;
    }
    else
    {
      mean = sum / count;
    }

    //write the statistics value to the vector data provider
    QgsChangedAttributesMap changeMap;
    QgsAttributeMap changeAttributeMap;
    changeAttributeMap.insert( countIndex, QVariant( count ) );
    changeAttributeMap.insert( sumIndex, QVariant( sum ) );
    changeAttributeMap.insert( meanIndex, QVariant( mean ) );
    changeMap.insert( f.id(), changeAttributeMap );
    vectorProvider->changeAttributeValues( changeMap );

    ++featureCounter;
  }

  if ( p )
  {
    p->setValue( featureCount );
  }

  GDALClose( inputDataset );
  return 0;
}

int QgsZonalStatistics::cellInfoForBBox( const QgsRectangle& rasterBBox, const QgsRectangle& featureBBox, double cellSizeX, double cellSizeY,
    int& offsetX, int& offsetY, int& nCellsX, int& nCellsY ) const
{
  //get intersecting bbox
  QgsRectangle intersectBox = rasterBBox.intersect( &featureBBox );
  if ( intersectBox.isEmpty() )
  {
    nCellsX = 0; nCellsY = 0; offsetX = 0; offsetY = 0;
    return 0;
  }

  //get offset in pixels in x- and y- direction
  offsetX = ( int )(( intersectBox.xMinimum() - rasterBBox.xMinimum() ) / cellSizeX );
  offsetY = ( int )(( rasterBBox.yMaximum() - intersectBox.yMaximum() ) / cellSizeY );

  int maxColumn = ( int )(( intersectBox.xMaximum() - rasterBBox.xMinimum() ) / cellSizeX ) + 1;
  int maxRow = ( int )(( rasterBBox.yMaximum() - intersectBox.yMinimum() ) / cellSizeY ) + 1;

  nCellsX = maxColumn - offsetX;
  nCellsY = maxRow - offsetY;

  return 0;
}

void QgsZonalStatistics::statisticsFromMiddlePointTest( void* band, QgsGeometry* poly, int pixelOffsetX,
    int pixelOffsetY, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, double& sum, double& count )
{
  double cellCenterX, cellCenterY;
  QgsPoint currentCellCenter;

  float* scanLine = ( float * ) CPLMalloc( sizeof( float ) * nCellsX );
  cellCenterY = rasterBBox.yMaximum() - pixelOffsetY * cellSizeY - cellSizeY / 2;
  count = 0;
  sum = 0;

  for ( int i = 0; i < nCellsY; ++i )
  {
    GDALRasterIO( band, GF_Read, pixelOffsetX, pixelOffsetY + i, nCellsX, 1, scanLine, nCellsX, 1, GDT_Float32, 0, 0 );
    cellCenterX = rasterBBox.xMinimum() + pixelOffsetX * cellSizeX + cellSizeX / 2;
    for ( int j = 0; j < nCellsX; ++j )
    {
      currentCellCenter = QgsPoint( cellCenterX, cellCenterY );
      if ( poly->contains( &currentCellCenter ) )
      {
        if ( scanLine[j] != mInputNodataValue ) //don't consider nodata values
        {
          sum += scanLine[j];
          ++count;
        }
      }
      cellCenterX += cellSizeX;
    }
    cellCenterY -= cellSizeY;
  }
  CPLFree( scanLine );
}

void QgsZonalStatistics::statisticsFromPreciseIntersection( void* band, QgsGeometry* poly, int pixelOffsetX,
    int pixelOffsetY, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, double& sum, double& count )
{
  sum = 0;
  count = 0;
  double currentY = rasterBBox.yMaximum() - pixelOffsetY * cellSizeY - cellSizeY / 2;
  float* pixelData = ( float * ) CPLMalloc( sizeof( float ) );
  QgsGeometry* pixelRectGeometry = 0;

  double hCellSizeX = cellSizeX / 2.0;
  double hCellSizeY = cellSizeY / 2.0;
  double pixelArea = cellSizeX * cellSizeY;
  double weight = 0;

  for ( int row = 0; row < nCellsY; ++row )
  {
    double currentX = rasterBBox.xMinimum() + cellSizeX / 2.0 + pixelOffsetX * cellSizeX;
    for ( int col = 0; col < nCellsX; ++col )
    {
      GDALRasterIO( band, GF_Read, pixelOffsetX + col, pixelOffsetY + row, nCellsX, 1, pixelData, 1, 1, GDT_Float32, 0, 0 );
      pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - hCellSizeX, currentY - hCellSizeY, currentX + hCellSizeX, currentY + hCellSizeY ) );
      if ( pixelRectGeometry )
      {
        //intersection
        QgsGeometry *intersectGeometry = pixelRectGeometry->intersection( poly );
        if ( intersectGeometry )
        {
          double intersectionArea = intersectGeometry->area();
          if ( intersectionArea >= 0.0 )
          {
            weight = intersectionArea / pixelArea;
            count += weight;
            sum += *pixelData * weight;
          }
          delete intersectGeometry;
        }
      }
      currentX += cellSizeX;
    }
    currentY -= cellSizeY;
  }
  CPLFree( pixelData );
}

void QgsZonalStatistics::statisticsFromMiddlePointTest_improved( void* band, QgsGeometry* poly, int pixelOffsetX, int pixelOffsetY, int nCellsX, int nCellsY,
    double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, double& sum, double& count )
{
  double cellCenterX, cellCenterY;
  QgsPoint currentCellCenter;

  float* scanLine = ( float * ) CPLMalloc( sizeof( float ) * nCellsX );
  cellCenterY = rasterBBox.yMaximum() - pixelOffsetY * cellSizeY - cellSizeY / 2;
  count = 0;
  sum = 0;

  for ( int i = 0; i < nCellsY; ++i )
  {
    GDALRasterIO( band, GF_Read, pixelOffsetX, pixelOffsetY + i, nCellsX, 1, scanLine, nCellsX, 1, GDT_Float32, 0, 0 );
    cellCenterX = rasterBBox.xMinimum() + pixelOffsetX * cellSizeX + cellSizeX / 2;

    //do intersection of scanline with geometry
    GEOSCoordSequence* scanLineSequence = GEOSCoordSeq_create( 2, 2 );
    GEOSCoordSeq_setX( scanLineSequence, 0, cellCenterX );
    GEOSCoordSeq_setY( scanLineSequence, 0, cellCenterY );
    GEOSCoordSeq_setX( scanLineSequence, 1, cellCenterX + nCellsX * cellSizeX );
    GEOSCoordSeq_setY( scanLineSequence, 1, cellCenterY );
    GEOSGeometry* scanLineGeos = GEOSGeom_createLineString( scanLineSequence ); //todo: delete
    GEOSGeometry* polyGeos = poly->asGeos();
    GEOSGeometry* scanLineIntersection = GEOSIntersection( scanLineGeos, polyGeos );
    GEOSGeom_destroy( scanLineGeos );
    if ( !scanLineIntersection )
    {
      cellCenterY -= cellSizeY;
      continue;
    }

    //debug
    //char* scanLineIntersectionType = GEOSGeomType( scanLineIntersection );

    int numGeoms = GEOSGetNumGeometries( scanLineIntersection );
    if ( numGeoms < 1 )
    {
      GEOSGeom_destroy( scanLineIntersection );
      cellCenterY -= cellSizeY;
      continue;
    }

    QList<double> scanLineList;
    double currentValue;
    GEOSGeometry* currentGeom = 0;
    for ( int z = 0; z < numGeoms; ++z )
    {
      if ( numGeoms == 1 )
      {
        currentGeom = scanLineIntersection;
      }
      else
      {
        currentGeom = GEOSGeom_clone( GEOSGetGeometryN( scanLineIntersection, z ) );
      }
      const GEOSCoordSequence* scanLineCoordSequence = GEOSGeom_getCoordSeq( currentGeom );
      if ( !scanLineCoordSequence )
      {
        //error
      }
      unsigned int scanLineIntersectionSize;
      GEOSCoordSeq_getSize( scanLineCoordSequence, &scanLineIntersectionSize );
      if ( !scanLineCoordSequence || scanLineIntersectionSize < 2 || ( scanLineIntersectionSize & 1 ) )
      {
        //error
      }
      for ( unsigned int k = 0; k < scanLineIntersectionSize; ++k )
      {
        GEOSCoordSeq_getX( scanLineCoordSequence, k, &currentValue );
        scanLineList.push_back( currentValue );
      }

      if ( numGeoms != 1 )
      {
        GEOSGeom_destroy( currentGeom );
      }
    }
    GEOSGeom_destroy( scanLineIntersection );
    qSort( scanLineList );

    if ( scanLineList.size() < 1 )
    {
      cellCenterY -= cellSizeY;
      continue;
    }

    int listPlace = -1;
    for ( int j = 0; j < nCellsX; ++j )
    {
      //currentCellCenter = QgsPoint( cellCenterX, cellCenterY );

      //instead of doing a contained test every time, find the place of scanLineList and check if even / odd
      if ( listPlace >= scanLineList.size() - 1 )
      {
        break;
      }
      if ( cellCenterX >= scanLineList.at( listPlace + 1 ) )
      {
        ++listPlace;
        if ( listPlace >= scanLineList.size() )
        {
          break;
        }
      }
      if ( listPlace >= 0 && listPlace < ( scanLineList.size() - 1 ) && !( listPlace & 1 ) )
      {
        if ( scanLine[j] != mInputNodataValue ) //don't consider nodata values
        {
          sum += scanLine[j];
          ++count;
        }
      }
      cellCenterX += cellSizeX;
    }
    cellCenterY -= cellSizeY;
  }
  CPLFree( scanLine );
}


