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

#include "qgsalignraster.h"
#include "qgsalignrasterdata.h"
#include "qgsprocessingparameteralignrasterlayers.h"

///@cond PRIVATE

Qgis::ProcessingAlgorithmFlags QgsAlignRastersAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::HideFromModeler;
}

QString QgsAlignRastersAlgorithm::name() const
{
  return u"alignrasters"_s;
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
  return u"rastertools"_s;
}

QString QgsAlignRastersAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm aligns rasters by resampling them to the same cell size and reprojecting to the same CRS." );
}

QString QgsAlignRastersAlgorithm::shortDescription() const
{
  return QObject::tr( "Aligns rasters by resampling them to the same cell size and reprojecting to the same CRS." );
}

QgsAlignRastersAlgorithm *QgsAlignRastersAlgorithm::createInstance() const
{
  return new QgsAlignRastersAlgorithm();
}

bool QgsAlignRastersAlgorithm::checkParameterValues( const QVariantMap &parameters, QgsProcessingContext &context, QString *message ) const
{
  const QVariant layersVariant = parameters.value( parameterDefinition( u"LAYERS"_s )->name() );
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
  addParameter( new QgsProcessingParameterAlignRasterLayers( u"LAYERS"_s, QObject::tr( "Input layers" ) ) );
  addParameter( new QgsProcessingParameterRasterLayer( u"REFERENCE_LAYER"_s, QObject::tr( "Reference layer" ) ) );

  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Override reference CRS" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterNumber( u"CELL_SIZE_X"_s, QObject::tr( "Override reference cell size X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( u"CELL_SIZE_Y"_s, QObject::tr( "Override reference cell size Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( u"GRID_OFFSET_X"_s, QObject::tr( "Override reference grid offset X" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterNumber( u"GRID_OFFSET_Y"_s, QObject::tr( "Override reference grid offset Y" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-9 ) );
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Clip to extent" ), QVariant(), true ) );

  addOutput( new QgsProcessingOutputMultipleLayers( u"OUTPUT_LAYERS"_s, QObject::tr( "Aligned rasters" ) ) );
}

QVariantMap QgsAlignRastersAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsRasterLayer *referenceLayer = parameterAsRasterLayer( parameters, u"REFERENCE_LAYER"_s, context );
  if ( !referenceLayer )
    throw QgsProcessingException( invalidRasterError( parameters, u"REFERENCE_LAYER"_s ) );

  const QVariant layersVariant = parameters.value( parameterDefinition( u"LAYERS"_s )->name() );
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
  outputs.insert( u"OUTPUT_LAYERS"_s, outputLayers );
  return outputs;
}

///@endcond
