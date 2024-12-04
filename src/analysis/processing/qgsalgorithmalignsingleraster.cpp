/***************************************************************************
                         qgsalgorithmalignsingleraster.cpp
                         ---------------------
    begin                : July 2023
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

#include "qgsalgorithmalignsingleraster.h"
#include "qgsprocessingparameteralignrasterlayers.h"
#include "qgsalignraster.h"
#include "qgsalignrasterdata.h"
#include "qgis.h"

///@cond PRIVATE

Qgis::ProcessingAlgorithmFlags QgsAlignSingleRasterAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromToolbox;
}

QString QgsAlignSingleRasterAlgorithm::name() const
{
  return QStringLiteral( "alignsingleraster" );
}

QString QgsAlignSingleRasterAlgorithm::displayName() const
{
  return QObject::tr( "Align raster" );
}

QStringList QgsAlignSingleRasterAlgorithm::tags() const
{
  return QObject::tr( "raster,align,resample,rescale" ).split( ',' );
}

QString QgsAlignSingleRasterAlgorithm::group() const
{
  return QObject::tr( "Raster tools" );
}

QString QgsAlignSingleRasterAlgorithm::groupId() const
{
  return QStringLiteral( "rastertools" );
}

QString QgsAlignSingleRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "Aligns raster by resampling it to the same cell size and reprojecting to the same CRS as a reference raster." );
}

QgsAlignSingleRasterAlgorithm *QgsAlignSingleRasterAlgorithm::createInstance() const
{
  return new QgsAlignSingleRasterAlgorithm();
}

void QgsAlignSingleRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  QStringList resamplingMethods;
  resamplingMethods << QObject::tr( "Nearest Neighbour" )
                    << QObject::tr( "Bilinear (2x2 Kernel)" )
                    << QObject::tr( "Cubic (4x4 Kernel)" )
                    << QObject::tr( "Cubic B-Spline (4x4 Kernel)" )
                    << QObject::tr( "Lanczos (6x6 Kernel)" )
                    << QObject::tr( "Average" )
                    << QObject::tr( "Mode" )
                    << QObject::tr( "Maximum" )
                    << QObject::tr( "Minimum" )
                    << QObject::tr( "Median" )
                    << QObject::tr( "First Quartile (Q1)" )
                    << QObject::tr( "Third Quartile (Q3)" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "RESAMPLING_METHOD" ), QObject::tr( "Resampling method" ), resamplingMethods, false, 0, false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "RESCALE" ), QObject::tr( "Rescale values according to the cell size" ), false ) );
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "REFERENCE_LAYER" ), QObject::tr( "Reference layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Override reference CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE_X" ), QObject::tr( "Override reference cell size X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE_Y" ), QObject::tr( "Override reference cell size Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "GRID_OFFSET_X" ), QObject::tr( "Override reference grid offset X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "GRID_OFFSET_Y" ), QObject::tr( "Override reference grid offset Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Clip to extent" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Aligned raster" ) ) );
}


QVariantMap QgsAlignSingleRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, QStringLiteral( "REFERENCE_LAYER" ), context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "REFERENCE_LAYER" ) ) );

  const int method = parameterAsInt( parameters, QStringLiteral( "RESAMPLING_METHOD" ), context );
  const bool rescale = parameterAsBoolean( parameters, QStringLiteral( "RESCALE" ), context );
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );

  Qgis::GdalResampleAlgorithm resampleAlg = Qgis::GdalResampleAlgorithm::RA_NearestNeighbour;
  switch ( method )
  {
    case 0:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_NearestNeighbour;
      break;
    case 1:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Bilinear;
      break;
    case 2:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Cubic;
      break;
    case 3:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_CubicSpline;
      break;
    case 4:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Lanczos;
      break;
    case 5:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Average;
      break;
    case 6:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Mode;
      break;
    case 7:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Max;
      break;
    case 8:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Min;
      break;
    case 9:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Median;
      break;
    case 10:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Q1;
      break;
    case 11:
      resampleAlg = Qgis::GdalResampleAlgorithm::RA_Q3;
      break;
    default:
      break;
  }

  QgsAlignRasterData::RasterItem item( inputLayer->source(), outputFile );
  item.resampleMethod = resampleAlg;
  item.rescaleValues = rescale;

  QgsAlignRaster::List items;
  items << item;

  QgsAlignRaster rasterAlign;
  rasterAlign.setRasters( items );

  QString customCRSWkt;
  QSizeF customCellSize;
  QPointF customGridOffset( -1, -1 );

  if ( parameters.value( QStringLiteral( "CRS" ) ).isValid() )
  {
    QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
    customCRSWkt = crs.toWkt( Qgis::CrsWktVariant::PreferredGdal );
  }

  bool hasXValue = parameters.value( QStringLiteral( "CELL_SIZE_X" ) ).isValid();
  bool hasYValue = parameters.value( QStringLiteral( "CELL_SIZE_Y" ) ).isValid();
  if ( ( hasXValue && !hasYValue ) || ( !hasXValue && hasYValue ) )
  {
    throw QgsProcessingException( QObject::tr( "Either set both X and Y cell size values or keep both as 'Not set'." ) );
  }
  else if ( hasXValue && hasYValue )
  {
    double xSize = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE_X" ), context );
    double ySize = parameterAsDouble( parameters, QStringLiteral( "CELL_SIZE_Y" ), context );
    customCellSize = QSizeF( xSize, ySize );
  }

  hasXValue = parameters.value( QStringLiteral( "GRID_OFFSET_X" ) ).isValid();
  hasYValue = parameters.value( QStringLiteral( "GRID_OFFSET_Y" ) ).isValid();
  if ( ( hasXValue && !hasYValue ) || ( !hasXValue && hasYValue ) )
  {
    throw QgsProcessingException( QObject::tr( "Either set both X and Y grid offset values or keep both as 'Not set'." ) );
  }
  else if ( hasXValue && hasYValue )
  {
    double xSize = parameterAsDouble( parameters, QStringLiteral( "GRID_OFFSET_X" ), context );
    double ySize = parameterAsDouble( parameters, QStringLiteral( "GRID_OFFSET_Y" ), context );
    customGridOffset = QPointF( xSize, ySize );
  }

  if ( parameters.value( QStringLiteral( "EXTENT" ) ).isValid() )
  {
    QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context );
    rasterAlign.setClipExtent( extent );
  }

  struct QgsAlignRasterProgress : public QgsAlignRaster::ProgressHandler
  {
      explicit QgsAlignRasterProgress( QgsFeedback *feedback )
        : mFeedback( feedback ) {}
      bool progress( double complete ) override
      {
        mFeedback->setProgress( complete * 100 );
        return true;
      }

    protected:
      QgsFeedback *mFeedback = nullptr;
  };

  rasterAlign.setProgressHandler( new QgsAlignRasterProgress( feedback ) );

  bool result = rasterAlign.setParametersFromRaster( referenceLayer->source(), customCRSWkt, customCellSize, customGridOffset );
  if ( !result )
  {
    throw QgsProcessingException( QObject::tr( "It is not possible to reproject reference raster to target CRS." ) );
  }

  result = rasterAlign.run();
  if ( !result )
  {
    throw QgsProcessingException( QObject::tr( "Failed to align rasters: %1" ).arg( rasterAlign.errorMessage() ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
