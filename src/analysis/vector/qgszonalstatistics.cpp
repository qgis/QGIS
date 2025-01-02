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
#include "qgsrasterlayer.h"
#include "qgsproject.h"

#include <QFile>

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer *polygonLayer, QgsRasterLayer *rasterLayer, const QString &attributePrefix, int rasterBand, Qgis::ZonalStatistics stats )
  : QgsZonalStatistics( polygonLayer, rasterLayer ? rasterLayer->dataProvider() : nullptr, rasterLayer ? rasterLayer->crs() : QgsCoordinateReferenceSystem(), rasterLayer ? rasterLayer->rasterUnitsPerPixelX() : 0, rasterLayer ? rasterLayer->rasterUnitsPerPixelY() : 0, attributePrefix, rasterBand, stats )
{
}

QgsZonalStatistics::QgsZonalStatistics( QgsVectorLayer *polygonLayer, QgsRasterInterface *rasterInterface, const QgsCoordinateReferenceSystem &rasterCrs, double rasterUnitsPerPixelX, double rasterUnitsPerPixelY, const QString &attributePrefix, int rasterBand, Qgis::ZonalStatistics stats )
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

Qgis::ZonalStatisticResult QgsZonalStatistics::calculateStatistics( QgsFeedback *feedback )
{
  if ( !mRasterInterface )
  {
    return Qgis::ZonalStatisticResult::RasterInvalid;
  }

  if ( mRasterInterface->bandCount() < mRasterBand )
  {
    return Qgis::ZonalStatisticResult::RasterBandInvalid;
  }

  if ( !mPolygonLayer || mPolygonLayer->geometryType() != Qgis::GeometryType::Polygon )
  {
    return Qgis::ZonalStatisticResult::LayerTypeWrong;
  }

  QgsVectorDataProvider *vectorProvider = mPolygonLayer->dataProvider();
  if ( !vectorProvider )
  {
    return Qgis::ZonalStatisticResult::LayerInvalid;
  }

  QMap<Qgis::ZonalStatistic, int> statFieldIndexes;

  //add the new fields to the provider
  int oldFieldCount = vectorProvider->fields().count();
  QList<QgsField> newFieldList;
  for ( Qgis::ZonalStatistic stat :
        {
          Qgis::ZonalStatistic::Count,
          Qgis::ZonalStatistic::Sum,
          Qgis::ZonalStatistic::Mean,
          Qgis::ZonalStatistic::Median,
          Qgis::ZonalStatistic::StDev,
          Qgis::ZonalStatistic::Min,
          Qgis::ZonalStatistic::Max,
          Qgis::ZonalStatistic::Range,
          Qgis::ZonalStatistic::Minority,
          Qgis::ZonalStatistic::Majority,
          Qgis::ZonalStatistic::Variety,
          Qgis::ZonalStatistic::Variance
        } )
  {
    if ( mStatistics & stat )
    {
      QString fieldName = getUniqueFieldName( mAttributePrefix + QgsZonalStatistics::shortName( stat ), newFieldList );
      QgsField field( fieldName, QMetaType::Type::Double, QStringLiteral( "double precision" ) );
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
      feedback->setProgress( 100.0 * static_cast<double>( featureCounter ) / featureCount );
    }

    QgsGeometry featureGeometry = feature.geometry();

    QMap<Qgis::ZonalStatistic, QVariant> results = calculateStatistics( mRasterInterface, featureGeometry, mCellSizeX, mCellSizeY, mRasterBand, mStatistics );

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
      return Qgis::ZonalStatisticResult::Canceled;

    feedback->setProgress( 100 );
  }

  return Qgis::ZonalStatisticResult::Success;
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

QString QgsZonalStatistics::displayName( Qgis::ZonalStatistic statistic )
{
  switch ( statistic )
  {
    case Qgis::ZonalStatistic::Count:
      return QObject::tr( "Count" );
    case Qgis::ZonalStatistic::Sum:
      return QObject::tr( "Sum" );
    case Qgis::ZonalStatistic::Mean:
      return QObject::tr( "Mean" );
    case Qgis::ZonalStatistic::Median:
      return QObject::tr( "Median" );
    case Qgis::ZonalStatistic::StDev:
      return QObject::tr( "St dev" );
    case Qgis::ZonalStatistic::Min:
      return QObject::tr( "Minimum" );
    case Qgis::ZonalStatistic::Max:
      return QObject::tr( "Maximum" );
    case Qgis::ZonalStatistic::MinimumPoint:
      return QObject::tr( "Minimum point" );
    case Qgis::ZonalStatistic::MaximumPoint:
      return QObject::tr( "Maximum point" );
    case Qgis::ZonalStatistic::Range:
      return QObject::tr( "Range" );
    case Qgis::ZonalStatistic::Minority:
      return QObject::tr( "Minority" );
    case Qgis::ZonalStatistic::Majority:
      return QObject::tr( "Majority" );
    case Qgis::ZonalStatistic::Variety:
      return QObject::tr( "Variety" );
    case Qgis::ZonalStatistic::Variance:
      return QObject::tr( "Variance" );
    case Qgis::ZonalStatistic::All:
    case Qgis::ZonalStatistic::Default:
      return QString();
  }
  return QString();
}

QString QgsZonalStatistics::shortName( Qgis::ZonalStatistic statistic )
{
  switch ( statistic )
  {
    case Qgis::ZonalStatistic::Count:
      return QStringLiteral( "count" );
    case Qgis::ZonalStatistic::Sum:
      return QStringLiteral( "sum" );
    case Qgis::ZonalStatistic::Mean:
      return QStringLiteral( "mean" );
    case Qgis::ZonalStatistic::Median:
      return QStringLiteral( "median" );
    case Qgis::ZonalStatistic::StDev:
      return QStringLiteral( "stdev" );
    case Qgis::ZonalStatistic::Min:
      return QStringLiteral( "min" );
    case Qgis::ZonalStatistic::Max:
      return QStringLiteral( "max" );
    case Qgis::ZonalStatistic::MinimumPoint:
      return QStringLiteral( "minpoint" );
    case Qgis::ZonalStatistic::MaximumPoint:
      return QStringLiteral( "maxpoint" );
    case Qgis::ZonalStatistic::Range:
      return QStringLiteral( "range" );
    case Qgis::ZonalStatistic::Minority:
      return QStringLiteral( "minority" );
    case Qgis::ZonalStatistic::Majority:
      return QStringLiteral( "majority" );
    case Qgis::ZonalStatistic::Variety:
      return QStringLiteral( "variety" );
    case Qgis::ZonalStatistic::Variance:
      return QStringLiteral( "variance" );
    case Qgis::ZonalStatistic::All:
    case Qgis::ZonalStatistic::Default:
      return QString();
  }
  return QString();
}

///@cond PRIVATE
QMap<int, QVariant> QgsZonalStatistics::calculateStatisticsInt( QgsRasterInterface *rasterInterface, const QgsGeometry &geometry, double cellSizeX, double cellSizeY, int rasterBand, Qgis::ZonalStatistics statistics )
{
  const auto result { QgsZonalStatistics::calculateStatistics( rasterInterface, geometry, cellSizeX, cellSizeY, rasterBand, statistics ) };
  QMap<int, QVariant> pyResult;
  for ( auto it = result.constBegin(); it != result.constEnd(); ++it )
  {
    pyResult.insert( static_cast<int>( it.key() ), it.value() );
  }
  return pyResult;
}
/// @endcond

QMap<Qgis::ZonalStatistic, QVariant> QgsZonalStatistics::calculateStatistics( QgsRasterInterface *rasterInterface, const QgsGeometry &geometry, double cellSizeX, double cellSizeY, int rasterBand, Qgis::ZonalStatistics statistics )
{
  QMap<Qgis::ZonalStatistic, QVariant> results;

  if ( geometry.isEmpty() )
    return results;

  QgsRectangle rasterBBox = rasterInterface->extent();

  QgsRectangle featureRect = geometry.boundingBox().intersect( rasterBBox );

  if ( featureRect.isEmpty() )
    return results;

  bool statsStoreValues = ( statistics & Qgis::ZonalStatistic::Median ) || ( statistics & Qgis::ZonalStatistic::StDev ) || ( statistics & Qgis::ZonalStatistic::Variance );
  bool statsStoreValueCount = ( statistics & Qgis::ZonalStatistic::Minority ) || ( statistics & Qgis::ZonalStatistic::Majority );

  FeatureStats featureStats( statsStoreValues, statsStoreValueCount );

  int nCellsXProvider = rasterInterface->xSize();
  int nCellsYProvider = rasterInterface->ySize();

  int nCellsX, nCellsY;
  QgsRectangle rasterBlockExtent;
  QgsRasterAnalysisUtils::cellInfoForBBox( rasterBBox, featureRect, cellSizeX, cellSizeY, nCellsX, nCellsY, nCellsXProvider, nCellsYProvider, rasterBlockExtent );

  featureStats.reset();
  QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( rasterInterface, rasterBand, geometry, nCellsX, nCellsY, cellSizeX, cellSizeY, rasterBlockExtent, [&featureStats]( double value, const QgsPointXY &point ) { featureStats.addValue( value, point ); } );

  if ( featureStats.count <= 1 )
  {
    //the cell resolution is probably larger than the polygon area. We switch to precise pixel - polygon intersection in this case
    featureStats.reset();
    QgsRasterAnalysisUtils::statisticsFromPreciseIntersection( rasterInterface, rasterBand, geometry, nCellsX, nCellsY, cellSizeX, cellSizeY, rasterBlockExtent, [&featureStats]( double value, double weight, const QgsPointXY &point ) { featureStats.addValue( value, point, weight ); } );
  }

  // calculate the statistics

  if ( statistics & Qgis::ZonalStatistic::Count )
    results.insert( Qgis::ZonalStatistic::Count, QVariant( featureStats.count ) );
  if ( statistics & Qgis::ZonalStatistic::Sum )
    results.insert( Qgis::ZonalStatistic::Sum, QVariant( featureStats.sum ) );
  if ( featureStats.count > 0 )
  {
    double mean = featureStats.sum / featureStats.count;
    if ( statistics & Qgis::ZonalStatistic::Mean )
      results.insert( Qgis::ZonalStatistic::Mean, QVariant( mean ) );
    if ( statistics & Qgis::ZonalStatistic::Median )
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
      results.insert( Qgis::ZonalStatistic::Median, QVariant( medianValue ) );
    }
    if ( statistics & Qgis::ZonalStatistic::StDev || statistics & Qgis::ZonalStatistic::Variance )
    {
      double sumSquared = 0;
      for ( int i = 0; i < featureStats.values.count(); ++i )
      {
        double diff = featureStats.values.at( i ) - mean;
        sumSquared += diff * diff;
      }
      double variance = sumSquared / featureStats.values.count();
      if ( statistics & Qgis::ZonalStatistic::StDev )
      {
        double stdev = std::pow( variance, 0.5 );
        results.insert( Qgis::ZonalStatistic::StDev, QVariant( stdev ) );
      }
      if ( statistics & Qgis::ZonalStatistic::Variance )
        results.insert( Qgis::ZonalStatistic::Variance, QVariant( variance ) );
    }
    if ( statistics & Qgis::ZonalStatistic::Min )
      results.insert( Qgis::ZonalStatistic::Min, QVariant( featureStats.min ) );
    if ( statistics & Qgis::ZonalStatistic::Max )
      results.insert( Qgis::ZonalStatistic::Max, QVariant( featureStats.max ) );
    if ( statistics & Qgis::ZonalStatistic::MinimumPoint )
      results.insert( Qgis::ZonalStatistic::MinimumPoint, QVariant( featureStats.minPoint ) );
    if ( statistics & Qgis::ZonalStatistic::MaximumPoint )
      results.insert( Qgis::ZonalStatistic::MaximumPoint, QVariant( featureStats.maxPoint ) );
    if ( statistics & Qgis::ZonalStatistic::Range )
      results.insert( Qgis::ZonalStatistic::Range, QVariant( featureStats.max - featureStats.min ) );
    if ( statistics & Qgis::ZonalStatistic::Minority || statistics & Qgis::ZonalStatistic::Majority )
    {
      QList<int> vals = featureStats.valueCount.values();
      std::sort( vals.begin(), vals.end() );
      if ( statistics & Qgis::ZonalStatistic::Minority )
      {
        double minorityKey = featureStats.valueCount.key( vals.first() );
        results.insert( Qgis::ZonalStatistic::Minority, QVariant( minorityKey ) );
      }
      if ( statistics & Qgis::ZonalStatistic::Majority )
      {
        double majKey = featureStats.valueCount.key( vals.last() );
        results.insert( Qgis::ZonalStatistic::Majority, QVariant( majKey ) );
      }
    }
    if ( statistics & Qgis::ZonalStatistic::Variety )
      results.insert( Qgis::ZonalStatistic::Variety, QVariant( featureStats.valueCount.count() ) );
  }

  return results;
}
