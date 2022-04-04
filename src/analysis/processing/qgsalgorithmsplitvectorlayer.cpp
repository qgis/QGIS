/***************************************************************************
                         qgsalgorithmsplitvectorlayer.cpp
                         ---------------------
    begin                : May 2020
    copyright            : (C) 2020 by Alexander Bruy
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

#include "qgsalgorithmsplitvectorlayer.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsSplitVectorLayerAlgorithm::name() const
{
  return QStringLiteral( "splitvectorlayer" );
}

QString QgsSplitVectorLayerAlgorithm::displayName() const
{
  return QObject::tr( "Split vector layer" );
}

QStringList QgsSplitVectorLayerAlgorithm::tags() const
{
  return QObject::tr( "vector,split,field,unique" ).split( ',' );
}

QString QgsSplitVectorLayerAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSplitVectorLayerAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsSplitVectorLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "Splits input vector layer into multiple layers by specified unique ID field." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Each of the layers created in the output folder contains all features from "
                        "the input layer with the same value for the specified attribute. The number "
                        "of files generated is equal to the number of different values found for the "
                        "specified attribute." );
}

QgsSplitVectorLayerAlgorithm *QgsSplitVectorLayerAlgorithm::createInstance() const
{
  return new QgsSplitVectorLayerAlgorithm();
}

void QgsSplitVectorLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID field" ),
                QVariant(), QStringLiteral( "INPUT" ) ) );

  const QStringList options = QgsVectorFileWriter::supportedFormatExtensions();
  auto fileTypeParam = std::make_unique < QgsProcessingParameterEnum >( QStringLiteral( "FILE_TYPE" ), QObject::tr( "Output file type" ), options, false, QVariantList() << 0, true );
  fileTypeParam->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( fileTypeParam.release() );

  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output directory" ) ) );
  addOutput( new QgsProcessingOutputMultipleLayers( QStringLiteral( "OUTPUT_LAYERS" ), QObject::tr( "Output layers" ) ) );
}

QVariantMap QgsSplitVectorLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  const QString outputDir = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFormat;
  if ( parameters.value( QStringLiteral( "FILE_TYPE" ) ).isValid() )
  {
    const int idx = parameterAsEnum( parameters, QStringLiteral( "FILE_TYPE" ), context );
    outputFormat = QgsVectorFileWriter::supportedFormatExtensions().at( idx );
  }
  else
  {
    outputFormat = context.preferredVectorFormat();
    if ( !QgsVectorFileWriter::supportedFormatExtensions().contains( outputFormat, Qt::CaseInsensitive ) )
      outputFormat = QStringLiteral( "gpkg" );
  }

  if ( !QDir().mkpath( outputDir ) )
    throw QgsProcessingException( QStringLiteral( "Failed to create output directory." ) );

  const QgsFields fields = source->fields();
  const QgsCoordinateReferenceSystem crs = source->sourceCrs();
  const QgsWkbTypes::Type geometryType = source->wkbType();
  const int fieldIndex = fields.lookupField( fieldName );
  const QSet< QVariant > uniqueValues = source->uniqueValues( fieldIndex );
  const QString baseName = outputDir + QDir::separator() + fieldName;

  int current = 0;
  const double step = uniqueValues.size() > 0 ? 100.0 / uniqueValues.size() : 1;

  int count = 0;
  QgsFeature feat;
  QStringList outputLayers;
  std::unique_ptr< QgsFeatureSink > sink;

  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    if ( feedback->isCanceled() )
      break;

    QString fileName;
    if ( ( *it ).isNull() )
    {
      fileName = QStringLiteral( "%1_NULL.%2" ).arg( baseName ).arg( outputFormat );
    }
    else if ( ( *it ).toString().isEmpty() )
    {
      fileName = QStringLiteral( "%1_EMPTY.%2" ).arg( baseName ).arg( outputFormat );
    }
    else
    {
      fileName = QStringLiteral( "%1_%2.%3" ).arg( baseName ).arg( ( *it ).toString() ).arg( outputFormat );
    }
    feedback->pushInfo( QObject::tr( "Creating layer: %1" ).arg( fileName ) );

    sink.reset( QgsProcessingUtils::createFeatureSink( fileName, context, fields, geometryType, crs ) );
    const QString expr = QgsExpression::createFieldEqualityExpression( fieldName, *it );
    QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setFilterExpression( expr ) );
    count = 0;
    while ( features.nextFeature( feat ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      count += 1;
    }

    feedback->pushInfo( QObject::tr( "Added %n feature(s) to layer", nullptr, count ) );
    outputLayers << fileName;

    current += 1;
    feedback->setProgress( current * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputDir );
  outputs.insert( QStringLiteral( "OUTPUT_LAYERS" ), outputLayers );
  return outputs;
}

///@endcond
