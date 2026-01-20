/***************************************************************************
                         qgsalgorithmjoinbyattribute.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmjoinbyattribute.h"

#include "qgsprocessingoutputs.h"

///@cond PRIVATE

QString QgsJoinByAttributeAlgorithm::name() const
{
  return u"joinattributestable"_s;
}

QString QgsJoinByAttributeAlgorithm::displayName() const
{
  return QObject::tr( "Join attributes by field value" );
}

QStringList QgsJoinByAttributeAlgorithm::tags() const
{
  return QObject::tr( "join,connect,attributes,values,fields,tables" ).split( ',' );
}

QString QgsJoinByAttributeAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsJoinByAttributeAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

void QgsJoinByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList methods;
  methods << QObject::tr( "Create separate feature for each matching feature (one-to-many)" )
          << QObject::tr( "Take attributes of the first matching feature only (one-to-one)" );

  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Table field" ), QVariant(), u"INPUT"_s ) );

  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT_2"_s, QObject::tr( "Input layer 2" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD_2"_s, QObject::tr( "Table field 2" ), QVariant(), u"INPUT_2"_s ) );

  addParameter( new QgsProcessingParameterField( u"FIELDS_TO_COPY"_s, QObject::tr( "Layer 2 fields to copy (leave empty to copy all fields)" ), QVariant(), u"INPUT_2"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Join type" ), methods, false, 1 ) );
  addParameter( new QgsProcessingParameterBoolean( u"DISCARD_NONMATCHING"_s, QObject::tr( "Discard records which could not be joined" ), false ) );

  addParameter( new QgsProcessingParameterString( u"PREFIX"_s, QObject::tr( "Joined field prefix" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Joined layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, true ) );

  auto nonMatchingSink = std::make_unique<QgsProcessingParameterFeatureSink>(
    u"NON_MATCHING"_s, QObject::tr( "Unjoinable features from first layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false
  );
  // TODO GUI doesn't support advanced outputs yet
  //nonMatchingSink->setFlags(nonMatchingSink->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( nonMatchingSink.release() );

  addOutput( new QgsProcessingOutputNumber( u"JOINED_COUNT"_s, QObject::tr( "Number of joined features from input table" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"UNJOINABLE_COUNT"_s, QObject::tr( "Number of unjoinable features from input table" ) ) );
}

QString QgsJoinByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes an input vector layer and creates a new vector layer that is an extended version of the "
                      "input one, with additional attributes in its attribute table.\n\n"
                      "The additional attributes and their values are taken from a second vector layer. An attribute is selected "
                      "in each of them to define the join criteria." );
}

QString QgsJoinByAttributeAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer that is an extended version of the input one, "
                      "with additional attributes taken from a second vector layer." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsJoinByAttributeAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsJoinByAttributeAlgorithm *QgsJoinByAttributeAlgorithm::createInstance() const
{
  return new QgsJoinByAttributeAlgorithm();
}

QVariantMap QgsJoinByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int joinMethod = parameterAsEnum( parameters, u"METHOD"_s, context );
  const bool discardNonMatching = parameterAsBoolean( parameters, u"DISCARD_NONMATCHING"_s, context );

  std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> input2( parameterAsSource( parameters, u"INPUT_2"_s, context ) );
  if ( !input2 )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT_2"_s ) );

  const QString prefix = parameterAsString( parameters, u"PREFIX"_s, context );

  const QString field1Name = parameterAsString( parameters, u"FIELD"_s, context );
  const QString field2Name = parameterAsString( parameters, u"FIELD_2"_s, context );
  const QStringList fieldsToCopy = parameterAsStrings( parameters, u"FIELDS_TO_COPY"_s, context );

  const int joinField1Index = input->fields().lookupField( field1Name );
  if ( joinField1Index < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid join field from layer 1: “%1” does not exist" ).arg( field1Name ) );

  const int joinField2Index = input2->fields().lookupField( field2Name );
  if ( joinField2Index < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid join field from layer 2: “%1” does not exist" ).arg( field2Name ) );

  QgsFields outFields2;
  QgsAttributeList fields2Indices;
  if ( fieldsToCopy.empty() )
  {
    outFields2 = input2->fields();
    fields2Indices.reserve( outFields2.count() );
    for ( int i = 0; i < outFields2.count(); ++i )
    {
      fields2Indices << i;
    }
  }
  else
  {
    fields2Indices.reserve( fieldsToCopy.count() );
    for ( const QString &field : fieldsToCopy )
    {
      const int index = input2->fields().lookupField( field );
      if ( index >= 0 )
      {
        fields2Indices << index;
        outFields2.append( input2->fields().at( index ) );
      }
    }
  }

  if ( !prefix.isEmpty() )
  {
    for ( int i = 0; i < outFields2.count(); ++i )
    {
      outFields2.rename( i, prefix + outFields2[i].name() );
    }
  }

  QgsAttributeList fields2Fetch = fields2Indices;
  fields2Fetch << joinField2Index;

  const QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), outFields2 );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outFields, input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( u"OUTPUT"_s ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QString destNonMatching1;
  std::unique_ptr<QgsFeatureSink> sinkNonMatching1( parameterAsSink( parameters, u"NON_MATCHING"_s, context, destNonMatching1, input->fields(), input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( u"NON_MATCHING"_s ).isValid() && !sinkNonMatching1 )
    throw QgsProcessingException( invalidSinkError( parameters, u"NON_MATCHING"_s ) );

  // cache attributes of input2
  QMultiHash<QVariant, QgsAttributes> input2AttributeCache;
  QgsFeatureIterator features = input2->getFeatures( QgsFeatureRequest().setFlags( Qgis::FeatureRequestFlag::NoGeometry ).setSubsetOfAttributes( fields2Fetch ), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  double step = input2->featureCount() > 0 ? 50.0 / input2->featureCount() : 1;
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

    if ( joinMethod == 1 && input2AttributeCache.contains( feat.attribute( joinField2Index ) ) )
      continue;

    // only keep selected attributes
    QgsAttributes attributes;
    for ( int field2Index : fields2Indices )
    {
      attributes << feat.attribute( field2Index );
    }

    input2AttributeCache.insert( feat.attribute( joinField2Index ), attributes );
  }

  // Create output vector layer with additional attribute
  step = input->featureCount() > 0 ? 50.0 / input->featureCount() : 1;
  features = input->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  i = 0;
  long long joinedCount = 0;
  long long unjoinedCount = 0;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );

    if ( input2AttributeCache.count( feat.attribute( joinField1Index ) ) > 0 )
    {
      joinedCount++;
      if ( sink )
      {
        const QgsAttributes attrs = feat.attributes();

        QList<QgsAttributes> attributes = input2AttributeCache.values( feat.attribute( joinField1Index ) );
        QList<QgsAttributes>::iterator attrsIt = attributes.begin();
        for ( ; attrsIt != attributes.end(); ++attrsIt )
        {
          QgsAttributes newAttrs = attrs;
          newAttrs.append( *attrsIt );
          feat.setAttributes( newAttrs );
          if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
      }
    }
    else
    {
      // no matching for input feature
      if ( sink && !discardNonMatching )
      {
        if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
      if ( sinkNonMatching1 )
      {
        if ( !sinkNonMatching1->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sinkNonMatching1.get(), parameters, u"NON_MATCHING"_s ) );
      }
      unjoinedCount++;
    }
  }

  feedback->pushInfo( QObject::tr( "%n feature(s) from input layer were successfully matched", nullptr, joinedCount ) );
  if ( unjoinedCount > 0 )
    feedback->reportError( QObject::tr( "%n feature(s) from input layer could not be matched", nullptr, unjoinedCount ) );

  QVariantMap outputs;
  if ( sink )
  {
    sink->finalize();
    outputs.insert( u"OUTPUT"_s, dest );
  }
  outputs.insert( u"JOINED_COUNT"_s, joinedCount );
  outputs.insert( u"UNJOINABLE_COUNT"_s, unjoinedCount );
  if ( sinkNonMatching1 )
  {
    sinkNonMatching1->finalize();
    outputs.insert( u"NON_MATCHING"_s, destNonMatching1 );
  }
  return outputs;
}


///@endcond
