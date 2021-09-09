/***************************************************************************
                         qgsalgorithmoffsetlines.cpp
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

#include "qgsalgorithmoffsetlines.h"

///@cond PRIVATE

QString QgsOffsetLinesAlgorithm::name() const
{
  return QStringLiteral( "offsetline" );
}

QString QgsOffsetLinesAlgorithm::displayName() const
{
  return QObject::tr( "Offset lines" );
}

QStringList QgsOffsetLinesAlgorithm::tags() const
{
  return QObject::tr( "offset,linestring" ).split( ',' );
}

QString QgsOffsetLinesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsOffsetLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsOffsetLinesAlgorithm::outputName() const
{
  return QObject::tr( "Offset" );
}

QString QgsOffsetLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm offsets lines by a specified distance. Positive distances will offset lines to the left, and negative distances "
                      "will offset to the right of lines.\n\n"
                      "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets.\n\n"
                      "The join style parameter specifies whether round, miter or beveled joins should be used when offsetting corners in a line.\n\n"
                      "The miter limit parameter is only applicable for miter join styles, and controls the maximum distance from the offset curve to "
                      "use when creating a mitered join." );
}

QString QgsOffsetLinesAlgorithm::shortDescription() const
{
  return QObject::tr( "Offsets lines by a specified distance." );
}

QgsOffsetLinesAlgorithm *QgsOffsetLinesAlgorithm::createInstance() const
{
  return new QgsOffsetLinesAlgorithm();
}

void QgsOffsetLinesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance > offset = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DISTANCE" ),
      QObject::tr( "Distance" ),
      10.0, QStringLiteral( "INPUT" ) );
  offset->setIsDynamic( true );
  offset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsPropertyDefinition::Double ) );
  offset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( offset.release() );

  auto segmentParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 8, false, 1 );
  addParameter( segmentParam.release() );

  auto joinStyleParam = std::make_unique< QgsProcessingParameterEnum>( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 );
  addParameter( joinStyleParam.release() );

  auto miterLimitParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MITER_LIMIT" ), QObject::tr( "Miter limit" ), QgsProcessingParameterNumber::Double, 2, false, 1 );
  addParameter( miterLimitParam.release() );
}

QList<int> QgsOffsetLinesAlgorithm::inputLayerTypes() const
{
  return QList< int >() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsOffsetLinesAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

bool QgsOffsetLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mOffset = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicOffset )
    mOffsetProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();

  mSegments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  mJoinStyle = static_cast< Qgis::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  mMiterLimit = parameterAsDouble( parameters, QStringLiteral( "MITER_LIMIT" ), context );

  return true;
}

QgsFeatureList QgsOffsetLinesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( feature.hasGeometry() )
  {
    QgsFeature f = feature;
    const QgsGeometry geometry = feature.geometry();

    double offset = mOffset;
    if ( mDynamicOffset )
      offset = mOffsetProperty.valueAsDouble( context.expressionContext(), offset );

    const QgsGeometry offsetGeometry = geometry.offsetCurve( offset, mSegments, mJoinStyle, mMiterLimit );
    f.setGeometry( offsetGeometry );
    return QgsFeatureList() << f;
  }
  else
  {
    return QgsFeatureList() << feature;
  }
}

///@endcond


