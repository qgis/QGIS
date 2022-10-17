/***************************************************************************
  qgsalgorithmdifference.cpp
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

#include "qgsalgorithmdifference.h"

#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE


QString QgsDifferenceAlgorithm::name() const
{
  return QStringLiteral( "difference" );
}

QString QgsDifferenceAlgorithm::displayName() const
{
  return QObject::tr( "Difference" );
}

QStringList QgsDifferenceAlgorithm::tags() const
{
  return QObject::tr( "difference,erase,not overlap" ).split( ',' );
}

QString QgsDifferenceAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsDifferenceAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsDifferenceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts features from the Input layer that fall outside, or partially overlap, features in the Overlay layer. "
                      "Input layer features that partially overlap feature(s) in the Overlay layer are split along those features' boundary "
                      "and only the portions outside the Overlay layer features are retained." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Attributes are not modified, although properties such as area or length of the features will "
                        "be modified by the difference operation. If such properties are stored as attributes, those attributes will have to "
                        "be manually updated." );
}

bool QgsDifferenceAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  return layer->isSpatial();
}

QgsProcessingAlgorithm::Flags QgsDifferenceAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
  return f;
}

QgsProcessingAlgorithm *QgsDifferenceAlgorithm::createInstance() const
{
  return new QgsDifferenceAlgorithm();
}

void QgsDifferenceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Overlay layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Difference" ) ) );

  std::unique_ptr< QgsProcessingParameterNumber > gridSize = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "GRID_SIZE" ),
      QObject::tr( "Grid size" ), QgsProcessingParameterNumber::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( gridSize.release() );
}


QVariantMap QgsDifferenceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "OVERLAY" ) ) );

  const QgsWkbTypes::Type geomType = QgsWkbTypes::promoteNonPointTypesToMulti( sourceA->wkbType() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, sourceA->fields(), geomType, sourceA->sourceCrs() ) );
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

  QgsOverlayUtils::difference( *sourceA, *sourceB, *sink, context, feedback, count, total, QgsOverlayUtils::OutputA, geometryParameters, QgsOverlayUtils::SanitizeFlag::DontPromotePointGeometryToMultiPoint );

  return outputs;
}

///@endcond PRIVATE
