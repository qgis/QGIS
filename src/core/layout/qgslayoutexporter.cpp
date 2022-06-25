/***************************************************************************
                              qgslayoutexporter.cpp
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutexporter.h"
#ifndef QT_NO_PRINTER

#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutpagecollection.h"
#include "qgsogrutils.h"
#include "qgspaintenginehack.h"
#include "qgslayoutguidecollection.h"
#include "qgsabstractlayoutiterator.h"
#include "qgsfeedback.h"
#include "qgslayoutgeopdfexporter.h"
#include "qgslinestring.h"
#include "qgsmessagelog.h"
#include "qgslabelingresults.h"
#include <QImageWriter>
#include <QSize>
#include <QSvgGenerator>
#include <QBuffer>
#include <QTimeZone>
#include <QTextStream>

#include "gdal.h"
#include "cpl_conv.h"

///@cond PRIVATE
class LayoutContextPreviewSettingRestorer
{
  public:

    LayoutContextPreviewSettingRestorer( QgsLayout *layout )
      : mLayout( layout )
      , mPreviousSetting( layout->renderContext().mIsPreviewRender )
    {
      mLayout->renderContext().mIsPreviewRender = false;
    }

    ~LayoutContextPreviewSettingRestorer()
    {
      mLayout->renderContext().mIsPreviewRender = mPreviousSetting;
    }

    LayoutContextPreviewSettingRestorer( const LayoutContextPreviewSettingRestorer &other ) = delete;
    LayoutContextPreviewSettingRestorer &operator=( const LayoutContextPreviewSettingRestorer &other ) = delete;

  private:
    QgsLayout *mLayout = nullptr;
    bool mPreviousSetting = false;
};

class LayoutGuideHider
{
  public:

    LayoutGuideHider( QgsLayout *layout )
      : mLayout( layout )
    {
      const QList< QgsLayoutGuide * > guides = mLayout->guides().guides();
      for ( QgsLayoutGuide *guide : guides )
      {
        mPrevVisibility.insert( guide, guide->item()->isVisible() );
        guide->item()->setVisible( false );
      }
    }

    ~LayoutGuideHider()
    {
      for ( auto it = mPrevVisibility.constBegin(); it != mPrevVisibility.constEnd(); ++it )
      {
        it.key()->item()->setVisible( it.value() );
      }
    }

    LayoutGuideHider( const LayoutGuideHider &other ) = delete;
    LayoutGuideHider &operator=( const LayoutGuideHider &other ) = delete;

  private:
    QgsLayout *mLayout = nullptr;
    QHash< QgsLayoutGuide *, bool > mPrevVisibility;
};

class LayoutItemHider
{
  public:
    explicit LayoutItemHider( const QList<QGraphicsItem *> &items )
    {
      mItemsToIterate.reserve( items.count() );
      for ( QGraphicsItem *item : items )
      {
        const bool isVisible = item->isVisible();
        mPrevVisibility[item] = isVisible;
        if ( isVisible )
          mItemsToIterate.append( item );
        if ( QgsLayoutItem *layoutItem = dynamic_cast< QgsLayoutItem * >( item ) )
          layoutItem->setProperty( "wasVisible", isVisible );

        item->hide();
      }
    }

    void hideAll()
    {
      for ( auto it = mPrevVisibility.constBegin(); it != mPrevVisibility.constEnd(); ++it )
      {
        it.key()->hide();
      }
    }

    ~LayoutItemHider()
    {
      for ( auto it = mPrevVisibility.constBegin(); it != mPrevVisibility.constEnd(); ++it )
      {
        it.key()->setVisible( it.value() );
        if ( QgsLayoutItem *layoutItem = dynamic_cast< QgsLayoutItem * >( it.key() ) )
          layoutItem->setProperty( "wasVisible", QVariant() );
      }
    }

    QList< QGraphicsItem * > itemsToIterate() const { return mItemsToIterate; }

    LayoutItemHider( const LayoutItemHider &other ) = delete;
    LayoutItemHider &operator=( const LayoutItemHider &other ) = delete;

  private:

    QList<QGraphicsItem * > mItemsToIterate;
    QHash<QGraphicsItem *, bool> mPrevVisibility;
};

///@endcond PRIVATE

QgsLayoutExporter::QgsLayoutExporter( QgsLayout *layout )
  : mLayout( layout )
{

}

QgsLayoutExporter::~QgsLayoutExporter()
{
  qDeleteAll( mLabelingResults );
}

QgsLayout *QgsLayoutExporter::layout() const
{
  return mLayout;
}

void QgsLayoutExporter::renderPage( QPainter *painter, int page ) const
{
  if ( !mLayout )
    return;

  if ( mLayout->pageCollection()->pageCount() <= page || page < 0 )
  {
    return;
  }

  QgsLayoutItemPage *pageItem = mLayout->pageCollection()->page( page );
  if ( !pageItem )
  {
    return;
  }

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;

  QRectF paperRect = QRectF( pageItem->pos().x(), pageItem->pos().y(), pageItem->rect().width(), pageItem->rect().height() );
  renderRegion( painter, paperRect );
}

QImage QgsLayoutExporter::renderPageToImage( int page, QSize imageSize, double dpi ) const
{
  if ( !mLayout )
    return QImage();

  if ( mLayout->pageCollection()->pageCount() <= page || page < 0 )
  {
    return QImage();
  }

  QgsLayoutItemPage *pageItem = mLayout->pageCollection()->page( page );
  if ( !pageItem )
  {
    return QImage();
  }

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;

  QRectF paperRect = QRectF( pageItem->pos().x(), pageItem->pos().y(), pageItem->rect().width(), pageItem->rect().height() );

  const double imageAspectRatio = static_cast< double >( imageSize.width() ) / imageSize.height();
  const double paperAspectRatio = paperRect.width() / paperRect.height();
  if ( imageSize.isValid() && ( !qgsDoubleNear( imageAspectRatio, paperAspectRatio, 0.008 ) ) )
  {
    // specified image size is wrong aspect ratio for paper rect - so ignore it and just use dpi
    // this can happen e.g. as a result of data defined page sizes
    // see https://github.com/qgis/QGIS/issues/26422
    QgsMessageLog::logMessage( QObject::tr( "Ignoring custom image size because aspect ratio %1 does not match paper ratio %2" ).arg( QString::number( imageAspectRatio, 'g', 3 ), QString::number( paperAspectRatio, 'g', 3 ) ), QStringLiteral( "Layout" ), Qgis::MessageLevel::Warning );
    imageSize = QSize();
  }

  return renderRegionToImage( paperRect, imageSize, dpi );
}

///@cond PRIVATE
class LayoutItemCacheSettingRestorer
{
  public:

    LayoutItemCacheSettingRestorer( QgsLayout *layout )
      : mLayout( layout )
    {
      const QList< QGraphicsItem * > items = mLayout->items();
      for ( QGraphicsItem *item : items )
      {
        mPrevCacheMode.insert( item, item->cacheMode() );
        item->setCacheMode( QGraphicsItem::NoCache );
      }
    }

    ~LayoutItemCacheSettingRestorer()
    {
      for ( auto it = mPrevCacheMode.constBegin(); it != mPrevCacheMode.constEnd(); ++it )
      {
        it.key()->setCacheMode( it.value() );
      }
    }

    LayoutItemCacheSettingRestorer( const LayoutItemCacheSettingRestorer &other ) = delete;
    LayoutItemCacheSettingRestorer &operator=( const LayoutItemCacheSettingRestorer &other ) = delete;

  private:
    QgsLayout *mLayout = nullptr;
    QHash< QGraphicsItem *, QGraphicsItem::CacheMode > mPrevCacheMode;
};

///@endcond PRIVATE

void QgsLayoutExporter::renderRegion( QPainter *painter, const QRectF &region ) const
{
  QPaintDevice *paintDevice = painter->device();
  if ( !paintDevice || !mLayout )
  {
    return;
  }

  LayoutItemCacheSettingRestorer cacheRestorer( mLayout );
  ( void )cacheRestorer;
  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;
  LayoutGuideHider guideHider( mLayout );
  ( void ) guideHider;

  painter->setRenderHint( QPainter::Antialiasing, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagAntialiasing );

  mLayout->render( painter, QRectF( 0, 0, paintDevice->width(), paintDevice->height() ), region );
}

QImage QgsLayoutExporter::renderRegionToImage( const QRectF &region, QSize imageSize, double dpi ) const
{
  if ( !mLayout )
    return QImage();

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;

  double resolution = mLayout->renderContext().dpi();
  double oneInchInLayoutUnits = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( 1, QgsUnitTypes::LayoutInches ) );
  if ( imageSize.isValid() )
  {
    //output size in pixels specified, calculate resolution using average of
    //derived x/y dpi
    resolution = ( imageSize.width() / region.width()
                   + imageSize.height() / region.height() ) / 2.0 * oneInchInLayoutUnits;
  }
  else if ( dpi > 0 )
  {
    //dpi overridden by function parameters
    resolution = dpi;
  }

  int width = imageSize.isValid() ? imageSize.width()
              : static_cast< int >( resolution * region.width() / oneInchInLayoutUnits );
  int height = imageSize.isValid() ? imageSize.height()
               : static_cast< int >( resolution * region.height() / oneInchInLayoutUnits );

  QImage image( QSize( width, height ), QImage::Format_ARGB32 );
  if ( !image.isNull() )
  {
    // see https://doc.qt.io/qt-5/qpainter.html#limitations
    if ( width > 32768 || height > 32768 )
      QgsMessageLog::logMessage( QObject::tr( "Error: output width or height is larger than 32768 pixel, result will be clipped" ) );
    image.setDotsPerMeterX( static_cast< int >( std::round( resolution / 25.4 * 1000 ) ) );
    image.setDotsPerMeterY( static_cast< int>( std::round( resolution / 25.4 * 1000 ) ) );
    image.fill( Qt::transparent );
    QPainter imagePainter( &image );
    renderRegion( &imagePainter, region );
    if ( !imagePainter.isActive() )
      return QImage();
  }

  return image;
}

///@cond PRIVATE
class LayoutContextSettingsRestorer
{
  public:

    Q_NOWARN_DEPRECATED_PUSH
    LayoutContextSettingsRestorer( QgsLayout *layout )
      : mLayout( layout )
      , mPreviousDpi( layout->renderContext().dpi() )
      , mPreviousFlags( layout->renderContext().flags() )
      , mPreviousTextFormat( layout->renderContext().textRenderFormat() )
      , mPreviousExportLayer( layout->renderContext().currentExportLayer() )
      , mPreviousSimplifyMethod( layout->renderContext().simplifyMethod() )
      , mExportThemes( layout->renderContext().exportThemes() )
      , mPredefinedScales( layout->renderContext().predefinedScales() )
    {
    }
    Q_NOWARN_DEPRECATED_POP

    ~LayoutContextSettingsRestorer()
    {
      mLayout->renderContext().setDpi( mPreviousDpi );
      mLayout->renderContext().setFlags( mPreviousFlags );
      mLayout->renderContext().setTextRenderFormat( mPreviousTextFormat );
      Q_NOWARN_DEPRECATED_PUSH
      mLayout->renderContext().setCurrentExportLayer( mPreviousExportLayer );
      Q_NOWARN_DEPRECATED_POP
      mLayout->renderContext().setSimplifyMethod( mPreviousSimplifyMethod );
      mLayout->renderContext().setExportThemes( mExportThemes );
      mLayout->renderContext().setPredefinedScales( mPredefinedScales );
    }

    LayoutContextSettingsRestorer( const LayoutContextSettingsRestorer &other ) = delete;
    LayoutContextSettingsRestorer &operator=( const LayoutContextSettingsRestorer &other ) = delete;

  private:
    QgsLayout *mLayout = nullptr;
    double mPreviousDpi = 0;
    QgsLayoutRenderContext::Flags mPreviousFlags = QgsLayoutRenderContext::Flags();
    Qgis::TextRenderFormat mPreviousTextFormat = Qgis::TextRenderFormat::AlwaysOutlines;
    int mPreviousExportLayer = 0;
    QgsVectorSimplifyMethod mPreviousSimplifyMethod;
    QStringList mExportThemes;
    QVector< double > mPredefinedScales;

};
///@endcond PRIVATE

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToImage( const QString &filePath, const QgsLayoutExporter::ImageExportSettings &s )
{
  if ( !mLayout )
    return PrintError;

  ImageExportSettings settings = s;
  if ( settings.dpi <= 0 )
    settings.dpi = mLayout->renderContext().dpi();

  mErrorFileName.clear();

  int worldFilePageNo = -1;
  if ( QgsLayoutItemMap *referenceMap = mLayout->referenceMap() )
  {
    worldFilePageNo = referenceMap->page();
  }

  QFileInfo fi( filePath );
  QDir dir;
  if ( !dir.exists( fi.absolutePath() ) )
  {
    dir.mkpath( fi.absolutePath() );
  }

  PageExportDetails pageDetails;
  pageDetails.directory = fi.path();
  pageDetails.baseName = fi.completeBaseName();
  pageDetails.extension = fi.suffix();

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;
  LayoutContextSettingsRestorer dpiRestorer( mLayout );
  ( void )dpiRestorer;
  mLayout->renderContext().setDpi( settings.dpi );
  mLayout->renderContext().setFlags( settings.flags );
  mLayout->renderContext().setPredefinedScales( settings.predefinedMapScales );

  QList< int > pages;
  if ( settings.pages.empty() )
  {
    for ( int page = 0; page < mLayout->pageCollection()->pageCount(); ++page )
      pages << page;
  }
  else
  {
    for ( int page : std::as_const( settings.pages ) )
    {
      if ( page >= 0 && page < mLayout->pageCollection()->pageCount() )
        pages << page;
    }
  }

  for ( int page : std::as_const( pages ) )
  {
    if ( !mLayout->pageCollection()->shouldExportPage( page ) )
    {
      continue;
    }

    bool skip = false;
    QRectF bounds;
    QImage image = createImage( settings, page, bounds, skip );

    if ( skip )
      continue; // should skip this page, e.g. null size

    pageDetails.page = page;
    QString outputFilePath = generateFileName( pageDetails );

    if ( image.isNull() )
    {
      mErrorFileName = outputFilePath;
      return MemoryError;
    }

    if ( !saveImage( image, outputFilePath, pageDetails.extension, settings.exportMetadata ? mLayout->project() : nullptr ) )
    {
      mErrorFileName = outputFilePath;
      return FileError;
    }

    const bool shouldGeoreference = ( page == worldFilePageNo );
    if ( shouldGeoreference )
    {
      georeferenceOutputPrivate( outputFilePath, nullptr, bounds, settings.dpi, shouldGeoreference );

      if ( settings.generateWorldFile )
      {
        // should generate world file for this page
        double a, b, c, d, e, f;
        if ( bounds.isValid() )
          computeWorldFileParameters( bounds, a, b, c, d, e, f, settings.dpi );
        else
          computeWorldFileParameters( a, b, c, d, e, f, settings.dpi );

        QFileInfo fi( outputFilePath );
        // build the world file name
        QString outputSuffix = fi.suffix();
        QString worldFileName = fi.absolutePath() + '/' + fi.completeBaseName() + '.'
                                + outputSuffix.at( 0 ) + outputSuffix.at( fi.suffix().size() - 1 ) + 'w';

        writeWorldFile( worldFileName, a, b, c, d, e, f );
      }
    }

  }
  captureLabelingResults();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToImage( QgsAbstractLayoutIterator *iterator, const QString &baseFilePath, const QString &extension, const QgsLayoutExporter::ImageExportSettings &settings, QString &error, QgsFeedback *feedback )
{
  error.clear();

  if ( !iterator->beginRender() )
    return IteratorError;

  int total = iterator->count();
  double step = total > 0 ? 100.0 / total : 100.0;
  int i = 0;
  while ( iterator->next() )
  {
    if ( feedback )
    {
      if ( total > 0 )
        feedback->setProperty( "progress", QObject::tr( "Exporting %1 of %2" ).arg( i + 1 ).arg( total ) );
      else
        feedback->setProperty( "progress", QObject::tr( "Exporting section %1" ).arg( i + 1 ).arg( total ) );
      feedback->setProgress( step * i );
    }
    if ( feedback && feedback->isCanceled() )
    {
      iterator->endRender();
      return Canceled;
    }

    QgsLayoutExporter exporter( iterator->layout() );
    QString filePath = iterator->filePath( baseFilePath, extension );
    ExportResult result = exporter.exportToImage( filePath, settings );
    if ( result != Success )
    {
      if ( result == FileError )
        error = QObject::tr( "Cannot write to %1. This file may be open in another application or may be an invalid path." ).arg( QDir::toNativeSeparators( filePath ) );
      iterator->endRender();
      return result;
    }
    i++;
  }

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  iterator->endRender();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToPdf( const QString &filePath, const QgsLayoutExporter::PdfExportSettings &s )
{
  if ( !mLayout || mLayout->pageCollection()->pageCount() == 0 )
    return PrintError;

  PdfExportSettings settings = s;
  if ( settings.dpi <= 0 )
    settings.dpi = mLayout->renderContext().dpi();

  mErrorFileName.clear();

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;
  LayoutContextSettingsRestorer contextRestorer( mLayout );
  ( void )contextRestorer;
  mLayout->renderContext().setDpi( settings.dpi );
  mLayout->renderContext().setPredefinedScales( settings.predefinedMapScales );

  if ( settings.simplifyGeometries )
  {
    mLayout->renderContext().setSimplifyMethod( createExportSimplifyMethod() );
  }

  std::unique_ptr< QgsLayoutGeoPdfExporter > geoPdfExporter;
  if ( settings.writeGeoPdf || settings.exportLayersAsSeperateFiles )  //#spellok
    geoPdfExporter = std::make_unique< QgsLayoutGeoPdfExporter >( mLayout );

  mLayout->renderContext().setFlags( settings.flags );

  // If we are not printing as raster, temporarily disable advanced effects
  // as QPrinter does not support composition modes and can result
  // in items missing from the output
  mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects, !settings.forceVectorOutput );
  mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagForceVectorOutput, settings.forceVectorOutput );
  mLayout->renderContext().setTextRenderFormat( settings.textRenderFormat );
  mLayout->renderContext().setExportThemes( settings.exportThemes );

  ExportResult result = Success;
  if ( settings.writeGeoPdf || settings.exportLayersAsSeperateFiles )  //#spellok
  {
    mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagRenderLabelsByMapLayer, true );

    // here we need to export layers to individual PDFs
    PdfExportSettings subSettings = settings;
    subSettings.writeGeoPdf = false;
    subSettings.exportLayersAsSeperateFiles = false;  //#spellok

    const QList<QGraphicsItem *> items = mLayout->items( Qt::AscendingOrder );

    QList< QgsLayoutGeoPdfExporter::ComponentLayerDetail > pdfComponents;

    const QDir baseDir = settings.exportLayersAsSeperateFiles ? QFileInfo( filePath ).dir() : QDir();  //#spellok
    const QString baseFileName = settings.exportLayersAsSeperateFiles ? QFileInfo( filePath ).completeBaseName() : QString();  //#spellok

    auto exportFunc = [this, &subSettings, &pdfComponents, &geoPdfExporter, &settings, &baseDir, &baseFileName]( unsigned int layerId, const QgsLayoutItem::ExportLayerDetail & layerDetail )->QgsLayoutExporter::ExportResult
    {
      ExportResult layerExportResult = Success;
      QPrinter printer;
      QgsLayoutGeoPdfExporter::ComponentLayerDetail component;
      component.name = layerDetail.name;
      component.mapLayerId = layerDetail.mapLayerId;
      component.opacity = layerDetail.opacity;
      component.compositionMode = layerDetail.compositionMode;
      component.group = layerDetail.mapTheme;
      component.sourcePdfPath = settings.writeGeoPdf ? geoPdfExporter->generateTemporaryFilepath( QStringLiteral( "layer_%1.pdf" ).arg( layerId ) ) : baseDir.filePath( QStringLiteral( "%1_%2.pdf" ).arg( baseFileName ).arg( layerId, 4, 10, QChar( '0' ) ) );
      pdfComponents << component;
      preparePrintAsPdf( mLayout, printer, component.sourcePdfPath );
      preparePrint( mLayout, printer, false );
      QPainter p;
      if ( !p.begin( &printer ) )
      {
        //error beginning print
        return FileError;
      }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
      p.setRenderHint( QPainter::LosslessImageRendering, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering );
#endif
      layerExportResult = printPrivate( printer, p, false, subSettings.dpi, subSettings.rasterizeWholeImage );
      p.end();
      return layerExportResult;
    };
    result = handleLayeredExport( items, exportFunc );
    if ( result != Success )
      return result;

    if ( settings.writeGeoPdf )
    {
      QgsAbstractGeoPdfExporter::ExportDetails details;
      details.dpi = settings.dpi;
      // TODO - multipages
      QgsLayoutSize pageSize = mLayout->pageCollection()->page( 0 )->sizeWithUnits();
      QgsLayoutSize pageSizeMM = mLayout->renderContext().measurementConverter().convert( pageSize, QgsUnitTypes::LayoutMillimeters );
      details.pageSizeMm = pageSizeMM.toQSizeF();

      if ( settings.exportMetadata )
      {
        // copy layout metadata to GeoPDF export settings
        details.author = mLayout->project()->metadata().author();
        details.producer = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
        details.creator = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
        details.creationDateTime = mLayout->project()->metadata().creationDateTime();
        details.subject = mLayout->project()->metadata().abstract();
        details.title = mLayout->project()->metadata().title();
        details.keywords = mLayout->project()->metadata().keywords();
      }

      const QList< QgsMapLayer * > layers = mLayout->project()->mapLayers().values();
      for ( const QgsMapLayer *layer : layers )
      {
        details.layerIdToPdfLayerTreeNameMap.insert( layer->id(), layer->name() );
      }

      if ( settings.appendGeoreference )
      {
        // setup georeferencing
        QList< QgsLayoutItemMap * > maps;
        mLayout->layoutItems( maps );
        for ( QgsLayoutItemMap *map : std::as_const( maps ) )
        {
          QgsAbstractGeoPdfExporter::GeoReferencedSection georef;
          georef.crs = map->crs();

          const QPointF topLeft = map->mapToScene( QPointF( 0, 0 ) );
          const QPointF topRight = map->mapToScene( QPointF( map->rect().width(), 0 ) );
          const QPointF bottomLeft = map->mapToScene( QPointF( 0, map->rect().height() ) );
          const QPointF bottomRight = map->mapToScene( QPointF( map->rect().width(), map->rect().height() ) );
          const QgsLayoutPoint topLeftMm = mLayout->convertFromLayoutUnits( topLeft, QgsUnitTypes::LayoutMillimeters );
          const QgsLayoutPoint topRightMm = mLayout->convertFromLayoutUnits( topRight, QgsUnitTypes::LayoutMillimeters );
          const QgsLayoutPoint bottomLeftMm = mLayout->convertFromLayoutUnits( bottomLeft, QgsUnitTypes::LayoutMillimeters );
          const QgsLayoutPoint bottomRightMm = mLayout->convertFromLayoutUnits( bottomRight, QgsUnitTypes::LayoutMillimeters );

          georef.pageBoundsPolygon.setExteriorRing( new QgsLineString( QVector< QgsPointXY >() << QgsPointXY( topLeftMm.x(), topLeftMm.y() )
              << QgsPointXY( topRightMm.x(), topRightMm.y() )
              << QgsPointXY( bottomRightMm.x(), bottomRightMm.y() )
              << QgsPointXY( bottomLeftMm.x(), bottomLeftMm.y() )
              << QgsPointXY( topLeftMm.x(), topLeftMm.y() ) ) );

          georef.controlPoints.reserve( 4 );
          const QTransform t = map->layoutToMapCoordsTransform();
          const QgsPointXY topLeftMap = t.map( topLeft );
          const QgsPointXY topRightMap = t.map( topRight );
          const QgsPointXY bottomLeftMap = t.map( bottomLeft );
          const QgsPointXY bottomRightMap = t.map( bottomRight );

          georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( topLeftMm.x(), topLeftMm.y() ), topLeftMap );
          georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( topRightMm.x(), topRightMm.y() ), topRightMap );
          georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( bottomLeftMm.x(), bottomLeftMm.y() ), bottomLeftMap );
          georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( bottomRightMm.x(), bottomRightMm.y() ), bottomRightMap );
          details.georeferencedSections << georef;
        }
      }

      details.customLayerTreeGroups = geoPdfExporter->customLayerTreeGroups();
      details.initialLayerVisibility = geoPdfExporter->initialLayerVisibility();
      details.layerOrder = geoPdfExporter->layerOrder();
      details.includeFeatures = settings.includeGeoPdfFeatures;
      details.useOgcBestPracticeFormatGeoreferencing = settings.useOgcBestPracticeFormatGeoreferencing;
      details.useIso32000ExtensionFormatGeoreferencing = settings.useIso32000ExtensionFormatGeoreferencing;

      if ( !geoPdfExporter->finalize( pdfComponents, filePath, details ) )
        result = PrintError;
    }
    else
    {
      result = Success;
    }
  }
  else
  {
    QPrinter printer;
    preparePrintAsPdf( mLayout, printer, filePath );
    preparePrint( mLayout, printer, false );
    QPainter p;
    if ( !p.begin( &printer ) )
    {
      //error beginning print
      return FileError;
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    p.setRenderHint( QPainter::LosslessImageRendering, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering );
#endif
    result = printPrivate( printer, p, false, settings.dpi, settings.rasterizeWholeImage );
    p.end();

    bool shouldAppendGeoreference = settings.appendGeoreference && mLayout && mLayout->referenceMap() && mLayout->referenceMap()->page() == 0;
    if ( settings.appendGeoreference || settings.exportMetadata )
    {
      georeferenceOutputPrivate( filePath, nullptr, QRectF(), settings.dpi, shouldAppendGeoreference, settings.exportMetadata );
    }
  }
  captureLabelingResults();
  return result;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToPdf( QgsAbstractLayoutIterator *iterator, const QString &fileName, const QgsLayoutExporter::PdfExportSettings &s, QString &error, QgsFeedback *feedback )
{
  error.clear();

  if ( !iterator->beginRender() )
    return IteratorError;

  PdfExportSettings settings = s;

  QPrinter printer;
  QPainter p;

  int total = iterator->count();
  double step = total > 0 ? 100.0 / total : 100.0;
  int i = 0;
  bool first = true;
  while ( iterator->next() )
  {
    if ( feedback )
    {
      if ( total > 0 )
        feedback->setProperty( "progress", QObject::tr( "Exporting %1 of %2" ).arg( i + 1 ).arg( total ) );
      else
        feedback->setProperty( "progress", QObject::tr( "Exporting section %1" ).arg( i + 1 ) );
      feedback->setProgress( step * i );
    }
    if ( feedback && feedback->isCanceled() )
    {
      iterator->endRender();
      return Canceled;
    }

    if ( s.dpi <= 0 )
      settings.dpi = iterator->layout()->renderContext().dpi();

    LayoutContextPreviewSettingRestorer restorer( iterator->layout() );
    ( void )restorer;
    LayoutContextSettingsRestorer contextRestorer( iterator->layout() );
    ( void )contextRestorer;
    iterator->layout()->renderContext().setDpi( settings.dpi );

    iterator->layout()->renderContext().setFlags( settings.flags );
    iterator->layout()->renderContext().setPredefinedScales( settings.predefinedMapScales );

    if ( settings.simplifyGeometries )
    {
      iterator->layout()->renderContext().setSimplifyMethod( createExportSimplifyMethod() );
    }

    // If we are not printing as raster, temporarily disable advanced effects
    // as QPrinter does not support composition modes and can result
    // in items missing from the output
    iterator->layout()->renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects, !settings.forceVectorOutput );

    iterator->layout()->renderContext().setFlag( QgsLayoutRenderContext::FlagForceVectorOutput, settings.forceVectorOutput );

    iterator->layout()->renderContext().setTextRenderFormat( settings.textRenderFormat );

    if ( first )
    {
      preparePrintAsPdf( iterator->layout(), printer, fileName );
      preparePrint( iterator->layout(), printer, false );

      if ( !p.begin( &printer ) )
      {
        //error beginning print
        return PrintError;
      }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
      p.setRenderHint( QPainter::LosslessImageRendering, iterator->layout()->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering );
#endif
    }

    QgsLayoutExporter exporter( iterator->layout() );

    ExportResult result = exporter.printPrivate( printer, p, !first, settings.dpi, settings.rasterizeWholeImage );
    if ( result != Success )
    {
      if ( result == FileError )
        error = QObject::tr( "Cannot write to %1. This file may be open in another application or may be an invalid path." ).arg( QDir::toNativeSeparators( fileName ) );
      iterator->endRender();
      return result;
    }
    first = false;
    i++;
  }

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  iterator->endRender();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToPdfs( QgsAbstractLayoutIterator *iterator, const QString &baseFilePath, const QgsLayoutExporter::PdfExportSettings &settings, QString &error, QgsFeedback *feedback )
{
  error.clear();

  if ( !iterator->beginRender() )
    return IteratorError;

  int total = iterator->count();
  double step = total > 0 ? 100.0 / total : 100.0;
  int i = 0;
  while ( iterator->next() )
  {
    if ( feedback )
    {
      if ( total > 0 )
        feedback->setProperty( "progress", QObject::tr( "Exporting %1 of %2" ).arg( i + 1 ).arg( total ) );
      else
        feedback->setProperty( "progress", QObject::tr( "Exporting section %1" ).arg( i + 1 ).arg( total ) );
      feedback->setProgress( step * i );
    }
    if ( feedback && feedback->isCanceled() )
    {
      iterator->endRender();
      return Canceled;
    }

    QString filePath = iterator->filePath( baseFilePath, QStringLiteral( "pdf" ) );

    QgsLayoutExporter exporter( iterator->layout() );
    ExportResult result = exporter.exportToPdf( filePath, settings );
    if ( result != Success )
    {
      if ( result == FileError )
        error = QObject::tr( "Cannot write to %1. This file may be open in another application or may be an invalid path." ).arg( QDir::toNativeSeparators( filePath ) );
      iterator->endRender();
      return result;
    }
    i++;
  }

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  iterator->endRender();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::print( QPrinter &printer, const QgsLayoutExporter::PrintExportSettings &s )
{
  if ( !mLayout )
    return PrintError;

  QgsLayoutExporter::PrintExportSettings settings = s;
  if ( settings.dpi <= 0 )
    settings.dpi = mLayout->renderContext().dpi();

  mErrorFileName.clear();

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;
  LayoutContextSettingsRestorer contextRestorer( mLayout );
  ( void )contextRestorer;
  mLayout->renderContext().setDpi( settings.dpi );

  mLayout->renderContext().setFlags( settings.flags );
  mLayout->renderContext().setPredefinedScales( settings.predefinedMapScales );
  // If we are not printing as raster, temporarily disable advanced effects
  // as QPrinter does not support composition modes and can result
  // in items missing from the output
  mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects, !settings.rasterizeWholeImage );

  preparePrint( mLayout, printer, true );
  QPainter p;
  if ( !p.begin( &printer ) )
  {
    //error beginning print
    return PrintError;
  }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  p.setRenderHint( QPainter::LosslessImageRendering, mLayout->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering );
#endif
  ExportResult result = printPrivate( printer, p, false, settings.dpi, settings.rasterizeWholeImage );
  p.end();

  captureLabelingResults();
  return result;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::print( QgsAbstractLayoutIterator *iterator, QPrinter &printer, const QgsLayoutExporter::PrintExportSettings &s, QString &error, QgsFeedback *feedback )
{
  error.clear();

  if ( !iterator->beginRender() )
    return IteratorError;

  PrintExportSettings settings = s;

  QPainter p;

  int total = iterator->count();
  double step = total > 0 ? 100.0 / total : 100.0;
  int i = 0;
  bool first = true;
  while ( iterator->next() )
  {
    if ( feedback )
    {
      if ( total > 0 )
        feedback->setProperty( "progress", QObject::tr( "Printing %1 of %2" ).arg( i + 1 ).arg( total ) );
      else
        feedback->setProperty( "progress", QObject::tr( "Printing section %1" ).arg( i + 1 ).arg( total ) );
      feedback->setProgress( step * i );
    }
    if ( feedback && feedback->isCanceled() )
    {
      iterator->endRender();
      return Canceled;
    }

    if ( s.dpi <= 0 )
      settings.dpi = iterator->layout()->renderContext().dpi();

    LayoutContextPreviewSettingRestorer restorer( iterator->layout() );
    ( void )restorer;
    LayoutContextSettingsRestorer contextRestorer( iterator->layout() );
    ( void )contextRestorer;
    iterator->layout()->renderContext().setDpi( settings.dpi );

    iterator->layout()->renderContext().setFlags( settings.flags );
    iterator->layout()->renderContext().setPredefinedScales( settings.predefinedMapScales );

    // If we are not printing as raster, temporarily disable advanced effects
    // as QPrinter does not support composition modes and can result
    // in items missing from the output
    iterator->layout()->renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects, !settings.rasterizeWholeImage );

    if ( first )
    {
      preparePrint( iterator->layout(), printer, true );

      if ( !p.begin( &printer ) )
      {
        //error beginning print
        return PrintError;
      }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
      p.setRenderHint( QPainter::LosslessImageRendering, iterator->layout()->renderContext().flags() & QgsLayoutRenderContext::FlagLosslessImageRendering );
#endif
    }

    QgsLayoutExporter exporter( iterator->layout() );

    ExportResult result = exporter.printPrivate( printer, p, !first, settings.dpi, settings.rasterizeWholeImage );
    if ( result != Success )
    {
      iterator->endRender();
      return result;
    }
    first = false;
    i++;
  }

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  iterator->endRender();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToSvg( const QString &filePath, const QgsLayoutExporter::SvgExportSettings &s )
{
  if ( !mLayout )
    return PrintError;

  SvgExportSettings settings = s;
  if ( settings.dpi <= 0 )
    settings.dpi = mLayout->renderContext().dpi();

  mErrorFileName.clear();

  LayoutContextPreviewSettingRestorer restorer( mLayout );
  ( void )restorer;
  LayoutContextSettingsRestorer contextRestorer( mLayout );
  ( void )contextRestorer;
  mLayout->renderContext().setDpi( settings.dpi );

  mLayout->renderContext().setFlags( settings.flags );
  mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagForceVectorOutput, settings.forceVectorOutput );
  mLayout->renderContext().setTextRenderFormat( s.textRenderFormat );
  mLayout->renderContext().setPredefinedScales( settings.predefinedMapScales );

  if ( settings.simplifyGeometries )
  {
    mLayout->renderContext().setSimplifyMethod( createExportSimplifyMethod() );
  }

  QFileInfo fi( filePath );
  PageExportDetails pageDetails;
  pageDetails.directory = fi.path();
  pageDetails.baseName = fi.baseName();
  pageDetails.extension = fi.completeSuffix();

  double inchesToLayoutUnits = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( 1, QgsUnitTypes::LayoutInches ) );

  for ( int i = 0; i < mLayout->pageCollection()->pageCount(); ++i )
  {
    if ( !mLayout->pageCollection()->shouldExportPage( i ) )
    {
      continue;
    }

    pageDetails.page = i;
    QString fileName = generateFileName( pageDetails );

    QgsLayoutItemPage *pageItem = mLayout->pageCollection()->page( i );
    QRectF bounds;
    if ( settings.cropToContents )
    {
      if ( mLayout->pageCollection()->pageCount() == 1 )
      {
        // single page, so include everything
        bounds = mLayout->layoutBounds( true );
      }
      else
      {
        // multi page, so just clip to items on current page
        bounds = mLayout->pageItemBounds( i, true );
      }
      bounds = bounds.adjusted( -settings.cropMargins.left(),
                                -settings.cropMargins.top(),
                                settings.cropMargins.right(),
                                settings.cropMargins.bottom() );
    }
    else
    {
      bounds = QRectF( pageItem->pos().x(), pageItem->pos().y(), pageItem->rect().width(), pageItem->rect().height() );
    }

    //width in pixel
    int width = static_cast< int >( bounds.width() * settings.dpi / inchesToLayoutUnits );
    //height in pixel
    int height = static_cast< int >( bounds.height() * settings.dpi / inchesToLayoutUnits );
    if ( width == 0 || height == 0 )
    {
      //invalid size, skip this page
      continue;
    }

    if ( settings.exportAsLayers )
    {
      mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagRenderLabelsByMapLayer, settings.exportLabelsToSeparateLayers );
      const QRectF paperRect = QRectF( pageItem->pos().x(),
                                       pageItem->pos().y(),
                                       pageItem->rect().width(),
                                       pageItem->rect().height() );
      QDomDocument svg;
      QDomNode svgDocRoot;
      const QList<QGraphicsItem *> items = mLayout->items( paperRect,
                                           Qt::IntersectsItemBoundingRect,
                                           Qt::AscendingOrder );

      auto exportFunc = [this, &settings, width, height, i, bounds, fileName, &svg, &svgDocRoot]( unsigned int layerId, const QgsLayoutItem::ExportLayerDetail & layerDetail )->QgsLayoutExporter::ExportResult
      {
        return renderToLayeredSvg( settings, width, height, i, bounds, fileName, layerId, layerDetail.name, svg, svgDocRoot, settings.exportMetadata );
      };
      ExportResult res = handleLayeredExport( items, exportFunc );
      if ( res != Success )
        return res;

      if ( settings.exportMetadata )
        appendMetadataToSvg( svg );

      QFile out( fileName );
      bool openOk = out.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate );
      if ( !openOk )
      {
        mErrorFileName = fileName;
        return FileError;
      }

      out.write( svg.toByteArray() );
    }
    else
    {
      QBuffer svgBuffer;
      {
        QSvgGenerator generator;
        if ( settings.exportMetadata )
        {
          generator.setTitle( mLayout->project()->metadata().title() );
          generator.setDescription( mLayout->project()->metadata().abstract() );
        }
        generator.setOutputDevice( &svgBuffer );
        generator.setSize( QSize( width, height ) );
        generator.setViewBox( QRect( 0, 0, width, height ) );
        generator.setResolution( static_cast< int >( std::round( settings.dpi ) ) );

        QPainter p;
        bool createOk = p.begin( &generator );
        if ( !createOk )
        {
          mErrorFileName = fileName;
          return FileError;
        }

        if ( settings.cropToContents )
          renderRegion( &p, bounds );
        else
          renderPage( &p, i );

        p.end();
      }
      {
        svgBuffer.close();
        svgBuffer.open( QIODevice::ReadOnly );
        QDomDocument svg;
        QString errorMsg;
        int errorLine;
        if ( ! svg.setContent( &svgBuffer, false, &errorMsg, &errorLine ) )
        {
          mErrorFileName = fileName;
          return SvgLayerError;
        }

        if ( settings.exportMetadata )
          appendMetadataToSvg( svg );

        QFile out( fileName );
        bool openOk = out.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate );
        if ( !openOk )
        {
          mErrorFileName = fileName;
          return FileError;
        }

        out.write( svg.toByteArray() );
      }
    }
  }
  captureLabelingResults();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToSvg( QgsAbstractLayoutIterator *iterator, const QString &baseFilePath, const QgsLayoutExporter::SvgExportSettings &settings, QString &error, QgsFeedback *feedback )
{
  error.clear();

  if ( !iterator->beginRender() )
    return IteratorError;

  int total = iterator->count();
  double step = total > 0 ? 100.0 / total : 100.0;
  int i = 0;
  while ( iterator->next() )
  {
    if ( feedback )
    {
      if ( total > 0 )
        feedback->setProperty( "progress", QObject::tr( "Exporting %1 of %2" ).arg( i + 1 ).arg( total ) );
      else
        feedback->setProperty( "progress", QObject::tr( "Exporting section %1" ).arg( i + 1 ).arg( total ) );

      feedback->setProgress( step * i );
    }
    if ( feedback && feedback->isCanceled() )
    {
      iterator->endRender();
      return Canceled;
    }

    QString filePath = iterator->filePath( baseFilePath, QStringLiteral( "svg" ) );

    QgsLayoutExporter exporter( iterator->layout() );
    ExportResult result = exporter.exportToSvg( filePath, settings );
    if ( result != Success )
    {
      if ( result == FileError )
        error = QObject::tr( "Cannot write to %1. This file may be open in another application or may be an invalid path." ).arg( QDir::toNativeSeparators( filePath ) );
      iterator->endRender();
      return result;
    }
    i++;
  }

  if ( feedback )
  {
    feedback->setProgress( 100 );
  }

  iterator->endRender();
  return Success;

}

QMap<QString, QgsLabelingResults *> QgsLayoutExporter::labelingResults()
{
  return mLabelingResults;
}

QMap<QString, QgsLabelingResults *> QgsLayoutExporter::takeLabelingResults()
{
  QMap<QString, QgsLabelingResults *> res;
  std::swap( mLabelingResults, res );
  return res;
}

void QgsLayoutExporter::preparePrintAsPdf( QgsLayout *layout, QPrinter &printer, const QString &filePath )
{
  QFileInfo fi( filePath );
  QDir dir;
  if ( !dir.exists( fi.absolutePath() ) )
  {
    dir.mkpath( fi.absolutePath() );
  }

  printer.setOutputFileName( filePath );
  printer.setOutputFormat( QPrinter::PdfFormat );

  updatePrinterPageSize( layout, printer, firstPageToBeExported( layout ) );

  // TODO: add option for this in layout
  // May not work on Windows or non-X11 Linux. Works fine on Mac using QPrinter::NativeFormat
  //printer.setFontEmbeddingEnabled( true );

#if defined(HAS_KDE_QT5_PDF_TRANSFORM_FIX) || QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  // paint engine hack not required, fixed upstream
#else
  QgsPaintEngineHack::fixEngineFlags( printer.paintEngine() );
#endif
}

void QgsLayoutExporter::preparePrint( QgsLayout *layout, QPrinter &printer, bool setFirstPageSize )
{
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );

  //set user-defined resolution
  printer.setResolution( static_cast< int>( std::round( layout->renderContext().dpi() ) ) );

  if ( setFirstPageSize )
  {
    updatePrinterPageSize( layout, printer, firstPageToBeExported( layout ) );
  }
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::print( QPrinter &printer )
{
  if ( mLayout->pageCollection()->pageCount() == 0 )
    return PrintError;

  preparePrint( mLayout, printer, true );
  QPainter p;
  if ( !p.begin( &printer ) )
  {
    //error beginning print
    return PrintError;
  }

  printPrivate( printer, p );
  p.end();
  return Success;
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::printPrivate( QPrinter &printer, QPainter &painter, bool startNewPage, double dpi, bool rasterize )
{
  //layout starts page numbering at 0
  int fromPage = ( printer.fromPage() < 1 ) ? 0 : printer.fromPage() - 1;
  int toPage = ( printer.toPage() < 1 ) ? mLayout->pageCollection()->pageCount() - 1 : printer.toPage() - 1;

  bool pageExported = false;
  if ( rasterize )
  {
    for ( int i = fromPage; i <= toPage; ++i )
    {
      if ( !mLayout->pageCollection()->shouldExportPage( i ) )
      {
        continue;
      }

      updatePrinterPageSize( mLayout, printer, i );
      if ( ( pageExported && i > fromPage ) || startNewPage )
      {
        printer.newPage();
      }

      QImage image = renderPageToImage( i, QSize(), dpi );
      if ( !image.isNull() )
      {
        QRectF targetArea( 0, 0, image.width(), image.height() );
        painter.drawImage( targetArea, image, targetArea );
      }
      else
      {
        return MemoryError;
      }
      pageExported = true;
    }
  }
  else
  {
    for ( int i = fromPage; i <= toPage; ++i )
    {
      if ( !mLayout->pageCollection()->shouldExportPage( i ) )
      {
        continue;
      }

      updatePrinterPageSize( mLayout, printer, i );

      if ( ( pageExported && i > fromPage ) || startNewPage )
      {
        printer.newPage();
      }
      renderPage( &painter, i );
      pageExported = true;
    }
  }
  return Success;
}

void QgsLayoutExporter::updatePrinterPageSize( QgsLayout *layout, QPrinter &printer, int page )
{
  QgsLayoutSize pageSize = layout->pageCollection()->page( page )->sizeWithUnits();
  QgsLayoutSize pageSizeMM = layout->renderContext().measurementConverter().convert( pageSize, QgsUnitTypes::LayoutMillimeters );

  QPageLayout pageLayout( QPageSize( pageSizeMM.toQSizeF(), QPageSize::Millimeter ),
                          QPageLayout::Portrait,
                          QMarginsF( 0, 0, 0, 0 ) );
  pageLayout.setMode( QPageLayout::FullPageMode );
  printer.setPageLayout( pageLayout );
  printer.setFullPage( true );
  printer.setPageMargins( QMarginsF( 0, 0, 0, 0 ) );
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::renderToLayeredSvg( const SvgExportSettings &settings, double width, double height, int page, const QRectF &bounds, const QString &filename, unsigned int svgLayerId, const QString &layerName, QDomDocument &svg, QDomNode &svgDocRoot, bool includeMetadata ) const
{
  QBuffer svgBuffer;
  {
    QSvgGenerator generator;
    if ( includeMetadata )
    {
      if ( const QgsMasterLayoutInterface *l = dynamic_cast< const QgsMasterLayoutInterface * >( mLayout.data() ) )
        generator.setTitle( l->name() );
      else if ( mLayout->project() )
        generator.setTitle( mLayout->project()->title() );
    }

    generator.setOutputDevice( &svgBuffer );
    generator.setSize( QSize( static_cast< int >( std::round( width ) ),
                              static_cast< int >( std::round( height ) ) ) );
    generator.setViewBox( QRect( 0, 0,
                                 static_cast< int >( std::round( width ) ),
                                 static_cast< int >( std::round( height ) ) ) );
    generator.setResolution( static_cast< int >( std::round( settings.dpi ) ) ); //because the rendering is done in mm, convert the dpi

    QPainter svgPainter( &generator );
    if ( settings.cropToContents )
      renderRegion( &svgPainter, bounds );
    else
      renderPage( &svgPainter, page );
  }

// post-process svg output to create groups in a single svg file
// we create inkscape layers since it's nice and clean and free
// and fully svg compatible
  {
    svgBuffer.close();
    svgBuffer.open( QIODevice::ReadOnly );
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    if ( ! doc.setContent( &svgBuffer, false, &errorMsg, &errorLine ) )
    {
      mErrorFileName = filename;
      return SvgLayerError;
    }
    if ( 1 == svgLayerId )
    {
      svg = QDomDocument( doc.doctype() );
      svg.appendChild( svg.importNode( doc.firstChild(), false ) );
      svgDocRoot = svg.importNode( doc.elementsByTagName( QStringLiteral( "svg" ) ).at( 0 ), false );
      svgDocRoot.toElement().setAttribute( QStringLiteral( "xmlns:inkscape" ), QStringLiteral( "http://www.inkscape.org/namespaces/inkscape" ) );
      svg.appendChild( svgDocRoot );
    }
    QDomNode mainGroup = svg.importNode( doc.elementsByTagName( QStringLiteral( "g" ) ).at( 0 ), true );
    mainGroup.toElement().setAttribute( QStringLiteral( "id" ), layerName );
    mainGroup.toElement().setAttribute( QStringLiteral( "inkscape:label" ), layerName );
    mainGroup.toElement().setAttribute( QStringLiteral( "inkscape:groupmode" ), QStringLiteral( "layer" ) );
    QDomNode defs = svg.importNode( doc.elementsByTagName( QStringLiteral( "defs" ) ).at( 0 ), true );
    svgDocRoot.appendChild( defs );
    svgDocRoot.appendChild( mainGroup );
  }
  return Success;
}

void QgsLayoutExporter::appendMetadataToSvg( QDomDocument &svg ) const
{
  const QgsProjectMetadata &metadata = mLayout->project()->metadata();
  QDomElement metadataElement = svg.createElement( QStringLiteral( "metadata" ) );
  QDomElement rdfElement = svg.createElement( QStringLiteral( "rdf:RDF" ) );
  rdfElement.setAttribute( QStringLiteral( "xmlns:rdf" ), QStringLiteral( "http://www.w3.org/1999/02/22-rdf-syntax-ns#" ) );
  rdfElement.setAttribute( QStringLiteral( "xmlns:rdfs" ), QStringLiteral( "http://www.w3.org/2000/01/rdf-schema#" ) );
  rdfElement.setAttribute( QStringLiteral( "xmlns:dc" ), QStringLiteral( "http://purl.org/dc/elements/1.1/" ) );
  QDomElement descriptionElement = svg.createElement( QStringLiteral( "rdf:Description" ) );
  QDomElement workElement = svg.createElement( QStringLiteral( "cc:Work" ) );
  workElement.setAttribute( QStringLiteral( "rdf:about" ), QString() );

  auto addTextNode = [&workElement, &descriptionElement, &svg]( const QString & tag, const QString & value )
  {
    // inkscape compatible
    QDomElement element = svg.createElement( tag );
    QDomText t = svg.createTextNode( value );
    element.appendChild( t );
    workElement.appendChild( element );

    // svg spec compatible
    descriptionElement.setAttribute( tag, value );
  };

  addTextNode( QStringLiteral( "dc:format" ), QStringLiteral( "image/svg+xml" ) );
  addTextNode( QStringLiteral( "dc:title" ), metadata.title() );
  addTextNode( QStringLiteral( "dc:date" ), metadata.creationDateTime().toString( Qt::ISODate ) );
  addTextNode( QStringLiteral( "dc:identifier" ), metadata.identifier() );
  addTextNode( QStringLiteral( "dc:description" ), metadata.abstract() );

  auto addAgentNode = [&workElement, &descriptionElement, &svg]( const QString & tag, const QString & value )
  {
    // inkscape compatible
    QDomElement inkscapeElement = svg.createElement( tag );
    QDomElement agentElement = svg.createElement( QStringLiteral( "cc:Agent" ) );
    QDomElement titleElement = svg.createElement( QStringLiteral( "dc:title" ) );
    QDomText t = svg.createTextNode( value );
    titleElement.appendChild( t );
    agentElement.appendChild( titleElement );
    inkscapeElement.appendChild( agentElement );
    workElement.appendChild( inkscapeElement );

    // svg spec compatible
    QDomElement bagElement = svg.createElement( QStringLiteral( "rdf:Bag" ) );
    QDomElement liElement = svg.createElement( QStringLiteral( "rdf:li" ) );
    t = svg.createTextNode( value );
    liElement.appendChild( t );
    bagElement.appendChild( liElement );

    QDomElement element = svg.createElement( tag );
    element.appendChild( bagElement );
    descriptionElement.appendChild( element );
  };

  addAgentNode( QStringLiteral( "dc:creator" ), metadata.author() );
  addAgentNode( QStringLiteral( "dc:publisher" ), QStringLiteral( "QGIS %1" ).arg( Qgis::version() ) );

  // keywords
  {
    QDomElement element = svg.createElement( QStringLiteral( "dc:subject" ) );
    QDomElement bagElement = svg.createElement( QStringLiteral( "rdf:Bag" ) );
    QgsAbstractMetadataBase::KeywordMap keywords = metadata.keywords();
    for ( auto it = keywords.constBegin(); it != keywords.constEnd(); ++it )
    {
      const QStringList words = it.value();
      for ( const QString &keyword : words )
      {
        QDomElement liElement = svg.createElement( QStringLiteral( "rdf:li" ) );
        QDomText t = svg.createTextNode( keyword );
        liElement.appendChild( t );
        bagElement.appendChild( liElement );
      }
    }
    element.appendChild( bagElement );
    workElement.appendChild( element );
    descriptionElement.appendChild( element );
  }

  rdfElement.appendChild( descriptionElement );
  rdfElement.appendChild( workElement );
  metadataElement.appendChild( rdfElement );
  svg.documentElement().appendChild( metadataElement );
  svg.documentElement().setAttribute( QStringLiteral( "xmlns:cc" ), QStringLiteral( "http://creativecommons.org/ns#" ) );
}

std::unique_ptr<double[]> QgsLayoutExporter::computeGeoTransform( const QgsLayoutItemMap *map, const QRectF &region, double dpi ) const
{
  if ( !map )
    map = mLayout->referenceMap();

  if ( !map )
    return nullptr;

  if ( dpi < 0 )
    dpi = mLayout->renderContext().dpi();

  // calculate region of composition to export (in mm)
  QRectF exportRegion = region;
  if ( !exportRegion.isValid() )
  {
    int pageNumber = map->page();

    QgsLayoutItemPage *page = mLayout->pageCollection()->page( pageNumber );
    double pageY = page->pos().y();
    QSizeF pageSize = page->rect().size();
    exportRegion = QRectF( 0, pageY, pageSize.width(), pageSize.height() );
  }

  // map rectangle (in mm)
  QRectF mapItemSceneRect = map->mapRectToScene( map->rect() );

  // destination width/height in mm
  double outputHeightMM = exportRegion.height();
  double outputWidthMM = exportRegion.width();

  // map properties
  QgsRectangle mapExtent = map->extent();
  double mapXCenter = mapExtent.center().x();
  double mapYCenter = mapExtent.center().y();
  double alpha = - map->mapRotation() / 180 * M_PI;
  double sinAlpha = std::sin( alpha );
  double cosAlpha = std::cos( alpha );

  // get the extent (in map units) for the exported region
  QPointF mapItemPos = map->pos();
  //adjust item position so it is relative to export region
  mapItemPos.rx() -= exportRegion.left();
  mapItemPos.ry() -= exportRegion.top();

  // calculate extent of entire page in map units
  double xRatio = mapExtent.width() / mapItemSceneRect.width();
  double yRatio = mapExtent.height() / mapItemSceneRect.height();
  double xmin = mapExtent.xMinimum() - mapItemPos.x() * xRatio;
  double ymax = mapExtent.yMaximum() + mapItemPos.y() * yRatio;
  QgsRectangle paperExtent( xmin, ymax - outputHeightMM * yRatio, xmin + outputWidthMM * xRatio, ymax );

  // calculate origin of page
  double X0 = paperExtent.xMinimum();
  double Y0 = paperExtent.yMaximum();

  if ( !qgsDoubleNear( alpha, 0.0 ) )
  {
    // translate origin to account for map rotation
    double X1 = X0 - mapXCenter;
    double Y1 = Y0 - mapYCenter;
    double X2 = X1 * cosAlpha + Y1 * sinAlpha;
    double Y2 = -X1 * sinAlpha + Y1 * cosAlpha;
    X0 = X2 + mapXCenter;
    Y0 = Y2 + mapYCenter;
  }

  // calculate scaling of pixels
  int pageWidthPixels = static_cast< int >( dpi * outputWidthMM / 25.4 );
  int pageHeightPixels = static_cast< int >( dpi * outputHeightMM / 25.4 );
  double pixelWidthScale = paperExtent.width() / pageWidthPixels;
  double pixelHeightScale = paperExtent.height() / pageHeightPixels;

  // transform matrix
  std::unique_ptr<double[]> t( new double[6] );
  t[0] = X0;
  t[1] = cosAlpha * pixelWidthScale;
  t[2] = -sinAlpha * pixelWidthScale;
  t[3] = Y0;
  t[4] = -sinAlpha * pixelHeightScale;
  t[5] = -cosAlpha * pixelHeightScale;

  return t;
}

void QgsLayoutExporter::writeWorldFile( const QString &worldFileName, double a, double b, double c, double d, double e, double f ) const
{
  QFile worldFile( worldFileName );
  if ( !worldFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return;
  }
  QTextStream fout( &worldFile );

  // QString::number does not use locale settings (for the decimal point)
  // which is what we want here
  fout << QString::number( a, 'f', 12 ) << "\r\n";
  fout << QString::number( d, 'f', 12 ) << "\r\n";
  fout << QString::number( b, 'f', 12 ) << "\r\n";
  fout << QString::number( e, 'f', 12 ) << "\r\n";
  fout << QString::number( c, 'f', 12 ) << "\r\n";
  fout << QString::number( f, 'f', 12 ) << "\r\n";
}

bool QgsLayoutExporter::georeferenceOutput( const QString &file, QgsLayoutItemMap *map, const QRectF &exportRegion, double dpi ) const
{
  return georeferenceOutputPrivate( file, map, exportRegion, dpi, false );
}

bool QgsLayoutExporter::georeferenceOutputPrivate( const QString &file, QgsLayoutItemMap *map, const QRectF &exportRegion, double dpi, bool includeGeoreference, bool includeMetadata ) const
{
  if ( !mLayout )
    return false;

  if ( !map && includeGeoreference )
    map = mLayout->referenceMap();

  std::unique_ptr<double[]> t;

  if ( map && includeGeoreference )
  {
    if ( dpi < 0 )
      dpi = mLayout->renderContext().dpi();

    t = computeGeoTransform( map, exportRegion, dpi );
  }

  // important - we need to manually specify the DPI in advance, as GDAL will otherwise
  // assume a DPI of 150
  CPLSetConfigOption( "GDAL_PDF_DPI", QString::number( dpi ).toLocal8Bit().constData() );
  gdal::dataset_unique_ptr outputDS( GDALOpen( file.toLocal8Bit().constData(), GA_Update ) );
  if ( outputDS )
  {
    if ( t )
      GDALSetGeoTransform( outputDS.get(), t.get() );

    if ( includeMetadata )
    {
      QString creationDateString;
      const QDateTime creationDateTime = mLayout->project()->metadata().creationDateTime();
      if ( creationDateTime.isValid() )
      {
        creationDateString = QStringLiteral( "D:%1" ).arg( mLayout->project()->metadata().creationDateTime().toString( QStringLiteral( "yyyyMMddHHmmss" ) ) );
        if ( creationDateTime.timeZone().isValid() )
        {
          int offsetFromUtc = creationDateTime.timeZone().offsetFromUtc( creationDateTime );
          creationDateString += ( offsetFromUtc >= 0 ) ? '+' : '-';
          offsetFromUtc = std::abs( offsetFromUtc );
          int offsetHours = offsetFromUtc / 3600;
          int offsetMins = ( offsetFromUtc % 3600 ) / 60;
          creationDateString += QStringLiteral( "%1'%2'" ).arg( offsetHours ).arg( offsetMins );
        }
      }
      GDALSetMetadataItem( outputDS.get(), "CREATION_DATE", creationDateString.toUtf8().constData(), nullptr );

      GDALSetMetadataItem( outputDS.get(), "AUTHOR", mLayout->project()->metadata().author().toUtf8().constData(), nullptr );
      const QString creator = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
      GDALSetMetadataItem( outputDS.get(), "CREATOR", creator.toUtf8().constData(), nullptr );
      GDALSetMetadataItem( outputDS.get(), "PRODUCER", creator.toUtf8().constData(), nullptr );
      GDALSetMetadataItem( outputDS.get(), "SUBJECT", mLayout->project()->metadata().abstract().toUtf8().constData(), nullptr );
      GDALSetMetadataItem( outputDS.get(), "TITLE", mLayout->project()->metadata().title().toUtf8().constData(), nullptr );

      const QgsAbstractMetadataBase::KeywordMap keywords = mLayout->project()->metadata().keywords();
      QStringList allKeywords;
      for ( auto it = keywords.constBegin(); it != keywords.constEnd(); ++it )
      {
        allKeywords.append( QStringLiteral( "%1: %2" ).arg( it.key(), it.value().join( ',' ) ) );
      }
      const QString keywordString = allKeywords.join( ';' );
      GDALSetMetadataItem( outputDS.get(), "KEYWORDS", keywordString.toUtf8().constData(), nullptr );
    }

    if ( t )
      GDALSetProjection( outputDS.get(), map->crs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ).toLocal8Bit().constData() );
  }
  CPLSetConfigOption( "GDAL_PDF_DPI", nullptr );

  return true;
}

QString nameForLayerWithItems( const QList< QGraphicsItem * > &items, unsigned int layerId )
{
  if ( items.count() == 1 )
  {
    if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( items.at( 0 ) ) )
    {
      QString name = layoutItem->displayName();
      // cleanup default item ID format
      if ( name.startsWith( '<' ) && name.endsWith( '>' ) )
        name = name.mid( 1, name.length() - 2 );
      return name;
    }
  }
  else if ( items.count() > 1 )
  {
    QStringList currentLayerItemTypes;
    for ( QGraphicsItem *item : items )
    {
      if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
      {
        const QString itemType = QgsApplication::layoutItemRegistry()->itemMetadata( layoutItem->type() )->visibleName();
        const QString itemTypePlural = QgsApplication::layoutItemRegistry()->itemMetadata( layoutItem->type() )->visiblePluralName();
        if ( !currentLayerItemTypes.contains( itemType ) && !currentLayerItemTypes.contains( itemTypePlural ) )
          currentLayerItemTypes << itemType;
        else if ( currentLayerItemTypes.contains( itemType ) )
        {
          currentLayerItemTypes.replace( currentLayerItemTypes.indexOf( itemType ), itemTypePlural );
        }
      }
      else
      {
        if ( !currentLayerItemTypes.contains( QObject::tr( "Other" ) ) )
          currentLayerItemTypes.append( QObject::tr( "Other" ) );
      }
    }
    return currentLayerItemTypes.join( QLatin1String( ", " ) );
  }
  return QObject::tr( "Layer %1" ).arg( layerId );
}

QgsLayoutExporter::ExportResult QgsLayoutExporter::handleLayeredExport( const QList<QGraphicsItem *> &items,
    const std::function<QgsLayoutExporter::ExportResult( unsigned int, const QgsLayoutItem::ExportLayerDetail & )> &exportFunc )
{
  LayoutItemHider itemHider( items );
  ( void )itemHider;

  int prevType = -1;
  QgsLayoutItem::ExportLayerBehavior prevItemBehavior = QgsLayoutItem::CanGroupWithAnyOtherItem;
  unsigned int layerId = 1;
  QgsLayoutItem::ExportLayerDetail layerDetails;
  itemHider.hideAll();
  const QList< QGraphicsItem * > itemsToIterate = itemHider.itemsToIterate();
  QList< QGraphicsItem * > currentLayerItems;
  for ( QGraphicsItem *item : itemsToIterate )
  {
    QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item );

    bool canPlaceInExistingLayer = false;
    if ( layoutItem )
    {
      switch ( layoutItem->exportLayerBehavior() )
      {
        case QgsLayoutItem::CanGroupWithAnyOtherItem:
        {
          switch ( prevItemBehavior )
          {
            case QgsLayoutItem::CanGroupWithAnyOtherItem:
              canPlaceInExistingLayer = true;
              break;

            case QgsLayoutItem::CanGroupWithItemsOfSameType:
              canPlaceInExistingLayer = prevType == -1 || prevType == layoutItem->type();
              break;

            case QgsLayoutItem::MustPlaceInOwnLayer:
            case QgsLayoutItem::ItemContainsSubLayers:
              canPlaceInExistingLayer = false;
              break;
          }
          break;
        }

        case QgsLayoutItem::CanGroupWithItemsOfSameType:
        {
          switch ( prevItemBehavior )
          {
            case QgsLayoutItem::CanGroupWithAnyOtherItem:
            case QgsLayoutItem::CanGroupWithItemsOfSameType:
              canPlaceInExistingLayer = prevType == -1 || prevType == layoutItem->type();
              break;

            case QgsLayoutItem::MustPlaceInOwnLayer:
            case QgsLayoutItem::ItemContainsSubLayers:
              canPlaceInExistingLayer = false;
              break;
          }
          break;
        }

        case QgsLayoutItem::MustPlaceInOwnLayer:
        {
          canPlaceInExistingLayer = false;
          break;
        }

        case QgsLayoutItem::ItemContainsSubLayers:
          canPlaceInExistingLayer = false;
          break;
      }
      prevItemBehavior = layoutItem->exportLayerBehavior();
      prevType = layoutItem->type();
    }
    else
    {
      prevItemBehavior = QgsLayoutItem::MustPlaceInOwnLayer;
    }

    if ( canPlaceInExistingLayer )
    {
      currentLayerItems << item;
      item->show();
    }
    else
    {
      if ( !currentLayerItems.isEmpty() )
      {
        layerDetails.name = nameForLayerWithItems( currentLayerItems, layerId );

        ExportResult result = exportFunc( layerId, layerDetails );
        if ( result != Success )
          return result;
        layerId++;
        currentLayerItems.clear();
      }

      itemHider.hideAll();
      item->show();

      if ( layoutItem && layoutItem->exportLayerBehavior() == QgsLayoutItem::ItemContainsSubLayers )
      {
        int layoutItemLayerIdx = 0;
        Q_NOWARN_DEPRECATED_PUSH
        mLayout->renderContext().setCurrentExportLayer( layoutItemLayerIdx );
        Q_NOWARN_DEPRECATED_POP
        layoutItem->startLayeredExport();
        while ( layoutItem->nextExportPart() )
        {
          Q_NOWARN_DEPRECATED_PUSH
          mLayout->renderContext().setCurrentExportLayer( layoutItemLayerIdx );
          Q_NOWARN_DEPRECATED_POP

          layerDetails = layoutItem->exportLayerDetails();
          ExportResult result = exportFunc( layerId, layerDetails );
          if ( result != Success )
            return result;
          layerId++;

          layoutItemLayerIdx++;
        }
        layerDetails.mapLayerId.clear();
        Q_NOWARN_DEPRECATED_PUSH
        mLayout->renderContext().setCurrentExportLayer( -1 );
        Q_NOWARN_DEPRECATED_POP
        layoutItem->stopLayeredExport();
        currentLayerItems.clear();
      }
      else
      {
        currentLayerItems << item;
      }
    }
  }
  if ( !currentLayerItems.isEmpty() )
  {
    layerDetails.name = nameForLayerWithItems( currentLayerItems, layerId );
    ExportResult result = exportFunc( layerId, layerDetails );
    if ( result != Success )
      return result;
  }
  return Success;
}

QgsVectorSimplifyMethod QgsLayoutExporter::createExportSimplifyMethod()
{
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::GeometrySimplification );
  simplifyMethod.setForceLocalOptimization( true );
  // we use SnappedToGridGlobal, because it avoids gaps and slivers between previously adjacent polygons
  simplifyMethod.setSimplifyAlgorithm( QgsVectorSimplifyMethod::SnappedToGridGlobal );
  simplifyMethod.setThreshold( 0.1f ); // (pixels). We are quite conservative here. This could possibly be bumped all the way up to 1. But let's play it safe.
  return simplifyMethod;
}

void QgsLayoutExporter::computeWorldFileParameters( double &a, double &b, double &c, double &d, double &e, double &f, double dpi ) const
{
  if ( !mLayout )
    return;

  QgsLayoutItemMap *map = mLayout->referenceMap();
  if ( !map )
  {
    return;
  }

  int pageNumber = map->page();
  QgsLayoutItemPage *page = mLayout->pageCollection()->page( pageNumber );
  double pageY = page->pos().y();
  QSizeF pageSize = page->rect().size();
  QRectF pageRect( 0, pageY, pageSize.width(), pageSize.height() );
  computeWorldFileParameters( pageRect, a, b, c, d, e, f, dpi );
}

void QgsLayoutExporter::computeWorldFileParameters( const QRectF &exportRegion, double &a, double &b, double &c, double &d, double &e, double &f, double dpi ) const
{
  if ( !mLayout )
    return;

  // World file parameters : affine transformation parameters from pixel coordinates to map coordinates
  QgsLayoutItemMap *map = mLayout->referenceMap();
  if ( !map )
  {
    return;
  }

  double destinationHeight = exportRegion.height();
  double destinationWidth = exportRegion.width();

  QRectF mapItemSceneRect = map->mapRectToScene( map->rect() );
  QgsRectangle mapExtent = map->extent();

  double alpha = map->mapRotation() / 180 * M_PI;

  double xRatio = mapExtent.width() / mapItemSceneRect.width();
  double yRatio = mapExtent.height() / mapItemSceneRect.height();

  double xCenter = mapExtent.center().x();
  double yCenter = mapExtent.center().y();

  // get the extent (in map units) for the region
  QPointF mapItemPos = map->pos();
  //adjust item position so it is relative to export region
  mapItemPos.rx() -= exportRegion.left();
  mapItemPos.ry() -= exportRegion.top();

  double xmin = mapExtent.xMinimum() - mapItemPos.x() * xRatio;
  double ymax = mapExtent.yMaximum() + mapItemPos.y() * yRatio;
  QgsRectangle paperExtent( xmin, ymax - destinationHeight * yRatio, xmin + destinationWidth * xRatio, ymax );

  double X0 = paperExtent.xMinimum();
  double Y0 = paperExtent.yMinimum();

  if ( dpi < 0 )
    dpi = mLayout->renderContext().dpi();

  int widthPx = static_cast< int >( dpi * destinationWidth / 25.4 );
  int heightPx = static_cast< int >( dpi * destinationHeight / 25.4 );

  double Ww = paperExtent.width() / widthPx;
  double Hh = paperExtent.height() / heightPx;

  // scaling matrix
  double s[6];
  s[0] = Ww;
  s[1] = 0;
  s[2] = X0;
  s[3] = 0;
  s[4] = -Hh;
  s[5] = Y0 + paperExtent.height();

  // rotation matrix
  double r[6];
  r[0] = std::cos( alpha );
  r[1] = -std::sin( alpha );
  r[2] = xCenter * ( 1 - std::cos( alpha ) ) + yCenter * std::sin( alpha );
  r[3] = std::sin( alpha );
  r[4] = std::cos( alpha );
  r[5] = - xCenter * std::sin( alpha ) + yCenter * ( 1 - std::cos( alpha ) );

  // result = rotation x scaling = rotation(scaling(X))
  a = r[0] * s[0] + r[1] * s[3];
  b = r[0] * s[1] + r[1] * s[4];
  c = r[0] * s[2] + r[1] * s[5] + r[2];
  d = r[3] * s[0] + r[4] * s[3];
  e = r[3] * s[1] + r[4] * s[4];
  f = r[3] * s[2] + r[4] * s[5] + r[5];
}

bool QgsLayoutExporter::requiresRasterization( const QgsLayout *layout )
{
  if ( !layout )
    return false;

  QList< QgsLayoutItem *> items;
  layout->layoutItems( items );

  for ( QgsLayoutItem *currentItem : std::as_const( items ) )
  {
    // ignore invisible items, they won't affect the output in any way...
    if ( currentItem->isVisible() && currentItem->requiresRasterization() )
      return true;
  }
  return false;
}

bool QgsLayoutExporter::containsAdvancedEffects( const QgsLayout *layout )
{
  if ( !layout )
    return false;

  QList< QgsLayoutItem *> items;
  layout->layoutItems( items );

  for ( QgsLayoutItem *currentItem : std::as_const( items ) )
  {
    // ignore invisible items, they won't affect the output in any way...
    if ( currentItem->isVisible() && currentItem->containsAdvancedEffects() )
      return true;
  }
  return false;
}

QImage QgsLayoutExporter::createImage( const QgsLayoutExporter::ImageExportSettings &settings, int page, QRectF &bounds, bool &skipPage ) const
{
  bounds = QRectF();
  skipPage = false;

  if ( settings.cropToContents )
  {
    if ( mLayout->pageCollection()->pageCount() == 1 )
    {
      // single page, so include everything
      bounds = mLayout->layoutBounds( true );
    }
    else
    {
      // multi page, so just clip to items on current page
      bounds = mLayout->pageItemBounds( page, true );
    }
    if ( bounds.width() <= 0 || bounds.height() <= 0 )
    {
      //invalid size, skip page
      skipPage = true;
      return QImage();
    }

    double pixelToLayoutUnits = mLayout->convertToLayoutUnits( QgsLayoutMeasurement( 1, QgsUnitTypes::LayoutPixels ) );
    bounds = bounds.adjusted( -settings.cropMargins.left() * pixelToLayoutUnits,
                              -settings.cropMargins.top() * pixelToLayoutUnits,
                              settings.cropMargins.right() * pixelToLayoutUnits,
                              settings.cropMargins.bottom() * pixelToLayoutUnits );
    return renderRegionToImage( bounds, QSize(), settings.dpi );
  }
  else
  {
    return renderPageToImage( page, settings.imageSize, settings.dpi );
  }
}

int QgsLayoutExporter::firstPageToBeExported( QgsLayout *layout )
{
  const int pageCount = layout->pageCollection()->pageCount();
  for ( int i = 0; i < pageCount; ++i )
  {
    if ( !layout->pageCollection()->shouldExportPage( i ) )
    {
      continue;
    }

    return i;
  }
  return 0; // shouldn't really matter -- we aren't exporting ANY pages!
}

QString QgsLayoutExporter::generateFileName( const PageExportDetails &details ) const
{
  if ( details.page == 0 )
  {
    return details.directory + '/' + details.baseName + '.' + details.extension;
  }
  else
  {
    return details.directory + '/' + details.baseName + '_' + QString::number( details.page + 1 ) + '.' + details.extension;
  }
}

void QgsLayoutExporter::captureLabelingResults()
{
  qDeleteAll( mLabelingResults );
  mLabelingResults.clear();

  QList< QgsLayoutItemMap * > maps;
  mLayout->layoutItems( maps );

  for ( QgsLayoutItemMap *map : std::as_const( maps ) )
  {
    mLabelingResults[ map->uuid() ] = map->mExportLabelingResults.release();
  }
}

bool QgsLayoutExporter::saveImage( const QImage &image, const QString &imageFilename, const QString &imageFormat, QgsProject *projectForMetadata )
{
  QImageWriter w( imageFilename, imageFormat.toLocal8Bit().constData() );
  if ( imageFormat.compare( QLatin1String( "tiff" ), Qt::CaseInsensitive ) == 0 || imageFormat.compare( QLatin1String( "tif" ), Qt::CaseInsensitive ) == 0 )
  {
    w.setCompression( 1 ); //use LZW compression
  }
  if ( projectForMetadata )
  {
    w.setText( QStringLiteral( "Author" ), projectForMetadata->metadata().author() );
    const QString creator = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
    w.setText( QStringLiteral( "Creator" ), creator );
    w.setText( QStringLiteral( "Producer" ), creator );
    w.setText( QStringLiteral( "Subject" ), projectForMetadata->metadata().abstract() );
    w.setText( QStringLiteral( "Created" ), projectForMetadata->metadata().creationDateTime().toString( Qt::ISODate ) );
    w.setText( QStringLiteral( "Title" ), projectForMetadata->metadata().title() );

    const QgsAbstractMetadataBase::KeywordMap keywords = projectForMetadata->metadata().keywords();
    QStringList allKeywords;
    for ( auto it = keywords.constBegin(); it != keywords.constEnd(); ++it )
    {
      allKeywords.append( QStringLiteral( "%1: %2" ).arg( it.key(), it.value().join( ',' ) ) );
    }
    const QString keywordString = allKeywords.join( ';' );
    w.setText( QStringLiteral( "Keywords" ), keywordString );
  }
  return w.write( image );
}

#endif // ! QT_NO_PRINTER
