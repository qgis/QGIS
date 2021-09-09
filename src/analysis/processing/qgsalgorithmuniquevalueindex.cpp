/***************************************************************************
                         qgsalgorithmuniquevalueindex.cpp
                         ---------------------
    begin                : January 2018
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

#include "qgsalgorithmuniquevalueindex.h"

///@cond PRIVATE

QString QgsAddUniqueValueIndexAlgorithm::name() const
{
  return QStringLiteral( "adduniquevalueindexfield" );
}

QString QgsAddUniqueValueIndexAlgorithm::displayName() const
{
  return QObject::tr( "Add unique value index field" );
}

QStringList QgsAddUniqueValueIndexAlgorithm::tags() const
{
  return QObject::tr( "categorize,categories,category,reclassify,classes,create" ).split( ',' );
}

QString QgsAddUniqueValueIndexAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsAddUniqueValueIndexAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

void QgsAddUniqueValueIndexAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Class field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD_NAME" ),
                QObject::tr( "Output field name" ), QStringLiteral( "NUM_FIELD" ) ) );

  std::unique_ptr< QgsProcessingParameterFeatureSink > classedOutput = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer with index field" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true );
  classedOutput->setCreateByDefault( true );
  addParameter( classedOutput.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > summaryOutput = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "SUMMARY_OUTPUT" ),  QObject::tr( "Class summary" ),
      QgsProcessing::TypeVector, QVariant(), true );
  summaryOutput->setCreateByDefault( false );
  addParameter( summaryOutput.release() );
}

QString QgsAddUniqueValueIndexAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and an attribute and adds a new numeric field. Values in this field correspond to values in the specified attribute, so features with the same "
                      "value for the attribute will have the same value in the new numeric field. This creates a numeric equivalent of the specified attribute, which defines the same classes.\n\n"
                      "The new attribute is not added to the input layer but a new layer is generated instead.\n\n"
                      "Optionally, a separate table can be output which contains a summary of the class field values mapped to the new unique numeric value." );
}

QgsAddUniqueValueIndexAlgorithm *QgsAddUniqueValueIndexAlgorithm::createInstance() const
{
  return new QgsAddUniqueValueIndexAlgorithm();
}

QVariantMap QgsAddUniqueValueIndexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString newFieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  QgsFields fields = source->fields();
  const QgsField newField = QgsField( newFieldName, QVariant::Int );
  fields.append( newField );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, source->wkbType(), source->sourceCrs() ) );

  const QString sourceFieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  const int fieldIndex = source->fields().lookupField( sourceFieldName );
  if ( fieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field name %1" ).arg( sourceFieldName ) );

  QString summaryDest;
  QgsFields summaryFields;
  summaryFields.append( newField );
  summaryFields.append( source->fields().at( fieldIndex ) );
  std::unique_ptr< QgsFeatureSink > summarySink( parameterAsSink( parameters, QStringLiteral( "SUMMARY_OUTPUT" ), context, summaryDest, summaryFields, QgsWkbTypes::NoGeometry ) );

  QHash< QVariant, int > classes;

  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );

  const long count = source->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsAttributes attributes = feature.attributes();
    const QVariant clazz = attributes.at( fieldIndex );

    int thisValue = classes.value( clazz, -1 );
    if ( thisValue == -1 )
    {
      thisValue = classes.count();
      classes.insert( clazz, thisValue );
    }

    if ( sink )
    {
      attributes.append( thisValue );
      feature.setAttributes( attributes );
      if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  if ( summarySink )
  {
    //generate summary table - first we make a sorted version of the classes
    QMap< int, QVariant > sorted;
    for ( auto classIt = classes.constBegin(); classIt != classes.constEnd(); ++classIt )
    {
      sorted.insert( classIt.value(), classIt.key() );
    }
    // now save them
    for ( auto sortedIt = sorted.constBegin(); sortedIt != sorted.constEnd(); ++sortedIt )
    {
      QgsFeature f;
      f.setAttributes( QgsAttributes() << sortedIt.key() << sortedIt.value() );
      if ( !summarySink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( summarySink.get(), parameters, QStringLiteral( "SUMMARY_OUTPUT" ) ) );
    }
  }

  QVariantMap results;
  if ( sink )
    results.insert( QStringLiteral( "OUTPUT" ), dest );
  if ( summarySink )
    results.insert( QStringLiteral( "SUMMARY_OUTPUT" ), summaryDest );
  return results;
}

///@endcond
