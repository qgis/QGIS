/***************************************************************************
                         qgsalgorithmconverttocurves.cpp
                         ---------------------
    begin                : March 2018
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

#include "qgsalgorithmconverttocurves.h"

///@cond PRIVATE

QString QgsConvertToCurvesAlgorithm::name() const
{
  return QStringLiteral( "converttocurves" );
}

QString QgsConvertToCurvesAlgorithm::displayName() const
{
  return QObject::tr( "Convert to curved geometries" );
}

QStringList QgsConvertToCurvesAlgorithm::tags() const
{
  return QObject::tr( "straight,segmentize,curves,curved,circular" ).split( ',' );
}

QString QgsConvertToCurvesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsConvertToCurvesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsConvertToCurvesAlgorithm::outputName() const
{
  return QObject::tr( "Curves" );
}

QString QgsConvertToCurvesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm converts a geometry into its curved geometry equivalent.\n\n"
                      "Already curved geometries will be retained without change." );
}

QgsConvertToCurvesAlgorithm *QgsConvertToCurvesAlgorithm::createInstance() const
{
  return new QgsConvertToCurvesAlgorithm();
}

QList<int> QgsConvertToCurvesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsConvertToCurvesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > tolerance = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DISTANCE" ),
      QObject::tr( "Maximum distance tolerance" ), QgsProcessingParameterNumber::Double,
      0.000001, false, 0, 10000000.0 );
  tolerance->setIsDynamic( true );
  tolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Maximum distance tolerance" ), QgsPropertyDefinition::DoublePositive ) );
  tolerance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( tolerance.release() );

  std::unique_ptr< QgsProcessingParameterNumber > angleTolerance = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "ANGLE" ),
      QObject::tr( "Maximum angle tolerance" ), QgsProcessingParameterNumber::Double,
      0.000001, false, 0, 45.0 );
  angleTolerance->setIsDynamic( true );
  angleTolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "ANGLE" ), QObject::tr( "Maximum angle tolerance" ), QgsPropertyDefinition::DoublePositive ) );
  angleTolerance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( angleTolerance.release() );
}

bool QgsConvertToCurvesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();

  mAngleTolerance = parameterAsDouble( parameters, QStringLiteral( "ANGLE" ), context );
  mDynamicAngleTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ANGLE" ) );
  if ( mDynamicAngleTolerance )
    mAngleToleranceProperty = parameters.value( QStringLiteral( "ANGLE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsConvertToCurvesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();
    double tolerance = mTolerance;
    if ( mDynamicTolerance )
      tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), tolerance );
    double angleTolerance = mAngleTolerance;
    if ( mDynamicAngleTolerance )
      angleTolerance = mAngleToleranceProperty.valueAsDouble( context.expressionContext(), angleTolerance );

    f.setGeometry( geometry.convertToCurves( tolerance, angleTolerance * M_PI / 180.0 ) );
  }
  return QgsFeatureList() << f;
}

QgsWkbTypes::Type QgsConvertToCurvesAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  if ( QgsWkbTypes::isCurvedType( inputWkbType ) )
    return inputWkbType;

  QgsWkbTypes::Type outType = QgsWkbTypes::Unknown;
  switch ( QgsWkbTypes::geometryType( inputWkbType ) )
  {
    case QgsWkbTypes::PointGeometry:
    case QgsWkbTypes::NullGeometry:
    case QgsWkbTypes::UnknownGeometry:
      return inputWkbType;

    case QgsWkbTypes::LineGeometry:
      outType = QgsWkbTypes::CompoundCurve;
      break;

    case QgsWkbTypes::PolygonGeometry:
      outType = QgsWkbTypes::CurvePolygon;
      break;
  }

  if ( QgsWkbTypes::isMultiType( inputWkbType ) )
    outType = QgsWkbTypes::multiType( outType );

  if ( QgsWkbTypes::hasZ( inputWkbType ) )
    outType = QgsWkbTypes::addZ( outType );

  if ( QgsWkbTypes::hasM( inputWkbType ) )
    outType = QgsWkbTypes::addM( outType );

  return outType;
}


///@endcond


