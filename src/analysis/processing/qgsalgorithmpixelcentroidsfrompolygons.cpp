/***************************************************************************
                         qgsalgorithmpixelcentroidsfrompolygons.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpixelcentroidsfrompolygons.h"

#include "qgsgeos.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

QString QgsPixelCentroidsFromPolygonsAlgorithm::name() const
{
  return u"generatepointspixelcentroidsinsidepolygons"_s;
}

QString QgsPixelCentroidsFromPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Generate points (pixel centroids) inside polygons" );
}

QStringList QgsPixelCentroidsFromPolygonsAlgorithm::tags() const
{
  return QObject::tr( "raster,polygon,centroid,pixel,create" ).split( ',' );
}

QString QgsPixelCentroidsFromPolygonsAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsPixelCentroidsFromPolygonsAlgorithm::groupId() const
{
  return u"vectorcreation"_s;
}

QString QgsPixelCentroidsFromPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates pixel centroids for the raster area falling inside polygons. Used to generate points "
                      "for further raster sampling." );
}

QString QgsPixelCentroidsFromPolygonsAlgorithm::shortDescription() const
{
  return QObject::tr( "Generates pixel centroids for the raster area falling inside polygons." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsPixelCentroidsFromPolygonsAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsPixelCentroidsFromPolygonsAlgorithm *QgsPixelCentroidsFromPolygonsAlgorithm::createInstance() const
{
  return new QgsPixelCentroidsFromPolygonsAlgorithm();
}

void QgsPixelCentroidsFromPolygonsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_RASTER"_s, QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT_VECTOR"_s, QObject::tr( "Vector layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Pixel centroids" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

QVariantMap QgsPixelCentroidsFromPolygonsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *rasterLayer = parameterAsRasterLayer( parameters, u"INPUT_RASTER"_s, context );

  if ( !rasterLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_RASTER"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT_VECTOR"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT_VECTOR"_s ) );

  QgsFields fields;
  fields.append( QgsField( u"id"_s, QMetaType::Type::LongLong ) );
  fields.append( QgsField( u"poly_id"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"point_id"_s, QMetaType::Type::Int ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Point, rasterLayer->crs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const double step = source->featureCount() ? 100.0 / source->featureCount() : 1;
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest().setDestinationCrs( rasterLayer->crs(), context.transformContext() ).setSubsetOfAttributes( QList<int>() ) );

  const double xPixel = rasterLayer->rasterUnitsPerPixelX();
  const double yPixel = rasterLayer->rasterUnitsPerPixelY();
  const QgsRectangle extent = rasterLayer->extent();

  QgsFeature feature;
  feature.setFields( fields );

  int fid = 0;
  int pointId = 0;

  int i = 0;
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !f.hasGeometry() )
    {
      continue;
    }

    const QgsRectangle bbox = f.geometry().boundingBox();
    const double xMin = bbox.xMinimum();
    const double xMax = bbox.xMaximum();
    const double yMin = bbox.yMinimum();
    const double yMax = bbox.yMaximum();

    double x, y;
    int startRow, startColumn;
    int endRow, endColumn;
    QgsRasterAnalysisUtils::mapToPixel( xMin, yMax, extent, xPixel, yPixel, startRow, startColumn );
    QgsRasterAnalysisUtils::mapToPixel( xMax, yMin, extent, xPixel, yPixel, endRow, endColumn );

    auto engine = std::make_unique<QgsGeos>( f.geometry().constGet() );
    engine->prepareGeometry();

    for ( int row = startRow; row <= endRow; row++ )
    {
      for ( int col = startColumn; col <= endColumn; col++ )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }

        QgsRasterAnalysisUtils::pixelToMap( row, col, extent, xPixel, yPixel, x, y );
        if ( engine->contains( x, y ) )
        {
          feature.setGeometry( std::make_unique<QgsPoint>( x, y ) );
          feature.setAttributes( QgsAttributes() << fid << i << pointId );
          if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

          fid++;
          pointId++;
        }
      }
    }

    pointId = 0;

    feedback->setProgress( i * step );
    i++;
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
