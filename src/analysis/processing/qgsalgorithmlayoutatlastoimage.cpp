/***************************************************************************
                         qgsalgorithmlayoutatlastoimage.cpp
                         ---------------------
    begin                : June 2020
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

#include "qgsalgorithmlayoutatlastoimage.h"

#include "qgslayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutexporter.h"
#include "qgslayoutitemmap.h"
#include "qgslayoututils.h"
#include "qgsprintlayout.h"
#include "qgsprocessingoutputs.h"

#include <QImageWriter>

///@cond PRIVATE

QString QgsLayoutAtlasToImageAlgorithm::name() const
{
  return u"atlaslayouttoimage"_s;
}

QString QgsLayoutAtlasToImageAlgorithm::displayName() const
{
  return QObject::tr( "Export atlas layout as image" );
}

QStringList QgsLayoutAtlasToImageAlgorithm::tags() const
{
  return QObject::tr( "layout,atlas,composer,composition,save,png,jpeg,jpg" ).split( ',' );
}

QString QgsLayoutAtlasToImageAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsLayoutAtlasToImageAlgorithm::groupId() const
{
  return u"cartography"_s;
}

QString QgsLayoutAtlasToImageAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports an atlas layout as a set of images." );
}

QString QgsLayoutAtlasToImageAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm outputs an atlas layout to a set of image files (e.g. PNG or JPEG images).\n\n"
                      "If a coverage layer is set, the selected layout's atlas settings exposed in this algorithm "
                      "will be overwritten. In this case, an empty filter or sort by expression will turn those "
                      "settings off." );
}

void QgsLayoutAtlasToImageAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterLayout( u"LAYOUT"_s, QObject::tr( "Atlas layout" ) ) );

  addParameter( new QgsProcessingParameterVectorLayer( u"COVERAGE_LAYER"_s, QObject::tr( "Coverage layer" ), QList<int>(), QVariant(), true ) );
  addParameter( new QgsProcessingParameterExpression( u"FILTER_EXPRESSION"_s, QObject::tr( "Filter expression" ), QString(), u"COVERAGE_LAYER"_s, true ) );
  addParameter( new QgsProcessingParameterExpression( u"SORTBY_EXPRESSION"_s, QObject::tr( "Sort expression" ), QString(), u"COVERAGE_LAYER"_s, true ) );
  addParameter( new QgsProcessingParameterBoolean( u"SORTBY_REVERSE"_s, QObject::tr( "Reverse sort order (used when a sort expression is provided)" ), false ) );

  addParameter( new QgsProcessingParameterExpression( u"FILENAME_EXPRESSION"_s, QObject::tr( "Output filename expression" ), u"'output_'||@atlas_featurenumber"_s, u"COVERAGE_LAYER"_s ) );
  addParameter( new QgsProcessingParameterFile( u"FOLDER"_s, QObject::tr( "Output folder" ), Qgis::ProcessingFileParameterBehavior::Folder ) );


  auto layersParam = std::make_unique<QgsProcessingParameterMultipleLayers>( u"LAYERS"_s, QObject::tr( "Map layers to assign to unlocked map item(s)" ), Qgis::ProcessingSourceType::MapLayer, QVariant(), true );
  layersParam->setFlags( layersParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( layersParam.release() );

  QStringList imageFormats;
  const QList<QByteArray> supportedImageFormats { QImageWriter::supportedImageFormats() };
  for ( const QByteArray &format : supportedImageFormats )
  {
    if ( format == QByteArray( "svg" ) )
      continue;
    imageFormats << QString( format );
  }
  auto extensionParam = std::make_unique<QgsProcessingParameterEnum>( u"EXTENSION"_s, QObject::tr( "Image format" ), imageFormats, false, imageFormats.indexOf( "png"_L1 ) );
  extensionParam->setFlags( extensionParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( extensionParam.release() );

  auto dpiParam = std::make_unique<QgsProcessingParameterNumber>( u"DPI"_s, QObject::tr( "DPI (leave blank for default layout DPI)" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  dpiParam->setFlags( dpiParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( dpiParam.release() );

  auto appendGeorefParam = std::make_unique<QgsProcessingParameterBoolean>( u"GEOREFERENCE"_s, QObject::tr( "Generate world file" ), true );
  appendGeorefParam->setFlags( appendGeorefParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( appendGeorefParam.release() );

  auto exportRDFParam = std::make_unique<QgsProcessingParameterBoolean>( u"INCLUDE_METADATA"_s, QObject::tr( "Export RDF metadata (title, author, etc.)" ), true );
  exportRDFParam->setFlags( exportRDFParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( exportRDFParam.release() );

  auto antialias = std::make_unique<QgsProcessingParameterBoolean>( u"ANTIALIAS"_s, QObject::tr( "Enable antialiasing" ), true );
  antialias->setFlags( antialias->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( antialias.release() );
}

Qgis::ProcessingAlgorithmFlags QgsLayoutAtlasToImageAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsLayoutAtlasToImageAlgorithm *QgsLayoutAtlasToImageAlgorithm::createInstance() const
{
  return new QgsLayoutAtlasToImageAlgorithm();
}

QVariantMap QgsLayoutAtlasToImageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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
    throw QgsProcessingException( QObject::tr( "Layout being export doesn't have an enabled atlas" ) );
  }

  expression = parameterAsString( parameters, u"FILENAME_EXPRESSION"_s, context );
  atlas->setFilenameExpression( expression, error );
  if ( !error.isEmpty() )
  {
    throw QgsProcessingException( QObject::tr( "Error setting atlas filename expression" ) );
  }

  const QString directory = parameterAsFileOutput( parameters, u"FOLDER"_s, context );
  const QString fileName = QDir( directory ).filePath( u"atlas"_s );

  QStringList imageFormats;
  const QList<QByteArray> supportedImageFormats { QImageWriter::supportedImageFormats() };
  for ( const QByteArray &format : supportedImageFormats )
  {
    if ( format == QByteArray( "svg" ) )
      continue;
    imageFormats << QString( format );
  }
  const int idx = parameterAsEnum( parameters, u"EXTENSION"_s, context );
  const QString extension = '.' + imageFormats.at( idx );

  QgsLayoutExporter::ImageExportSettings settings;

  if ( parameters.value( u"DPI"_s ).isValid() )
  {
    settings.dpi = parameterAsDouble( parameters, u"DPI"_s, context );
  }

  settings.exportMetadata = parameterAsBool( parameters, u"INCLUDE_METADATA"_s, context );
  settings.generateWorldFile = parameterAsBool( parameters, u"GEOREFERENCE"_s, context );

  if ( parameterAsBool( parameters, u"ANTIALIAS"_s, context ) )
    settings.flags = settings.flags | Qgis::LayoutRenderFlag::Antialiasing;
  else
    settings.flags = settings.flags & ~static_cast< int >( Qgis::LayoutRenderFlag::Antialiasing );

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

  if ( atlas->updateFeatures() )
  {
    feedback->pushInfo( QObject::tr( "Exporting %n atlas feature(s)", "", atlas->count() ) );
    switch ( QgsLayoutExporter::exportToImage( atlas, fileName, extension, settings, error, feedback ) )
    {
      case QgsLayoutExporter::Success:
      {
        feedback->pushInfo( QObject::tr( "Successfully exported layout to %1" ).arg( QDir::toNativeSeparators( directory ) ) );
        break;
      }

      case QgsLayoutExporter::FileError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( directory ) ) );

      case QgsLayoutExporter::MemoryError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Trying to create the image "
                                                                              "resulted in a memory overflow.\n\n"
                                                                              "Please try a lower resolution or a smaller paper size." ) );

      case QgsLayoutExporter::IteratorError:
        throw QgsProcessingException( !error.isEmpty() ? error : QObject::tr( "Error encountered while exporting atlas." ) );

      case QgsLayoutExporter::SvgLayerError:
      case QgsLayoutExporter::PrintError:
      case QgsLayoutExporter::Canceled:
        // no meaning for imageexports, will not be encountered
        break;
    }
  }
  else
  {
    feedback->reportError( QObject::tr( "No atlas features found" ) );
  }

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"FOLDER"_s, directory );
  return outputs;
}

///@endcond
