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
#include "qgsfeatureiterator.h"
#include "qgsfeedback.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasterblock.h"
#include "qgslogger.h"
#include "qgsgeos.h"

#include <QFile>

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer *polygonLayer, QgsRasterLayer *rasterLayer, const QString &attributePrefix, int rasterBand, QgsZonalStatistics::Statistics stats )
  : mRasterLayer( rasterLayer )
  , mRasterBand( rasterBand )
  , mPolygonLayer( polygonLayer )
  , mAttributePrefix( attributePrefix )
  , mStatistics( stats )
{}

int QgsZonalStatistics::calculateStatistics( QgsFeedback *feedback )
{
  if ( !mPolygonLayer || mPolygonLayer->geometryType() != QgsWkbTypes::PolygonGeometry )
  {
    return 1;
  }

  QgsVectorDataProvider *vectorProvider = mPolygonLayer->dataProvider();
  if ( !vectorProvider )
  {
    return 2;
  }

  if ( !mRasterLayer )
  {
    return 3;
  }

  if ( mRasterLayer->bandCount() < mRasterBand )
  {
    return 4;
  }

  mRasterProvider = mRasterLayer->dataProvider();
  mInputNodataValue = mRasterProvider->sourceNoDataValue( mRasterBand );

  //get geometry info about raster layer
  int nCellsXProvider = mRasterProvider->xSize();
  int nCellsYProvider = mRasterProvider->ySize();
  double cellsizeX = mRasterLayer->rasterUnitsPerPixelX();
  if ( cellsizeX < 0 )
  {
    cellsizeX = -cellsizeX;
  }
  double cellsizeY = mRasterLayer->rasterUnitsPerPixelY();
  if ( cellsizeY < 0 )
  {
    cellsizeY = -cellsizeY;
  }
  QgsRectangle rasterBBox = mRasterProvider->extent();

  //add the new fields to the provider
  QList<QgsField> newFieldList;
  QString countFieldName;
  if ( mStatistics & QgsZonalStatistics::Count )
  {
    countFieldName = getUniqueFieldName( mAttributePrefix + "count", newFieldList );
    QgsField countField( countFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( countField );
  }
  QString sumFieldName;
  if ( mStatistics & QgsZonalStatistics::Sum )
  {
    sumFieldName = getUniqueFieldName( mAttributePrefix + "sum", newFieldList );
    QgsField sumField( sumFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( sumField );
  }
  QString meanFieldName;
  if ( mStatistics & QgsZonalStatistics::Mean )
  {
    meanFieldName = getUniqueFieldName( mAttributePrefix + "mean", newFieldList );
    QgsField meanField( meanFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( meanField );
  }
  QString medianFieldName;
  if ( mStatistics & QgsZonalStatistics::Median )
  {
    medianFieldName = getUniqueFieldName( mAttributePrefix + "median", newFieldList );
    QgsField medianField( medianFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( medianField );
  }
  QString stdevFieldName;
  if ( mStatistics & QgsZonalStatistics::StDev )
  {
    stdevFieldName = getUniqueFieldName( mAttributePrefix + "stdev", newFieldList );
    QgsField stdField( stdevFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( stdField );
  }
  QString minFieldName;
  if ( mStatistics & QgsZonalStatistics::Min )
  {
    minFieldName = getUniqueFieldName( mAttributePrefix + "min", newFieldList );
    QgsField minField( minFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( minField );
  }
  QString maxFieldName;
  if ( mStatistics & QgsZonalStatistics::Max )
  {
    maxFieldName = getUniqueFieldName( mAttributePrefix + "max", newFieldList );
    QgsField maxField( maxFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( maxField );
  }
  QString rangeFieldName;
  if ( mStatistics & QgsZonalStatistics::Range )
  {
    rangeFieldName = getUniqueFieldName( mAttributePrefix + "range", newFieldList );
    QgsField rangeField( rangeFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( rangeField );
  }
  QString minorityFieldName;
  if ( mStatistics & QgsZonalStatistics::Minority )
  {
    minorityFieldName = getUniqueFieldName( mAttributePrefix + "minority", newFieldList );
    QgsField minorityField( minorityFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( minorityField );
  }
  QString majorityFieldName;
  if ( mStatistics & QgsZonalStatistics::Majority )
  {
    majorityFieldName = getUniqueFieldName( mAttributePrefix + "majority", newFieldList );
    QgsField majField( majorityFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( majField );
  }
  QString varietyFieldName;
  if ( mStatistics & QgsZonalStatistics::Variety )
  {
    varietyFieldName = getUniqueFieldName( mAttributePrefix + "variety", newFieldList );
    QgsField varietyField( varietyFieldName, QVariant::Int, QStringLiteral( "int" ) );
    newFieldList.push_back( varietyField );
  }
  QString varianceFieldName;
  if ( mStatistics & QgsZonalStatistics::Variance )
  {
    varianceFieldName = getUniqueFieldName( mAttributePrefix + "variance", newFieldList );
    QgsField varianceField( varianceFieldName, QVariant::Double, QStringLiteral( "double precision" ) );
    newFieldList.push_back( varianceField );
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
  int varianceIndex = mStatistics & QgsZonalStatistics::Variance ? vectorProvider->fieldNameIndex( varianceFieldName ) : -1;

  if ( ( mStatistics & QgsZonalStatistics::Count && countIndex == -1 )
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
       || ( mStatistics & QgsZonalStatistics::Variance && varianceIndex == -1 )
     )
  {
    //failed to create a required field
    return 8;
  }

  //progress dialog
  long featureCount = vectorProvider->featureCount();

  //iterate over each polygon
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator fi = vectorProvider->getFeatures( request );
  QgsFeature f;

  bool statsStoreValues = ( mStatistics & QgsZonalStatistics::Median ) ||
                          ( mStatistics & QgsZonalStatistics::StDev ) ||
                          ( mStatistics & QgsZonalStatistics::Variance );
  bool statsStoreValueCount = ( mStatistics & QgsZonalStatistics::Minority ) ||
                              ( mStatistics & QgsZonalStatistics::Majority );

  FeatureStats featureStats( statsStoreValues, statsStoreValueCount );
  int featureCounter = 0;

  QgsChangedAttributesMap changeMap;
  while ( fi.nextFeature( f ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( featureCounter ) / featureCount );
    }

    if ( !f.hasGeometry() )
    {
      ++featureCounter;
      continue;
    }
    QgsGeometry featureGeometry = f.geometry();

    QgsRectangle featureRect = featureGeometry.boundingBox().intersect( &rasterBBox );
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
    if ( ( offsetX + nCellsX ) > nCellsXProvider )
    {
      nCellsX = nCellsXProvider - offsetX;
    }
    if ( ( offsetY + nCellsY ) > nCellsYProvider )
    {
      nCellsY = nCellsYProvider - offsetY;
    }

    statisticsFromMiddlePointTest( featureGeometry, offsetX, offsetY, nCellsX, nCellsY, cellsizeX, cellsizeY,
                                   rasterBBox, featureStats );

    if ( featureStats.count <= 1 )
    {
      //the cell resolution is probably larger than the polygon area. We switch to precise pixel - polygon intersection in this case
      statisticsFromPreciseIntersection( featureGeometry, offsetX, offsetY, nCellsX, nCellsY, cellsizeX, cellsizeY,
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
        std::sort( featureStats.values.begin(), featureStats.values.end() );
        int size = featureStats.values.count();
        bool even = ( size % 2 ) < 1;
        double medianValue;
        if ( even )
        {
          medianValue = ( featureStats.values.at( size / 2 - 1 ) + featureStats.values.at( size / 2 ) ) / 2;
        }
        else //odd
        {
          medianValue = featureStats.values.at( ( size + 1 ) / 2 - 1 );
        }
        changeAttributeMap.insert( medianIndex, QVariant( medianValue ) );
      }
      if ( mStatistics & QgsZonalStatistics::StDev || mStatistics & QgsZonalStatistics::Variance )
      {
        double sumSquared = 0;
        for ( int i = 0; i < featureStats.values.count(); ++i )
        {
          double diff = featureStats.values.at( i ) - mean;
          sumSquared += diff * diff;
        }
        double variance = sumSquared / featureStats.values.count();
        if ( mStatistics & QgsZonalStatistics::StDev )
        {
          double stdev = std::pow( variance, 0.5 );
          changeAttributeMap.insert( stdevIndex, QVariant( stdev ) );
        }
        if ( mStatistics & QgsZonalStatistics::Variance )
          changeAttributeMap.insert( varianceIndex, QVariant( variance ) );
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
        std::sort( vals.begin(), vals.end() );
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

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  mPolygonLayer->updateFields();

  if ( feedback && feedback->isCanceled() )
  {
    return 9;
  }

  return 0;
}

int QgsZonalStatistics::cellInfoForBBox( const QgsRectangle &rasterBBox, const QgsRectangle &featureBBox, double cellSizeX, double cellSizeY,
    int &offsetX, int &offsetY, int &nCellsX, int &nCellsY ) const
{
  //get intersecting bbox
  QgsRectangle intersectBox = rasterBBox.intersect( &featureBBox );
  if ( intersectBox.isEmpty() )
  {
    nCellsX = 0;
    nCellsY = 0;
    offsetX = 0;
    offsetY = 0;
    return 0;
  }

  //get offset in pixels in x- and y- direction
  offsetX = ( int )( ( intersectBox.xMinimum() - rasterBBox.xMinimum() ) / cellSizeX );
  offsetY = ( int )( ( rasterBBox.yMaximum() - intersectBox.yMaximum() ) / cellSizeY );

  int maxColumn = ( int )( ( intersectBox.xMaximum() - rasterBBox.xMinimum() ) / cellSizeX ) + 1;
  int maxRow = ( int )( ( rasterBBox.yMaximum() - intersectBox.yMinimum() ) / cellSizeY ) + 1;

  nCellsX = maxColumn - offsetX;
  nCellsY = maxRow - offsetY;

  return 0;
}

void QgsZonalStatistics::statisticsFromMiddlePointTest( const QgsGeometry &poly, int pixelOffsetX,
    int pixelOffsetY, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox, FeatureStats &stats )
{
  double cellCenterX, cellCenterY;

  cellCenterY = rasterBBox.yMaximum() - pixelOffsetY * cellSizeY - cellSizeY / 2;
  stats.reset();

  geos::unique_ptr polyGeos( poly.exportToGeos() );
  if ( !polyGeos )
  {
    return;
  }

  GEOSContextHandle_t geosctxt = QgsGeometry::getGEOSHandler();
  geos::prepared_unique_ptr polyGeosPrepared( GEOSPrepare_r( geosctxt, polyGeos.get() ) );
  if ( !polyGeosPrepared )
  {
    return;
  }

  GEOSCoordSequence *cellCenterCoords = nullptr;
  geos::unique_ptr currentCellCenter;

  QgsRectangle featureBBox = poly.boundingBox().intersect( &rasterBBox );
  QgsRectangle intersectBBox = rasterBBox.intersect( &featureBBox );

  std::unique_ptr< QgsRasterBlock > block( mRasterProvider->block( mRasterBand, intersectBBox, nCellsX, nCellsY ) );
  for ( int i = 0; i < nCellsY; ++i )
  {
    cellCenterX = rasterBBox.xMinimum() + pixelOffsetX * cellSizeX + cellSizeX / 2;
    for ( int j = 0; j < nCellsX; ++j )
    {
      if ( validPixel( block->value( i, j ) ) )
      {
        cellCenterCoords = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
        GEOSCoordSeq_setX_r( geosctxt, cellCenterCoords, 0, cellCenterX );
        GEOSCoordSeq_setY_r( geosctxt, cellCenterCoords, 0, cellCenterY );
        currentCellCenter.reset( GEOSGeom_createPoint_r( geosctxt, cellCenterCoords ) );
        if ( GEOSPreparedContains_r( geosctxt, polyGeosPrepared.get(), currentCellCenter.get() ) )
        {
          stats.addValue( block->value( i, j ) );
        }
      }
      cellCenterX += cellSizeX;
    }
    cellCenterY -= cellSizeY;
  }
}

void QgsZonalStatistics::statisticsFromPreciseIntersection( const QgsGeometry &poly, int pixelOffsetX,
    int pixelOffsetY, int nCellsX, int nCellsY, double cellSizeX, double cellSizeY, const QgsRectangle &rasterBBox, FeatureStats &stats )
{
  stats.reset();

  double currentY = rasterBBox.yMaximum() - pixelOffsetY * cellSizeY - cellSizeY / 2;
  QgsGeometry pixelRectGeometry;

  double hCellSizeX = cellSizeX / 2.0;
  double hCellSizeY = cellSizeY / 2.0;
  double pixelArea = cellSizeX * cellSizeY;
  double weight = 0;

  QgsRectangle featureBBox = poly.boundingBox().intersect( &rasterBBox );
  QgsRectangle intersectBBox = rasterBBox.intersect( &featureBBox );

  QgsRasterBlock *block = mRasterProvider->block( mRasterBand, intersectBBox, nCellsX, nCellsY );
  for ( int i = 0; i < nCellsY; ++i )
  {
    double currentX = rasterBBox.xMinimum() + cellSizeX / 2.0 + pixelOffsetX * cellSizeX;
    for ( int j = 0; j < nCellsX; ++j )
    {
      if ( !validPixel( block->value( i, j ) ) )
      {
        continue;
      }

      pixelRectGeometry = QgsGeometry::fromRect( QgsRectangle( currentX - hCellSizeX, currentY - hCellSizeY, currentX + hCellSizeX, currentY + hCellSizeY ) );
      if ( !pixelRectGeometry.isNull() )
      {
        //intersection
        QgsGeometry intersectGeometry = pixelRectGeometry.intersection( poly );
        if ( !intersectGeometry.isNull() )
        {
          double intersectionArea = intersectGeometry.area();
          if ( intersectionArea >= 0.0 )
          {
            weight = intersectionArea / pixelArea;
            stats.addValue( block->value( i, j ), weight );
          }
        }
        pixelRectGeometry = QgsGeometry();
      }
      currentX += cellSizeX;
    }
    currentY -= cellSizeY;
  }
  delete block;
}

bool QgsZonalStatistics::validPixel( float value ) const
{
  return !( value == mInputNodataValue || std::isnan( value ) );
}

QString QgsZonalStatistics::getUniqueFieldName( const QString &fieldName, const QList<QgsField> &newFields )
{
  QgsVectorDataProvider *dp = mPolygonLayer->dataProvider();

  if ( !dp->storageType().contains( QLatin1String( "ESRI Shapefile" ) ) )
  {
    return fieldName;
  }

  QList<QgsField> allFields = dp->fields().toList();
  allFields.append( newFields );
  QString shortName = fieldName.mid( 0, 10 );

  bool found = false;
  for ( int idx = 0; idx < allFields.count(); ++idx )
  {
    if ( shortName == allFields.at( idx ).name() )
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
  shortName = QStringLiteral( "%1_%2" ).arg( fieldName.mid( 0, 8 ) ).arg( n );
  found = true;
  while ( found )
  {
    found = false;
    for ( int idx = 0; idx < allFields.count(); ++idx )
    {
      if ( shortName == allFields.at( idx ).name() )
      {
        n += 1;
        if ( n < 9 )
        {
          shortName = QStringLiteral( "%1_%2" ).arg( fieldName.mid( 0, 8 ) ).arg( n );
        }
        else
        {
          shortName = QStringLiteral( "%1_%2" ).arg( fieldName.mid( 0, 7 ) ).arg( n );
        }
        found = true;
      }
    }
  }
  return shortName;
}
