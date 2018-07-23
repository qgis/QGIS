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

///@cond PRIVATE


QString QgsDifferenceAlgorithm::name() const
{
  return QStringLiteral( "difference" );
}

QString QgsDifferenceAlgorithm::displayName() const
{
  return QObject::tr( "Difference" );
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
  return QObject::tr( "This algorithm extracts features from the Input layer that fall outside, or partially overlap, features in the Difference layer. Input layer features that partially overlap the difference layer feature(s) are split along the boundary of the difference layer feature(s) and only the portions outside the difference layer features are retained." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Attributes are not modified." );
}

QgsProcessingAlgorithm *QgsDifferenceAlgorithm::createInstance() const
{
  return new QgsDifferenceAlgorithm();
}

void QgsDifferenceAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Difference layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Difference" ) ) );
}


QVariantMap QgsDifferenceAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !sourceB )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "OVERLAY" ) ) );

  QgsWkbTypes::Type geomType = QgsWkbTypes::multiType( sourceA->wkbType() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, sourceA->fields(), geomType, sourceA->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  int count = 0;
  int total = sourceA->featureCount();
  QgsOverlayUtils::difference( *sourceA, *sourceB, *sink, context, feedback, count, total, QgsOverlayUtils::OutputA );

  return outputs;
}

///@endcond PRIVATE
