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

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalExportVectorAlgorithm::name() const
{
  return u"exportvector"_s;
}

QString QgsPdalExportVectorAlgorithm::displayName() const
{
  return QObject::tr( "Export point cloud to vector" );
}

QString QgsPdalExportVectorAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalExportVectorAlgorithm::groupId() const
{
  return u"pointcloudconversion"_s;
}

QStringList QgsPdalExportVectorAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,export,vector,attribute,create" ).split( ',' );
}

QString QgsPdalExportVectorAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports point cloud data to a vector layer with 3D points (a GeoPackage), optionally with extra attributes." );
}

QString QgsPdalExportVectorAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a point cloud to a vector layer with 3D points." );
}

QgsPdalExportVectorAlgorithm *QgsPdalExportVectorAlgorithm::createInstance() const
{
  return new QgsPdalExportVectorAlgorithm();
}

void QgsPdalExportVectorAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterPointCloudAttribute( u"ATTRIBUTE"_s, QObject::tr( "Attribute" ), QVariant(), u"INPUT"_s, true, true ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterVectorDestination( u"OUTPUT"_s, QObject::tr( "Exported" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

QStringList QgsPdalExportVectorAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args = {
    u"to_vector"_s,
    u"--input=%1"_s.arg( layer->source() ),
    u"--output=%1"_s.arg( outputFile ),
  };

  if ( parameters.value( u"ATTRIBUTE"_s ).isValid() )
  {
    const QStringList attributes = parameterAsStrings( parameters, u"ATTRIBUTE"_s, context );
    for ( const QString &attr : attributes )
    {
      args << u"--attribute=%1"_s.arg( attr );
    }
  }

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
