/***************************************************************************
                         qgsalgorithmpdalexportvector.cpp
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

#include "qgsalgorithmpdalexportvector.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalExportVectorAlgorithm::name() const
{
  return QStringLiteral( "exportvector" );
}

QString QgsPdalExportVectorAlgorithm::displayName() const
{
  return QObject::tr( "Export to vector" );
}

QString QgsPdalExportVectorAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalExportVectorAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudconversion" );
}

QStringList QgsPdalExportVectorAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,export,vector,attribute,create" ).split( ',' );
}

QString QgsPdalExportVectorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports point cloud data to a vector layer with 3D points (a GeoPackage), optionally with extra attributes." );
}

QgsPdalExportVectorAlgorithm *QgsPdalExportVectorAlgorithm::createInstance() const
{
  return new QgsPdalExportVectorAlgorithm();
}

void QgsPdalExportVectorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterPointCloudAttribute( QStringLiteral( "ATTRIBUTE" ), QObject::tr( "Attribute" ), QVariant(), QStringLiteral( "INPUT" ), true, true ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterVectorDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Exported" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

QStringList QgsPdalExportVectorAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = {
    QStringLiteral( "to_vector" ),
    QStringLiteral( "--input=%1" ).arg( layer->source() ),
    QStringLiteral( "--output=%1" ).arg( outputFile ),
  };

  if ( parameters.value( QStringLiteral( "ATTRIBUTE" ) ).isValid() )
  {
    const QStringList attributes = parameterAsStrings( parameters, QStringLiteral( "ATTRIBUTE" ), context );
    for ( const QString &attr : attributes )
    {
      args << QStringLiteral( "--attribute=%1" ).arg( attr );
    }
  }

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
