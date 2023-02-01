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
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"
#include "qgsprocessingoutputs.h"
#include "qgslayoutexporter.h"
#include <QImageWriter>

///@cond PRIVATE

QString QgsLayoutToImageAlgorithm::name() const
{
  return QStringLiteral( "printlayouttoimage" );
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
  return QStringLiteral( "cartography" );
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
  addParameter( new QgsProcessingParameterLayout( QStringLiteral( "LAYOUT" ), QObject::tr( "Print layout" ) ) );

  std::unique_ptr< QgsProcessingParameterMultipleLayers > layersParam = std::make_unique< QgsProcessingParameterMultipleLayers>( QStringLiteral( "LAYERS" ), QObject::tr( "Map layers to assign to unlocked map item(s)" ), QgsProcessing::TypeMapLayer, QVariant(), true );
  layersParam->setFlags( layersParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( layersParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > dpiParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DPI" ), QObject::tr( "DPI (leave blank for default layout DPI)" ), QgsProcessingParameterNumber::Double, QVariant(), true, 0 );
  dpiParam->setFlags( dpiParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( dpiParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > appendGeorefParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "GEOREFERENCE" ), QObject::tr( "Generate world file" ), true );
  appendGeorefParam->setFlags( appendGeorefParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( appendGeorefParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > exportRDFParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "INCLUDE_METADATA" ), QObject::tr( "Export RDF metadata (title, author, etc.)" ), true );
  exportRDFParam->setFlags( exportRDFParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( exportRDFParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > antialias = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "ANTIALIAS" ), QObject::tr( "Enable antialiasing" ), true );
  antialias->setFlags( antialias->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( antialias.release() );

  QStringList imageFilters;
  const auto supportedImageFormats { QImageWriter::supportedImageFormats() };
  for ( const QByteArray &format : supportedImageFormats )
  {
    if ( format == "svg" )
      continue;

    const QString longName = format.toUpper() + QObject::tr( " format" );
    const QString glob = QStringLiteral( "*." ) + format;

    if ( format == "png" && !imageFilters.empty() )
      imageFilters.insert( 0, QStringLiteral( "%1 (%2 %3)" ).arg( longName, glob.toLower(), glob.toUpper() ) );
    else
      imageFilters.append( QStringLiteral( "%1 (%2 %3)" ).arg( longName, glob.toLower(), glob.toUpper() ) );
  }

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Image file" ), imageFilters.join( QLatin1String( ";;" ) ) ) );
}

QgsProcessingAlgorithm::Flags QgsLayoutToImageAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagNoThreading | FlagRequiresProject;
}

QgsLayoutToImageAlgorithm *QgsLayoutToImageAlgorithm::createInstance() const
{
  return new QgsLayoutToImageAlgorithm();
}

QVariantMap QgsLayoutToImageAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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
  QgsLayoutExporter::ImageExportSettings settings;

  if ( parameters.value( QStringLiteral( "DPI" ) ).isValid() )
  {
    settings.dpi = parameterAsDouble( parameters, QStringLiteral( "DPI" ), context );
  }

  settings.exportMetadata = parameterAsBool( parameters, QStringLiteral( "INCLUDE_METADATA" ), context );
  settings.generateWorldFile = parameterAsBool( parameters, QStringLiteral( "GEOREFERENCE" ), context );

  if ( parameterAsBool( parameters, QStringLiteral( "ANTIALIAS" ), context ) )
    settings.flags = settings.flags | QgsLayoutRenderContext::FlagAntialiasing;
  else
    settings.flags = settings.flags & ~QgsLayoutRenderContext::FlagAntialiasing;

  switch ( exporter.exportToImage( dest, settings ) )
  {
    case QgsLayoutExporter::Success:
    {
      feedback->pushInfo( QObject::tr( "Successfully exported layout to %1" ).arg( QDir::toNativeSeparators( dest ) ) );
      break;
    }

    case QgsLayoutExporter::FileError:
      throw QgsProcessingException( QObject::tr( "Cannot write to %1.\n\nThis file may be open in another application." ).arg( QDir::toNativeSeparators( dest ) ) );

    case QgsLayoutExporter::MemoryError:
      throw QgsProcessingException( QObject::tr( "Trying to create the image "
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
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond

