/***************************************************************************
                         qgsalgorithmpdaltransform.cpp
                         ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdaltransform.h"

#include "qgsmatrix4x4.h"
#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

#include <QMatrix3x3>
#include <QQuaternion>

///@cond PRIVATE

QString QgsPdalTransformAlgorithm::name() const
{
  return QStringLiteral( "transformcoordinates" );
}

QString QgsPdalTransformAlgorithm::displayName() const
{
  return QObject::tr( "Transform coordinates of point cloud" );
}

QString QgsPdalTransformAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalTransformAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalTransformAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,assign,set,transform,coordinates" ).split( ',' );
}

QString QgsPdalTransformAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transforms coordinates of point cloud using 4x4 matrix." );
}

QString QgsPdalTransformAlgorithm::shortDescription() const
{
  return QObject::tr( "This algorithm transforms coordinates of point cloud using 4x4 matrix." );
}

QgsPdalTransformAlgorithm *QgsPdalTransformAlgorithm::createInstance() const
{
  return new QgsPdalTransformAlgorithm();
}

void QgsPdalTransformAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TRANSLATE_X" ), QObject::tr( "X Translation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TRANSLATE_Y" ), QObject::tr( "Y Translation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TRANSLATE_Z" ), QObject::tr( "Z Translation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SCALE_X" ), QObject::tr( "X Scale" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SCALE_Y" ), QObject::tr( "Y Scale" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SCALE_Z" ), QObject::tr( "Z Scale" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ROTATE_X" ), QObject::tr( "X Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ROTATE_Y" ), QObject::tr( "Y Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ROTATE_Z" ), QObject::tr( "Z Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );

  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Transformed" ) ) );
}

QStringList QgsPdalTransformAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  const double translateX = parameterAsDouble( parameters, QStringLiteral( "TRANSLATE_X" ), context );
  const double translateY = parameterAsDouble( parameters, QStringLiteral( "TRANSLATE_Y" ), context );
  const double translateZ = parameterAsDouble( parameters, QStringLiteral( "TRANSLATE_Z" ), context );
  const double scaleX = parameterAsDouble( parameters, QStringLiteral( "SCALE_X" ), context );
  const double scaleY = parameterAsDouble( parameters, QStringLiteral( "SCALE_Y" ), context );
  const double scaleZ = parameterAsDouble( parameters, QStringLiteral( "SCALE_Z" ), context );
  const float rotateX = static_cast<float>( parameterAsDouble( parameters, QStringLiteral( "ROTATE_X" ), context ) );
  const float rotateY = static_cast<float>( parameterAsDouble( parameters, QStringLiteral( "ROTATE_Y" ), context ) );
  const float rotateZ = static_cast<float>( parameterAsDouble( parameters, QStringLiteral( "ROTATE_Z" ), context ) );

  const QMatrix3x3 rotation3x3 = QQuaternion::fromEulerAngles( rotateX, rotateY, rotateZ ).toRotationMatrix();

  const QgsMatrix4x4 rotateMatrix = QgsMatrix4x4( rotation3x3( 0, 0 ), rotation3x3( 0, 1 ), rotation3x3( 0, 2 ), 0, rotation3x3( 1, 0 ), rotation3x3( 1, 1 ), rotation3x3( 1, 2 ), 0, rotation3x3( 2, 0 ), rotation3x3( 2, 1 ), rotation3x3( 2, 2 ), 0, 0, 0, 0, 1 );

  const QgsMatrix4x4 translateMatrix = QgsMatrix4x4( 1, 0, 0, translateX, 0, 1, 0, translateY, 0, 0, 1, translateZ, 0, 0, 0, 1 );

  const QgsMatrix4x4 scaleMatrix = QgsMatrix4x4( scaleX, 0, 0, 0, 0, scaleY, 0, 0, 0, 0, scaleZ, 0, 0, 0, 0, 1 );

  QgsMatrix4x4 transformMatrix = translateMatrix * rotateMatrix * scaleMatrix;

  const QString transformString = QStringLiteral( "%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16" )
                                    .arg( transformMatrix.data()[0] )
                                    .arg( transformMatrix.data()[4] )
                                    .arg( transformMatrix.data()[8] )
                                    .arg( transformMatrix.data()[12] )
                                    .arg( transformMatrix.data()[1] )
                                    .arg( transformMatrix.data()[5] )
                                    .arg( transformMatrix.data()[9] )
                                    .arg( transformMatrix.data()[13] )
                                    .arg( transformMatrix.data()[2] )
                                    .arg( transformMatrix.data()[6] )
                                    .arg( transformMatrix.data()[10] )
                                    .arg( transformMatrix.data()[14] )
                                    .arg( transformMatrix.data()[3] )
                                    .arg( transformMatrix.data()[7] )
                                    .arg( transformMatrix.data()[11] )
                                    .arg( transformMatrix.data()[15] );

  QStringList args = { QStringLiteral( "translate" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--transform-matrix=%1" ).arg( transformString ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
