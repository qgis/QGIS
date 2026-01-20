/***************************************************************************
                         qgsalgorithmsinglesidedbuffer.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmsinglesidedbuffer.h"

#include "qgsprocessing.h"

///@cond PRIVATE

QString QgsSingleSidedBufferAlgorithm::name() const
{
  return u"singlesidedbuffer"_s;
}

QString QgsSingleSidedBufferAlgorithm::displayName() const
{
  return QObject::tr( "Single sided buffer" );
}

QStringList QgsSingleSidedBufferAlgorithm::tags() const
{
  return QObject::tr( "rectangle,perpendicular,right,angles,square,quadrilateralise" ).split( ',' );
}

QString QgsSingleSidedBufferAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSingleSidedBufferAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsSingleSidedBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm buffers lines by a specified distance on one "
                      "side of the line only.\n\nThe segments parameter controls "
                      "the number of line segments to use to approximate a quarter "
                      "circle when creating rounded buffers. The join style parameter "
                      "specifies whether round, miter or beveled joins should be used "
                      "when buffering corners in a line. The miter limit parameter is "
                      "only applicable for miter join styles, and controls the maximum "
                      "distance from the buffer to use when creating a mitered join." );
}

QString QgsSingleSidedBufferAlgorithm::shortDescription() const
{
  return QObject::tr( "Buffers lines by a specified distance on one side of the line only." );
}

QString QgsSingleSidedBufferAlgorithm::outputName() const
{
  return QObject::tr( "Buffered" );
}

QList<int> QgsSingleSidedBufferAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

Qgis::ProcessingSourceType QgsSingleSidedBufferAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsSingleSidedBufferAlgorithm::outputWkbType( Qgis::WkbType type ) const
{
  Q_UNUSED( type );
  return Qgis::WkbType::Polygon;
}

QgsSingleSidedBufferAlgorithm *QgsSingleSidedBufferAlgorithm::createInstance() const
{
  return new QgsSingleSidedBufferAlgorithm();
}

void QgsSingleSidedBufferAlgorithm::initParameters( const QVariantMap & )
{
  auto bufferParam = std::make_unique<QgsProcessingParameterDistance>( u"DISTANCE"_s, QObject::tr( "Distance" ), 10, u"INPUT"_s );
  bufferParam->setIsDynamic( true );
  bufferParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Distance"_s, QObject::tr( "Buffer distance" ), QgsPropertyDefinition::Double ) );
  bufferParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( bufferParam.release() );

  addParameter( new QgsProcessingParameterEnum( u"SIDE"_s, QObject::tr( "Side" ), QStringList() << QObject::tr( "Left" ) << QObject::tr( "Right" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"SEGMENTS"_s, QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1 ) );
  addParameter( new QgsProcessingParameterEnum( u"JOIN_STYLE"_s, QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( u"MITER_LIMIT"_s, QObject::tr( "Miter limit" ), Qgis::ProcessingNumberParameterType::Double, 2, false, 1 ) );
}

bool QgsSingleSidedBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDistance = parameterAsDouble( parameters, u"DISTANCE"_s, context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, u"DISTANCE"_s );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( u"DISTANCE"_s ).value<QgsProperty>();

  mSide = static_cast<Qgis::BufferSide>( parameterAsInt( parameters, u"SIDE"_s, context ) );
  mSegments = parameterAsInt( parameters, u"SEGMENTS"_s, context );
  mJoinStyle = static_cast<Qgis::JoinStyle>( 1 + parameterAsInt( parameters, u"JOIN_STYLE"_s, context ) );
  mMiterLimit = parameterAsDouble( parameters, u"MITER_LIMIT"_s, context );

  return true;
}

QgsFeatureList QgsSingleSidedBufferAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;

  if ( f.hasGeometry() )
  {
    double distance = mDistance;
    if ( mDynamicDistance )
      distance = mDistanceProperty.valueAsDouble( context.expressionContext(), distance );

    const QgsGeometry outputGeometry = f.geometry().singleSidedBuffer( distance, mSegments, mSide, mJoinStyle, mMiterLimit );
    if ( outputGeometry.isNull() )
      throw QgsProcessingException( QObject::tr( "Error calculating single sided buffer" ) );

    f.setGeometry( outputGeometry );
  }

  return QgsFeatureList() << f;
}

///@endcond
