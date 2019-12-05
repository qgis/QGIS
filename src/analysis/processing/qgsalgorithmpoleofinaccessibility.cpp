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

QgsPoleOfInaccessibilityAlgorithm *QgsPoleOfInaccessibilityAlgorithm::createInstance() const
{
  return new QgsPoleOfInaccessibilityAlgorithm();
}

void QgsPoleOfInaccessibilityAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );

  auto toleranceParam = qgis::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), 1.0, QStringLiteral( "INPUT" ), 0.0 );
  toleranceParam->setIsDynamic( true );
  toleranceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Tolerance" ), QObject::tr( "Tolerance" ), QgsPropertyDefinition::Double ) );
  toleranceParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( toleranceParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Point" ), QgsProcessing::TypeVectorPoint ) );
}

QVariantMap QgsPoleOfInaccessibilityAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  double toleranceValue = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  bool dynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "TOLERANCE" ) );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, source.get() );
  QgsProperty toleranceProperty;
  if ( dynamicTolerance )
    toleranceProperty = parameters.value( QStringLiteral( "TOLERANCE" ) ).value< QgsProperty >();

  QgsFields fields = source->fields();
  fields.append( QgsField( QStringLiteral( "dist_pole" ), QVariant::Double ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Point, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;

  int i = 0;
  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature outFeature = f;
    if ( f.hasGeometry() )
    {
      double tolerance = toleranceValue;
      if ( dynamicTolerance )
      {
        expressionContext.setFeature( f );
        tolerance = toleranceProperty.valueAsDouble( expressionContext, toleranceValue );
      }

      double distance;
      QgsGeometry outputGeom = f.geometry().poleOfInaccessibility( tolerance, &distance );
      if ( outputGeom.isNull() )
      {
        throw QgsProcessingException( QObject::tr( "Error calculating pole of inaccessibility" ) );
      }
      QgsAttributes attrs = f.attributes();
      attrs.append( distance );
      outFeature.setAttributes( attrs );
      outFeature.setGeometry( outputGeom );
    }
    else
    {
      QgsAttributes attrs = f.attributes();
      attrs.append( QVariant() );
      outFeature.setAttributes( attrs );
    }

    sink->addFeature( outFeature, QgsFeatureSink::FastInsert );
    feedback->setProgress( i * step );
    i++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
