/***************************************************************************
                         qgsalgorithmdensifygeometries.cpp
                         ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmdensifygeometriesbycount.h"

///@cond PRIVATE

QString QgsDensifyGeometriesByCountAlgorithm::name() const
{
  return u"densifygeometries"_s;
}

QString QgsDensifyGeometriesByCountAlgorithm::displayName() const
{
  return QObject::tr( "Densify by count" );
}

QStringList QgsDensifyGeometriesByCountAlgorithm::tags() const
{
  return QObject::tr( "add,vertex,vertices,points,nodes" ).split( ',' );
}

QString QgsDensifyGeometriesByCountAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsDensifyGeometriesByCountAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsDensifyGeometriesByCountAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon or line layer "
                      "and generates a new one in which the geometries "
                      "have a larger number of vertices than the "
                      "original one.\n\n If the geometries have z or m values "
                      "present then these will be linearly interpolated "
                      "at the added nodes.\n\n The number of new vertices to "
                      "add to each feature geometry is specified "
                      "as an input parameter." );
}

QString QgsDensifyGeometriesByCountAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a densified version of geometries by adding a fixed number of vertices to segments." );
}

QgsDensifyGeometriesByCountAlgorithm *QgsDensifyGeometriesByCountAlgorithm::createInstance() const
{
  return new QgsDensifyGeometriesByCountAlgorithm;
}

QList<int> QgsDensifyGeometriesByCountAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

void QgsDensifyGeometriesByCountAlgorithm::initParameters( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )
  auto verticesCnt = std::make_unique<QgsProcessingParameterNumber>( u"VERTICES"_s, QObject::tr( "Number of vertices to add" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1, 10000000 );
  verticesCnt->setIsDynamic( true );
  verticesCnt->setDynamicPropertyDefinition( QgsPropertyDefinition( u"VerticesCount"_s, QObject::tr( "Vertices count" ), QgsPropertyDefinition::IntegerPositive ) );
  verticesCnt->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( verticesCnt.release() );
}

QString QgsDensifyGeometriesByCountAlgorithm::outputName() const
{
  return QObject::tr( "Densified" );
}

bool QgsDensifyGeometriesByCountAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )
  mVerticesCnt = parameterAsInt( parameters, u"VERTICES"_s, context );

  mDynamicVerticesCnt = QgsProcessingParameters::isDynamic( parameters, u"VERTICES"_s );
  if ( mDynamicVerticesCnt )
    mVerticesCntProperty = parameters.value( u"VERTICES"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsDensifyGeometriesByCountAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( context )
  Q_UNUSED( feedback )

  QgsFeature densifiedFeature = feature;

  if ( feature.hasGeometry() )
  {
    int verticesCnt = mVerticesCnt;
    if ( mDynamicVerticesCnt )
      verticesCnt = mVerticesCntProperty.valueAsInt( context.expressionContext(), verticesCnt );

    if ( verticesCnt > 0 )
      densifiedFeature.setGeometry( feature.geometry().densifyByCount( verticesCnt ) );
  }

  return QgsFeatureList() << densifiedFeature;
}


///@endcond PRIVATE
