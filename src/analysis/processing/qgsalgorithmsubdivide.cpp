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

void QgsSubdivideAlgorithm::initParameters( const QVariantMap & )
{
  auto nodes = std::make_unique<QgsProcessingParameterNumber>( u"MAX_NODES"_s, QObject::tr( "Maximum nodes in parts" ), Qgis::ProcessingNumberParameterType::Integer, 256, false, 8, 100000 );
  nodes->setIsDynamic( true );
  nodes->setDynamicPropertyDefinition( QgsPropertyDefinition( u"MAX_NODES"_s, QObject::tr( "Maximum nodes in parts" ), QgsPropertyDefinition::Integer ) );
  nodes->setDynamicLayerParameterName( u"INPUT"_s );

  addParameter( nodes.release() );
}

QString QgsSubdivideAlgorithm::name() const
{
  return u"subdivide"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsSubdivideAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm subdivides the geometry. The returned geometry will be a collection containing subdivided parts "
                      "from the original geometry, where no part has more than the specified maximum number of nodes.\n\n"
                      "This is useful for dividing a complex geometry into less complex parts, which are better able to be spatially "
                      "indexed and faster to perform further operations such as intersects on. The returned geometry parts may "
                      "not be valid and may contain self-intersections.\n\n"
                      "Curved geometries will be segmentized before subdivision." );
}

QString QgsSubdivideAlgorithm::shortDescription() const
{
  return QObject::tr( "Subdivides the geometry into parts that have less than a specified maximum number of nodes." );
}

QgsSubdivideAlgorithm *QgsSubdivideAlgorithm::createInstance() const
{
  return new QgsSubdivideAlgorithm();
}

QString QgsSubdivideAlgorithm::outputName() const
{
  return QObject::tr( "Subdivided" );
}

Qgis::WkbType QgsSubdivideAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  return QgsWkbTypes::multiType( inputWkbType );
}

QgsFeatureList QgsSubdivideAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    int maxNodes = mMaxNodes;
    if ( mDynamicMaxNodes )
      maxNodes = mMaxNodesProperty.valueAsDouble( context.expressionContext(), maxNodes );

    feature.setGeometry( feature.geometry().subdivide( maxNodes ) );
    if ( !feature.hasGeometry() )
    {
      feedback->reportError( QObject::tr( "Error calculating subdivision for feature %1" ).arg( feature.id() ) );
    }
  }
  return QgsFeatureList() << feature;
}

bool QgsSubdivideAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMaxNodes = parameterAsInt( parameters, u"MAX_NODES"_s, context );
  mDynamicMaxNodes = QgsProcessingParameters::isDynamic( parameters, u"MAX_NODES"_s );
  if ( mDynamicMaxNodes )
    mMaxNodesProperty = parameters.value( u"MAX_NODES"_s ).value<QgsProperty>();

  return true;
}


///@endcond
