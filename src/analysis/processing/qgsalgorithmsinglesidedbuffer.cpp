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
  return QStringLiteral( "singlesidedbuffer" );
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
  return QStringLiteral( "vectorgeometry" );
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

QString QgsSingleSidedBufferAlgorithm::outputName() const
{
  return QObject::tr( "Buffered" );
}

QList<int> QgsSingleSidedBufferAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsSingleSidedBufferAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsSingleSidedBufferAlgorithm::outputWkbType( QgsWkbTypes::Type type ) const
{
  Q_UNUSED( type );
  return QgsWkbTypes::Polygon;
}

QgsSingleSidedBufferAlgorithm *QgsSingleSidedBufferAlgorithm::createInstance() const
{
  return new QgsSingleSidedBufferAlgorithm();
}

void QgsSingleSidedBufferAlgorithm::initParameters( const QVariantMap & )
{
  auto bufferParam = std::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), 10, QStringLiteral( "INPUT" ) );
  bufferParam->setIsDynamic( true );
  bufferParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Distance" ), QObject::tr( "Buffer distance" ), QgsPropertyDefinition::Double ) );
  bufferParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( bufferParam.release() );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "SIDE" ), QObject::tr( "Side" ), QStringList() << QObject::tr( "Left" ) << QObject::tr( "Right" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 8, false, 1 ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MITER_LIMIT" ), QObject::tr( "Miter limit" ), QgsProcessingParameterNumber::Double, 2, false, 1 ) );
}

bool QgsSingleSidedBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();

  mSide = static_cast< Qgis::BufferSide>( parameterAsInt( parameters, QStringLiteral( "SIDE" ), context ) );
  mSegments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  mJoinStyle = static_cast< Qgis::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  mMiterLimit = parameterAsDouble( parameters, QStringLiteral( "MITER_LIMIT" ), context );

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

    const QgsGeometry outputGeometry =  f.geometry().singleSidedBuffer( distance, mSegments, mSide, mJoinStyle, mMiterLimit );
    if ( outputGeometry.isNull() )
      throw QgsProcessingException( QObject::tr( "Error calculating single sided buffer" ) );

    f.setGeometry( outputGeometry );
  }

  return QgsFeatureList() << f;
}

///@endcond
