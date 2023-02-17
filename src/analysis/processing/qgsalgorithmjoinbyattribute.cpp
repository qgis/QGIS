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
  return QStringLiteral( "joinattributestable" );
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
  return QStringLiteral( "vectorgeneral" );
}

void QgsJoinByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList methods;
  methods << QObject::tr( "Create separate feature for each matching feature (one-to-many)" )
          << QObject::tr( "Take attributes of the first matching feature only (one-to-one)" );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int>() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ),
                QObject::tr( "Table field" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_2" ),
                QObject::tr( "Input layer 2" ), QList< int>() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD_2" ),
                QObject::tr( "Table field 2" ), QVariant(), QStringLiteral( "INPUT_2" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELDS_TO_COPY" ),
                QObject::tr( "Layer 2 fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "INPUT_2" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Join type" ),
                methods, false, 1 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISCARD_NONMATCHING" ),
                QObject::tr( "Discard records which could not be joined" ),
                false ) );

  addParameter( new QgsProcessingParameterString( QStringLiteral( "PREFIX" ),
                QObject::tr( "Joined field prefix" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Joined layer" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, true ) );

  std::unique_ptr< QgsProcessingParameterFeatureSink > nonMatchingSink = std::make_unique< QgsProcessingParameterFeatureSink >(
        QStringLiteral( "NON_MATCHING" ), QObject::tr( "Unjoinable features from first layer" ), QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, false );
  // TODO GUI doesn't support advanced outputs yet
  //nonMatchingSink->setFlags(nonMatchingSink->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( nonMatchingSink.release() );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "JOINED_COUNT" ), QObject::tr( "Number of joined features from input table" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "UNJOINABLE_COUNT" ), QObject::tr( "Number of unjoinable features from input table" ) ) );
}

QString QgsJoinByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes an input vector layer and creates a new vector layer that is an extended version of the "
                      "input one, with additional attributes in its attribute table.\n\n"
                      "The additional attributes and their values are taken from a second vector layer. An attribute is selected "
                      "in each of them to define the join criteria." );
}

QgsJoinByAttributeAlgorithm *QgsJoinByAttributeAlgorithm::createInstance() const
{
  return new QgsJoinByAttributeAlgorithm();
}

QVariantMap QgsJoinByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const int joinMethod = parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context );
  const bool discardNonMatching = parameterAsBoolean( parameters, QStringLiteral( "DISCARD_NONMATCHING" ), context );

  std::unique_ptr< QgsProcessingFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsProcessingFeatureSource > input2( parameterAsSource( parameters, QStringLiteral( "INPUT_2" ), context ) );
  if ( !input2 )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_2" ) ) );

  const QString prefix = parameterAsString( parameters, QStringLiteral( "PREFIX" ), context );

  const QString field1Name = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  const QString field2Name = parameterAsString( parameters, QStringLiteral( "FIELD_2" ), context );
  const QStringList fieldsToCopy = parameterAsFields( parameters, QStringLiteral( "FIELDS_TO_COPY" ), context );

  const int joinField1Index = input->fields().lookupField( field1Name );
  const int joinField2Index = input2->fields().lookupField( field2Name );
  if ( joinField1Index < 0 || joinField2Index < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid join fields" ) );

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
      outFields2.rename( i, prefix + outFields2[ i ].name() );
    }
  }

  QgsAttributeList fields2Fetch = fields2Indices;
  fields2Fetch << joinField2Index;

  const QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), outFields2 );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields,
                                          input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "OUTPUT" ) ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString destNonMatching1;
  std::unique_ptr< QgsFeatureSink > sinkNonMatching1( parameterAsSink( parameters, QStringLiteral( "NON_MATCHING" ), context, destNonMatching1, input->fields(),
      input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "NON_MATCHING" ) ).isValid() && !sinkNonMatching1 )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NON_MATCHING" ) ) );

  // cache attributes of input2
  QMultiHash< QVariant, QgsAttributes > input2AttributeCache;
  QgsFeatureIterator features = input2->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( fields2Fetch ), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
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
    for ( int j = 0; j < feat.attributes().count(); ++j )
    {
      if ( ! fields2Indices.contains( j ) )
        continue;
      attributes << feat.attribute( j );
    }

    input2AttributeCache.insert( feat.attribute( joinField2Index ), attributes );
  }

  // Create output vector layer with additional attribute
  step = input->featureCount() > 0 ? 50.0 / input->featureCount() : 1;
  features = input->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
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

        QList< QgsAttributes > attributes = input2AttributeCache.values( feat.attribute( joinField1Index ) );
        QList< QgsAttributes >::iterator attrsIt = attributes.begin();
        for ( ; attrsIt != attributes.end(); ++attrsIt )
        {
          QgsAttributes newAttrs = attrs;
          newAttrs.append( *attrsIt );
          feat.setAttributes( newAttrs );
          if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        }
      }
    }
    else
    {
      // no matching for input feature
      if ( sink && !discardNonMatching )
      {
        if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
      if ( sinkNonMatching1 )
      {
        if ( !sinkNonMatching1->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sinkNonMatching1.get(), parameters, QStringLiteral( "NON_MATCHING" ) ) );
      }
      unjoinedCount++;
    }
  }

  feedback->pushInfo( QObject::tr( "%n feature(s) from input layer were successfully matched", nullptr, joinedCount ) );
  if ( unjoinedCount > 0 )
    feedback->reportError( QObject::tr( "%n feature(s) from input layer could not be matched", nullptr, unjoinedCount ) );

  QVariantMap outputs;
  if ( sink )
    outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "JOINED_COUNT" ), joinedCount );
  outputs.insert( QStringLiteral( "UNJOINABLE_COUNT" ), unjoinedCount );
  if ( sinkNonMatching1 )
    outputs.insert( QStringLiteral( "NON_MATCHING" ), destNonMatching1 );
  return outputs;
}


///@endcond
