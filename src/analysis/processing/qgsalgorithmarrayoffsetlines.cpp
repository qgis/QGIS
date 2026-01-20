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
  return u"arrayoffsetlines"_s;
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
  return u"vectorcreation"_s;
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

Qgis::ProcessingAlgorithmDocumentationFlags QgsCreateArrayOffsetLinesAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsCreateArrayOffsetLinesAlgorithm *QgsCreateArrayOffsetLinesAlgorithm::createInstance() const
{
  return new QgsCreateArrayOffsetLinesAlgorithm();
}

void QgsCreateArrayOffsetLinesAlgorithm::initParameters( const QVariantMap & )
{
  auto count = std::make_unique<QgsProcessingParameterNumber>( u"COUNT"_s, QObject::tr( "Number of features to create" ), Qgis::ProcessingNumberParameterType::Integer, 10, false, 1 );
  count->setIsDynamic( true );
  count->setDynamicPropertyDefinition( QgsPropertyDefinition( u"COUNT"_s, QObject::tr( "Number of features to create" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  count->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( count.release() );

  auto offset = std::make_unique<QgsProcessingParameterDistance>( u"OFFSET"_s, QObject::tr( "Offset step distance" ), 1.0, u"INPUT"_s );
  offset->setIsDynamic( true );
  offset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"OFFSET"_s, QObject::tr( "Step distance" ), QgsPropertyDefinition::Double ) );
  offset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( offset.release() );

  auto segmentParam = std::make_unique<QgsProcessingParameterNumber>( u"SEGMENTS"_s, QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1 );
  segmentParam->setFlags( segmentParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( segmentParam.release() );

  auto joinStyleParam = std::make_unique<QgsProcessingParameterEnum>( u"JOIN_STYLE"_s, QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 );
  joinStyleParam->setFlags( joinStyleParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( joinStyleParam.release() );

  auto miterLimitParam = std::make_unique<QgsProcessingParameterNumber>( u"MITER_LIMIT"_s, QObject::tr( "Miter limit" ), Qgis::ProcessingNumberParameterType::Double, 2, false, 1 );
  miterLimitParam->setFlags( miterLimitParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( miterLimitParam.release() );
}

QList<int> QgsCreateArrayOffsetLinesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

bool QgsCreateArrayOffsetLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCount = parameterAsInt( parameters, u"COUNT"_s, context );
  mDynamicCount = QgsProcessingParameters::isDynamic( parameters, u"COUNT"_s );
  if ( mDynamicCount )
    mCountProperty = parameters.value( u"COUNT"_s ).value<QgsProperty>();

  mOffsetStep = parameterAsDouble( parameters, u"OFFSET"_s, context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, u"OFFSET"_s );
  if ( mDynamicOffset )
    mOffsetStepProperty = parameters.value( u"OFFSET"_s ).value<QgsProperty>();

  mSegments = parameterAsInt( parameters, u"SEGMENTS"_s, context );
  mJoinStyle = static_cast<Qgis::JoinStyle>( 1 + parameterAsInt( parameters, u"JOIN_STYLE"_s, context ) );
  mMiterLimit = parameterAsDouble( parameters, u"MITER_LIMIT"_s, context );

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
  QgsFields newFields;
  newFields.append( QgsField( u"instance"_s, QMetaType::Type::Int ) );
  newFields.append( QgsField( u"offset"_s, QMetaType::Type::Double ) );
  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

QgsFeatureSink::SinkFlags QgsCreateArrayOffsetLinesAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

///@endcond
