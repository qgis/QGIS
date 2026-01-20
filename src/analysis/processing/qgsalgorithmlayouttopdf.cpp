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
#include "qgslayoutexporter.h"
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"
#include "qgsprocessingoutputs.h"

///@cond PRIVATE

QString QgsLayoutToPdfAlgorithm::name() const
{
  return u"printlayouttopdf"_s;
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
  return u"cartography"_s;
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
  addParameter( new QgsProcessingParameterLayout( u"LAYOUT"_s, QObject::tr( "Print layout" ) ) );

  auto layersParam = std::make_unique<QgsProcessingParameterMultipleLayers>( u"LAYERS"_s, QObject::tr( "Map layers to assign to unlocked map item(s)" ), Qgis::ProcessingSourceType::MapLayer, QVariant(), true );
  layersParam->setFlags( layersParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( layersParam.release() );

  auto dpiParam = std::make_unique<QgsProcessingParameterNumber>( u"DPI"_s, QObject::tr( "DPI (leave blank for default layout DPI)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  dpiParam->setFlags( dpiParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( dpiParam.release() );

  auto forceVectorParam = std::make_unique<QgsProcessingParameterBoolean>( u"FORCE_VECTOR"_s, QObject::tr( "Always export as vectors" ), false );
  forceVectorParam->setFlags( forceVectorParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( forceVectorParam.release() );

  auto forceRasterParam = std::make_unique<QgsProcessingParameterBoolean>( u"FORCE_RASTER"_s, QObject::tr( "Always export as raster" ), false );
  forceRasterParam->setFlags( forceRasterParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( forceRasterParam.release() );

  auto appendGeorefParam = std::make_unique<QgsProcessingParameterBoolean>( u"GEOREFERENCE"_s, QObject::tr( "Append georeference information" ), true );
  appendGeorefParam->setFlags( appendGeorefParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( appendGeorefParam.release() );

  auto exportRDFParam = std::make_unique<QgsProcessingParameterBoolean>( u"INCLUDE_METADATA"_s, QObject::tr( "Export RDF metadata (title, author, etc.)" ), true );
  exportRDFParam->setFlags( exportRDFParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( exportRDFParam.release() );

  auto disableTiled = std::make_unique<QgsProcessingParameterBoolean>( u"DISABLE_TILED"_s, QObject::tr( "Disable tiled raster layer exports" ), false );
  disableTiled->setFlags( disableTiled->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( disableTiled.release() );

  auto simplify = std::make_unique<QgsProcessingParameterBoolean>( u"SIMPLIFY"_s, QObject::tr( "Simplify geometries to reduce output file size" ), true );
  simplify->setFlags( simplify->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( simplify.release() );

  const QStringList textExportOptions {
    QObject::tr( "Always Export Text as Paths (Recommended)" ),
    QObject::tr( "Always Export Text as Text Objects" ),
    QObject::tr( "Prefer Exporting Text as Text Objects" )
  };

  auto textFormat = std::make_unique<QgsProcessingParameterEnum>( u"TEXT_FORMAT"_s, QObject::tr( "Text export" ), textExportOptions, false, 0 );
  textFormat->setFlags( textFormat->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( textFormat.release() );

  const QStringList imageCompressionOptions {
    QObject::tr( "Lossy (JPEG)" ),
    QObject::tr( "Lossless" )
  };

  auto imageCompression = std::make_unique<QgsProcessingParameterEnum>( u"IMAGE_COMPRESSION"_s, QObject::tr( "Image compression" ), imageCompressionOptions, false, 0 );
  imageCompression->setFlags( imageCompression->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( imageCompression.release() );

  auto layeredExport = std::make_unique<QgsProcessingParameterBoolean>( u"SEPARATE_LAYERS"_s, QObject::tr( "Export layers as separate PDF files" ), false );
  layeredExport->setFlags( layeredExport->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( layeredExport.release() );

  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "PDF file" ), QObject::tr( "PDF Format" ) + " (*.pdf *.PDF)" ) );
}

Qgis::ProcessingAlgorithmFlags QgsLayoutToPdfAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsLayoutToPdfAlgorithm *QgsLayoutToPdfAlgorithm::createInstance() const
{
  return new QgsLayoutToPdfAlgorithm();
}

QVariantMap QgsLayoutToPdfAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // this needs to be done in main thread, layouts are not thread safe
  QgsPrintLayout *l = parameterAsLayout( parameters, u"LAYOUT"_s, context );
  if ( !l )
    throw QgsProcessingException( QObject::tr( "Cannot find layout with name \"%1\"" ).arg( parameters.value( u"LAYOUT"_s ).toString() ) );
  std::unique_ptr<QgsPrintLayout> layout( l->clone() );

  const QList<QgsMapLayer *> layers = parameterAsLayerList( parameters, u"LAYERS"_s, context );
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

  const QString dest = parameterAsFileOutput( parameters, u"OUTPUT"_s, context );

  QgsLayoutExporter exporter( layout.get() );
  QgsLayoutExporter::PdfExportSettings settings;

  if ( parameters.value( u"DPI"_s ).isValid() )
  {
    settings.dpi = parameterAsDouble( parameters, u"DPI"_s, context );
  }

  settings.forceVectorOutput = parameterAsBool( parameters, u"FORCE_VECTOR"_s, context );
  settings.rasterizeWholeImage = parameterAsBool( parameters, u"FORCE_RASTER"_s, context );
  settings.appendGeoreference = parameterAsBool( parameters, u"GEOREFERENCE"_s, context );
  settings.exportMetadata = parameterAsBool( parameters, u"INCLUDE_METADATA"_s, context );
  settings.simplifyGeometries = parameterAsBool( parameters, u"SIMPLIFY"_s, context );
  const int textFormat = parameterAsEnum( parameters, u"TEXT_FORMAT"_s, context );
  switch ( textFormat )
  {
    case 0:
      settings.textRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;
      break;
    case 1:
      settings.textRenderFormat = Qgis::TextRenderFormat::AlwaysText;
      break;
    case 2:
      settings.textRenderFormat = Qgis::TextRenderFormat::PreferText;
      break;
    default:
      break;
  }
  settings.exportLayersAsSeperateFiles = parameterAsBool( parameters, u"SEPARATE_LAYERS"_s, context ); //#spellok

  if ( parameterAsBool( parameters, u"DISABLE_TILED"_s, context ) )
    settings.flags = settings.flags | Qgis::LayoutRenderFlag::DisableTiledRasterLayerRenders;
  else
    settings.flags = settings.flags & ~static_cast< int >( Qgis::LayoutRenderFlag::DisableTiledRasterLayerRenders );

  if ( parameterAsEnum( parameters, u"IMAGE_COMPRESSION"_s, context ) == 1 )
    settings.flags = settings.flags | Qgis::LayoutRenderFlag::LosslessImageRendering;
  else
    settings.flags = settings.flags & ~static_cast< int >( Qgis::LayoutRenderFlag::LosslessImageRendering );

  switch ( exporter.exportToPdf( dest, settings ) )
  {
    case QgsLayoutExporter::Success:
    {
      feedback->pushInfo( QObject::tr( "Successfully exported layout to %1" ).arg( QDir::toNativeSeparators( dest ) ) );
      break;
    }

    case QgsLayoutExporter::FileError:
      throw QgsProcessingException( !exporter.errorMessage().isEmpty() ? exporter.errorMessage() : QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( dest ) ) );

    case QgsLayoutExporter::PrintError:
      throw QgsProcessingException( !exporter.errorMessage().isEmpty() ? exporter.errorMessage() : QObject::tr( "Could not create print device." ) );

    case QgsLayoutExporter::MemoryError:
      throw QgsProcessingException( !exporter.errorMessage().isEmpty() ? exporter.errorMessage() : QObject::tr( "Exporting the PDF "
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
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
