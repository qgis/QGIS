/***************************************************************************
                         qgsalgorithmexportlayersinformation.cpp
                         ---------------------------------
    begin                : December 2020
    copyright            : (C) 2020 by Mathieu Pellerin
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

#include "qgsalgorithmexportlayersinformation.h"
#include "qgsproviderregistry.h"

///@cond PRIVATE

QString QgsExportLayersInformationAlgorithm::name() const
{
  return QStringLiteral( "exportlayersinformation" );
}

QString QgsExportLayersInformationAlgorithm::displayName() const
{
  return QObject::tr( "Export layer(s) information" );
}

QStringList QgsExportLayersInformationAlgorithm::tags() const
{
  return QObject::tr( "metadata,details,extent" ).split( ',' );
}

QString QgsExportLayersInformationAlgorithm::group() const
{
  return QObject::tr( "Layer tools" );
}

QString QgsExportLayersInformationAlgorithm::groupId() const
{
  return QStringLiteral( "layertools" );
}

void QgsExportLayersInformationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layer(s)" ), QgsProcessing::TypeMapLayer ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ), QgsProcessing::TypeVectorPolygon, QVariant() ) );
}

QString QgsExportLayersInformationAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates a polygon layer with features corresponding to the extent of selected layer(s).\n\n"
                      "Additional layer details - CRS, provider name, file path, layer name, subset filter, abstract and attribution - are attached as attributes to each feature." );
}

QgsExportLayersInformationAlgorithm *QgsExportLayersInformationAlgorithm::createInstance() const
{
  return new QgsExportLayersInformationAlgorithm();
}

bool QgsExportLayersInformationAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  for ( QgsMapLayer *layer : layers )
  {
    if ( !mCrs.isValid() )
    {
      mCrs = layer->crs();
    }
    else if ( mCrs.authid() != QLatin1String( "EPSG:4326" ) )
    {
      if ( mCrs != layer->crs() )
      {
        // mixed CRSes, set output CRS to EPSG:4326
        mCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
      }
    }
    mLayers.emplace_back( layer->clone() );
  }

  if ( !mCrs.isValid() )
    mCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );

  if ( mLayers.empty() )
    feedback->reportError( QObject::tr( "No layers selected" ), false );

  return true;
}

QVariantMap QgsExportLayersInformationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFields outFields;
  outFields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "source" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "crs" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "provider" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "file_path" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "layer_name" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "subset" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "abstract" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "attribution" ), QVariant::String ) );

  QString outputDest;
  std::unique_ptr< QgsFeatureSink > outputSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, outputDest, outFields,
      QgsWkbTypes::Polygon, mCrs ) );

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );

  const double step = layers.size() > 0 ? 100.0 / layers.size() : 1;
  int i = 0;
  for ( const std::unique_ptr< QgsMapLayer > &layer : mLayers )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    QgsFeature feature;

    QgsAttributes attributes;
    attributes << layer->name()
               << layer->source()
               << layer->crs().authid();
    if ( layer->dataProvider() )
    {
      const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layer->dataProvider()->name(), layer->source() );
      attributes << layer->dataProvider()->name()
                 << parts[ QStringLiteral( "path" ) ]
                 << parts[ QStringLiteral( "layerName" ) ]
                 << parts[ QStringLiteral( "subset" ) ];
    }
    else
    {
      attributes << QVariant() << QVariant() << QVariant() << QVariant();
    }
    attributes << layer->metadata().rights().join( ';' )
               << layer->abstract();
    feature.setAttributes( attributes );

    QgsRectangle rect = layer->extent();
    if ( !rect.isEmpty() )
    {
      if ( layer->crs() != mCrs )
      {
        QgsCoordinateTransform transform( layer->crs(), mCrs, context.transformContext() );
        transform.setBallparkTransformsAreAppropriate( true );
        try
        {
          rect = transform.transformBoundingBox( rect );
        }
        catch ( QgsCsException &e )
        {
          Q_UNUSED( e )
          rect = QgsRectangle();
          feedback->pushInfo( QObject::tr( "Extent of layer %1 could not be reprojected" ).arg( layer->name() ) );
        }
      }
      feature.setGeometry( QgsGeometry::fromRect( rect ) );
    }
    if ( !outputSink->addFeature( feature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( outputSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputDest );
  return outputs;
}

///@endcond
