/***************************************************************************
                         qgsalgorithmcentroid.cpp
                         ------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmcentroid.h"

///@cond PRIVATE

QString QgsCentroidAlgorithm::name() const
{
  return QStringLiteral( "centroids" );
}

QString QgsCentroidAlgorithm::displayName() const
{
  return QObject::tr( "Centroids" );
}

QStringList QgsCentroidAlgorithm::tags() const
{
  return QObject::tr( "centroid,center,average,point,middle" ).split( ',' );
}

QString QgsCentroidAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsCentroidAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsCentroidAlgorithm::outputName() const
{
  return QObject::tr( "Centroids" );
}

QgsProcessingAlgorithm::Flags QgsCentroidAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsCentroidAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Centroids" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsCentroidAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer, with points representing the centroid of the geometries in an input layer.\n\n"
                      "The attributes associated to each point in the output layer are the same ones associated to the original features." );
}

QgsCentroidAlgorithm *QgsCentroidAlgorithm::createInstance() const
{
  return new QgsCentroidAlgorithm();
}

QgsFeature QgsCentroidAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    feature.setGeometry( feature.geometry().centroid() );
    if ( !feature.geometry() )
    {
      feedback->pushInfo( QObject::tr( "Error calculating centroid for feature %1" ).arg( feature.id() ) );
    }
  }
  return feature;
}

///@endcond
