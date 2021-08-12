/***************************************************************************
                         qgsalgorithmremoveholes.cpp
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

#include "qgsalgorithmremoveholes.h"

///@cond PRIVATE

QString QgsRemoveHolesAlgorithm::name() const
{
  return QStringLiteral( "deleteholes" );
}

QString QgsRemoveHolesAlgorithm::displayName() const
{
  return QObject::tr( "Delete holes" );
}

QStringList QgsRemoveHolesAlgorithm::tags() const
{
  return QObject::tr( "remove,delete,drop,holes,rings,fill" ).split( ',' );
}

QString QgsRemoveHolesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRemoveHolesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsRemoveHolesAlgorithm::outputName() const
{
  return QObject::tr( "Cleaned" );
}

QList<int> QgsRemoveHolesAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsProcessing::SourceType QgsRemoveHolesAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QString QgsRemoveHolesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon layer and removes holes in polygons. It creates a new vector "
                      "layer in which polygons with holes have been replaced by polygons with only their external ring. "
                      "Attributes are not modified.\n\n"
                      "An optional minimum area parameter allows removing only holes which are smaller than a specified "
                      "area threshold. Leaving this parameter as 0.0 results in all holes being removed." );
}

QgsRemoveHolesAlgorithm *QgsRemoveHolesAlgorithm::createInstance() const
{
  return new QgsRemoveHolesAlgorithm();
}

QgsProcessingFeatureSource::Flag QgsRemoveHolesAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm can be used to repair geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

void QgsRemoveHolesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > minArea = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MIN_AREA" ),
      QObject::tr( "Remove holes with area less than" ), QgsProcessingParameterNumber::Double,
      0.0, false, 0 );
  minArea->setIsDynamic( true );
  minArea->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "MIN_AREA" ), QObject::tr( "Remove holes with area less than" ), QgsPropertyDefinition::DoublePositive ) );
  minArea->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( minArea.release() );
}

bool QgsRemoveHolesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMinArea = parameterAsDouble( parameters, QStringLiteral( "MIN_AREA" ), context );
  mDynamicMinArea = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "MIN_AREA" ) );
  if ( mDynamicMinArea )
    mMinAreaProperty = parameters.value( QStringLiteral( "MIN_AREA" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsRemoveHolesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();

    double minArea = mMinArea;
    if ( mDynamicMinArea )
      minArea = mMinAreaProperty.valueAsDouble( context.expressionContext(), minArea );

    f.setGeometry( geometry.removeInteriorRings( minArea > 0 ? minArea : -1 ) );
  }
  return QgsFeatureList() << f;
}


///@endcond


