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
  return u"transformpointcloud"_s;
}

QString QgsPdalTransformAlgorithm::displayName() const
{
  return QObject::tr( "Transform point cloud" );
}

QString QgsPdalTransformAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalTransformAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalTransformAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,assign,set,transform,coordinates" ).split( ',' );
}

QString QgsPdalTransformAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transforms point cloud coordinates using translation, rotation, and scaling operations using a 4x4 transformation matrix." )
         + u"\n\n"_s
         + QObject::tr( "The algorithm applies transformations in the following order: scaling, rotation (using Euler angles), then translation." )
         + u"\n\n"_s
         + QObject::tr( "Rotation angles are specified in degrees around the X, Y, and Z axes." )
         + u"\n\n"_s
         + QObject::tr( "All parameters are combined into a 4×4 transformation matrix that is passed to PDAL Wrench." );
}

QString QgsPdalTransformAlgorithm::shortDescription() const
{
  return QObject::tr( "Transforms point cloud coordinates using translation, rotation, and scaling using 4x4 matrix." );
}

QgsPdalTransformAlgorithm *QgsPdalTransformAlgorithm::createInstance() const
{
  return new QgsPdalTransformAlgorithm();
}

void QgsPdalTransformAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterNumber( u"TRANSLATE_X"_s, QObject::tr( "X Translation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"TRANSLATE_Y"_s, QObject::tr( "Y Translation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"TRANSLATE_Z"_s, QObject::tr( "Z Translation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"SCALE_X"_s, QObject::tr( "X Scale" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"SCALE_Y"_s, QObject::tr( "Y Scale" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"SCALE_Z"_s, QObject::tr( "Z Scale" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"ROTATE_X"_s, QObject::tr( "X Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"ROTATE_Y"_s, QObject::tr( "Y Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );
  addParameter( new QgsProcessingParameterNumber( u"ROTATE_Z"_s, QObject::tr( "Z Rotation" ), Qgis::ProcessingNumberParameterType::Double, 0.0 ) );

  createVpcOutputFormatParameter();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Transformed" ) ) );
}

QStringList QgsPdalTransformAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  const double translateX = parameterAsDouble( parameters, u"TRANSLATE_X"_s, context );
  const double translateY = parameterAsDouble( parameters, u"TRANSLATE_Y"_s, context );
  const double translateZ = parameterAsDouble( parameters, u"TRANSLATE_Z"_s, context );
  const double scaleX = parameterAsDouble( parameters, u"SCALE_X"_s, context );
  const double scaleY = parameterAsDouble( parameters, u"SCALE_Y"_s, context );
  const double scaleZ = parameterAsDouble( parameters, u"SCALE_Z"_s, context );
  const float rotateX = static_cast<float>( parameterAsDouble( parameters, u"ROTATE_X"_s, context ) );
  const float rotateY = static_cast<float>( parameterAsDouble( parameters, u"ROTATE_Y"_s, context ) );
  const float rotateZ = static_cast<float>( parameterAsDouble( parameters, u"ROTATE_Z"_s, context ) );

  const QMatrix3x3 rotation3x3 = QQuaternion::fromEulerAngles( rotateX, rotateY, rotateZ ).toRotationMatrix();

  const QgsMatrix4x4 rotateMatrix = QgsMatrix4x4( rotation3x3( 0, 0 ), rotation3x3( 0, 1 ), rotation3x3( 0, 2 ), 0, rotation3x3( 1, 0 ), rotation3x3( 1, 1 ), rotation3x3( 1, 2 ), 0, rotation3x3( 2, 0 ), rotation3x3( 2, 1 ), rotation3x3( 2, 2 ), 0, 0, 0, 0, 1 );

  const QgsMatrix4x4 translateMatrix = QgsMatrix4x4( 1, 0, 0, translateX, 0, 1, 0, translateY, 0, 0, 1, translateZ, 0, 0, 0, 1 );

  const QgsMatrix4x4 scaleMatrix = QgsMatrix4x4( scaleX, 0, 0, 0, 0, scaleY, 0, 0, 0, 0, scaleZ, 0, 0, 0, 0, 1 );

  QgsMatrix4x4 transformMatrix = translateMatrix * rotateMatrix * scaleMatrix;

  const QString transformString = u"%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16"_s
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

  QStringList args = { u"translate"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--transform-matrix=%1"_s.arg( transformString ) };

  applyVpcOutputFormatParameter( outputFile, args, parameters, context, feedback );
  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
