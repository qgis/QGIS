/***************************************************************************
                         qgsalgorithmarrayoffsetlines.cpp
                         ---------------------
    begin                : July 2018
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

#include "qgsalgorithmarrayoffsetlines.h"

///@cond PRIVATE

QString QgsCreateArrayOffsetLinesAlgorithm::name() const
{
  return QStringLiteral( "arrayoffsetlines" );
}

QString QgsCreateArrayOffsetLinesAlgorithm::displayName() const
{
  return QObject::tr( "Array of offset (parallel) lines" );
}

QStringList QgsCreateArrayOffsetLinesAlgorithm::tags() const
{
  return QObject::tr( "offset,parallel,duplicate,create,spaced,copy,features,objects,step,repeat" ).split( ',' );
}

QString QgsCreateArrayOffsetLinesAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsCreateArrayOffsetLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

QString QgsCreateArrayOffsetLinesAlgorithm::outputName() const
{
  return QObject::tr( "Offset lines" );
}

QString QgsCreateArrayOffsetLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates copies of line features in a layer, by creating multiple offset versions of each feature. "
                      "Each copy is offset by a preset distance." );
}

QString QgsCreateArrayOffsetLinesAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates multiple offset copies of lines from a layer." );
}

QgsCreateArrayOffsetLinesAlgorithm *QgsCreateArrayOffsetLinesAlgorithm::createInstance() const
{
  return new QgsCreateArrayOffsetLinesAlgorithm();
}

void QgsCreateArrayOffsetLinesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > count = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "COUNT" ),
      QObject::tr( "Number of features to create" ), QgsProcessingParameterNumber::Integer,
      10, false, 1 );
  count->setIsDynamic( true );
  count->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "COUNT" ), QObject::tr( "Number of features to create" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  count->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( count.release() );

  std::unique_ptr< QgsProcessingParameterDistance > offset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "OFFSET" ),
      QObject::tr( "Offset step distance" ),
      1.0, QStringLiteral( "INPUT" ) );
  offset->setIsDynamic( true );
  offset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "OFFSET" ), QObject::tr( "Step distance" ), QgsPropertyDefinition::Double ) );
  offset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( offset.release() );

  auto segmentParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 8, false, 1 );
  segmentParam->setFlags( segmentParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( segmentParam.release() );

  auto joinStyleParam = std::make_unique< QgsProcessingParameterEnum>( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 );
  joinStyleParam->setFlags( joinStyleParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( joinStyleParam.release() );

  auto miterLimitParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MITER_LIMIT" ), QObject::tr( "Miter limit" ), QgsProcessingParameterNumber::Double, 2, false, 1 );
  miterLimitParam->setFlags( miterLimitParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( miterLimitParam.release() );
}

QList<int> QgsCreateArrayOffsetLinesAlgorithm::inputLayerTypes() const
{
  return QList< int >() << QgsProcessing::TypeVectorLine;
}

bool QgsCreateArrayOffsetLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCount = parameterAsInt( parameters, QStringLiteral( "COUNT" ), context );
  mDynamicCount = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "COUNT" ) );
  if ( mDynamicCount )
    mCountProperty = parameters.value( QStringLiteral( "COUNT" ) ).value< QgsProperty >();

  mOffsetStep = parameterAsDouble( parameters, QStringLiteral( "OFFSET" ), context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "OFFSET" ) );
  if ( mDynamicOffset )
    mOffsetStepProperty = parameters.value( QStringLiteral( "OFFSET" ) ).value< QgsProperty >();

  mSegments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  mJoinStyle = static_cast< Qgis::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  mMiterLimit = parameterAsDouble( parameters, QStringLiteral( "MITER_LIMIT" ), context );

  return true;
}

QgsFeatureList QgsCreateArrayOffsetLinesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeatureList result = QgsFeatureList();

  if ( feature.hasGeometry() )
  {
    const QgsGeometry geometry = feature.geometry();
    const QgsAttributes attr = feature.attributes();

    int count = mCount;
    if ( mDynamicCount )
      count = mCountProperty.valueAsInt( context.expressionContext(), count );
    result.reserve( count + 1 );

    double offsetStep = mOffsetStep;
    if ( mDynamicOffset )
      offsetStep = mOffsetStepProperty.valueAsDouble( context.expressionContext(), offsetStep );

    {
      QgsFeature original = feature;
      original.setGeometry( geometry );
      QgsAttributes outAttrs = attr;
      outAttrs << QVariant( 0 ) << QVariant( 0 );
      original.setAttributes( outAttrs );
      result << original;
    }

    double offset = 0;
    for ( int i = 0; i < count; ++i )
    {
      offset += offsetStep;
      // FIXME: shouldn't we use QgsVectorLayerUtils::createFeature?
      QgsFeature offsetFeature = feature;
      const QgsGeometry offsetGeometry = geometry.offsetCurve( offset, mSegments, mJoinStyle, mMiterLimit );
      offsetFeature.setGeometry( offsetGeometry );
      QgsAttributes outAttrs = attr;
      outAttrs << QVariant( i + 1 ) << QVariant( offset );
      offsetFeature.setAttributes( outAttrs );
      result << offsetFeature;
    }
  }
  else
  {
    result << feature;
  }

  return result;
}

QgsFields QgsCreateArrayOffsetLinesAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields output = inputFields;
  output.append( QgsField( QStringLiteral( "instance" ), QVariant::Int ) );
  output.append( QgsField( QStringLiteral( "offset" ), QVariant::Double ) );
  return output;
}

QgsFeatureSink::SinkFlags QgsCreateArrayOffsetLinesAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

///@endcond


