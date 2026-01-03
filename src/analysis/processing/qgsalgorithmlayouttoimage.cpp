/***************************************************************************
                         qgsalgorithmlayouttoimage.cpp
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

#include "qgsalgorithmlayouttoimage.h"

#include "qgslayout.h"
#include "qgslayoutexporter.h"
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"
#include "qgsprocessingoutputs.h"

#include <QImageWriter>

///@cond PRIVATE

QString QgsLayoutToImageAlgorithm::name() const
{
  return u"printlayouttoimage"_s;
}

QString QgsLayoutToImageAlgorithm::displayName() const
{
  return QObject::tr( "Export print layout as image" );
}

QStringList QgsLayoutToImageAlgorithm::tags() const
{
  return QObject::tr( "layout,composer,composition,save,png,jpeg,jpg" ).split( ',' );
}

QString QgsLayoutToImageAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsLayoutToImageAlgorithm::groupId() const
{
  return u"cartography"_s;
}

QString QgsLayoutToImageAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a print layout as an image." );
}

QString QgsLayoutToImageAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm outputs a print layout as an image file (e.g. PNG or JPEG images)." );
}

void QgsLayoutToImageAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterLayout( u"LAYOUT"_s, QObject::tr( "Print layout" ) ) );

  auto layersParam = std::make_unique<QgsProcessingParameterMultipleLayers>( u"LAYERS"_s, QObject::tr( "Map layers to assign to unlocked map item(s)" ), Qgis::ProcessingSourceType::MapLayer, QVariant(), true );
  layersParam->setFlags( layersParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( layersParam.release() );

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

  QStringList imageFilters;
  const auto supportedImageFormats { QImageWriter::supportedImageFormats() };
  for ( const QByteArray &format : supportedImageFormats )
  {
    if ( format == "svg" )
      continue;

    const QString longName = format.toUpper() + QObject::tr( " format" );
    const QString glob = u"*."_s + format;

    if ( format == "png" && !imageFilters.empty() )
      imageFilters.insert( 0, u"%1 (%2 %3)"_s.arg( longName, glob.toLower(), glob.toUpper() ) );
    else
      imageFilters.append( u"%1 (%2 %3)"_s.arg( longName, glob.toLower(), glob.toUpper() ) );
  }

  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "Image file" ), imageFilters.join( ";;"_L1 ) ) );
}

Qgis::ProcessingAlgorithmFlags QgsLayoutToImageAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsLayoutToImageAlgorithm *QgsLayoutToImageAlgorithm::createInstance() const
{
  return new QgsLayoutToImageAlgorithm();
}

QVariantMap QgsLayoutToImageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  switch ( exporter.exportToImage( dest, settings ) )
  {
    case QgsLayoutExporter::Success:
    {
      feedback->pushInfo( QObject::tr( "Successfully exported layout to %1" ).arg( QDir::toNativeSeparators( dest ) ) );
      break;
    }

    case QgsLayoutExporter::FileError:
      throw QgsProcessingException( !exporter.errorMessage().isEmpty() ? exporter.errorMessage() : QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( dest ) ) );

    case QgsLayoutExporter::MemoryError:
      throw QgsProcessingException( !exporter.errorMessage().isEmpty() ? exporter.errorMessage() : QObject::tr( "Trying to create the image "
                                                                                                                "resulted in a memory overflow.\n\n"
                                                                                                                "Please try a lower resolution or a smaller paper size." ) );

    case QgsLayoutExporter::SvgLayerError:
    case QgsLayoutExporter::IteratorError:
    case QgsLayoutExporter::Canceled:
    case QgsLayoutExporter::PrintError:
      // no meaning for imageexports, will not be encountered
      break;
  }

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
