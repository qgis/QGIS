/***************************************************************************
                         qgsalgorithmextractbinary.cpp
                         -----------------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmextractbinary.h"
#include "qgsfeaturerequest.h"

///@cond PRIVATE

QString QgsExtractBinaryFieldAlgorithm::name() const
{
  return QStringLiteral( "extractbinary" );
}

QString QgsExtractBinaryFieldAlgorithm::displayName() const
{
  return QObject::tr( "Extract binary field" );
}

QString QgsExtractBinaryFieldAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts contents from a binary field, saving them to individual files.\n\n"
                      "Filenames can be generated using data defined values, allowing them to be taken from"
                      " an attribute in the source table or based on a more complex expression." );
}

QString QgsExtractBinaryFieldAlgorithm::shortDescription() const
{
  return QObject::tr( "This algorithm extracts contents from a binary field, saving them to individual files." );
}

QStringList QgsExtractBinaryFieldAlgorithm::tags() const
{
  return QObject::tr( "blob,binaries,save,file,contents,field,column" ).split( ',' );
}

QString QgsExtractBinaryFieldAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsExtractBinaryFieldAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QgsExtractBinaryFieldAlgorithm *QgsExtractBinaryFieldAlgorithm::createInstance() const
{
  return new QgsExtractBinaryFieldAlgorithm();
}

void QgsExtractBinaryFieldAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int>() << QgsProcessing::TypeVector ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Binary field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any ) );

  std::unique_ptr< QgsProcessingParameterString > fileNameParam = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "FILENAME" ), QObject::tr( "File name" ) );
  fileNameParam->setIsDynamic( true );
  fileNameParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Filename" ), QObject::tr( "File name" ), QgsPropertyDefinition::String ) );
  fileNameParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( fileNameParam.release() );

  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "FOLDER" ), QObject::tr( "Destination folder" ) ) );
}

QVariantMap QgsExtractBinaryFieldAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  int fieldIndex = input->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid binary field" ) );

  const QString folder = parameterAsString( parameters, QStringLiteral( "FOLDER" ), context );
  if ( !QFileInfo::exists( folder ) )
    throw QgsProcessingException( QObject::tr( "Destination folder %1 does not exist" ).arg( folder ) );

  QDir dir( folder );
  const QString filename = parameterAsString( parameters, QStringLiteral( "FILENAME" ), context );
  bool dynamicFilename = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "FILENAME" ) );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, input.get() );
  QgsProperty filenameProperty;

  QSet< QString > fields;
  fields.insert( fieldName );
  QgsFeatureRequest request;
  if ( dynamicFilename )
  {
    filenameProperty = parameters.value( QStringLiteral( "FILENAME" ) ).value< QgsProperty >();
    filenameProperty.prepare( expressionContext );
    fields.unite( filenameProperty.referencedFields() );
  }
  request.setSubsetOfAttributes( fields, input->fields() ).setFlags( QgsFeatureRequest::NoGeometry );
  QgsFeatureIterator features = input->getFeatures( request, QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  double step = input->featureCount() > 0 ? 100.0 / input->featureCount() : 1;
  int i = 0;
  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    QByteArray ba = feat.attribute( fieldIndex ).toByteArray();
    if ( ba.isEmpty() )
      continue;

    QString name = filename;
    if ( dynamicFilename )
    {
      expressionContext.setFeature( feat );
      name = filenameProperty.valueAsString( expressionContext, name );
    }

    const QString path = dir.filePath( name );
    QFile file( path );
    if ( !file.open( QIODevice::WriteOnly | QFile::Truncate ) )
    {
      feedback->reportError( QObject::tr( "Could not open %1 for writing" ).arg( path ) );
    }
    else
    {
      file.write( ba );
      file.close();
      feedback->pushInfo( QObject::tr( "Extracted %1" ).arg( path ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "FOLDER" ), folder );
  return outputs;
}


///@endcond
