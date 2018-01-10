/***************************************************************************
                         qgsalgorithmsmooth.cpp
                         ---------------------
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

#include "qgsalgorithmsmooth.h"

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsSmoothAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsSmoothAlgorithm::name() const
{
  return QStringLiteral( "smoothgeometry" );
}

QString QgsSmoothAlgorithm::displayName() const
{
  return QObject::tr( "Smooth" );
}

QStringList QgsSmoothAlgorithm::tags() const
{
  return QObject::tr( "smooth,curve,generalize,round,bend,corners" ).split( ',' );
}

QString QgsSmoothAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSmoothAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSmoothAlgorithm::outputName() const
{
  return QObject::tr( "Smoothed" );
}

QgsProcessing::SourceType QgsSmoothAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QString QgsSmoothAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm smooths the geometries in a line or polygon layer. It creates a new layer with the "
                      "same features as the ones in the input layer, but with geometries containing a higher number of vertices "
                      "and corners in the geometries smoothed out.\n\n"
                      "The iterations parameter dictates how many smoothing iterations will be applied to each "
                      "geometry. A higher number of iterations results in smoother geometries with the cost of "
                      "greater number of nodes in the geometries.\n\n"
                      "The offset parameter controls how \"tightly\" the smoothed geometries follow the original geometries. "
                      "Smaller values results in a tighter fit, and larger values will create a looser fit.\n\n"
                      "The maximum angle parameter can be used to prevent smoothing of "
                      "nodes with large angles. Any node where the angle of the segments to either "
                      "side is larger than this will not be smoothed. For example, setting the maximum "
                      "angle to 90 degrees or lower would preserve right angles in the geometry." );
}

QgsSmoothAlgorithm *QgsSmoothAlgorithm::createInstance() const
{
  return new QgsSmoothAlgorithm();
}

QList<int> QgsSmoothAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon;
}

void QgsSmoothAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ITERATIONS" ),
                QObject::tr( "Iterations" ), QgsProcessingParameterNumber::Integer,
                1, false, 1, 10 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "OFFSET" ),
                QObject::tr( "Offset" ), QgsProcessingParameterNumber::Double,
                0.25, false, 0.0, 0.5 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_ANGLE" ),
                QObject::tr( "Maximum node angle to smooth" ), QgsProcessingParameterNumber::Double,
                180.0, false, 0.0, 180.0 ) );
}

bool QgsSmoothAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIterations = parameterAsInt( parameters, QStringLiteral( "ITERATIONS" ), context );
  mOffset = parameterAsDouble( parameters, QStringLiteral( "OFFSET" ), context );
  mMaxAngle = parameterAsDouble( parameters, QStringLiteral( "MAX_ANGLE" ), context );
  return true;
}

QgsFeature QgsSmoothAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry = f.geometry().smooth( mIterations, mOffset, -1, mMaxAngle );
    if ( !outputGeometry )
    {
      feedback->reportError( QObject::tr( "Error smoothing geometry %1" ).arg( feature.id() ) );
    }
    f.setGeometry( outputGeometry );
  }
  return f;
}

///@endcond


