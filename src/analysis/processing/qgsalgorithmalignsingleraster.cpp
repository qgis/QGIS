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

#include "qgis.h"
#include "qgsalignraster.h"
#include "qgsalignrasterdata.h"
#include "qgsprocessingparameteralignrasterlayers.h"

///@cond PRIVATE

Qgis::ProcessingAlgorithmFlags QgsAlignSingleRasterAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromToolbox;
}

QString QgsAlignSingleRasterAlgorithm::name() const
{
  return u"alignsingleraster"_s;
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
  return u"rastertools"_s;
}

QString QgsAlignSingleRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm aligns raster by resampling it to the same cell size and reprojecting to the same CRS as a reference raster." );
}

QString QgsAlignSingleRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Aligns raster by resampling it to the same cell size and reprojecting to the same CRS as a reference raster." );
}

QgsAlignSingleRasterAlgorithm *QgsAlignSingleRasterAlgorithm::createInstance() const
{
  return new QgsAlignSingleRasterAlgorithm();
}

void QgsAlignSingleRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

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
  addParameter( new QgsProcessingParameterEnum( u"RESAMPLING_METHOD"_s, QObject::tr( "Resampling method" ), resamplingMethods, false, 0, false ) );
  addParameter( new QgsProcessingParameterBoolean( u"RESCALE"_s, QObject::tr( "Rescale values according to the cell size" ), false ) );
  addParameter( new QgsProcessingParameterRasterLayer( u"REFERENCE_LAYER"_s, QObject::tr( "Reference layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Override reference CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber( u"CELL_SIZE_X"_s, QObject::tr( "Override reference cell size X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( u"CELL_SIZE_Y"_s, QObject::tr( "Override reference cell size Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( u"GRID_OFFSET_X"_s, QObject::tr( "Override reference grid offset X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( u"GRID_OFFSET_Y"_s, QObject::tr( "Override reference grid offset Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Clip to extent" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Aligned raster" ) ) );
}


QVariantMap QgsAlignSingleRasterAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *inputLayer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  if ( !inputLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, u"REFERENCE_LAYER"_s, context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"REFERENCE_LAYER"_s ) );

  const int method = parameterAsInt( parameters, u"RESAMPLING_METHOD"_s, context );
  const bool rescale = parameterAsBoolean( parameters, u"RESCALE"_s, context );
  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );

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

  if ( parameters.value( u"CRS"_s ).isValid() )
  {
    QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );
    customCRSWkt = crs.toWkt( Qgis::CrsWktVariant::PreferredGdal );
  }

  bool hasXValue = parameters.value( u"CELL_SIZE_X"_s ).isValid();
  bool hasYValue = parameters.value( u"CELL_SIZE_Y"_s ).isValid();
  if ( ( hasXValue && !hasYValue ) || ( !hasXValue && hasYValue ) )
  {
    throw QgsProcessingException( QObject::tr( "Either set both X and Y cell size values or keep both as 'Not set'." ) );
  }
  else if ( hasXValue && hasYValue )
  {
    double xSize = parameterAsDouble( parameters, u"CELL_SIZE_X"_s, context );
    double ySize = parameterAsDouble( parameters, u"CELL_SIZE_Y"_s, context );
    customCellSize = QSizeF( xSize, ySize );
  }

  hasXValue = parameters.value( u"GRID_OFFSET_X"_s ).isValid();
  hasYValue = parameters.value( u"GRID_OFFSET_Y"_s ).isValid();
  if ( ( hasXValue && !hasYValue ) || ( !hasXValue && hasYValue ) )
  {
    throw QgsProcessingException( QObject::tr( "Either set both X and Y grid offset values or keep both as 'Not set'." ) );
  }
  else if ( hasXValue && hasYValue )
  {
    double xSize = parameterAsDouble( parameters, u"GRID_OFFSET_X"_s, context );
    double ySize = parameterAsDouble( parameters, u"GRID_OFFSET_Y"_s, context );
    customGridOffset = QPointF( xSize, ySize );
  }

  if ( parameters.value( u"EXTENT"_s ).isValid() )
  {
    QgsRectangle extent = parameterAsExtent( parameters, u"EXTENT"_s, context );
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
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
