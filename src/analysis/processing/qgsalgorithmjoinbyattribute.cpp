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

///@cond PRIVATE

QString QgsJoinByAttributeAlgorithm::name() const
{
  return QStringLiteral( "joinattributestable" );
}

QString QgsJoinByAttributeAlgorithm::displayName() const
{
  return QObject::tr( "Join attributes table" );
}

QStringList QgsJoinByAttributeAlgorithm::tags() const
{
  return QObject::tr( "join,connect,attributes,values,fields" ).split( ',' );
}

QString QgsJoinByAttributeAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsJoinByAttributeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QgsProcessingAlgorithm::Flags QgsJoinByAttributeAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsJoinByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
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

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Joined layer" ) ) );
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
  std::unique_ptr< QgsFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  std::unique_ptr< QgsFeatureSource > input2( parameterAsSource( parameters, QStringLiteral( "INPUT_2" ), context ) );
  if ( !input || !input2 )
    return QVariantMap();

  QString field1Name = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  QString field2Name = parameterAsString( parameters, QStringLiteral( "FIELD_2" ), context );
  const QStringList fieldsToCopy = parameterAsFields( parameters, QStringLiteral( "FIELDS_TO_COPY" ), context );

  int joinField1Index = input->fields().lookupField( field1Name );
  int joinField2Index = input2->fields().lookupField( field2Name );
  if ( joinField1Index < 0 || joinField2Index < 0 )
    return QVariantMap();

  QgsFields outFields2;
  QgsAttributeList fields2Indices;
  if ( fieldsToCopy.empty() )
  {
    outFields2 = input2->fields();
    for ( int i = 0; i < outFields2.count(); ++i )
    {
      fields2Indices << i;
    }
  }
  else
  {
    for ( const QString &field : fieldsToCopy )
    {
      int index = input2->fields().lookupField( field );
      if ( index >= 0 )
      {
        fields2Indices << index;
        outFields2.append( input2->fields().at( index ) );
      }
    }
  }

  QgsAttributeList fields2Fetch = fields2Indices;
  fields2Fetch << joinField2Index;

  QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), outFields2 );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields,
                                          input->wkbType(), input->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();


  // cache attributes of input2
  QHash< QVariant, QgsAttributes > input2AttributeCache;
  QgsFeatureIterator features = input2->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( fields2Fetch ) );
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

    if ( input2AttributeCache.contains( feat.attribute( joinField2Index ) ) )
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
  features = input->getFeatures();
  i = 0;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );

    QgsAttributes attrs = feat.attributes();
    attrs.append( input2AttributeCache.value( feat.attribute( joinField1Index ) ) );
    feat.setAttributes( attrs );
    sink->addFeature( feat, QgsFeatureSink::FastInsert );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


///@endcond
