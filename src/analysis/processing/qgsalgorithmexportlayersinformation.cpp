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
  return u"exportlayersinformation"_s;
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
  return u"layertools"_s;
}

void QgsExportLayersInformationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( u"LAYERS"_s, QObject::tr( "Input layer(s)" ), Qgis::ProcessingSourceType::MapLayer ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Output" ), Qgis::ProcessingSourceType::VectorPolygon, QVariant() ) );
}

QString QgsExportLayersInformationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a polygon layer with features corresponding to the extent of selected layer(s).\n\n"
                      "Additional layer details - CRS, provider name, file path, layer name, subset filter, abstract and attribution - are attached as attributes to each feature." );
}

QString QgsExportLayersInformationAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a polygon layer with features corresponding to the extent of selected layer(s)." );
}

QgsExportLayersInformationAlgorithm *QgsExportLayersInformationAlgorithm::createInstance() const
{
  return new QgsExportLayersInformationAlgorithm();
}

bool QgsExportLayersInformationAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );
  for ( QgsMapLayer *layer : layers )
  {
    if ( !mCrs.isValid() )
    {
      mCrs = layer->crs();
    }
    else if ( mCrs.authid() != "EPSG:4326"_L1 )
    {
      if ( mCrs != layer->crs() )
      {
        // mixed CRSes, set output CRS to EPSG:4326
        mCrs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
      }
    }
    mLayers.emplace_back( layer->clone() );
  }

  if ( !mCrs.isValid() )
    mCrs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );

  if ( mLayers.empty() )
    feedback->reportError( QObject::tr( "No layers selected" ), false );

  return true;
}

QVariantMap QgsExportLayersInformationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFields outFields;
  outFields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"source"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"crs"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"provider"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"file_path"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"layer_name"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"subset"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"abstract"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"attribution"_s, QMetaType::Type::QString ) );

  QString outputDest;
  std::unique_ptr<QgsFeatureSink> outputSink( parameterAsSink( parameters, u"OUTPUT"_s, context, outputDest, outFields, Qgis::WkbType::Polygon, mCrs ) );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );

  const double step = layers.size() > 0 ? 100.0 / layers.size() : 1;
  int i = 0;
  for ( const std::unique_ptr<QgsMapLayer> &layer : mLayers )
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
                 << parts[u"path"_s]
                 << parts[u"layerName"_s]
                 << parts[u"subset"_s];
    }
    else
    {
      attributes << QVariant() << QVariant() << QVariant() << QVariant();
    }
    attributes << layer->metadata().rights().join( ';' )
               << layer->serverProperties()->abstract();
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
      throw QgsProcessingException( writeFeatureError( outputSink.get(), parameters, u"OUTPUT"_s ) );
  }

  outputSink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputDest );
  return outputs;
}

///@endcond
