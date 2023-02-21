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
#include "processing/qgsrasteranalysisutils.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgslogger.h"
#include "qgsproject.h"

#include <QFile>

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer *polygonLayer, QgsRasterLayer *rasterLayer, const QString &attributePrefix, int rasterBand, QgsZonalStatistics::Statistics stats )
  : QgsZonalStatistics( polygonLayer,
                        rasterLayer ? rasterLayer->dataProvider() : nullptr,
                        rasterLayer ? rasterLayer->crs() : QgsCoordinateReferenceSystem(),
                        rasterLayer ? rasterLayer->rasterUnitsPerPixelX() : 0,
                        rasterLayer ? rasterLayer->rasterUnitsPerPixelY() : 0,
                        attributePrefix,
                        rasterBand,
                        stats )
{
}

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer *polygonLayer, QgsRasterInterface *rasterInterface,
                                        const QgsCoordinateReferenceSystem &rasterCrs, double rasterUnitsPerPixelX, double rasterUnitsPerPixelY, const QString &attributePrefix, int rasterBand, QgsZonalStatistics::Statistics stats )
  : mRasterInterface( rasterInterface )
  , mRasterCrs( rasterCrs )
  , mCellSizeX( std::fabs( rasterUnitsPerPixelX ) )
  , mCellSizeY( std::fabs( rasterUnitsPerPixelY ) )
  , mRasterBand( rasterBand )
  , mPolygonLayer( polygonLayer )
  , mAttributePrefix( attributePrefix )
  , mStatistics( stats )
{
}

QgsZonalStatistics::Result QgsZonalStatistics::calculateStatistics( QgsFeedback *feedback )
{
  if ( !mRasterInterface )
  {
    return RasterInvalid;
  }

  if ( mRasterInterface->bandCount() < mRasterBand )
  {
    return RasterBandInvalid;
  }

  if ( !mPolygonLayer || mPolygonLayer->geometryType() != Qgis::GeometryType::Polygon )
  {
    return LayerTypeWrong;
  }

  QgsVectorDataProvider *vectorProvider = mPolygonLayer->dataProvider();
  if ( !vectorProvider )
  {
    return LayerInvalid;
  }

  QMap<QgsZonalStatistics::Statistic, int> statFieldIndexes;

  //add the new fields to the provider
  int oldFieldCount = vectorProvider->fields().count();
  QList<QgsField> newFieldList;
  for ( QgsZonalStatistics::Statistic stat :
        {
          QgsZonalStatistics::Count,
          QgsZonalStatistics::Sum,
          QgsZonalStatistics::Mean,
          QgsZonalStatistics::Median,
          QgsZonalStatistics::StDev,
          QgsZonalStatistics::Min,
          QgsZonalStatistics::Max,
          QgsZonalStatistics::Range,
          QgsZonalStatistics::Minority,
          QgsZonalStatistics::Majority,
          QgsZonalStatistics::Variety,
          QgsZonalStatistics::Variance
        } )
  {
    if ( mStatistics & stat )
    {
      QString fieldName = getUniqueFieldName( mAttributePrefix + QgsZonalStatistics::shortName( stat ), newFieldList );
      QgsField field( fieldName, QVariant::Double, QStringLiteral( "double precision" ) );
      newFieldList.push_back( field );
      statFieldIndexes.insert( stat, oldFieldCount + newFieldList.count() - 1 );
    }
  }

  vectorProvider->addAttributes( newFieldList );

  long featureCount = vectorProvider->featureCount();

  QgsFeatureRequest request;
  request.setNoAttributes();

  request.setDestinationCrs( mRasterCrs, QgsProject::instance()->transformContext() );
  QgsFeatureIterator fi = vectorProvider->getFeatures( request );
  QgsFeature feature;

  int featureCounter = 0;

  QgsChangedAttributesMap changeMap;
  while ( fi.nextFeature( feature ) )
  {
    ++featureCounter;
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( featureCounter ) / featureCount );
    }

    QgsGeometry featureGeometry = feature.geometry();

    QMap<QgsZonalStatistics::Statistic, QVariant> results = calculateStatistics( mRasterInterface, featureGeometry, mCellSizeX, mCellSizeY, mRasterBand, mStatistics );

    if ( results.empty() )
      continue;

    QgsAttributeMap changeAttributeMap;
    for ( const auto &result : results.toStdMap() )
    {
      changeAttributeMap.insert( statFieldIndexes.value( result.first ), result.second );
    }

    changeMap.insert( feature.id(), changeAttributeMap );
  }

  vectorProvider->changeAttributeValues( changeMap );
  mPolygonLayer->updateFields();

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return Canceled;

    feedback->setProgress( 100 );
  }

  return Success;
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
  for ( const QgsField &field : std::as_const( allFields ) )
  {
    if ( shortName == field.name() )
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
    for ( const QgsField &field : std::as_const( allFields ) )
    {
      if ( shortName == field.name() )
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

QString QgsZonalStatistics::displayName( QgsZonalStatistics::Statistic statistic )
{
  switch ( statistic )
  {
    case Count:
      return QObject::tr( "Count" );
    case Sum:
      return QObject::tr( "Sum" );
    case Mean:
      return QObject::tr( "Mean" );
    case Median:
      return QObject::tr( "Median" );
    case StDev:
      return QObject::tr( "St dev" );
    case Min:
      return QObject::tr( "Minimum" );
    case Max:
      return QObject::tr( "Maximum" );
    case Range:
      return QObject::tr( "Range" );
    case Minority:
      return QObject::tr( "Minority" );
    case Majority:
      return QObject::tr( "Majority" );
    case Variety:
      return QObject::tr( "Variety" );
    case Variance:
      return QObject::tr( "Variance" );
    case All:
      return QString();
  }
  return QString();
}

QString QgsZonalStatistics::shortName( QgsZonalStatistics::Statistic statistic )
{
  switch ( statistic )
  {
    case Count:
      return QStringLiteral( "count" );
    case Sum:
      return QStringLiteral( "sum" );
    case Mean:
      return QStringLiteral( "mean" );
    case Median:
      return QStringLiteral( "median" );
    case StDev:
      return QStringLiteral( "stdev" );
    case Min:
      return QStringLiteral( "min" );
    case Max:
      return QStringLiteral( "max" );
    case Range:
      return QStringLiteral( "range" );
    case Minority:
      return QStringLiteral( "minority" );
    case Majority:
      return QStringLiteral( "majority" );
    case Variety:
      return QStringLiteral( "variety" );
    case Variance:
      return QStringLiteral( "variance" );
    case All:
      return QString();
  }
  return QString();
}

///@cond PRIVATE
QMap<int, QVariant> QgsZonalStatistics::calculateStatisticsInt( QgsRasterInterface *rasterInterface, const QgsGeometry &geometry, double cellSizeX, double cellSizeY, int rasterBand, QgsZonalStatistics::Statistics statistics )
{
  const auto result { QgsZonalStatistics::calculateStatistics( rasterInterface, geometry, cellSizeX, cellSizeY, rasterBand, statistics ) };
  QMap<int, QVariant> pyResult;
  for ( auto it = result.constBegin(); it != result.constEnd(); ++it )
  {
    pyResult.insert( it.key(), it.value() );
  }
  return pyResult;
}
/// @endcond

QMap<QgsZonalStatistics::Statistic, QVariant> QgsZonalStatistics::calculateStatistics( QgsRasterInterface *rasterInterface, const QgsGeometry &geometry, double cellSizeX, double cellSizeY, int rasterBand, QgsZonalStatistics::Statistics statistics )
{
  QMap<QgsZonalStatistics::Statistic, QVariant> results;

  if ( geometry.isEmpty() )
    return results;

  QgsRectangle rasterBBox = rasterInterface->extent();

  QgsRectangle featureRect = geometry.boundingBox().intersect( rasterBBox );

  if ( featureRect.isEmpty() )
    return results;

  bool statsStoreValues = ( statistics & QgsZonalStatistics::Median ) ||
                          ( statistics & QgsZonalStatistics::StDev ) ||
                          ( statistics & QgsZonalStatistics::Variance );
  bool statsStoreValueCount = ( statistics & QgsZonalStatistics::Minority ) ||
                              ( statistics & QgsZonalStatistics::Majority );

  FeatureStats featureStats( statsStoreValues, statsStoreValueCount );

  int nCellsXProvider = rasterInterface->xSize();
  int nCellsYProvider = rasterInterface->ySize();

  int nCellsX, nCellsY;
  QgsRectangle rasterBlockExtent;
  QgsRasterAnalysisUtils::cellInfoForBBox( rasterBBox, featureRect, cellSizeX, cellSizeY, nCellsX, nCellsY, nCellsXProvider, nCellsYProvider, rasterBlockExtent );

  featureStats.reset();
  QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( rasterInterface, rasterBand, geometry, nCellsX, nCellsY, cellSizeX, cellSizeY, rasterBlockExtent, [ &featureStats ]( double value ) { featureStats.addValue( value ); } );

  if ( featureStats.count <= 1 )
  {
    //the cell resolution is probably larger than the polygon area. We switch to precise pixel - polygon intersection in this case
    featureStats.reset();
    QgsRasterAnalysisUtils::statisticsFromPreciseIntersection( rasterInterface, rasterBand, geometry, nCellsX, nCellsY, cellSizeX, cellSizeY, rasterBlockExtent, [ &featureStats ]( double value, double weight ) { featureStats.addValue( value, weight ); } );
  }

  // calculate the statistics

  if ( statistics & QgsZonalStatistics::Count )
    results.insert( QgsZonalStatistics::Count, QVariant( featureStats.count ) );
  if ( statistics & QgsZonalStatistics::Sum )
    results.insert( QgsZonalStatistics::Sum, QVariant( featureStats.sum ) );
  if ( featureStats.count > 0 )
  {
    double mean = featureStats.sum / featureStats.count;
    if ( statistics & QgsZonalStatistics::Mean )
      results.insert( QgsZonalStatistics::Mean, QVariant( mean ) );
    if ( statistics & QgsZonalStatistics::Median )
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
      results.insert( QgsZonalStatistics::Median, QVariant( medianValue ) );
    }
    if ( statistics & QgsZonalStatistics::StDev || statistics & QgsZonalStatistics::Variance )
    {
      double sumSquared = 0;
      for ( int i = 0; i < featureStats.values.count(); ++i )
      {
        double diff = featureStats.values.at( i ) - mean;
        sumSquared += diff * diff;
      }
      double variance = sumSquared / featureStats.values.count();
      if ( statistics & QgsZonalStatistics::StDev )
      {
        double stdev = std::pow( variance, 0.5 );
        results.insert( QgsZonalStatistics::StDev, QVariant( stdev ) );
      }
      if ( statistics & QgsZonalStatistics::Variance )
        results.insert( QgsZonalStatistics::Variance, QVariant( variance ) );
    }
    if ( statistics & QgsZonalStatistics::Min )
      results.insert( QgsZonalStatistics::Min, QVariant( featureStats.min ) );
    if ( statistics & QgsZonalStatistics::Max )
      results.insert( QgsZonalStatistics::Max, QVariant( featureStats.max ) );
    if ( statistics & QgsZonalStatistics::Range )
      results.insert( QgsZonalStatistics::Range, QVariant( featureStats.max - featureStats.min ) );
    if ( statistics & QgsZonalStatistics::Minority || statistics & QgsZonalStatistics::Majority )
    {
      QList<int> vals = featureStats.valueCount.values();
      std::sort( vals.begin(), vals.end() );
      if ( statistics & QgsZonalStatistics::Minority )
      {
        double minorityKey = featureStats.valueCount.key( vals.first() );
        results.insert( QgsZonalStatistics::Minority, QVariant( minorityKey ) );
      }
      if ( statistics & QgsZonalStatistics::Majority )
      {
        double majKey = featureStats.valueCount.key( vals.last() );
        results.insert( QgsZonalStatistics::Majority, QVariant( majKey ) );
      }
    }
    if ( statistics & QgsZonalStatistics::Variety )
      results.insert( QgsZonalStatistics::Variety, QVariant( featureStats.valueCount.count() ) );
  }

  return results;
}
