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
#include "qgsgeometryengine.h"
#include "qgsrasteranalysisutils.h"

///@cond PRIVATE

QString QgsPixelCentroidsFromPolygonsAlgorithm::name() const
{
  return QStringLiteral( "generatepointspixelcentroidsinsidepolygons" );
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
  return QStringLiteral( "vectorcreation" );
}

QString QgsPixelCentroidsFromPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "Generates pixel centroids for the raster area falling inside polygons. Used to generate points "
                      "for further raster sampling." );
}

QgsPixelCentroidsFromPolygonsAlgorithm *QgsPixelCentroidsFromPolygonsAlgorithm::createInstance() const
{
  return new QgsPixelCentroidsFromPolygonsAlgorithm();
}

void QgsPixelCentroidsFromPolygonsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ), QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_VECTOR" ), QObject::tr( "Vector layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Pixel centroids" ), QgsProcessing::TypeVectorPoint ) );
}

QVariantMap QgsPixelCentroidsFromPolygonsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *rasterLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );

  if ( !rasterLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT_VECTOR" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_VECTOR" ) ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::LongLong ) );
  fields.append( QgsField( QStringLiteral( "poly_id" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "point_id" ), QVariant::Int ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Point, rasterLayer->crs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const double step = source->featureCount() ? 100.0 / source->featureCount() : 1;
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest().setDestinationCrs( rasterLayer->crs(), context.transformContext() ).setSubsetOfAttributes( QList< int >() ) );

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

    std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( f.geometry().constGet() ) );
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
        const QgsPoint point( x, y );
        const QgsGeometry geom( point.clone() );
        if ( engine->contains( geom.constGet() ) )
        {
          feature.setGeometry( geom );
          feature.setAttributes( QgsAttributes() << fid << i << pointId );
          if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

          fid++;
          pointId++;
        }
      }
    }

    pointId = 0;

    feedback->setProgress( i * step );
    i++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
