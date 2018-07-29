/***************************************************************************
                         qgsalgorithmdrape.cpp
                         --------------------------
    begin                : July 2018
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmdrape.h"

#include "qgscoordinatetransformcontext.h"
#include "qgsabstractgeometry.h"
#include "qgsgeometryutils.h"

///@cond PRIVATE

QString QgsDrapeAlgorithm::name() const
{
  return QStringLiteral( "drape" );
}

QString QgsDrapeAlgorithm::displayName() const
{
  return QObject::tr( "Drape features" );
}

QStringList QgsDrapeAlgorithm::tags() const
{
  return QObject::tr( "3d,vertex,vertices,elevation,sample" ).split( ',' );
}

QString QgsDrapeAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDrapeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsDrapeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm attaches a sampled raster value on vertices of vector features." );
}

QgsDrapeAlgorithm *QgsDrapeAlgorithm::createInstance() const
{
  return new QgsDrapeAlgorithm();
}

void QgsDrapeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "RASTER" ) ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SCALE" ), QObject::tr( "Scale factor" ), QgsProcessingParameterNumber::Double, 1.0, false, 0.0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Draped" ) ) );
}

bool QgsDrapeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "RASTER" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "RASTER" ) ) );

  mRasterDataProvider.reset( dynamic_cast< QgsRasterDataProvider *>( layer->dataProvider()->clone() ) );
  mRasterBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  mRasterExtent = layer->extent();
  mRasterCrs = layer->crs();

  mTransformContext = context.project() ? context.project()->transformContext() : QgsCoordinateTransformContext();

  return true;
}

QVariantMap QgsDrapeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsWkbTypes::Type outputWkbType = featureSource->wkbType();
  outputWkbType = QgsWkbTypes::addZ( outputWkbType );

  double scale = parameterAsDouble( parameters, QStringLiteral( "SCALE" ), context );

  QgsCoordinateTransform transform;
  if ( featureSource->sourceCrs() != mRasterCrs )
  {
    transform = QgsCoordinateTransform( featureSource->sourceCrs(), mRasterCrs, mTransformContext );
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, featureSource->fields(), outputWkbType, featureSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;
  QgsFeatureIterator fi = featureSource->getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  int i = -1;
  while ( fi.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsGeometry outputGeom = f.geometry();
    if ( outputGeom.isNull() )
    {
      sink->addFeature( f, QgsFeatureSink::FastInsert );
    }
    else
    {
      outputGeom.get()->addZValue( 0 );

      if ( outputGeom.boundingBox().intersects( mRasterExtent ) )
      {
        QgsGeometry sampleGeom = f.geometry();
        if ( featureSource->sourceCrs() != mRasterCrs )
        {
          sampleGeom.transform( transform );
        }
        QgsAbstractGeometry::vertex_iterator vi = sampleGeom.constGet()->vertices_begin();
        int vertexPos = 0;
        while ( vi != sampleGeom.constGet()->vertices_end() )
        {
          double value = mRasterDataProvider->sample( *vi, mRasterBand );

          QgsPoint point = outputGeom.vertexAt( vertexPos );
          point.setZ( std::isnan( value ) ? 0.0 : value * scale );
          outputGeom.moveVertex( point, vertexPos );

          vi++;
          vertexPos++;
        }
      }

      QgsFeature outputFeature = QgsFeature();
      outputFeature.setAttributes( f.attributes() );
      outputFeature.setGeometry( outputGeom );
      sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
    }
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
