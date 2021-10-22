/***************************************************************************
                         qgsalgorithmroundness.cpp
                         ---------------------
    begin                : September 2021
    copyright            : (C) 2021 by Antoine Facchini
    email                : antoine dot facchini @oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmroundness.h"
#include "qgscurvepolygon.h"

///@cond PRIVATE

QString QgsRoundnessAlgorithm::name() const
{
  return QStringLiteral( "roundness" );
}

QString QgsRoundnessAlgorithm::displayName() const
{
  return QObject::tr( "Roundness" );
}

QStringList QgsRoundnessAlgorithm::tags() const
{
  return QObject::tr( "roundness,circle" ).split( ',' );
}

QString QgsRoundnessAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRoundnessAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsRoundnessAlgorithm::outputName() const
{
  return QObject::tr( "Roundness" );
}

QString QgsRoundnessAlgorithm::shortHelpString() const
{
  return QObject::tr( "Adds in the attribute table the roundness of each feature. The input vector layer must contain polygons." );
}

QString QgsRoundnessAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates how close each feature shape is to a circle in a polygon vector layer." );
}

QgsRoundnessAlgorithm *QgsRoundnessAlgorithm::createInstance() const
{
  return new QgsRoundnessAlgorithm();
}

QList<int> QgsRoundnessAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsProcessing::SourceType QgsRoundnessAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsFields QgsRoundnessAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outputFields = inputFields;
  outputFields.append( QgsField( QStringLiteral( "roundness" ), QVariant::Double ) );
  return outputFields;
}

QgsFeatureList QgsRoundnessAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geom = f.geometry();
    const QgsCurvePolygon *poly = qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet() );

    double roundness = poly->roundness();
    QgsAttributes attributes = f.attributes();
    attributes << QVariant( roundness );
    f.setAttributes( attributes );
  }
  return QgsFeatureList() << f;
}

///@endcond


