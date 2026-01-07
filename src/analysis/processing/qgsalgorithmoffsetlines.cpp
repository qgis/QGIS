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

#include "qgsapplication.h"

///@cond PRIVATE

QString QgsOffsetLinesAlgorithm::name() const
{
  return u"offsetline"_s;
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
  return u"vectorgeometry"_s;
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

QIcon QgsOffsetLinesAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmOffsetLines.svg"_s );
}

QString QgsOffsetLinesAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmOffsetLines.svg"_s );
}

QgsOffsetLinesAlgorithm *QgsOffsetLinesAlgorithm::createInstance() const
{
  return new QgsOffsetLinesAlgorithm();
}

void QgsOffsetLinesAlgorithm::initParameters( const QVariantMap & )
{
  auto offset = std::make_unique<QgsProcessingParameterDistance>( u"DISTANCE"_s, QObject::tr( "Distance" ), 10.0, u"INPUT"_s );
  offset->setIsDynamic( true );
  offset->setDynamicPropertyDefinition( QgsPropertyDefinition( u"DISTANCE"_s, QObject::tr( "Distance" ), QgsPropertyDefinition::Double ) );
  offset->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( offset.release() );

  auto segmentParam = std::make_unique<QgsProcessingParameterNumber>( u"SEGMENTS"_s, QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1 );
  addParameter( segmentParam.release() );

  auto joinStyleParam = std::make_unique<QgsProcessingParameterEnum>( u"JOIN_STYLE"_s, QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 );
  addParameter( joinStyleParam.release() );

  auto miterLimitParam = std::make_unique<QgsProcessingParameterNumber>( u"MITER_LIMIT"_s, QObject::tr( "Miter limit" ), Qgis::ProcessingNumberParameterType::Double, 2, false, 1 );
  addParameter( miterLimitParam.release() );
}

QList<int> QgsOffsetLinesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

Qgis::ProcessingSourceType QgsOffsetLinesAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorLine;
}

bool QgsOffsetLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mOffset = parameterAsDouble( parameters, u"DISTANCE"_s, context );
  mDynamicOffset = QgsProcessingParameters::isDynamic( parameters, u"DISTANCE"_s );
  if ( mDynamicOffset )
    mOffsetProperty = parameters.value( u"DISTANCE"_s ).value<QgsProperty>();

  mSegments = parameterAsInt( parameters, u"SEGMENTS"_s, context );
  mJoinStyle = static_cast<Qgis::JoinStyle>( 1 + parameterAsInt( parameters, u"JOIN_STYLE"_s, context ) );
  mMiterLimit = parameterAsDouble( parameters, u"MITER_LIMIT"_s, context );

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
