/***************************************************************************
  qgsalgorithmintersection.cpp
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

#include "qgsalgorithmintersection.h"

#include "qgsgeometrycollection.h"
#include "qgsgeometryengine.h"
#include "qgsoverlayutils.h"

///@cond PRIVATE


QString QgsIntersectionAlgorithm::name() const
{
  return u"intersection"_s;
}

QString QgsIntersectionAlgorithm::displayName() const
{
  return QObject::tr( "Intersection" );
}

QString QgsIntersectionAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsIntersectionAlgorithm::groupId() const
{
  return u"vectoroverlay"_s;
}

QString QgsIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the overlapping portions of features in the Input and Overlay layers. "
                      "Features in the output Intersection layer are assigned the attributes of the overlapping features "
                      "from both the Input and Overlay layers." );
}

QString QgsIntersectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts overlapping portions of features between two layers." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsIntersectionAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsProcessingAlgorithm *QgsIntersectionAlgorithm::createInstance() const
{
  return new QgsIntersectionAlgorithm();
}

void QgsIntersectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"OVERLAY"_s, QObject::tr( "Overlay layer" ) ) );

  addParameter( new QgsProcessingParameterField(
    u"INPUT_FIELDS"_s,
    QObject::tr( "Input fields to keep (leave empty to keep all fields)" ), QVariant(),
    u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true
  ) );
  addParameter( new QgsProcessingParameterField(
    u"OVERLAY_FIELDS"_s,
    QObject::tr( "Overlay fields to keep (leave empty to keep all fields)" ), QVariant(),
    u"OVERLAY"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true
  ) );

  auto prefix = std::make_unique<QgsProcessingParameterString>( u"OVERLAY_FIELDS_PREFIX"_s, QObject::tr( "Overlay fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Intersection" ) ) );

  auto gridSize = std::make_unique<QgsProcessingParameterNumber>( u"GRID_SIZE"_s, QObject::tr( "Grid size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( gridSize.release() );
}


QVariantMap QgsIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> sourceA( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsFeatureSource> sourceB( parameterAsSource( parameters, u"OVERLAY"_s, context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, u"OVERLAY"_s ) );

  const Qgis::WkbType geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  const QStringList fieldsA = parameterAsStrings( parameters, u"INPUT_FIELDS"_s, context );
  const QStringList fieldsB = parameterAsStrings( parameters, u"OVERLAY_FIELDS"_s, context );

  const QList<int> fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( fieldsA, sourceA->fields() );
  const QList<int> fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( fieldsB, sourceB->fields() );

  const QString overlayFieldsPrefix = parameterAsString( parameters, u"OVERLAY_FIELDS_PREFIX"_s, context );
  const QgsFields outputFields = QgsProcessingUtils::combineFields(
    QgsProcessingUtils::indicesToFields( fieldIndicesA, sourceA->fields() ),
    QgsProcessingUtils::indicesToFields( fieldIndicesB, sourceB->fields() ),
    overlayFieldsPrefix
  );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outputFields, geomType, sourceA->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );

  long count = 0;
  const long total = sourceA->featureCount();

  QgsGeometryParameters geometryParameters;
  if ( parameters.value( u"GRID_SIZE"_s ).isValid() )
  {
    geometryParameters.setGridSize( parameterAsDouble( parameters, u"GRID_SIZE"_s, context ) );
  }

  QgsOverlayUtils::intersection( *sourceA, *sourceB, *sink, context, feedback, count, total, fieldIndicesA, fieldIndicesB, geometryParameters );

  sink->finalize();

  return outputs;
}

///@endcond
