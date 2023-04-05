/***************************************************************************
                         qgsalgorithmpdalfixprojection.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalfixprojection.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalFixProjectionAlgorithm::name() const
{
  return QStringLiteral( "fixprojection" );
}

QString QgsPdalFixProjectionAlgorithm::displayName() const
{
  return QObject::tr( "Fix projection" );
}

QString QgsPdalFixProjectionAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalFixProjectionAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalFixProjectionAlgorithm::tags() const
{
  return QObject::tr( "assign,set,fix,crs,srs" ).split( ',' );
}

QString QgsPdalFixProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm fixes (assigns) point cloud CRS if it is not present or wrong." );
}

QgsPdalFixProjectionAlgorithm *QgsPdalFixProjectionAlgorithm::createInstance() const
{
  return new QgsPdalFixProjectionAlgorithm();
}

void QgsPdalFixProjectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Desired CRS" ), QStringLiteral( "EPSG:4326" ) ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) ) );
}

QStringList QgsPdalFixProjectionAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );

  QStringList args = { QStringLiteral( "translate" ),
                       QStringLiteral( "--input=%1" ).arg( layer->source() ),
                       QStringLiteral( "--output=%1" ).arg( outputFile ),
                       QStringLiteral( "--assign-crs=%1" ).arg( crs.authid() )
                     };

  applyThreadsParameter( args );
  return args;
}

///@endcond
