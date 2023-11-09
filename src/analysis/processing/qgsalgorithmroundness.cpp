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
  return QObject::tr( "Calculates the roundness of each feature and stores it as a new field. The input vector layer must contain polygons.\n\n"
                      "The roundness of a polygon is defined as 4π × polygon area / perimeter². The roundness value varies between 0 and 1. A perfect circle has a roundness of 1, while a completely flat polygon has a roundness of 0." );
}

QString QgsRoundnessAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the roundness of polygon features." );
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
  QgsAttributes attributes = f.attributes();
  if ( f.hasGeometry() )
  {
    QgsGeometry geom = f.geometry();
    if ( const QgsCurvePolygon *poly = qgsgeometry_cast< const QgsCurvePolygon * >( geom.constGet()->simplifiedTypeRef() ) )
    {
      double roundness = poly->roundness();
      attributes << QVariant( roundness );
    }
    else
    {
      attributes << QVariant();
    }
  }
  else
  {
    attributes << QVariant();
  }
  f.setAttributes( attributes );
  return QgsFeatureList() << f;
}

///@endcond


