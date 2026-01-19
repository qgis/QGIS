/***************************************************************************
                         qgsalgorithmpdalcompare.cpp
                         ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Jan Caha
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

#include "qgsalgorithmpdalcompare.h"

#include <memory>

#include "qgspointcloudlayer.h"
#include "qgsprocessingutils.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalCompareAlgorithm::name() const
{
  return u"compare"_s;
}

QString QgsPdalCompareAlgorithm::displayName() const
{
  return QObject::tr( "Compare point clouds" );
}

QString QgsPdalCompareAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalCompareAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalCompareAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,export,compare" ).split( ',' );
}

QString QgsPdalCompareAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm compares two point clouds using M3C2 algorithm and outputs a point cloud." )
         + u"\n\n"_s
         + QObject::tr( "Resulting point clouds will have several new dimensions: m3c2_distance, m3c2_uncertainty, m3c2_significant, m3c2_std_dev1, m3c2_std_dev2, m3c2_count1 and m3c2_count2." );
}

QString QgsPdalCompareAlgorithm::shortDescription() const
{
  return QObject::tr( "Compares two point clouds using M3C2 algorithm and outputs a point cloud." );
}

QgsPdalCompareAlgorithm *QgsPdalCompareAlgorithm::createInstance() const
{
  return new QgsPdalCompareAlgorithm();
}

void QgsPdalCompareAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT-COMPARE"_s, QObject::tr( "Compare layer" ) ) );

  auto subsamplingCellSize = std::make_unique<QgsProcessingParameterNumber>( u"SUBSAMPLING-CELL-SIZE"_s, QObject::tr( "Subsampling Cell Size" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  subsamplingCellSize->setHelp( QObject::tr( "Minimum spacing between points (in map units)." ) );
  addParameter( subsamplingCellSize.release() );

  auto normalRadius = std::make_unique<QgsProcessingParameterNumber>( u"NORMAL-RADIUS"_s, QObject::tr( "Normal Radius" ), Qgis::ProcessingNumberParameterType::Double, 2.0 );
  normalRadius->setHelp( QObject::tr( "Radius of the sphere around each core point that defines the neighbors from which normals are calculated." ) );
  addParameter( normalRadius.release() );

  auto cylRadius = std::make_unique<QgsProcessingParameterNumber>( u"CYL-RADIUS"_s, QObject::tr( "Cylinder Radius" ), Qgis::ProcessingNumberParameterType::Double, 2.0 );
  cylRadius->setHelp( QObject::tr( "Radius of the cylinder inside of which points are searched for when calculating change." ) );
  addParameter( cylRadius.release() );

  auto cylHalflen = std::make_unique<QgsProcessingParameterNumber>( u"CYL-HALFLEN"_s, QObject::tr( "Cylinder Half-Length" ), Qgis::ProcessingNumberParameterType::Double, 5.0 );
  cylHalflen->setHelp( QObject::tr( "The half-length of the cylinder of neighbors used for calculating change." ) );
  addParameter( cylHalflen.release() );

  auto regError = std::make_unique<QgsProcessingParameterNumber>( u"REG-ERROR"_s, QObject::tr( "Registration Error" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  regError->setHelp( QObject::tr( "The estimated registration error between the two point clouds." ) );
  addParameter( regError.release() );

  auto cylOrientation = std::make_unique<QgsProcessingParameterEnum>( u"CYL-ORIENTATION"_s, QObject::tr( "Cylinder Orientation" ), QStringList() << u"up"_s << u"origin"_s << u"none"_s, false, u"up"_s );
  cylOrientation->setHelp( QObject::tr( "Which direction to orient the cylinder/normal vector used for comparison between the two point clouds." ) );
  addParameter( cylOrientation.release() );

  createCommonParameters();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Comparison Point Cloud" ) ) );
}

bool QgsPdalCompareAlgorithm::checkParameterValues( const QVariantMap &parameters, QgsProcessingContext &context, QString *message ) const
{
  QgsPointCloudLayer *inputLayer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  QgsPointCloudLayer *inputCompareLayer = parameterAsPointCloudLayer( parameters, u"INPUT-COMPARE"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputCompareLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT-COMPARE"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );

  if ( inputLayer->source().endsWith( ".vpc"_L1 ) )
  {
    *message = QObject::tr( "This algorithm does not support VPC files as the Input layer." );
    return false;
  }

  if ( inputCompareLayer->source().endsWith( ".vpc"_L1 ) )
  {
    *message = QObject::tr( "This algorithm does not support VPC files as the Compare layer." );
    return false;
  }

  if ( outputName.endsWith( ".vpc"_L1 ) )
  {
    *message = QObject::tr( "This algorithm does not support VPC files as the Output layer." );
    return false;
  }

  return QgsProcessingAlgorithm::checkParameterValues( parameters, context, message );
}

QStringList QgsPdalCompareAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *inputLayer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  QgsPointCloudLayer *inputCompareLayer = parameterAsPointCloudLayer( parameters, u"INPUT-COMPARE"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputCompareLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT-COMPARE"_s ) );

  const double subsamplingCellSize = parameterAsDouble( parameters, u"SUBSAMPLING-CELL-SIZE"_s, context );
  const double normalRadius = parameterAsDouble( parameters, u"NORMAL-RADIUS"_s, context );
  const double cylRadius = parameterAsDouble( parameters, u"CYL-RADIUS"_s, context );
  const double cylHalflen = parameterAsDouble( parameters, u"CYL-HALFLEN"_s, context );
  const double regError = parameterAsDouble( parameters, u"REG-ERROR"_s, context );
  const QString cylOrientation = parameterAsEnumString( parameters, u"CYL-ORIENTATION"_s, context );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( inputLayer->source(), outputName, context );
  checkOutputFormat( inputLayer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args = { u"compare"_s, u"--input=%1"_s.arg( inputLayer->source() ), u"--input-compare=%1"_s.arg( inputCompareLayer->source() ), u"--output=%1"_s.arg( outputFile ), u"--subsampling-cell-size=%1"_s.arg( subsamplingCellSize ), u"--normal-radius=%1"_s.arg( normalRadius ), u"--cyl-radius=%1"_s.arg( cylRadius ), u"--cyl-halflen=%1"_s.arg( cylHalflen ), u"--reg-error=%1"_s.arg( regError ), u"--cyl-orientation=%1"_s.arg( cylOrientation ) };

  applyCommonParameters( args, inputLayer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
