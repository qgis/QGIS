/***************************************************************************
                         qgsalgorithmpointonsurface.cpp
                         ------------------------
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

#include "qgsalgorithmpointonsurface.h"

#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsPointOnSurfaceAlgorithm::name() const
{
  return u"pointonsurface"_s;
}

QString QgsPointOnSurfaceAlgorithm::displayName() const
{
  return QObject::tr( "Point on surface" );
}

QStringList QgsPointOnSurfaceAlgorithm::tags() const
{
  return QObject::tr( "centroid,inside,within" ).split( ',' );
}

QString QgsPointOnSurfaceAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPointOnSurfaceAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsPointOnSurfaceAlgorithm::outputName() const
{
  return QObject::tr( "Point" );
}

QgsFeatureSink::SinkFlags QgsPointOnSurfaceAlgorithm::sinkFlags() const
{
  if ( mAllParts )
    return QgsProcessingFeatureBasedAlgorithm::sinkFlags() | QgsFeatureSink::RegeneratePrimaryKey;
  else
    return QgsProcessingFeatureBasedAlgorithm::sinkFlags();
}

QString QgsPointOnSurfaceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm returns a point guaranteed to lie on the surface of a geometry." );
}

QString QgsPointOnSurfaceAlgorithm::shortDescription() const
{
  return QObject::tr( "Returns a point guaranteed to lie on the surface of a geometry." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsPointOnSurfaceAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKeyInSomeScenarios;
}

QgsPointOnSurfaceAlgorithm *QgsPointOnSurfaceAlgorithm::createInstance() const
{
  return new QgsPointOnSurfaceAlgorithm();
}

void QgsPointOnSurfaceAlgorithm::initParameters( const QVariantMap & )
{
  auto allParts = std::make_unique<QgsProcessingParameterBoolean>(
    u"ALL_PARTS"_s,
    QObject::tr( "Create point on surface for each part" ),
    false
  );
  allParts->setIsDynamic( true );
  allParts->setDynamicPropertyDefinition( QgsPropertyDefinition( u"All parts"_s, QObject::tr( "Create point on surface for each part" ), QgsPropertyDefinition::Boolean ) );
  allParts->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( allParts.release() );
}

bool QgsPointOnSurfaceAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAllParts = parameterAsBoolean( parameters, u"ALL_PARTS"_s, context );
  mDynamicAllParts = QgsProcessingParameters::isDynamic( parameters, u"ALL_PARTS"_s );
  if ( mDynamicAllParts )
    mAllPartsProperty = parameters.value( u"ALL_PARTS"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsPointOnSurfaceAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeatureList list;
  QgsFeature feature = f;
  if ( feature.hasGeometry() && !feature.geometry().isEmpty() )
  {
    const QgsGeometry geom = feature.geometry();

    bool allParts = mAllParts;
    if ( mDynamicAllParts )
      allParts = mAllPartsProperty.valueAsBool( context.expressionContext(), allParts );

    if ( allParts && geom.isMultipart() )
    {
      const QgsGeometryCollection *geomCollection = static_cast<const QgsGeometryCollection *>( geom.constGet() );

      const int partCount = geomCollection->partCount();
      list.reserve( partCount );
      for ( int i = 0; i < partCount; ++i )
      {
        const QgsGeometry partGeometry( geomCollection->geometryN( i )->clone() );
        const QgsGeometry outputGeometry = partGeometry.pointOnSurface();
        if ( outputGeometry.isNull() )
        {
          feedback->reportError( QObject::tr( "Error calculating point on surface for feature %1 part %2: %3" ).arg( feature.id() ).arg( i ).arg( outputGeometry.lastError() ) );
        }
        feature.setGeometry( outputGeometry );
        list << feature;
      }
    }
    else
    {
      const QgsGeometry outputGeometry = feature.geometry().pointOnSurface();
      if ( outputGeometry.isNull() )
      {
        feedback->reportError( QObject::tr( "Error calculating point on surface for feature %1: %2" ).arg( feature.id() ).arg( outputGeometry.lastError() ) );
      }
      feature.setGeometry( outputGeometry );
      list << feature;
    }
  }
  else
  {
    list << feature;
  }
  return list;
}

///@endcond
