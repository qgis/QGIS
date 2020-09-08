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

QgsZonalStatistics::QgsZonalStatistics( QgsFeatureSource *source, QgsFeatureSink *sink, QgsRasterInterface *rasterInterface, const QgsCoordinateReferenceSystem &rasterCrs, const QMap<QgsZonalStatistics::Statistic, int> &statFieldIndexes, const QgsFields &fields, double rasterUnitsPerPixelX, double rasterUnitsPerPixelY, int rasterBand, QgsZonalStatistics::Statistics stats )
  : mRasterInterface( rasterInterface )
  , mRasterCrs( rasterCrs )
  , mCellSizeX( std::fabs( rasterUnitsPerPixelX ) )
  , mCellSizeY( std::fabs( rasterUnitsPerPixelY ) )
  , mRasterBand( rasterBand )
  , mStatistics( stats )
  , mSource( source )
  , mSink( sink )
  , mStatFieldIndexes( statFieldIndexes )
  , mFields( fields )
{
}

QgsZonalStatistics::Result QgsZonalStatistics::calculateStatistics( QgsFeedback *feedback )
{
  QgsFeatureSource *source = nullptr;
  QgsVectorDataProvider *vectorProvider = nullptr;

  if ( !mRasterInterface )
  {
    return RasterInvalid;
  }

  if ( mRasterInterface->bandCount() < mRasterBand )
  {
    return RasterBandInvalid;
  }

  //get geometry info about raster layer
  int nCellsXProvider = mRasterInterface->xSize();
  int nCellsYProvider = mRasterInterface->ySize();

  QgsRectangle rasterBBox = mRasterInterface->extent();

  if ( mSource )
  {
    if ( QgsWkbTypes::geometryType( mSource->wkbType() ) != QgsWkbTypes::PolygonGeometry )
      return LayerTypeWrong;
    source = mSource;
  }
  else
  {
    if ( !mPolygonLayer || QgsWkbTypes::geometryType( mSource->wkbType() ) != QgsWkbTypes::PolygonGeometry )
    {
      return LayerTypeWrong;
    }

    vectorProvider = mPolygonLayer->dataProvider();
    if ( !vectorProvider )
    {
      return LayerInvalid;
    }
    source = vectorProvider;

    //add the new fields to the provider
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
        mStatFieldIndexes.insert( stat, newFieldList.count() - 1 );
      }
    }

    vectorProvider->addAttributes( newFieldList );
  }

  long featureCount = source->featureCount();

  QgsFeatureRequest request;

  // If we edit in place, we don't need the other attributes
  if ( vectorProvider )
    request.setNoAttributes();

  request.setDestinationCrs( mRasterCrs, QgsProject::instance()->transformContext() );
  QgsFeatureIterator fi = source->getFeatures( request );
  QgsFeature feature;

  bool statsStoreValues = ( mStatistics & QgsZonalStatistics::Median ) ||
                          ( mStatistics & QgsZonalStatistics::StDev ) ||
                          ( mStatistics & QgsZonalStatistics::Variance );
  bool statsStoreValueCount = ( mStatistics & QgsZonalStatistics::Minority ) ||
                              ( mStatistics & QgsZonalStatistics::Majority );

  FeatureStats featureStats( statsStoreValues, statsStoreValueCount );
  int featureCounter = 0;

  QgsChangedAttributesMap changeMap;
  while ( fi.nextFeature( feature ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 * static_cast< double >( featureCounter ) / featureCount );
    }

    if ( !feature.hasGeometry() )
    {
      if ( mSink )
        mSink->addFeature( feature );
      ++featureCounter;
      continue;
    }

    QgsGeometry featureGeometry = feature.geometry();

    QgsRectangle featureRect = featureGeometry.boundingBox().intersect( rasterBBox );
    if ( featureRect.isEmpty() )
    {
      if ( mSink )
        mSink->addFeature( feature );

      ++featureCounter;
      continue;
    }

    int nCellsX, nCellsY;
    QgsRectangle rasterBlockExtent;
    QgsRasterAnalysisUtils::cellInfoForBBox( rasterBBox, featureRect, mCellSizeX, mCellSizeY, nCellsX, nCellsY, nCellsXProvider, nCellsYProvider, rasterBlockExtent );

    featureStats.reset();
    QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( mRasterInterface, mRasterBand, featureGeometry, nCellsX, nCellsY, mCellSizeX, mCellSizeY,
    rasterBlockExtent, [ &featureStats ]( double value ) { featureStats.addValue( value ); } );

    if ( featureStats.count <= 1 )
    {
      //the cell resolution is probably larger than the polygon area. We switch to precise pixel - polygon intersection in this case
      featureStats.reset();
      QgsRasterAnalysisUtils::statisticsFromPreciseIntersection( mRasterInterface, mRasterBand, featureGeometry, nCellsX, nCellsY, mCellSizeX, mCellSizeY,
      rasterBlockExtent, [ &featureStats ]( double value, double weight ) { featureStats.addValue( value, weight ); } );
    }

    // calculate the statistics
    QgsAttributeMap changeAttributeMap;
    if ( mStatistics & QgsZonalStatistics::Count )
      changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Count ), QVariant( featureStats.count ) );
    if ( mStatistics & QgsZonalStatistics::Sum )
      changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Sum ), QVariant( featureStats.sum ) );
    if ( featureStats.count > 0 )
    {
      double mean = featureStats.sum / featureStats.count;
      if ( mStatistics & QgsZonalStatistics::Mean )
        changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Mean ), QVariant( mean ) );
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
        changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Median ), QVariant( medianValue ) );
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
          changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::StDev ), QVariant( stdev ) );
        }
        if ( mStatistics & QgsZonalStatistics::Variance )
          changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Variance ), QVariant( variance ) );
      }
      if ( mStatistics & QgsZonalStatistics::Min )
        changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Min ), QVariant( featureStats.min ) );
      if ( mStatistics & QgsZonalStatistics::Max )
        changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Max ), QVariant( featureStats.max ) );
      if ( mStatistics & QgsZonalStatistics::Range )
        changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Range ), QVariant( featureStats.max - featureStats.min ) );
      if ( mStatistics & QgsZonalStatistics::Minority || mStatistics & QgsZonalStatistics::Majority )
      {
        QList<int> vals = featureStats.valueCount.values();
        std::sort( vals.begin(), vals.end() );
        if ( mStatistics & QgsZonalStatistics::Minority )
        {
          double minorityKey = featureStats.valueCount.key( vals.first() );
          changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Minority ), QVariant( minorityKey ) );
        }
        if ( mStatistics & QgsZonalStatistics::Majority )
        {
          double majKey = featureStats.valueCount.key( vals.last() );
          changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Majority ), QVariant( majKey ) );
        }
      }
      if ( mStatistics & QgsZonalStatistics::Variety )
        changeAttributeMap.insert( mStatFieldIndexes.value( QgsZonalStatistics::Variety ), QVariant( featureStats.valueCount.count() ) );
    }

    if ( mSink )
    {
      QgsFeature newFeature = feature;
      newFeature.setFields( mFields );
      for ( const auto &attr : changeAttributeMap.toStdMap() )
      {
        newFeature.setAttribute( attr.first, attr.second );
      }
      mSink->addFeature( newFeature );
    }
    else
    {
      changeMap.insert( feature.id(), changeAttributeMap );
    }
    ++featureCounter;
  }

  if ( vectorProvider )
    vectorProvider->changeAttributeValues( changeMap );
  mPolygonLayer->updateFields();

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return Cancelled;

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
  for ( const QgsField &field : qgis::as_const( allFields ) )
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
    for ( const QgsField &field : qgis::as_const( allFields ) )
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
