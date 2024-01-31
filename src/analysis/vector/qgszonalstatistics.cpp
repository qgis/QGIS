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
          QgsZonalStatistics::Statistic::Count,
          QgsZonalStatistics::Statistic::Sum,
          QgsZonalStatistics::Statistic::Mean,
          QgsZonalStatistics::Statistic::Median,
          QgsZonalStatistics::Statistic::StDev,
          QgsZonalStatistics::Statistic::Min,
          QgsZonalStatistics::Statistic::Max,
          QgsZonalStatistics::Statistic::Range,
          QgsZonalStatistics::Statistic::Minority,
          QgsZonalStatistics::Statistic::Majority,
          QgsZonalStatistics::Statistic::Variety,
          QgsZonalStatistics::Statistic::Variance
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
    case Statistic::Count:
      return QObject::tr( "Count" );
    case Statistic::Sum:
      return QObject::tr( "Sum" );
    case Statistic::Mean:
      return QObject::tr( "Mean" );
    case Statistic::Median:
      return QObject::tr( "Median" );
    case Statistic::StDev:
      return QObject::tr( "St dev" );
    case Statistic::Min:
      return QObject::tr( "Minimum" );
    case Statistic::Max:
      return QObject::tr( "Maximum" );
    case Statistic::Range:
      return QObject::tr( "Range" );
    case Statistic::Minority:
      return QObject::tr( "Minority" );
    case Statistic::Majority:
      return QObject::tr( "Majority" );
    case Statistic::Variety:
      return QObject::tr( "Variety" );
    case Statistic::Variance:
      return QObject::tr( "Variance" );
    case Statistic::All:
    case Statistic::Default:
      return QString();
  }
  return QString();
}

QString QgsZonalStatistics::shortName( QgsZonalStatistics::Statistic statistic )
{
  switch ( statistic )
  {
    case Statistic::Count:
      return QStringLiteral( "count" );
    case Statistic::Sum:
      return QStringLiteral( "sum" );
    case Statistic::Mean:
      return QStringLiteral( "mean" );
    case Statistic::Median:
      return QStringLiteral( "median" );
    case Statistic::StDev:
      return QStringLiteral( "stdev" );
    case Statistic::Min:
      return QStringLiteral( "min" );
    case Statistic::Max:
      return QStringLiteral( "max" );
    case Statistic::Range:
      return QStringLiteral( "range" );
    case Statistic::Minority:
      return QStringLiteral( "minority" );
    case Statistic::Majority:
      return QStringLiteral( "majority" );
    case Statistic::Variety:
      return QStringLiteral( "variety" );
    case Statistic::Variance:
      return QStringLiteral( "variance" );
    case Statistic::All:
    case Statistic::Default:
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
    pyResult.insert( static_cast< int >( it.key() ), it.value() );
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

  bool statsStoreValues = ( statistics & QgsZonalStatistics::Statistic::Median ) ||
                          ( statistics & QgsZonalStatistics::Statistic::StDev ) ||
                          ( statistics & QgsZonalStatistics::Statistic::Variance );
  bool statsStoreValueCount = ( statistics & QgsZonalStatistics::Statistic::Minority ) ||
                              ( statistics & QgsZonalStatistics::Statistic::Majority );

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

  if ( statistics & QgsZonalStatistics::Statistic::Count )
    results.insert( QgsZonalStatistics::Statistic::Count, QVariant( featureStats.count ) );
  if ( statistics & QgsZonalStatistics::Statistic::Sum )
    results.insert( QgsZonalStatistics::Statistic::Sum, QVariant( featureStats.sum ) );
  if ( featureStats.count > 0 )
  {
    double mean = featureStats.sum / featureStats.count;
    if ( statistics & QgsZonalStatistics::Statistic::Mean )
      results.insert( QgsZonalStatistics::Statistic::Mean, QVariant( mean ) );
    if ( statistics & QgsZonalStatistics::Statistic::Median )
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
      results.insert( QgsZonalStatistics::Statistic::Median, QVariant( medianValue ) );
    }
    if ( statistics & QgsZonalStatistics::Statistic::StDev || statistics & QgsZonalStatistics::Statistic::Variance )
    {
      double sumSquared = 0;
      for ( int i = 0; i < featureStats.values.count(); ++i )
      {
        double diff = featureStats.values.at( i ) - mean;
        sumSquared += diff * diff;
      }
      double variance = sumSquared / featureStats.values.count();
      if ( statistics & QgsZonalStatistics::Statistic::StDev )
      {
        double stdev = std::pow( variance, 0.5 );
        results.insert( QgsZonalStatistics::Statistic::StDev, QVariant( stdev ) );
      }
      if ( statistics & QgsZonalStatistics::Statistic::Variance )
        results.insert( QgsZonalStatistics::Statistic::Variance, QVariant( variance ) );
    }
    if ( statistics & QgsZonalStatistics::Statistic::Min )
      results.insert( QgsZonalStatistics::Statistic::Min, QVariant( featureStats.min ) );
    if ( statistics & QgsZonalStatistics::Statistic::Max )
      results.insert( QgsZonalStatistics::Statistic::Max, QVariant( featureStats.max ) );
    if ( statistics & QgsZonalStatistics::Statistic::Range )
      results.insert( QgsZonalStatistics::Statistic::Range, QVariant( featureStats.max - featureStats.min ) );
    if ( statistics & QgsZonalStatistics::Statistic::Minority || statistics & QgsZonalStatistics::Statistic::Majority )
    {
      QList<int> vals = featureStats.valueCount.values();
      std::sort( vals.begin(), vals.end() );
      if ( statistics & QgsZonalStatistics::Statistic::Minority )
      {
        double minorityKey = featureStats.valueCount.key( vals.first() );
        results.insert( QgsZonalStatistics::Statistic::Minority, QVariant( minorityKey ) );
      }
      if ( statistics & QgsZonalStatistics::Statistic::Majority )
      {
        double majKey = featureStats.valueCount.key( vals.last() );
        results.insert( QgsZonalStatistics::Statistic::Majority, QVariant( majKey ) );
      }
    }
    if ( statistics & QgsZonalStatistics::Statistic::Variety )
      results.insert( QgsZonalStatistics::Statistic::Variety, QVariant( featureStats.valueCount.count() ) );
  }

  return results;
}
