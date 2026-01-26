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
  return u"adduniquevalueindexfield"_s;
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
  return u"vectortable"_s;
}

void QgsAddUniqueValueIndexAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Class field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any ) );
  addParameter( new QgsProcessingParameterString( u"FIELD_NAME"_s, QObject::tr( "Output field name" ), u"NUM_FIELD"_s ) );

  auto classedOutput = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Layer with index field" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true );
  classedOutput->setCreateByDefault( true );
  addParameter( classedOutput.release() );

  auto summaryOutput = std::make_unique<QgsProcessingParameterFeatureSink>( u"SUMMARY_OUTPUT"_s, QObject::tr( "Class summary" ), Qgis::ProcessingSourceType::Vector, QVariant(), true );
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

QString QgsAddUniqueValueIndexAlgorithm::shortDescription() const
{
  return QObject::tr( "Adds a numeric field and assigns the same index to features of the same attribute value." );
}

QgsAddUniqueValueIndexAlgorithm *QgsAddUniqueValueIndexAlgorithm::createInstance() const
{
  return new QgsAddUniqueValueIndexAlgorithm();
}

QVariantMap QgsAddUniqueValueIndexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString newFieldName = parameterAsString( parameters, u"FIELD_NAME"_s, context );
  QgsFields fields = source->fields();

  if ( fields.lookupField( newFieldName ) >= 0 )
  {
    throw QgsProcessingException( QObject::tr( "A field with the same name (%1) already exists" ).arg( newFieldName ) );
  }

  const QgsField newField = QgsField( newFieldName, QMetaType::Type::Int );
  fields.append( newField );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, source->wkbType(), source->sourceCrs() ) );

  const QString sourceFieldName = parameterAsString( parameters, u"FIELD"_s, context );
  const int fieldIndex = source->fields().lookupField( sourceFieldName );
  if ( fieldIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field name %1" ).arg( sourceFieldName ) );

  QString summaryDest;
  QgsFields summaryFields;
  summaryFields.append( newField );
  summaryFields.append( source->fields().at( fieldIndex ) );
  std::unique_ptr<QgsFeatureSink> summarySink( parameterAsSink( parameters, u"SUMMARY_OUTPUT"_s, context, summaryDest, summaryFields, Qgis::WkbType::NoGeometry ) );

  QHash<QVariant, int> classes;

  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );

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
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  if ( summarySink )
  {
    //generate summary table - first we make a sorted version of the classes
    QMap<int, QVariant> sorted;
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
        throw QgsProcessingException( writeFeatureError( summarySink.get(), parameters, u"SUMMARY_OUTPUT"_s ) );
    }
  }

  QVariantMap results;
  if ( sink )
  {
    sink->finalize();
    results.insert( u"OUTPUT"_s, dest );
  }
  if ( summarySink )
  {
    summarySink->finalize();
    results.insert( u"SUMMARY_OUTPUT"_s, summaryDest );
  }
  return results;
}

///@endcond
