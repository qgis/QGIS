/***************************************************************************
  qgsalgorithmsymmetricaldifference.cpp
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

#include "qgsalgorithmsymmetricaldifference.h"

#include "qgsoverlayutils.h"

///@cond PRIVATE

QString QgsSymmetricalDifferenceAlgorithm::name() const
{
  return u"symmetricaldifference"_s;
}

QString QgsSymmetricalDifferenceAlgorithm::displayName() const
{
  return QObject::tr( "Symmetrical difference" );
}

QStringList QgsSymmetricalDifferenceAlgorithm::tags() const
{
  return QObject::tr( "difference,symdiff,not overlap" ).split( ',' );
}

QString QgsSymmetricalDifferenceAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsSymmetricalDifferenceAlgorithm::groupId() const
{
  return u"vectoroverlay"_s;
}

QString QgsSymmetricalDifferenceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the portions of features from both the Input and Overlay layers that do not overlap. "
                      "Overlapping areas between the two layers are removed. The attribute table of the Symmetrical Difference layer "
                      "contains original attributes from both the Input and Overlay layers." );
}

QString QgsSymmetricalDifferenceAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts the portions of features from two layers that do not overlap." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsSymmetricalDifferenceAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsProcessingAlgorithm *QgsSymmetricalDifferenceAlgorithm::createInstance() const
{
  return new QgsSymmetricalDifferenceAlgorithm();
}

void QgsSymmetricalDifferenceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"OVERLAY"_s, QObject::tr( "Overlay layer" ) ) );

  auto prefix = std::make_unique<QgsProcessingParameterString>( u"OVERLAY_FIELDS_PREFIX"_s, QObject::tr( "Overlay fields prefix" ), QString(), false, true );
  prefix->setFlags( prefix->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( prefix.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Symmetrical difference" ) ) );

  auto gridSize = std::make_unique<QgsProcessingParameterNumber>( u"GRID_SIZE"_s, QObject::tr( "Grid size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( gridSize.release() );
}


QVariantMap QgsSymmetricalDifferenceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> sourceA( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsFeatureSource> sourceB( parameterAsSource( parameters, u"OVERLAY"_s, context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, u"OVERLAY"_s ) );

  const Qgis::WkbType geomTypeA = QgsWkbTypes::promoteNonPointTypesToMulti( sourceA->wkbType() );
  const Qgis::WkbType geomTypeB = QgsWkbTypes::promoteNonPointTypesToMulti( sourceB->wkbType() );

  if ( geomTypeA != geomTypeB )
    feedback->pushWarning( QObject::tr( "Performing symmetrical difference between layers with different geometry types (INPUT has %1 and OVERLAY has %2) can lead to unexpected results" ).arg( QgsWkbTypes::displayString( sourceA->wkbType() ), QgsWkbTypes::displayString( sourceB->wkbType() ) ) );

  const QString overlayFieldsPrefix = parameterAsString( parameters, u"OVERLAY_FIELDS_PREFIX"_s, context );
  const QgsFields fields = QgsProcessingUtils::combineFields( sourceA->fields(), sourceB->fields(), overlayFieldsPrefix );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, geomTypeA, sourceA->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );

  long count = 0;
  const long total = sourceA->featureCount() + sourceB->featureCount();

  QgsGeometryParameters geometryParameters;
  if ( parameters.value( u"GRID_SIZE"_s ).isValid() )
  {
    geometryParameters.setGridSize( parameterAsDouble( parameters, u"GRID_SIZE"_s, context ) );
  }

  QgsOverlayUtils::difference( *sourceA, *sourceB, *sink, context, feedback, count, total, QgsOverlayUtils::OutputAB, geometryParameters, QgsOverlayUtils::SanitizeFlag::DontPromotePointGeometryToMultiPoint );
  if ( feedback->isCanceled() )
    return outputs;

  QgsOverlayUtils::difference( *sourceB, *sourceA, *sink, context, feedback, count, total, QgsOverlayUtils::OutputBA, geometryParameters, QgsOverlayUtils::SanitizeFlag::DontPromotePointGeometryToMultiPoint );

  sink->finalize();

  return outputs;
}

///@endcond PRIVATE
