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
                      "Filenames can be generated using values taken from "
                      "an attribute in the source table or based on a more complex expression." );
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
                QObject::tr( "Input layer" ), QList< int>() << static_cast< int >( Qgis::ProcessingSourceType::Vector ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Binary field" ), QVariant(),
                QStringLiteral( "INPUT" ), Qgis::ProcessingFieldParameterDataType::Any ) );

  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "FILENAME" ), QObject::tr( "File name" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFolderDestination( QStringLiteral( "FOLDER" ), QObject::tr( "Destination folder" ) ) );
}

QVariantMap QgsExtractBinaryFieldAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  const int fieldIndex = input->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid binary field" ) );

  const QString folder = parameterAsString( parameters, QStringLiteral( "FOLDER" ), context );
  if ( !QDir().mkpath( folder ) )
    throw QgsProcessingException( QObject::tr( "Failed to create output directory." ) );

  const QDir dir( folder );
  const QString filenameExpressionString = parameterAsString( parameters, QStringLiteral( "FILENAME" ), context );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, input.get() );

  QSet< QString > fields;
  fields.insert( fieldName );
  QgsFeatureRequest request;

  QgsExpression filenameExpression( filenameExpressionString );
  filenameExpression.prepare( &expressionContext );
  fields.unite( filenameExpression.referencedColumns() );
  request.setSubsetOfAttributes( fields, input->fields() );
  if ( !filenameExpression.needsGeometry() )
    request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );

  QgsFeatureIterator features = input->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  const double step = input->featureCount() > 0 ? 100.0 / input->featureCount() : 1;
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

    const QByteArray ba = feat.attribute( fieldIndex ).toByteArray();
    if ( ba.isEmpty() )
      continue;

    expressionContext.setFeature( feat );
    const QString name = filenameExpression.evaluate( &expressionContext ).toString();
    if ( filenameExpression.hasEvalError() )
    {
      feedback->reportError( QObject::tr( "Error evaluating filename: %1" ).arg( filenameExpression.evalErrorString() ) );
      continue;
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
