/***************************************************************************
  qgsalgorithmunion.cpp
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmunion.h"

#include "qgsoverlayutils.h"

///@cond PRIVATE


QString QgsUnionAlgorithm::name() const
{
  return u"union"_s;
}

QString QgsUnionAlgorithm::displayName() const
{
  return QObject::tr( "Union" );
}

QString QgsUnionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsUnionAlgorithm::groupId() const
{
  return u"vectoroverlay"_s;
}

QStringList QgsUnionAlgorithm::tags() const
{
  return QObject::tr( "overlap,clip,union,not overlap" ).split( ',' );
}

QString QgsUnionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks overlaps between features within the Input layer and creates separate features for overlapping "
                      "and non-overlapping parts. The area of overlap will create as many identical overlapping features as there are "
                      "features that participate in that overlap." )
         + u"\n\n"_s
         + QObject::tr( "An Overlay layer can also be used, in which case features from each layer are split at their overlap with features from "
                        "the other one, creating a layer containing all the portions from both Input and Overlay layers. "
                        "The attribute table of the Union layer is filled with attribute values from the respective original layer "
                        "for non-overlapping features, and attribute values from both layers for overlapping features." );
}

QString QgsUnionAlgorithm::shortDescription() const
{
  return QObject::tr( "Checks overlaps between features on the same layer or on two different layers "
                      "and creates separate features for overlapping and non-overlapping parts." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsUnionAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsProcessingAlgorithm *QgsUnionAlgorithm::createInstance() const
{
  return new QgsUnionAlgorithm();
}

void QgsUnionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"OVERLAY"_s, QObject::tr( "Overlay layer" ), QList<int>(), QVariant(), true ) );

  auto prefix = std::make_unique<QgsProcessingParameterString>( u"OVERLAY_FIELDS_PREFIX"_s, QObject::tr( "Overlay fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Union" ) ) );

  auto gridSize = std::make_unique<QgsProcessingParameterNumber>( u"GRID_SIZE"_s, QObject::tr( "Grid size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( gridSize.release() );
}

QVariantMap QgsUnionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> sourceA( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsFeatureSource> sourceB( parameterAsSource( parameters, u"OVERLAY"_s, context ) );
  if ( parameters.value( u"OVERLAY"_s ).isValid() && !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, u"OVERLAY"_s ) );

  const Qgis::WkbType geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  const QString overlayFieldsPrefix = parameterAsString( parameters, u"OVERLAY_FIELDS_PREFIX"_s, context );
  const QgsFields fields = sourceB ? QgsProcessingUtils::combineFields( sourceA->fields(), sourceB->fields(), overlayFieldsPrefix ) : sourceA->fields();

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, geomType, sourceA->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );

  if ( !sourceB )
  {
    // we are doing single layer union
    QgsOverlayUtils::resolveOverlaps( *sourceA, *sink, feedback );
    return outputs;
  }

  const QList<int> fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceA->fields() );
  const QList<int> fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( QStringList(), sourceB->fields() );

  long count = 0;
  const long total = sourceA->featureCount() * 2 + sourceB->featureCount();

  QgsGeometryParameters geometryParameters;
  if ( parameters.value( u"GRID_SIZE"_s ).isValid() )
  {
    geometryParameters.setGridSize( parameterAsDouble( parameters, u"GRID_SIZE"_s, context ) );
  }

  QgsOverlayUtils::intersection( *sourceA, *sourceB, *sink, context, feedback, count, total, fieldIndicesA, fieldIndicesB, geometryParameters );
  if ( feedback->isCanceled() )
    return outputs;

  QgsOverlayUtils::difference( *sourceA, *sourceB, *sink, context, feedback, count, total, QgsOverlayUtils::OutputAB, geometryParameters );
  if ( feedback->isCanceled() )
    return outputs;

  QgsOverlayUtils::difference( *sourceB, *sourceA, *sink, context, feedback, count, total, QgsOverlayUtils::OutputBA, geometryParameters );

  sink->finalize();

  return outputs;
}

///@endcond
