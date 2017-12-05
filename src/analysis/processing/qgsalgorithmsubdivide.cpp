/***************************************************************************
                         qgsalgorithmsubdivide.cpp
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

#include "qgsalgorithmsubdivide.h"

///@cond PRIVATE


QgsProcessingAlgorithm::Flags QgsSubdivideAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsSubdivideAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_NODES" ), QObject::tr( "Maximum nodes in parts" ), QgsProcessingParameterNumber::Integer,
                256, false, 8, 100000 ) );
}

QString QgsSubdivideAlgorithm::name() const
{
  return QStringLiteral( "subdivide" );
}

QString QgsSubdivideAlgorithm::displayName() const
{
  return QObject::tr( "Subdivide" );
}

QStringList QgsSubdivideAlgorithm::tags() const
{
  return QObject::tr( "subdivide,segmentize,split,tessellate" ).split( ',' );
}

QString QgsSubdivideAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSubdivideAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSubdivideAlgorithm::shortHelpString() const
{
  return QObject::tr( "Subdivides the geometry. The returned geometry will be a collection containing subdivided parts "
                      "from the original geometry, where no part has more then the specified maximum number of nodes.\n\n"
                      "This is useful for dividing a complex geometry into less complex parts, which are better able to be spatially "
                      "indexed and faster to perform further operations such as intersects on. The returned geometry parts may "
                      "not be valid and may contain self-intersections.\n\n"
                      "Curved geometries will be segmentized before subdivision." );
}

QgsSubdivideAlgorithm *QgsSubdivideAlgorithm::createInstance() const
{
  return new QgsSubdivideAlgorithm();
}

QString QgsSubdivideAlgorithm::outputName() const
{
  return QObject::tr( "Subdivided" );
}

QgsWkbTypes::Type QgsSubdivideAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::multiType( inputWkbType );
}

QgsFeature QgsSubdivideAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    feature.setGeometry( feature.geometry().subdivide( mMaxNodes ) );
    if ( !feature.geometry() )
    {
      feedback->reportError( QObject::tr( "Error calculating subdivision for feature %1" ).arg( feature.id() ) );
    }
  }
  return feature;
}

bool QgsSubdivideAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMaxNodes = parameterAsInt( parameters, QStringLiteral( "MAX_NODES" ), context );
  return true;
}



///@endcond



