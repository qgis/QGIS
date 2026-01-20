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
  return u"poleofinaccessibility"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsPoleOfInaccessibilityAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the pole of inaccessibility for a polygon layer, which is the most "
                      "distant internal point from the boundary of the surface. This algorithm uses the 'polylabel' "
                      "algorithm (Vladimir Agafonkin, 2016), which is an iterative approach guaranteed to find the "
                      "true pole of inaccessibility within a specified tolerance (in layer units). More precise "
                      "tolerances require more iterations and will take longer to calculate." )
         + u"\n\n"_s
         + QObject::tr( "The distance from the calculated pole to the polygon boundary will be stored as a new "
                        "attribute in the output layer." );
}

QString QgsPoleOfInaccessibilityAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a point layer with features representing the most "
                      "distant internal point from the boundary of the surface for a polygon layer." );
}

QString QgsPoleOfInaccessibilityAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmCentroids.svg"_s );
}

QIcon QgsPoleOfInaccessibilityAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmCentroids.svg"_s );
}

QString QgsPoleOfInaccessibilityAlgorithm::outputName() const
{
  return QObject::tr( "Point" );
}

QList<int> QgsPoleOfInaccessibilityAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

Qgis::ProcessingSourceType QgsPoleOfInaccessibilityAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPoint;
}

Qgis::WkbType QgsPoleOfInaccessibilityAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Q_UNUSED( inputWkbType );

  return Qgis::WkbType::Point;
}

QgsFields QgsPoleOfInaccessibilityAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields newFields;
  newFields.append( QgsField( u"dist_pole"_s, QMetaType::Type::Double ) );

  return QgsProcessingUtils::combineFields( inputFields, newFields );
}

QgsPoleOfInaccessibilityAlgorithm *QgsPoleOfInaccessibilityAlgorithm::createInstance() const
{
  return new QgsPoleOfInaccessibilityAlgorithm();
}

void QgsPoleOfInaccessibilityAlgorithm::initParameters( const QVariantMap & )
{
  auto toleranceParam = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE"_s, QObject::tr( "Tolerance" ), 1.0, u"INPUT"_s, 0.0 );
  toleranceParam->setIsDynamic( true );
  toleranceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Tolerance"_s, QObject::tr( "Tolerance" ), QgsPropertyDefinition::Double ) );
  toleranceParam->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( toleranceParam.release() );
}

bool QgsPoleOfInaccessibilityAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, u"TOLERANCE"_s );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( u"TOLERANCE"_s ).value<QgsProperty>();

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
