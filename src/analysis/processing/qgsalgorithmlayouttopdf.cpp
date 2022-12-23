/***************************************************************************
                         qgsalgorithmlayouttopdf.cpp
                         ---------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmlayouttopdf.h"
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"
#include "qgsprocessingoutputs.h"
#include "qgslayoutexporter.h"

///@cond PRIVATE

QString QgsLayoutToPdfAlgorithm::name() const
{
  return QStringLiteral( "printlayouttopdf" );
}

QString QgsLayoutToPdfAlgorithm::displayName() const
{
  return QObject::tr( "Export print layout as PDF" );
}

QStringList QgsLayoutToPdfAlgorithm::tags() const
{
  return QObject::tr( "layout,composer,composition,save" ).split( ',' );
}

QString QgsLayoutToPdfAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsLayoutToPdfAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

QString QgsLayoutToPdfAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a print layout as a PDF." );
}

QString QgsLayoutToPdfAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm outputs a print layout as a PDF file." );
}

void QgsLayoutToPdfAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterLayout( QStringLiteral( "LAYOUT" ), QObject::tr( "Print layout" ) ) );

  std::unique_ptr< QgsProcessingParameterMultipleLayers > layersParam = std::make_unique< QgsProcessingParameterMultipleLayers>( QStringLiteral( "LAYERS" ), QObject::tr( "Map layers to assign to unlocked map item(s)" ), QgsProcessing::TypeMapLayer, QVariant(), true );
  layersParam->setFlags( layersParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( layersParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > dpiParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DPI" ), QObject::tr( "DPI (leave blank for default layout DPI)" ), QgsProcessingParameterNumber::Double, QVariant(), true, 0 );
  dpiParam->setFlags( dpiParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( dpiParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > forceVectorParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "FORCE_VECTOR" ), QObject::tr( "Always export as vectors" ), false );
  forceVectorParam->setFlags( forceVectorParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( forceVectorParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > forceRasterParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "FORCE_RASTER" ), QObject::tr( "Always export as raster" ), false );
  forceRasterParam->setFlags( forceRasterParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( forceRasterParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > appendGeorefParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "GEOREFERENCE" ), QObject::tr( "Append georeference information" ), true );
  appendGeorefParam->setFlags( appendGeorefParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( appendGeorefParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > exportRDFParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "INCLUDE_METADATA" ), QObject::tr( "Export RDF metadata (title, author, etc.)" ), true );
  exportRDFParam->setFlags( exportRDFParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( exportRDFParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > disableTiled = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "DISABLE_TILED" ), QObject::tr( "Disable tiled raster layer exports" ), false );
  disableTiled->setFlags( disableTiled->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( disableTiled.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > simplify = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "SIMPLIFY" ), QObject::tr( "Simplify geometries to reduce output file size" ), true );
  simplify->setFlags( simplify->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( simplify.release() );

  const QStringList textExportOptions
  {
    QObject::tr( "Always Export Text as Paths (Recommended)" ),
    QObject::tr( "Always Export Text as Text Objects" )
  };

  std::unique_ptr< QgsProcessingParameterEnum > textFormat = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "TEXT_FORMAT" ), QObject::tr( "Text export" ), textExportOptions, false, 0 );
  textFormat->setFlags( textFormat->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( textFormat.release() );

  const QStringList imageCompressionOptions
  {
    QObject::tr( "Lossy (JPEG)" ),
    QObject::tr( "Lossless" )
  };

  std::unique_ptr< QgsProcessingParameterEnum > imageCompression = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "IMAGE_COMPRESSION" ), QObject::tr( "Image compression" ), imageCompressionOptions, false, 0 );
  imageCompression->setFlags( imageCompression->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( imageCompression.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > layeredExport = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "SEPARATE_LAYERS" ), QObject::tr( "Export layers as separate PDF files" ), false );
  layeredExport->setFlags( layeredExport->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( layeredExport.release() );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "PDF file" ), QObject::tr( "PDF Format" ) + " (*.pdf *.PDF)" ) );
}

QgsProcessingAlgorithm::Flags QgsLayoutToPdfAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagNoThreading;
}

QgsLayoutToPdfAlgorithm *QgsLayoutToPdfAlgorithm::createInstance() const
{
  return new QgsLayoutToPdfAlgorithm();
}

QVariantMap QgsLayoutToPdfAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // this needs to be done in main thread, layouts are not thread safe
  QgsPrintLayout *l = parameterAsLayout( parameters, QStringLiteral( "LAYOUT" ), context );
  if ( !l )
    throw QgsProcessingException( QObject::tr( "Cannot find layout with name \"%1\"" ).arg( parameters.value( QStringLiteral( "LAYOUT" ) ).toString() ) );
  std::unique_ptr< QgsPrintLayout > layout( l->clone() );

  const QList< QgsMapLayer * > layers = parameterAsLayerList( parameters, QStringLiteral( "LAYERS" ), context );
  if ( layers.size() > 0 )
  {
    const QList<QGraphicsItem *> items = layout->items();
    for ( QGraphicsItem *graphicsItem : items )
    {
      QgsLayoutItem *item = dynamic_cast<QgsLayoutItem *>( graphicsItem );
      QgsLayoutItemMap *map = dynamic_cast<QgsLayoutItemMap *>( item );
      if ( map && !map->followVisibilityPreset() && !map->keepLayerSet() )
      {
        map->setKeepLayerSet( true );
        map->setLayers( layers );
      }
    }
  }

  const QString dest = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );

  QgsLayoutExporter exporter( layout.get() );
  QgsLayoutExporter::PdfExportSettings settings;

  if ( parameters.value( QStringLiteral( "DPI" ) ).isValid() )
  {
    settings.dpi = parameterAsDouble( parameters, QStringLiteral( "DPI" ), context );
  }

  settings.forceVectorOutput = parameterAsBool( parameters, QStringLiteral( "FORCE_VECTOR" ), context );
  settings.rasterizeWholeImage = parameterAsBool( parameters, QStringLiteral( "FORCE_RASTER" ), context );
  settings.appendGeoreference = parameterAsBool( parameters, QStringLiteral( "GEOREFERENCE" ), context );
  settings.exportMetadata = parameterAsBool( parameters, QStringLiteral( "INCLUDE_METADATA" ), context );
  settings.simplifyGeometries = parameterAsBool( parameters, QStringLiteral( "SIMPLIFY" ), context );
  settings.textRenderFormat = parameterAsEnum( parameters, QStringLiteral( "TEXT_FORMAT" ), context ) == 0 ? Qgis::TextRenderFormat::AlwaysOutlines : Qgis::TextRenderFormat::AlwaysText;
  settings.exportLayersAsSeperateFiles = parameterAsBool( parameters, QStringLiteral( "SEPARATE_LAYERS" ), context );  //#spellok

  if ( parameterAsBool( parameters, QStringLiteral( "DISABLE_TILED" ), context ) )
    settings.flags = settings.flags | QgsLayoutRenderContext::FlagDisableTiledRasterLayerRenders;
  else
    settings.flags = settings.flags & ~QgsLayoutRenderContext::FlagDisableTiledRasterLayerRenders;

  if ( parameterAsEnum( parameters, QStringLiteral( "IMAGE_COMPRESSION" ), context ) == 1 )
    settings.flags = settings.flags | QgsLayoutRenderContext::FlagLosslessImageRendering;
  else
    settings.flags = settings.flags & ~QgsLayoutRenderContext::FlagLosslessImageRendering;

  switch ( exporter.exportToPdf( dest, settings ) )
  {
    case QgsLayoutExporter::Success:
    {
      feedback->pushInfo( QObject::tr( "Successfully exported layout to %1" ).arg( QDir::toNativeSeparators( dest ) ) );
      break;
    }

    case QgsLayoutExporter::FileError:
      throw QgsProcessingException( QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( dest ) ) );

    case QgsLayoutExporter::PrintError:
      throw QgsProcessingException( QObject::tr( "Could not create print device." ) );

    case QgsLayoutExporter::MemoryError:
      throw QgsProcessingException( QObject::tr( "Exporting the PDF "
                                    "resulted in a memory overflow.\n\n"
                                    "Please try a lower resolution or a smaller paper size." ) );

    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::IteratorError:
    case QgsLayoutExporter::Canceled:
      // no meaning for PDF exports, will not be encountered
      break;
  }

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond

