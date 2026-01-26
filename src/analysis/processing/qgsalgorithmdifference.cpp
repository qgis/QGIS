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
  return u"difference"_s;
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
  return u"vectoroverlay"_s;
}

QString QgsDifferenceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts features from the Input layer that fall outside, or partially overlap, features in the Overlay layer. "
                      "Input layer features that partially overlap feature(s) in the Overlay layer are split along those features' boundary "
                      "and only the portions outside the Overlay layer features are retained." )
         + u"\n\n"_s
         + QObject::tr( "Attributes are not modified, although properties such as area or length of the features will "
                        "be modified by the difference operation. If such properties are stored as attributes, those attributes will have to "
                        "be manually updated." );
}

QString QgsDifferenceAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts features from an input layer that fall outside, or partially overlap, features in an overlay layer." );
}

bool QgsDifferenceAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  return layer->isSpatial();
}

Qgis::ProcessingAlgorithmFlags QgsDifferenceAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::SupportsInPlaceEdits;
  return f;
}

QgsProcessingAlgorithm *QgsDifferenceAlgorithm::createInstance() const
{
  return new QgsDifferenceAlgorithm();
}

void QgsDifferenceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"OVERLAY"_s, QObject::tr( "Overlay layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Difference" ) ) );

  auto gridSize = std::make_unique<QgsProcessingParameterNumber>( u"GRID_SIZE"_s, QObject::tr( "Grid size" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  gridSize->setFlags( gridSize->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( gridSize.release() );
}


QVariantMap QgsDifferenceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> sourceA( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const std::unique_ptr<QgsFeatureSource> sourceB( parameterAsSource( parameters, u"OVERLAY"_s, context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, u"OVERLAY"_s ) );

  const Qgis::WkbType geomType = QgsWkbTypes::promoteNonPointTypesToMulti( sourceA->wkbType() );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, sourceA->fields(), geomType, sourceA->sourceCrs() ) );
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

  QgsOverlayUtils::difference( *sourceA, *sourceB, *sink, context, feedback, count, total, QgsOverlayUtils::OutputA, geometryParameters, QgsOverlayUtils::SanitizeFlag::DontPromotePointGeometryToMultiPoint );

  sink->finalize();
  return outputs;
}

///@endcond PRIVATE
