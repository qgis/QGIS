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
  return QStringLiteral( "symmetricaldifference" );
}

QString QgsSymmetricalDifferenceAlgorithm::displayName() const
{
  return QObject::tr( "Symmetrical difference" );
}

QString QgsSymmetricalDifferenceAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsSymmetricalDifferenceAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

QString QgsSymmetricalDifferenceAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts the portions of features from both the Input and Overlay layers that do not overlap. "
                      "Overlapping areas between the two layers are removed. The attribute table of the Symmetrical Difference layer "
                      "contains original attributes from both the Input and Difference layers." );
}

QgsProcessingAlgorithm *QgsSymmetricalDifferenceAlgorithm::createInstance() const
{
  return new QgsSymmetricalDifferenceAlgorithm();
}

void QgsSymmetricalDifferenceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Overlay layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Symmetrical difference" ) ) );
}


QVariantMap QgsSymmetricalDifferenceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "OVERLAY" ) ) );

  QgsWkbTypes::Type geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  QgsFields fields = QgsProcessingUtils::combineFields( sourceA->fields(), sourceB->fields() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, geomType, sourceA->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  int count = 0;
  int total = sourceA->featureCount() + sourceB->featureCount();

  QgsOverlayUtils::difference( *sourceA, *sourceB, *sink, context, feedback, count, total, QgsOverlayUtils::OutputAB );

  QgsOverlayUtils::difference( *sourceB, *sourceA, *sink, context, feedback, count, total, QgsOverlayUtils::OutputBA );

  return outputs;
}

///@endcond PRIVATE
