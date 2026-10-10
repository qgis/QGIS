#include "qgsagentplugin.h"

#include "qgis.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslayoutexporter.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempage.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutpoint.h"
#include "qgslayoutsize.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsprintlayout.h"
#include "qgsproject.h"
#include "qgstextformat.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QJsonArray>
#include <QJsonValue>
#include <QMessageBox>
#include <QRectF>
#include <QSize>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <memory>

using namespace Qt::StringLiterals;

namespace
{
  struct PageMetrics
  {
    double width = 297.0;
    double height = 210.0;
  };

  enum class LayoutExportFormat
  {
    Pdf,
    Image,
    Svg,
  };

  QStringList jsonStringList( const QJsonValue &value )
  {
    QStringList values;
    for ( const QJsonValue &entry : value.toArray() )
    {
      if ( entry.isString() )
        values.append( entry.toString() );
    }
    return values;
  }

  bool valueAsBool( const QJsonObject &arguments, const QString &key, bool defaultValue )
  {
    const QJsonValue value = arguments.value( key );
    return value.isBool() ? value.toBool() : defaultValue;
  }

  double valueAsDouble( const QJsonObject &arguments, const QString &key, double defaultValue )
  {
    const QJsonValue value = arguments.value( key );
    return value.isDouble() ? value.toDouble() : defaultValue;
  }

  LayoutExportFormat formatForPath( const QFileInfo &outputInfo, QString &formatName, QString &error )
  {
    const QString suffix = outputInfo.suffix().toLower();
    if ( suffix == u"pdf"_s )
    {
      formatName = u"pdf"_s;
      return LayoutExportFormat::Pdf;
    }
    if ( suffix == u"svg"_s )
    {
      formatName = u"svg"_s;
      return LayoutExportFormat::Svg;
    }
    if ( QStringList{ u"png"_s, u"jpg"_s, u"jpeg"_s, u"tif"_s, u"tiff"_s, u"bmp"_s }.contains( suffix ) )
    {
      formatName = suffix;
      return LayoutExportFormat::Image;
    }

    error = QObject::tr( "Unsupported layout export extension '%1'. Use pdf, svg, png, jpg, jpeg, tif, tiff, or bmp." ).arg( suffix );
    return LayoutExportFormat::Pdf;
  }

  PageMetrics pageMetrics( const QString &orientation )
  {
    PageMetrics metrics;
    if ( orientation.toLower() == u"portrait"_s )
      std::swap( metrics.width, metrics.height );
    return metrics;
  }

  QgsTextFormat labelFormat( double pointSize, bool bold )
  {
    QFont font;
    font.setBold( bold );
    QgsTextFormat format = QgsTextFormat::fromQFont( font );
    format.setSize( pointSize );
    format.setSizeUnit( Qgis::RenderUnit::Points );
    return format;
  }

  void addLabel( QgsPrintLayout *layout, const QString &text, const QRectF &rect, double pointSize, bool bold = false )
  {
    if ( text.isEmpty() )
      return;

    QgsLayoutItemLabel *label = new QgsLayoutItemLabel( layout );
    label->setText( text );
    label->setTextFormat( labelFormat( pointSize, bold ) );
    label->setMargin( 0 );
    label->attemptSetSceneRect( rect );
    layout->addLayoutItem( label );
  }

  QJsonObject exportResultObject( QgsLayoutExporter::ExportResult result, const QString &path, const QString &format, QgsLayoutExporter &exporter )
  {
    return QJsonObject{
      { u"ok"_s, result == QgsLayoutExporter::Success },
      { u"path"_s, path },
      { u"format"_s, format },
      { u"error_code"_s, static_cast<int>( result ) },
      { u"error_file"_s, exporter.errorFile() },
    };
  }
}

QJsonObject QgsAgentServer::exportMapLayout( const QJsonObject &arguments )
{
  QgsMapCanvas *canvas = mInterface->mapCanvas();
  if ( !canvas )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "No active QGIS map canvas is available." ) } };

  const QString path = arguments.value( u"path"_s ).toString();
  if ( path.isEmpty() || !QDir::isAbsolutePath( path ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "An absolute output path is required." ) } };

  const QFileInfo outputInfo( path );
  if ( !outputInfo.dir().exists() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Output directory does not exist: %1" ).arg( outputInfo.absolutePath() ) } };
  if ( outputInfo.exists() && !arguments.value( u"overwrite"_s ).toBool( false ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Output already exists. Set overwrite=true to replace it." ) } };

  QString formatName;
  QString formatError;
  const LayoutExportFormat exportFormat = formatForPath( outputInfo, formatName, formatError );
  if ( !formatError.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, formatError } };

  QList<QgsMapLayer *> layers = canvas->layers();
  const QStringList layerReferences = jsonStringList( arguments.value( u"layers"_s ) );
  if ( !layerReferences.isEmpty() )
  {
    layers.clear();
    for ( const QString &reference : layerReferences )
    {
      QString error;
      QgsMapLayer *layer = findLayer( reference, error );
      if ( !layer )
        return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error } };
      layers.append( layer );
    }
  }
  if ( layers.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "No map layers are available for layout export." ) } };

  const PageMetrics page = pageMetrics( arguments.value( u"orientation"_s ).toString( u"landscape"_s ) );
  const double margin = std::clamp( valueAsDouble( arguments, u"margin_mm"_s, 10.0 ), 0.0, 40.0 );
  const bool includeLegend = valueAsBool( arguments, u"include_legend"_s, true );
  const bool includeScaleBar = valueAsBool( arguments, u"include_scale_bar"_s, true );
  const bool includeNorthArrow = valueAsBool( arguments, u"include_north_arrow"_s, true );
  const bool includeNote = valueAsBool( arguments, u"include_note"_s, true );
  const bool filterLegendByMap = valueAsBool( arguments, u"filter_legend_by_map"_s, true );
  const double dpi = std::clamp( valueAsDouble( arguments, u"dpi"_s, 300.0 ), 72.0, 1200.0 );
  const double requestedScale = valueAsDouble( arguments, u"scale"_s, 0.0 );

  const QString title = arguments.value( u"title"_s ).toString( QgsProject::instance()->title().isEmpty() ? tr( "QGIS Map" ) : QgsProject::instance()->title() );
  const QString note = arguments.value( u"note"_s ).toString(
    tr( "CRS: %1 | Scale: 1:%2 | Exported: %3" )
      .arg( canvas->mapSettings().destinationCrs().authid() )
      .arg( QString::number( std::llround( canvas->scale() ) ) )
      .arg( QDateTime::currentDateTime().toString( Qt::ISODate ) )
  );

  const double titleHeight = title.isEmpty() ? 0.0 : 12.0;
  const double noteHeight = includeNote && !note.isEmpty() ? 8.0 : 0.0;
  const double legendWidth = includeLegend ? std::min( 54.0, std::max( 38.0, page.width * 0.19 ) ) : 0.0;
  const double gutter = includeLegend ? 6.0 : 0.0;
  const double mapX = margin;
  const double mapY = margin + titleHeight;
  const double mapWidth = page.width - margin * 2.0 - legendWidth - gutter;
  const double mapHeight = page.height - margin * 2.0 - titleHeight - noteHeight;
  if ( mapWidth <= 20.0 || mapHeight <= 20.0 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The requested margins and layout elements leave no room for the map." ) } };

  if ( !confirmWriteAction(
         tr( "Export QGIS map layout?" ),
         tr( "Output: %1\nFormat: %2\nLayers: %3\nLayout: A4 %4\nElements: %5%6%7%8" )
           .arg(
             path,
             formatName,
             QString::number( layers.size() ),
             page.width > page.height ? tr( "landscape" ) : tr( "portrait" ),
             includeScaleBar ? tr( "scale bar" ) : tr( "no scale bar" ),
             includeNorthArrow ? tr( ", north arrow" ) : QString(),
             includeLegend ? tr( ", legend" ) : QString(),
             includeNote ? tr( ", note" ) : QString()
           )
       ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  std::unique_ptr<QgsPrintLayout> layout = std::make_unique<QgsPrintLayout>( QgsProject::instance() );
  layout->initializeDefaults();
  layout->setName( arguments.value( u"layout_name"_s ).toString( u"QGIS Agent Export"_s ) );
  if ( QgsLayoutItemPage *layoutPage = layout->pageCollection()->page( 0 ) )
    layoutPage->setPageSize( QgsLayoutSize( page.width, page.height, Qgis::LayoutUnit::Millimeters ) );

  addLabel( layout.get(), title, QRectF( margin, margin - 1.0, page.width - margin * 2.0, 10.0 ), 16.0, true );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout.get() );
  map->attemptSetSceneRect( QRectF( mapX, mapY, mapWidth, mapHeight ) );
  map->setFrameEnabled( true );
  map->setBackgroundEnabled( true );
  map->setLayers( layers );
  map->setCrs( canvas->mapSettings().destinationCrs() );
  map->zoomToExtent( canvas->extent() );
  if ( requestedScale > 0.0 )
    map->setScale( requestedScale );
  map->setMapRotation( canvas->rotation() );
  layout->addLayoutItem( map );

  if ( includeScaleBar )
  {
    QgsLayoutItemScaleBar *scaleBar = new QgsLayoutItemScaleBar( layout.get() );
    scaleBar->attemptSetSceneRect( QRectF( mapX + 5.0, mapY + mapHeight - 16.0, std::min( 65.0, mapWidth * 0.35 ), 10.0 ) );
    layout->addLayoutItem( scaleBar );
    scaleBar->setLinkedMap( map );
    scaleBar->setStyle( u"Single Box"_s );
    scaleBar->applyDefaultSize( scaleBar->guessUnits() );
    scaleBar->setTextFormat( labelFormat( 7.0, false ) );
    scaleBar->update();
  }

  if ( includeNorthArrow )
  {
    QgsLayoutItemPicture *northArrow = new QgsLayoutItemPicture( layout.get() );
    northArrow->attemptSetSceneRect( QRectF( mapX + mapWidth - 18.0, mapY + 5.0, 13.0, 13.0 ) );
    northArrow->setResizeMode( QgsLayoutItemPicture::Zoom );
    northArrow->setPicturePath( u":/images/north_arrows/layout_default_north_arrow.svg"_s, Qgis::PictureFormat::SVG );
    northArrow->setLinkedMap( map );
    northArrow->setNorthMode( QgsLayoutItemPicture::GridNorth );
    layout->addLayoutItem( northArrow );
  }

  if ( includeLegend )
  {
    QgsLayoutItemLegend *legend = new QgsLayoutItemLegend( layout.get() );
    legend->setTitle( arguments.value( u"legend_title"_s ).toString( tr( "Legend" ) ) );
    legend->setLinkedMap( map );
    legend->setSyncMode( Qgis::LegendSyncMode::VisibleLayers );
    legend->setLegendFilterByMapEnabled( filterLegendByMap );
    legend->setResizeToContents( true );
    legend->attemptSetSceneRect( QRectF( mapX + mapWidth + gutter, mapY, legendWidth, std::min( 90.0, mapHeight ) ) );
    layout->addLayoutItem( legend );
    legend->refresh();
    legend->adjustBoxSize();
  }

  if ( includeNote && !note.isEmpty() )
    addLabel( layout.get(), note, QRectF( margin, page.height - margin - noteHeight + 1.0, page.width - margin * 2.0, noteHeight ), 7.0 );

  QgsLayoutExporter exporter( layout.get() );
  QgsLayoutExporter::ExportResult result = QgsLayoutExporter::FileError;
  if ( exportFormat == LayoutExportFormat::Pdf )
  {
    QgsLayoutExporter::PdfExportSettings settings;
    settings.dpi = dpi;
    result = exporter.exportToPdf( path, settings );
  }
  else if ( exportFormat == LayoutExportFormat::Svg )
  {
    QgsLayoutExporter::SvgExportSettings settings;
    settings.dpi = dpi;
    result = exporter.exportToSvg( path, settings );
  }
  else
  {
    QgsLayoutExporter::ImageExportSettings settings;
    settings.dpi = dpi;
    result = exporter.exportToImage( path, settings );
  }

  const bool ok = result == QgsLayoutExporter::Success;
  if ( ok )
    emit fileCreated( path );

  QJsonArray exportedLayers;
  for ( QgsMapLayer *layer : std::as_const( layers ) )
  {
    if ( layer )
      exportedLayers.append( QJsonObject{
        { u"id"_s, layer->id() },
        { u"name"_s, layer->name() },
      } );
  }

  QJsonObject response = exportResultObject( result, path, formatName, exporter );
  response.insert( u"dpi"_s, dpi );
  response.insert( u"page_width_mm"_s, page.width );
  response.insert( u"page_height_mm"_s, page.height );
  response.insert( u"map_extent"_s, QJsonArray{ map->extent().xMinimum(), map->extent().yMinimum(), map->extent().xMaximum(), map->extent().yMaximum() } );
  response.insert( u"map_scale"_s, map->scale() );
  response.insert( u"layers"_s, exportedLayers );
  return response;
}
