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
#include "qmath.h"
#include "gdal.h"
#include "cpl_string.h"
#include <QProgressDialog>
#include <QFile>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#endif

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer* polygonLayer, const QString& rasterFile, const QString& attributePrefix, int rasterBand , Statistics stats )
    : mRasterFilePath( rasterFile )
    , mRasterBand( rasterBand )
    , mPolygonLayer( polygonLayer )
    , mAttributePrefix( attributePrefix )
    , mInputNodataValue( -1 )
    , mStatistics( stats )
{

}

QgsZonalStatistics::QgsZonalStatistics()
    : mRasterBand( 0 )
    , mPolygonLayer( 0 )
    , mInputNodataValue( -1 )
    , mStatistics( QgsZonalStatistics::All )
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
  GDALDatasetH inputDataset = GDALOpen( TO8F( mRasterFilePath ), GA_ReadOnly );
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
  int nCellsXGDAL = GDALGetRasterXSize( inputDataset );
  int nCellsYGDAL = GDALGetRasterYSize( inputDataset );
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
  QgsRectangle rasterBBox( geoTransform[0], geoTransform[3] - ( nCellsYGDAL * cellsizeY ),
                           geoTransform[0] + ( nCellsXGDAL * cellsizeX ), geoTransform[3] );

  //add the new fields to the provider
  QList<QgsField> newFieldList;
  QString countFieldName;
  if ( mStatistics & QgsZonalStatistics::Count )
  {
    countFieldName = getUniqueFieldName( mAttributePrefix + "count" );
    QgsField countField( countFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( countField );
  }
  QString sumFieldName;
  if ( mStatistics & QgsZonalStatistics::Sum )
  {
    sumFieldName = getUniqueFieldName( mAttributePrefix + "sum" );
    QgsField sumField( sumFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( sumField );
  }
  QString meanFieldName;
  if ( mStatistics & QgsZonalStatistics::Mean )
  {
    meanFieldName = getUniqueFieldName( mAttributePrefix + "mean" );
    QgsField meanField( meanFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( meanField );
  }
  QString medianFieldName;
  if ( mStatistics & QgsZonalStatistics::Median )
  {
    medianFieldName = getUniqueFieldName( mAttributePrefix + "median" );
    QgsField medianField( medianFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( medianField );
  }
  QString stdevFieldName;
  if ( mStatistics & QgsZonalStatistics::StDev )
  {
    stdevFieldName = getUniqueFieldName( mAttributePrefix + "stdev" );
    QgsField stdField( stdevFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( stdField );
  }
  QString minFieldName;
  if ( mStatistics & QgsZonalStatistics::Min )
  {
    minFieldName = getUniqueFieldName( mAttributePrefix + "min" );
    QgsField minField( minFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( minField );
  }
  QString maxFieldName;
  if ( mStatistics & QgsZonalStatistics::Max )
  {
    maxFieldName = getUniqueFieldName( mAttributePrefix + "max" );
    QgsField maxField( maxFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( maxField );
  }
  QString rangeFieldName;
  if ( mStatistics & QgsZonalStatistics::Range )
  {
    rangeFieldName = getUniqueFieldName( mAttributePrefix + "range" );
    QgsField rangeField( rangeFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( rangeField );
  }
  QString minorityFieldName;
  if ( mStatistics & QgsZonalStatistics::Minority )
  {
    minorityFieldName = getUniqueFieldName( mAttributePrefix + "minority" );
    QgsField minorityField( minorityFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( minorityField );
  }
  QString majorityFieldName;
  if ( mStatistics & QgsZonalStatistics::Majority )
  {
    majorityFieldName = getUniqueFieldName( mAttributePrefix + "majority" );
    QgsField majField( majorityFieldName, QVariant::Double, "double precision" );
    newFieldList.push_back( majField );
  }
  QString varietyFieldName;
  if ( mStatistics & QgsZonalStatistics::Variety )
  {
    varietyFieldName = getUniqueFieldName( mAttributePrefix + "variety" );
    QgsField varietyField( varietyFieldName, QVariant::Int, "int" );
    newFieldList.push_back( varietyField );
  }
  vectorProvider->addAttributes( newFieldList );

  //index of the new fields
  int countIndex = mStatistics & QgsZonalStatistics::Count ? vectorProvider->fieldNameIndex( countFieldName ) : -1;
  int sumIndex = mStatistics & QgsZonalStatistics::Sum ? vectorProvider->fieldNameIndex( sumFieldName ) : -1;
  int meanIndex = mStatistics & QgsZonalStatistics::Mean ? vectorProvider->fieldNameIndex( meanFieldName ) : -1;
  int medianIndex = mStatistics & QgsZonalStatistics::Median ? vectorProvider->fieldNameIndex( medianFieldName ) : -1;
  int stdevIndex = mStatistics & QgsZonalStatistics::StDev ? vectorProvider->fieldNameIndex( stdevFieldName ) : -1;
  int minIndex = mStatistics & QgsZonalStatistics::Min ? vectorProvider->fieldNameIndex( minFieldName ) : -1;
  int maxIndex = mStatistics & QgsZonalStatistics::Max ? vectorProvider->fieldNameIndex( maxFieldName ) : -1;
  int rangeIndex = mStatistics & QgsZonalStatistics::Range ? vectorProvider->fieldNameIndex( rangeFieldName ) : -1;
  int minorityIndex = mStatistics & QgsZonalStatistics::Minority ? vectorProvider->fieldNameIndex( minorityFieldName ) : -1;
  int majorityIndex = mStatistics & QgsZonalStatistics::Majority ? vectorProvider->fieldNameIndex( majorityFieldName ) : -1;
  int varietyIndex = mStatistics & QgsZonalStatistics::Variety ? vectorProvider->fieldNameIndex( varietyFieldName ) : -1;

  if (( mStatistics & QgsZonalStatistics::Count && countIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Sum && sumIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Mean && meanIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Median && medianIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::StDev && stdevIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Min && minIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Max && maxIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Range && rangeIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Minority && minorityIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Majority && majorityIndex == -1 )
      || ( mStatistics & QgsZonalStatistics::Variety && varietyIndex == -1 )
     )
  {
    //failed to create a required field
    return 8;
  }

  //progress dialog
  long featureCount = vectorProvider->featureCount();
  if ( p )
  {
    p->setMaximum( featureCount );
  }


  //iterate over each polygon
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator fi = vectorProvider->getFeatures( request );
  QgsFeature f;

  bool statsStoreValues = ( mStatistics & QgsZonalStatistics::Median ) ||
                          ( mStatistics & QgsZonalStatistics::StDev );
  bool statsStoreValueCount = ( mStatistics & QgsZonalStatistics::Minority ) ||
                              ( mStatistics & QgsZonalStatistics::Majority );

  FeatureStats featureStats( statsStoreValues, statsStoreValueCount );
  int featureCounter = 0;

  QgsChangedAttributesMap changeMap;
  while ( fi.nextFeature( f ) )
  {
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

    QgsRectangle featureRect = featureGeometry->boundingBox().intersect( &rasterBBox );
    if ( featureRect.isEmpty() )
    {
      ++featureCounter;
      continue;
    }

    int offsetX, offsetY, nCellsX, nCellsY;
    if ( cellInfoForBBox( rasterBBox, featureRect, cellsizeX, cellsizeY, offsetX, offsetY, nCellsX, nCellsY ) != 0 )
    {
      ++featureCounter;
      continue;
    }

    //avoid access to cells outside of the raster (may occur because of rounding)
    if (( offsetX + nCellsX ) > nCellsXGDAL )
    {
      nCellsX = nCellsXGDAL - offsetX;
    }
    if (( offsetY + nCellsY ) > nCellsYGDAL )
    {
      nCellsY = nCellsYGDAL - offsetY;
    }

    statisticsFromMiddlePointTest( rasterBand, featureGeometry, offsetX, offsetY, nCellsX, nCellsY, cellsizeX, cellsizeY,
                                   rasterBBox, featureStats );

    if ( featureStats.count <= 1 )
    {
      //the cell resolution is probably larger than the polygon area. We switch to precise pixel - polygon intersection in this case
      statisticsFromPreciseIntersection( rasterBand, featureGeometry, offsetX, offsetY, nCellsX, nCellsY, cellsizeX, cellsizeY,
                                         rasterBBox, featureStats );
    }

    //write the statistics value to the vector data provider
    QgsAttributeMap changeAttributeMap;
    if ( mStatistics & QgsZonalStatistics::Count )
      changeAttributeMap.insert( countIndex, QVariant( featureStats.count ) );
    if ( mStatistics & QgsZonalStatistics::Sum )
      changeAttributeMap.insert( sumIndex, QVariant( featureStats.sum ) );
    if ( featureStats.count > 0 )
    {
      double mean = featureStats.sum / featureStats.count;
      if ( mStatistics & QgsZonalStatistics::Mean )
        changeAttributeMap.insert( meanIndex, QVariant( mean ) );
      if ( mStatistics & QgsZonalStatistics::Median )
      {
        qSort( featureStats.values.begin(), featureStats.values.end() );
        int size =  featureStats.values.count();
        bool even = ( size % 2 ) < 1;
        double medianValue;
        if ( even )
        {
          medianValue = ( featureStats.values[size / 2 - 1] + featureStats.values[size / 2] ) / 2;
        }
        else //odd
        {
          medianValue = featureStats.values[( size + 1 ) / 2 - 1];
        }
        changeAttributeMap.insert( medianIndex, QVariant( medianValue ) );
      }
      if ( mStatistics & QgsZonalStatistics::StDev )
      {
        double sumSquared = 0;
        for ( int i = 0; i < featureStats.values.count(); ++i )
        {
          double diff = featureStats.values.at( i ) - mean;
          sumSquared += diff * diff;
        }
        double stdev = qPow( sumSquared / featureStats.values.count(), 0.5 );
        changeAttributeMap.insert( stdevIndex, QVariant( stdev ) );
      }
      if ( mStatistics & QgsZonalStatistics::Min )
        changeAttributeMap.insert( minIndex, QVariant( featureStats.min ) );
      if ( mStatistics & QgsZonalStatistics::Max )
        changeAttributeMap.insert( maxIndex, QVariant( featureStats.max ) );
      if ( mStatistics & QgsZonalStatistics::Range )
        changeAttributeMap.insert( rangeIndex, QVariant( featureStats.max - featureStats.min ) );
      if ( mStatistics & QgsZonalStatistics::Minority || mStatistics & QgsZonalStatistics::Majority )
      {
        QList<int> vals = featureStats.valueCount.values();
        qSort( vals.begin(), vals.end() );
        if ( mStatistics & QgsZonalStatistics::Minority )
        {
          float minorityKey = featureStats.valueCount.key( vals.first() );
          changeAttributeMap.insert( minorityIndex, QVariant( minorityKey ) );
        }
        if ( mStatistics & QgsZonalStatistics::Majority )
        {
          float majKey = featureStats.valueCount.key( vals.last() );
          changeAttributeMap.insert( majorityIndex, QVariant( majKey ) );
        }
      }
      if ( mStatistics & QgsZonalStatistics::Variety )
        changeAttributeMap.insert( varietyIndex, QVariant( featureStats.valueCount.count() ) );
    }

    changeMap.insert( f.id(), changeAttributeMap );
    ++featureCounter;
  }

  vectorProvider->changeAttributeValues( changeMap );

  if ( p )
  {
    p->setValue( featureCount );
  }

  GDALClose( inputDataset );
  mPolygonLayer->updateFields();

  if ( p && p->wasCanceled() )
  {
    return 9;
  }

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
    int pixelOffsetY, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, FeatureStats &stats )
{
  double cellCenterX, cellCenterY;

  float* scanLine = ( float * ) CPLMalloc( sizeof( float ) * nCellsX );
  cellCenterY = rasterBBox.yMaximum() - pixelOffsetY * cellSizeY - cellSizeY / 2;
  stats.reset();

  const GEOSGeometry* polyGeos = poly->asGeos();
  if ( !polyGeos )
  {
    return;
  }

  GEOSContextHandle_t geosctxt = QgsGeometry::getGEOSHandler();
  const GEOSPreparedGeometry* polyGeosPrepared = GEOSPrepare_r( geosctxt, poly->asGeos() );
  if ( !polyGeosPrepared )
  {
    return;
  }

  GEOSCoordSequence* cellCenterCoords = 0;
  GEOSGeometry* currentCellCenter = 0;

  for ( int i = 0; i < nCellsY; ++i )
  {
    if ( GDALRasterIO( band, GF_Read, pixelOffsetX, pixelOffsetY + i, nCellsX, 1, scanLine, nCellsX, 1, GDT_Float32, 0, 0 )
         != CPLE_None )
    {
      continue;
    }
    cellCenterX = rasterBBox.xMinimum() + pixelOffsetX * cellSizeX + cellSizeX / 2;
    for ( int j = 0; j < nCellsX; ++j )
    {
      if ( validPixel( scanLine[j] ) )
      {
        GEOSGeom_destroy_r( geosctxt, currentCellCenter );
        cellCenterCoords = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
        GEOSCoordSeq_setX_r( geosctxt, cellCenterCoords, 0, cellCenterX );
        GEOSCoordSeq_setY_r( geosctxt, cellCenterCoords, 0, cellCenterY );
        currentCellCenter = GEOSGeom_createPoint_r( geosctxt, cellCenterCoords );
        if ( GEOSPreparedContains_r( geosctxt, polyGeosPrepared, currentCellCenter ) )
        {
          stats.addValue( scanLine[j] );
        }
      }
      cellCenterX += cellSizeX;
    }
    cellCenterY -= cellSizeY;
  }
  GEOSGeom_destroy_r( geosctxt, currentCellCenter );
  CPLFree( scanLine );
  GEOSPreparedGeom_destroy_r( geosctxt, polyGeosPrepared );
}

void QgsZonalStatistics::statisticsFromPreciseIntersection( void* band, QgsGeometry* poly, int pixelOffsetX,
    int pixelOffsetY, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle& rasterBBox, FeatureStats &stats )
{
  stats.reset();

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
      if ( !validPixel( *pixelData ) )
        continue;

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
            stats.addValue( *pixelData, weight );
          }
          delete intersectGeometry;
        }
        delete pixelRectGeometry;
        pixelRectGeometry = 0;
      }
      currentX += cellSizeX;
    }
    currentY -= cellSizeY;
  }
  CPLFree( pixelData );
}

bool QgsZonalStatistics::validPixel( float value ) const
{
  if ( value == mInputNodataValue || qIsNaN( value ) )
  {
    return false;
  }
  return true;
}

QString QgsZonalStatistics::getUniqueFieldName( QString fieldName )
{
  QgsVectorDataProvider* dp = mPolygonLayer->dataProvider();

  if ( !dp->storageType().contains( "ESRI Shapefile" ) )
  {
    return fieldName;
  }

  const QgsFields& providerFields = dp->fields();
  QString shortName = fieldName.mid( 0, 10 );

  bool found = false;
  for ( int idx = 0; idx < providerFields.count(); ++idx )
  {
    if ( shortName == providerFields[idx].name() )
    {
      found = true;
      break;
    }
  }

  if ( !found )
  {
    return shortName;
  }

  int n = 1;
  shortName = QString( "%1_%2" ).arg( fieldName.mid( 0, 8 ) ).arg( n );
  found = true;
  while ( found )
  {
    found = false;
    for ( int idx = 0; idx < providerFields.count(); ++idx )
    {
      if ( shortName == providerFields[idx].name() )
      {
        n += 1;
        if ( n < 9 )
        {
          shortName = QString( "%1_%2" ).arg( fieldName.mid( 0, 8 ) ).arg( n );
        }
        else
        {
          shortName = QString( "%1_%2" ).arg( fieldName.mid( 0, 7 ) ).arg( n );
        }
        found = true;
      }
    }
  }
  return shortName;
}
