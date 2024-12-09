/***************************************************************************
                         qgsalgorithmalignrasters.cpp
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

#include "qgsalgorithmalignrasters.h"
#include "qgsprocessingparameteralignrasterlayers.h"
#include "qgsalignraster.h"
#include "qgsalignrasterdata.h"

///@cond PRIVATE

Qgis::ProcessingAlgorithmFlags QgsAlignRastersAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromModeler;
}

QString QgsAlignRastersAlgorithm::name() const
{
  return QStringLiteral( "alignrasters" );
}

QString QgsAlignRastersAlgorithm::displayName() const
{
  return QObject::tr( "Align rasters" );
}

QStringList QgsAlignRastersAlgorithm::tags() const
{
  return QObject::tr( "raster,align,resample,rescale" ).split( ',' );
}

QString QgsAlignRastersAlgorithm::group() const
{
  return QObject::tr( "Raster tools" );
}

QString QgsAlignRastersAlgorithm::groupId() const
{
  return QStringLiteral( "rastertools" );
}

QString QgsAlignRastersAlgorithm::shortHelpString() const
{
  return QObject::tr( "Aligns rasters by resampling them to the same cell size and reprojecting to the same CRS." );
}

QgsAlignRastersAlgorithm *QgsAlignRastersAlgorithm::createInstance() const
{
  return new QgsAlignRastersAlgorithm();
}

bool QgsAlignRastersAlgorithm::checkParameterValues( const QVariantMap &parameters, QgsProcessingContext &context, QString *message ) const
{
  const QVariant layersVariant = parameters.value( parameterDefinition( QStringLiteral( "LAYERS" ) )->name() );
  const QList<QgsAlignRasterData::RasterItem> items = QgsProcessingParameterAlignRasterLayers::parameterAsItems( layersVariant, context );
  bool unconfiguredLayers = false;
  for ( const QgsAlignRasterData::RasterItem &item : items )
  {
    if ( item.outputFilename.isEmpty() )
    {
      unconfiguredLayers = true;
      break;
    }
  }
  if ( unconfiguredLayers )
  {
    *message = QObject::tr( "An output file is not configured for one or more input layers. Configure output files via 'Configure Raster…' under Input layers parameter." );
    return false;
  }
  return QgsProcessingAlgorithm::checkParameterValues( parameters, context );
}

void QgsAlignRastersAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterAlignRasterLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ) ) );
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "REFERENCE_LAYER" ), QObject::tr( "Reference layer" ) ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Override reference CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE_X" ), QObject::tr( "Override reference cell size X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CELL_SIZE_Y" ), QObject::tr( "Override reference cell size Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "GRID_OFFSET_X" ), QObject::tr( "Override reference grid offset X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "GRID_OFFSET_Y" ), QObject::tr( "Override reference grid offset Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Clip to extent" ), QVariant(), true ) );

  addOutput( new QgsProcessingOutputMultipleLayers( QStringLiteral( "OUTPUT_LAYERS" ), QObject::tr( "Aligned rasters" ) ) );
}

QVariantMap QgsAlignRastersAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, QStringLiteral( "REFERENCE_LAYER" ), context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "REFERENCE_LAYER" ) ) );

  const QVariant layersVariant = parameters.value( parameterDefinition( QStringLiteral( "LAYERS" ) )->name() );
  const QList<QgsAlignRasterData::RasterItem> items = QgsProcessingParameterAlignRasterLayers::parameterAsItems( layersVariant, context );
  QStringList outputLayers;
  outputLayers.reserve( items.size() );
  for ( const QgsAlignRasterData::RasterItem &item : items )
  {
    outputLayers << item.outputFilename;
  }

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
  outputs.insert( QStringLiteral( "OUTPUT_LAYERS" ), outputLayers );
  return outputs;
}

///@endcond
