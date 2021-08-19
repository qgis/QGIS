/***************************************************************************
                         qgsalgorithmsavefeatures.cpp
                         ---------------------
    begin                : July 2020
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

#include "qgsalgorithmsavefeatures.h"
#include "qgsvectorfilewriter.h"
#include <QRegularExpression>

///@cond PRIVATE

QString QgsSaveFeaturesAlgorithm::name() const
{
  return QStringLiteral( "savefeatures" );
}

QString QgsSaveFeaturesAlgorithm::displayName() const
{
  return QObject::tr( "Save vector features to file" );
}

QStringList QgsSaveFeaturesAlgorithm::tags() const
{
  return QObject::tr( "save,write,export" ).split( ',' );
}

QString QgsSaveFeaturesAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSaveFeaturesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsSaveFeaturesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm saves vector features to a specified file dataset.\n\n"
                      "For dataset formats supporting layers, an optional layer name parameter can be used to specify a custom string.\n\n"
                      "Optional GDAL-defined dataset and layer options can be specified. For more information on this, "
                      "read the online GDAL documentation." );
}

QgsSaveFeaturesAlgorithm *QgsSaveFeaturesAlgorithm::createInstance() const
{
  return new QgsSaveFeaturesAlgorithm();
}

void QgsSaveFeaturesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Vector features" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Saved features" ), QgsVectorFileWriter::fileFilterString(), QVariant(), false ) );

  std::unique_ptr< QgsProcessingParameterString > param = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "LAYER_NAME" ), QObject::tr( "Layer name" ), QVariant(), false, true );
  param->setFlags( param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( param.release() );
  param = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "DATASOURCE_OPTIONS" ), QObject::tr( "GDAL dataset options (separate individual options with semicolons)" ), QVariant(), false, true );
  param->setFlags( param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( param.release() );
  param = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "LAYER_OPTIONS" ), QObject::tr( "GDAL layer options (separate individual options with semicolons)" ), QVariant(), false, true );
  param->setFlags( param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( param.release() );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "FILE_PATH" ), QObject::tr( "File name and path" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "LAYER_NAME" ), QObject::tr( "Layer name" ) ) );
}

QVariantMap QgsSaveFeaturesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );

  QString layerName = parameterAsString( parameters, QStringLiteral( "LAYER_NAME" ), context ).trimmed();
  QVariantMap createOptions;
  if ( !layerName.isEmpty() )
  {
    createOptions[QStringLiteral( "layerName" )] = layerName;
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList datasourceOptions = parameterAsString( parameters, QStringLiteral( "DATASOURCE_OPTIONS" ), context ).trimmed().split( ';', QString::SkipEmptyParts );
  QStringList layerOptions = parameterAsString( parameters, QStringLiteral( "LAYER_OPTIONS" ), context ).trimmed().split( ';', QString::SkipEmptyParts );
#else
  const QStringList datasourceOptions = parameterAsString( parameters, QStringLiteral( "DATASOURCE_OPTIONS" ), context ).trimmed().split( ';', Qt::SkipEmptyParts );
  const QStringList layerOptions = parameterAsString( parameters, QStringLiteral( "LAYER_OPTIONS" ), context ).trimmed().split( ';', Qt::SkipEmptyParts );
#endif

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          source->wkbType(), source->sourceCrs(), QgsFeatureSink::SinkFlags(), createOptions, datasourceOptions, layerOptions ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  long long i = 0;

  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QString filePath = dest;
  layerName.clear(); // value of final layer name will be extracted from the destination string
  const int separatorIndex = dest.indexOf( '|' );
  if ( separatorIndex > -1 )
  {
    const QRegularExpression layerNameRx( QStringLiteral( "\\|layername=([^\\|]*)" ) );
    const QRegularExpressionMatch match = layerNameRx.match( dest );
    if ( match.hasMatch() )
    {
      layerName = match.captured( 1 );
    }
    filePath = dest.mid( 0, separatorIndex );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "FILE_PATH" ), filePath );
  outputs.insert( QStringLiteral( "LAYER_NAME" ), layerName );
  return outputs;
}

///@endcond
