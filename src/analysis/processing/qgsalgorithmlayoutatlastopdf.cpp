/***************************************************************************
                         qgsalgorithmlayoutatlastopdf.cpp
                         ---------------------
    begin                : August 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmlayoutatlastopdf.h"

#include "qgslayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutexporter.h"
#include "qgslayoutitemmap.h"
#include "qgslayoututils.h"
#include "qgsprintlayout.h"
#include "qgsprocessingoutputs.h"

///@cond PRIVATE

// QgsLayoutAtlasToPdfAlgorithmBase

QStringList QgsLayoutAtlasToPdfAlgorithmBase::tags() const
{
  return QObject::tr( "layout,atlas,composer,composition,save" ).split( ',' );
}

QString QgsLayoutAtlasToPdfAlgorithmBase::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsLayoutAtlasToPdfAlgorithmBase::groupId() const
{
  return u"cartography"_s;
}

Qgis::ProcessingAlgorithmFlags QgsLayoutAtlasToPdfAlgorithmBase::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

void QgsLayoutAtlasToPdfAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterLayout( u"LAYOUT"_s, QObject::tr( "Atlas layout" ) ) );

  addParameter( new QgsProcessingParameterVectorLayer( u"COVERAGE_LAYER"_s, QObject::tr( "Coverage layer" ), QList<int>(), QVariant(), true ) );
  addParameter( new QgsProcessingParameterExpression( u"FILTER_EXPRESSION"_s, QObject::tr( "Filter expression" ), QString(), u"COVERAGE_LAYER"_s, true ) );
  addParameter( new QgsProcessingParameterExpression( u"SORTBY_EXPRESSION"_s, QObject::tr( "Sort expression" ), QString(), u"COVERAGE_LAYER"_s, true ) );
  addParameter( new QgsProcessingParameterBoolean( u"SORTBY_REVERSE"_s, QObject::tr( "Reverse sort order (used when a sort expression is provided)" ), false ) );

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
    QObject::tr( "Prefer Exporting Text as Text Objects" ),
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
}

QVariantMap QgsLayoutAtlasToPdfAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // this needs to be done in main thread, layouts are not thread safe
  QgsPrintLayout *l = parameterAsLayout( parameters, u"LAYOUT"_s, context );
  if ( !l )
    throw QgsProcessingException( QObject::tr( "Cannot find layout with name \"%1\"" ).arg( parameters.value( u"LAYOUT"_s ).toString() ) );

  std::unique_ptr<QgsPrintLayout> layout( l->clone() );
  QgsLayoutAtlas *atlas = layout->atlas();

  QString expression, error;
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"COVERAGE_LAYER"_s, context );
  if ( layer )
  {
    atlas->setEnabled( true );
    atlas->setCoverageLayer( layer );

    expression = parameterAsString( parameters, u"FILTER_EXPRESSION"_s, context );
    atlas->setFilterExpression( expression, error );
    atlas->setFilterFeatures( !expression.isEmpty() && error.isEmpty() );
    if ( !expression.isEmpty() && !error.isEmpty() )
    {
      throw QgsProcessingException( QObject::tr( "Error setting atlas filter expression" ) );
    }
    error.clear();

    expression = parameterAsString( parameters, u"SORTBY_EXPRESSION"_s, context );
    if ( !expression.isEmpty() )
    {
      const bool sortByReverse = parameterAsBool( parameters, u"SORTBY_REVERSE"_s, context );
      atlas->setSortFeatures( true );
      atlas->setSortExpression( expression );
      atlas->setSortAscending( !sortByReverse );
    }
    else
    {
      atlas->setSortFeatures( false );
    }
  }
  else if ( !atlas->enabled() )
  {
    throw QgsProcessingException( QObject::tr( "Selected layout does not have atlas functionality enabled" ) );
  }

  const QgsLayoutExporter exporter( layout.get() );
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

  if ( parameterAsBool( parameters, u"DISABLE_TILED"_s, context ) )
    settings.flags = settings.flags | Qgis::LayoutRenderFlag::DisableTiledRasterLayerRenders;
  else
    settings.flags = settings.flags & ~static_cast< int >( Qgis::LayoutRenderFlag::DisableTiledRasterLayerRenders );

  if ( parameterAsEnum( parameters, u"IMAGE_COMPRESSION"_s, context ) == 1 )
    settings.flags = settings.flags | Qgis::LayoutRenderFlag::LosslessImageRendering;
  else
    settings.flags = settings.flags & ~static_cast< int >( Qgis::LayoutRenderFlag::LosslessImageRendering );

  settings.predefinedMapScales = QgsLayoutUtils::predefinedScales( layout.get() );

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

  return exportAtlas( atlas, exporter, settings, parameters, context, feedback );
}

//
// QgsLayoutAtlasToPdfAlgorithm
//

QString QgsLayoutAtlasToPdfAlgorithm::name() const
{
  return u"atlaslayouttopdf"_s;
}

QString QgsLayoutAtlasToPdfAlgorithm::displayName() const
{
  return QObject::tr( "Export atlas layout as PDF (single file)" );
}

QString QgsLayoutAtlasToPdfAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports an atlas layout as a single PDF file." );
}

QString QgsLayoutAtlasToPdfAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm outputs an atlas layout as a single PDF file.\n\n"
                      "If a coverage layer is set, the selected layout's atlas settings exposed in this algorithm "
                      "will be overwritten. In this case, an empty filter or sort by expression will turn those "
                      "settings off." );
}

void QgsLayoutAtlasToPdfAlgorithm::initAlgorithm( const QVariantMap & )
{
  QgsLayoutAtlasToPdfAlgorithmBase::initAlgorithm();
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "PDF file" ), QObject::tr( "PDF Format" ) + " (*.pdf *.PDF)" ) );
}

QgsLayoutAtlasToPdfAlgorithm *QgsLayoutAtlasToPdfAlgorithm::createInstance() const
{
  return new QgsLayoutAtlasToPdfAlgorithm();
}

QVariantMap QgsLayoutAtlasToPdfAlgorithm::exportAtlas( QgsLayoutAtlas *atlas, const QgsLayoutExporter &exporter, const QgsLayoutExporter::PdfExportSettings &settings, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( exporter )

  const QString dest = parameterAsFileOutput( parameters, u"OUTPUT"_s, context );

  QString error;
  if ( atlas->updateFeatures() )
  {
    feedback->pushInfo( QObject::tr( "Exporting %n atlas feature(s)", "", atlas->count() ) );
    switch ( QgsLayoutExporter::exportToPdf( atlas, dest, settings, error, feedback ) )
    {
      case QgsLayoutExporter::Success:
      {
        feedback->pushInfo( QObject::tr( "Successfully exported atlas to %1" ).arg( QDir::toNativeSeparators( dest ) ) );
        break;
      }

      case QgsLayoutExporter::FileError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( dest ) ) );

      case QgsLayoutExporter::PrintError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Could not create print device." ) );

      case QgsLayoutExporter::MemoryError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Trying to create the image "
                                                                              "resulted in a memory overflow.\n\n"
                                                                              "Please try a lower resolution or a smaller paper size." ) );

      case QgsLayoutExporter::IteratorError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Error encountered while exporting atlas." ) );

      case QgsLayoutExporter::SvgLayerError:
      case QgsLayoutExporter::Canceled:
        // no meaning for PDF exports, will not be encountered
        break;
    }
  }
  else
  {
    feedback->reportError( QObject::tr( "No atlas features found" ) );
  }

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

//
// QgsLayoutAtlasToMultiplePdfAlgorithm
//

QString QgsLayoutAtlasToMultiplePdfAlgorithm::name() const
{
  return u"atlaslayouttomultiplepdf"_s;
}

QString QgsLayoutAtlasToMultiplePdfAlgorithm::displayName() const
{
  return QObject::tr( "Export atlas layout as PDF (multiple files)" );
}

QString QgsLayoutAtlasToMultiplePdfAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports an atlas layout to multiple PDF files." );
}

QString QgsLayoutAtlasToMultiplePdfAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm outputs an atlas layout to multiple PDF files.\n\n"
                      "If a coverage layer is set, the selected layout's atlas settings exposed in this algorithm "
                      "will be overwritten. In this case, an empty filter or sort by expression will turn those "
                      "settings off.\n"
  );
}

void QgsLayoutAtlasToMultiplePdfAlgorithm::initAlgorithm( const QVariantMap & )
{
  QgsLayoutAtlasToPdfAlgorithmBase::initAlgorithm();
  addParameter( new QgsProcessingParameterExpression( u"OUTPUT_FILENAME"_s, QObject::tr( "Output filename" ), QString(), u"COVERAGE_LAYER"_s, true ) );
  addParameter( new QgsProcessingParameterFile( u"OUTPUT_FOLDER"_s, QObject::tr( "Output folder" ), Qgis::ProcessingFileParameterBehavior::Folder ) );
}

QgsLayoutAtlasToMultiplePdfAlgorithm *QgsLayoutAtlasToMultiplePdfAlgorithm::createInstance() const
{
  return new QgsLayoutAtlasToMultiplePdfAlgorithm();
}

QVariantMap QgsLayoutAtlasToMultiplePdfAlgorithm::exportAtlas( QgsLayoutAtlas *atlas, const QgsLayoutExporter &exporter, const QgsLayoutExporter::PdfExportSettings &settings, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( exporter )

  QString error;

  const QString filename = parameterAsString( parameters, u"OUTPUT_FILENAME"_s, context );
  const QString destFolder = parameterAsFile( parameters, u"OUTPUT_FOLDER"_s, context );

  // the "atlas.pdf" part will be overridden, only the folder is important
  const QString dest = u"%1/atlas.pdf"_s.arg( destFolder );

  if ( atlas->updateFeatures() )
  {
    feedback->pushInfo( QObject::tr( "Exporting %n atlas feature(s)", "", atlas->count() ) );

    QgsLayoutExporter::ExportResult result;
    if ( atlas->filenameExpression().isEmpty() && filename.isEmpty() )
    {
      atlas->setFilenameExpression( u"'output_'||@atlas_featurenumber"_s, error );
    }
    else if ( !filename.isEmpty() )
    {
      if ( !atlas->setFilenameExpression( filename, error ) )
      {
        throw QgsProcessingException( QObject::tr( "Output file name expression is not valid: %1" ).arg( error ) );
      }
    }

    result = QgsLayoutExporter::exportToPdfs( atlas, dest, settings, error, feedback );

    switch ( result )
    {
      case QgsLayoutExporter::Success:
      {
        feedback->pushInfo( QObject::tr( "Successfully exported atlas to %1" ).arg( QDir::toNativeSeparators( destFolder ) ) );
        break;
      }

      case QgsLayoutExporter::FileError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( dest ) ) );

      case QgsLayoutExporter::PrintError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Could not create print device." ) );

      case QgsLayoutExporter::MemoryError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Trying to create the image "
                                                                              "resulted in a memory overflow.\n\n"
                                                                              "Please try a lower resolution or a smaller paper size." ) );

      case QgsLayoutExporter::IteratorError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Error encountered while exporting atlas." ) );

      case QgsLayoutExporter::SvgLayerError:
      case QgsLayoutExporter::Canceled:
        // no meaning for PDF exports, will not be encountered
        break;
    }
  }
  else
  {
    feedback->reportError( QObject::tr( "No atlas features found" ) );
  }

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT_FOLDER"_s, destFolder );
  return outputs;
}

///@endcond
