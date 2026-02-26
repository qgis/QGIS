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

#include <QString>

using namespace Qt::StringLiterals;

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
  return QObject::tr( "This algorithm compares two point clouds using an M3C2 (Multiscale Model-to-Model Cloud Comparison) algorithm and outputs a subset (filtered using Poisson sampling, based on Subsampling cell size parameter ) of the original point cloud." )
         + u"\n\n"_s
         + QObject::tr( "The output point cloud will have several new dimensions: m3c2_distance, m3c2_uncertainty, m3c2_significant, m3c2_std_dev1, m3c2_std_dev2, m3c2_count1 and m3c2_count2." )
         + u"\n\n"_s
         + QObject::tr( "The M3C2 algorithm calculates the distance between two point clouds by considering the local orientation of the surface. It estimates surface normals at a user-defined scale and measures the average distance between clouds within a cylindrical projection along those normals." )
         + u"\n\n"_s
         + QObject::tr( "The approach is highly robust against sensor noise and surface roughness, making it the standard for detecting change in complex natural environments. It also provides a sign (indicating whether a surface has moved in or out) and a statistically significant level of detection to distinguish real change from measurement error." )
         + u"\n\n"_s
         + QObject::tr( "References: Lague, Dimitri, Nicolas Brodu, and Jérôme Leroux. Accurate 3D Comparison of Complex Topography with Terrestrial Laser Scanner: Application to the Rangitikei Canyon (N-Z). arXiv, 2013, https://arxiv.org/abs/1302.1183." );
  ;
}

QString QgsPdalCompareAlgorithm::shortDescription() const
{
  return QObject::tr( "Compares two point clouds using M3C2 algorithm and outputs a subsample of the input point cloud containing additional dimensions describing the differences." );
}

QgsPdalCompareAlgorithm *QgsPdalCompareAlgorithm::createInstance() const
{
  return new QgsPdalCompareAlgorithm();
}

void QgsPdalCompareAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT_COMPARE"_s, QObject::tr( "Compare layer" ) ) );

  auto subsamplingCellSize = std::make_unique<QgsProcessingParameterDistance>( u"SUBSAMPLING_CELL_SIZE"_s, QObject::tr( "Subsampling cell size" ), 0.0, u"INPUT"_s );
  subsamplingCellSize->setHelp( QObject::tr( "Minimum spacing between points (in map units)." ) );
  addParameter( subsamplingCellSize.release() );

  auto normalRadius = std::make_unique<QgsProcessingParameterDistance>( u"NORMAL_RADIUS"_s, QObject::tr( "Normal Radius" ), 2.0, u"INPUT"_s );
  normalRadius->setHelp( QObject::tr( "Radius of the sphere around each core point that defines the neighbors from which normals are calculated." ) );
  addParameter( normalRadius.release() );

  auto cylRadius = std::make_unique<QgsProcessingParameterDistance>( u"CYLINDER_RADIUS"_s, QObject::tr( "Cylinder Radius" ), 2.0, u"INPUT"_s );
  cylRadius->setHelp( QObject::tr( "Radius of the cylinder inside of which points are searched for when calculating change." ) );
  addParameter( cylRadius.release() );

  auto cylHalflen = std::make_unique<QgsProcessingParameterDistance>( u"CYLINDER_HALF_LENGTH"_s, QObject::tr( "Cylinder Half-Length" ), 5.0, u"INPUT"_s );
  cylHalflen->setHelp( QObject::tr( "The half-length of the cylinder of neighbors used for calculating change." ) );
  addParameter( cylHalflen.release() );

  auto regError = std::make_unique<QgsProcessingParameterNumber>( u"REGISTRATION_ERROR"_s, QObject::tr( "Registration Error" ), Qgis::ProcessingNumberParameterType::Double, 0.0 );
  regError->setHelp( QObject::tr( "The estimated registration error between the two point clouds." ) );
  addParameter( regError.release() );

  auto cylOrientation = std::make_unique<QgsProcessingParameterEnum>( u"CYLINDER_ORIENTATION"_s, QObject::tr( "Cylinder Orientation" ), QStringList() << QObject::tr( "Up" ) << QObject::tr( "Origin" ) << QObject::tr( "None" ), false, 0 );
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

  QgsPointCloudLayer *inputCompareLayer = parameterAsPointCloudLayer( parameters, u"INPUT_COMPARE"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputCompareLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT_COMPARE"_s ) );

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
// raise exception if PDAL version is older than 2.10 - can be removed when PDAL 2.10 is minimum requirement
#ifdef HAVE_PDAL_QGIS
#if PDAL_VERSION_MAJOR_INT < 2 || ( PDAL_VERSION_MAJOR_INT == 2 && PDAL_VERSION_MINOR_INT < 10 )
  throw QgsProcessingException( QObject::tr( "This algorithm requires PDAL version 2.10 or higher." ) );
#endif
#endif

  Q_UNUSED( feedback );

  QgsPointCloudLayer *inputLayer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  QgsPointCloudLayer *inputCompareLayer = parameterAsPointCloudLayer( parameters, u"INPUT_COMPARE"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !inputCompareLayer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT_COMPARE"_s ) );

  const double subsamplingCellSize = parameterAsDouble( parameters, u"SUBSAMPLING_CELL_SIZE"_s, context );
  const double normalRadius = parameterAsDouble( parameters, u"NORMAL_RADIUS"_s, context );
  const double cylRadius = parameterAsDouble( parameters, u"CYLINDER_RADIUS"_s, context );
  const double cylHalflen = parameterAsDouble( parameters, u"CYLINDER_HALF_LENGTH"_s, context );
  const double regError = parameterAsDouble( parameters, u"REGISTRATION_ERROR"_s, context );
  const int cylOrientationIndex = parameterAsEnum( parameters, u"CYLINDER_ORIENTATION"_s, context );
  const QString cylOrientation = mCylinderOrientationOptions.at( cylOrientationIndex );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( inputLayer->source(), outputName, context );
  checkOutputFormat( inputLayer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args = { u"compare"_s, u"--input=%1"_s.arg( inputLayer->source() ), u"--input-compare=%1"_s.arg( inputCompareLayer->source() ), u"--output=%1"_s.arg( outputFile ), u"--subsampling-cell-size=%1"_s.arg( subsamplingCellSize ), u"--normal-radius=%1"_s.arg( normalRadius ), u"--cyl-radius=%1"_s.arg( cylRadius ), u"--cyl-halflen=%1"_s.arg( cylHalflen ), u"--reg-error=%1"_s.arg( regError ), u"--cyl-orientation=%1"_s.arg( cylOrientation.toLower() ) };

  applyCommonParameters( args, inputLayer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
