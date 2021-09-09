/***************************************************************************
                         qgsalgorithmpoleofinaccessibility.cpp
                         ---------------------
    begin                : December 2019
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

#include "qgsalgorithmpoleofinaccessibility.h"
#include "qgsapplication.h"

///@cond PRIVATE

QString QgsPoleOfInaccessibilityAlgorithm::name() const
{
  return QStringLiteral( "poleofinaccessibility" );
}

QString QgsPoleOfInaccessibilityAlgorithm::displayName() const
{
  return QObject::tr( "Pole of inaccessibility" );
}

QStringList QgsPoleOfInaccessibilityAlgorithm::tags() const
{
  return QObject::tr( "furthest,point,distant,extreme,maximum,centroid,center,centre" ).split( ',' );
}

QString QgsPoleOfInaccessibilityAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPoleOfInaccessibilityAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsPoleOfInaccessibilityAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the pole of inaccessibility for a polygon layer, which is the most "
                      "distant internal point from the boundary of the surface. This algorithm uses the 'polylabel' "
                      "algorithm (Vladimir Agafonkin, 2016), which is an iterative approach guaranteed to find the "
                      "true pole of inaccessibility within a specified tolerance (in layer units). More precise "
                      "tolerances require more iterations and will take longer to calculate." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The distance from the calculated pole to the polygon boundary will be stored as a new "
                        "attribute in the output layer." );
}

QString QgsPoleOfInaccessibilityAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCentroids.svg" ) );
}

QIcon QgsPoleOfInaccessibilityAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCentroids.svg" ) );
}

QString QgsPoleOfInaccessibilityAlgorithm::outputName() const
{
  return QObject::tr( "Point" );
}

QList<int> QgsPoleOfInaccessibilityAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsProcessing::SourceType QgsPoleOfInaccessibilityAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPoint;
}

QgsWkbTypes::Type QgsPoleOfInaccessibilityAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  Q_UNUSED( inputWkbType );

  return QgsWkbTypes::Point;
}

QgsFields QgsPoleOfInaccessibilityAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outputFields = inputFields;
  outputFields.append( QgsField( QStringLiteral( "dist_pole" ), QVariant::Double ) );

  return outputFields;
}

QgsPoleOfInaccessibilityAlgorithm *QgsPoleOfInaccessibilityAlgorithm::createInstance() const
{
  return new QgsPoleOfInaccessibilityAlgorithm();
}

void QgsPoleOfInaccessibilityAlgorithm::initParameters( const QVariantMap & )
{
  auto toleranceParam = std::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), 1.0, QStringLiteral( "INPUT" ), 0.0 );
  toleranceParam->setIsDynamic( true );
  toleranceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Tolerance" ), QObject::tr( "Tolerance" ), QgsPropertyDefinition::Double ) );
  toleranceParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( toleranceParam.release() );
}

bool QgsPoleOfInaccessibilityAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "TOLERANCE" ) );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( QStringLiteral( "TOLERANCE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsPoleOfInaccessibilityAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature outFeature = feature;
  if ( outFeature.hasGeometry() )
  {
    double tolerance = mTolerance;
    if ( mDynamicTolerance )
      tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), tolerance );

    double distance;
    const QgsGeometry outputGeom = outFeature.geometry().poleOfInaccessibility( tolerance, &distance );
    if ( outputGeom.isNull() )
    {
      throw QgsProcessingException( QObject::tr( "Error calculating pole of inaccessibility" ) );
    }
    QgsAttributes attrs = outFeature.attributes();
    attrs.append( distance );
    outFeature.setAttributes( attrs );
    outFeature.setGeometry( outputGeom );
  }
  else
  {
    QgsAttributes attrs = outFeature.attributes();
    attrs.append( QVariant() );
    outFeature.setAttributes( attrs );
  }

  return QgsFeatureList() << outFeature;
}

///@endcond
