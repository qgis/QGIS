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
  return QStringLiteral( "intersection" );
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
  return QStringLiteral( "vectoroverlay" );
}

QString QgsIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the overlapping portions of features in the Input and Overlay layers. "
                      "Features in the output Intersection layer are assigned the attributes of the overlapping features "
                      "from both the Input and Overlay layers." );
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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Overlay layer" ) ) );

  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "INPUT_FIELDS" ),
                  QObject::tr( "Input fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "INPUT" ), Qgis::ProcessingFieldParameterDataType::Any, true, true ) );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "OVERLAY_FIELDS" ),
                  QObject::tr( "Overlay fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "OVERLAY" ), Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  std::unique_ptr< QgsProcessingParameterString > prefix = std::make_unique< QgsProcessingParameterString >( QStringLiteral( "OVERLAY_FIELDS_PREFIX" ), QObject::tr( "Overlay fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Intersection" ) ) );

  std::unique_ptr< QgsProcessingParameterNumber > gridSize = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "GRID_SIZE" ),
      QObject::tr( "Grid size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( gridSize.release() );
}


QVariantMap QgsIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "OVERLAY" ) ) );

  const Qgis::WkbType geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  const QStringList fieldsA = parameterAsStrings( parameters, QStringLiteral( "INPUT_FIELDS" ), context );
  const QStringList fieldsB = parameterAsStrings( parameters, QStringLiteral( "OVERLAY_FIELDS" ), context );

  const QList<int> fieldIndicesA = QgsProcessingUtils::fieldNamesToIndices( fieldsA, sourceA->fields() );
  const QList<int> fieldIndicesB = QgsProcessingUtils::fieldNamesToIndices( fieldsB, sourceB->fields() );

  const QString overlayFieldsPrefix = parameterAsString( parameters, QStringLiteral( "OVERLAY_FIELDS_PREFIX" ), context );
  const QgsFields outputFields = QgsProcessingUtils::combineFields(
                                   QgsProcessingUtils::indicesToFields( fieldIndicesA, sourceA->fields() ),
                                   QgsProcessingUtils::indicesToFields( fieldIndicesB, sourceB->fields() ),
                                   overlayFieldsPrefix );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, geomType, sourceA->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  long count = 0;
  const long total = sourceA->featureCount();

  QgsGeometryParameters geometryParameters;
  if ( parameters.value( QStringLiteral( "GRID_SIZE" ) ).isValid() )
  {
    geometryParameters.setGridSize( parameterAsDouble( parameters, QStringLiteral( "GRID_SIZE" ), context ) );
  }

  QgsOverlayUtils::intersection( *sourceA, *sourceB, *sink, context, feedback, count, total, fieldIndicesA, fieldIndicesB, geometryParameters );

  return outputs;
}

///@endcond
