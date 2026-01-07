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

#include "qgsfileutils.h"
#include "qgsvariantutils.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsSplitVectorLayerAlgorithm::name() const
{
  return u"splitvectorlayer"_s;
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
  return u"vectorgeneral"_s;
}

QString QgsSplitVectorLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits input vector layer into multiple layers by specified unique ID field." )
         + u"\n\n"_s
         + QObject::tr( "Each of the layers created in the output folder contains all features from "
                        "the input layer with the same value for the specified attribute. The number "
                        "of files generated is equal to the number of different values found for the "
                        "specified attribute." );
}

QString QgsSplitVectorLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Splits a vector layer into multiple layers, each containing "
                      "all the features with the same value for a specified attribute." );
}

QgsSplitVectorLayerAlgorithm *QgsSplitVectorLayerAlgorithm::createInstance() const
{
  return new QgsSplitVectorLayerAlgorithm();
}

void QgsSplitVectorLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Unique ID field" ), QVariant(), u"INPUT"_s ) );
  auto prefixFieldParam = std::make_unique<QgsProcessingParameterBoolean>( u"PREFIX_FIELD"_s, QObject::tr( "Add field prefix to file names" ), true );
  addParameter( prefixFieldParam.release() );

  const QStringList options = QgsVectorFileWriter::supportedFormatExtensions();
  auto fileTypeParam = std::make_unique<QgsProcessingParameterEnum>( u"FILE_TYPE"_s, QObject::tr( "Output file type" ), options, false, 0, true );
  fileTypeParam->setFlags( fileTypeParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( fileTypeParam.release() );

  addParameter( new QgsProcessingParameterFolderDestination( u"OUTPUT"_s, QObject::tr( "Output directory" ) ) );
  addOutput( new QgsProcessingOutputMultipleLayers( u"OUTPUT_LAYERS"_s, QObject::tr( "Output layers" ) ) );
}

QVariantMap QgsSplitVectorLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString fieldName = parameterAsString( parameters, u"FIELD"_s, context );
  const QString outputDir = parameterAsString( parameters, u"OUTPUT"_s, context );
  QString outputFormat;
  if ( parameters.value( u"FILE_TYPE"_s ).isValid() )
  {
    const int idx = parameterAsEnum( parameters, u"FILE_TYPE"_s, context );
    outputFormat = QgsVectorFileWriter::supportedFormatExtensions().at( idx );
  }
  else
  {
    outputFormat = context.preferredVectorFormat();
    if ( !QgsVectorFileWriter::supportedFormatExtensions().contains( outputFormat, Qt::CaseInsensitive ) )
      outputFormat = u"gpkg"_s;
  }

  if ( !QDir().mkpath( outputDir ) )
    throw QgsProcessingException( QObject::tr( "Failed to create output directory." ) );

  const QgsFields fields = source->fields();
  const QgsCoordinateReferenceSystem crs = source->sourceCrs();
  const Qgis::WkbType geometryType = source->wkbType();
  const int fieldIndex = fields.lookupField( fieldName );
  const QSet<QVariant> uniqueValues = source->uniqueValues( fieldIndex );
  QString baseName = outputDir + QDir::separator();

  if ( parameterAsBool( parameters, u"PREFIX_FIELD"_s, context ) )
  {
    baseName.append( fieldName + "_" );
  }

  int current = 0;
  const double step = uniqueValues.size() > 0 ? 100.0 / uniqueValues.size() : 1;

  int count = 0;
  QgsFeature feat;
  QStringList outputLayers;
  std::unique_ptr<QgsFeatureSink> sink;

  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    if ( feedback->isCanceled() )
      break;

    QString fileName;
    if ( QgsVariantUtils::isNull( *it ) )
    {
      fileName = u"%1NULL.%2"_s.arg( baseName ).arg( outputFormat );
    }
    else if ( ( *it ).toString().isEmpty() )
    {
      fileName = u"%1EMPTY.%2"_s.arg( baseName ).arg( outputFormat );
    }
    else
    {
      fileName = u"%1%2.%3"_s.arg( baseName ).arg( QgsFileUtils::stringToSafeFilename( ( *it ).toString() ) ).arg( outputFormat );
    }
    feedback->pushInfo( QObject::tr( "Creating layer: %1" ).arg( fileName ) );

    sink.reset( QgsProcessingUtils::createFeatureSink( fileName, context, fields, geometryType, crs ) );
    const QString expr = QgsExpression::createFieldEqualityExpression( fieldName, *it );
    QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setFilterExpression( expr ), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
    count = 0;
    while ( features.nextFeature( feat ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      count += 1;
    }
    sink->finalize();

    feedback->pushInfo( QObject::tr( "Added %n feature(s) to layer", nullptr, count ) );
    outputLayers << fileName;

    current += 1;
    feedback->setProgress( current * step );
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputDir );
  outputs.insert( u"OUTPUT_LAYERS"_s, outputLayers );
  return outputs;
}

///@endcond
