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
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutpagecollection.h"
#include <QImageWriter>
#include <QSize>

QgsLayoutExporter::QgsLayoutExporter( QgsLayout *layout )
  : mLayout( layout )
{

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

  QRectF paperRect = QRectF( pageItem->pos().x(), pageItem->pos().y(), pageItem->rect().width(), pageItem->rect().height() );
  return renderRegionToImage( paperRect, imageSize, dpi );
}

void QgsLayoutExporter::renderRegion( QPainter *painter, const QRectF &region ) const
{
  QPaintDevice *paintDevice = painter->device();
  if ( !paintDevice || !mLayout )
  {
    return;
  }

  mLayout->context().mIsPreviewRender = false;

#if 0 //TODO
  setSnapLinesVisible( false );
#endif

  mLayout->render( painter, QRectF( 0, 0, paintDevice->width(), paintDevice->height() ), region );

#if 0 // TODO
  setSnapLinesVisible( true );
#endif

  mLayout->context().mIsPreviewRender = true;
}

QImage QgsLayoutExporter::renderRegionToImage( const QRectF &region, QSize imageSize, double dpi ) const
{
  double resolution = mLayout->context().dpi();
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
    image.setDotsPerMeterX( resolution / 25.4 * 1000 );
    image.setDotsPerMeterY( resolution / 25.4 * 1000 );
    image.fill( Qt::transparent );
    QPainter imagePainter( &image );
    renderRegion( &imagePainter, region );
    if ( !imagePainter.isActive() )
      return QImage();
  }

  return image;
}

///@cond PRIVATE
class LayoutDpiRestorer
{
  public:

    LayoutDpiRestorer( QgsLayout *layout )
      : mLayout( layout )
      , mPreviousSetting( layout->context().dpi() )
    {
    }

    ~LayoutDpiRestorer()
    {
      mLayout->context().setDpi( mPreviousSetting );
    }

  private:
    QgsLayout *mLayout = nullptr;
    double mPreviousSetting = 0;
};
///@endcond PRIVATE
QgsLayoutExporter::ExportResult QgsLayoutExporter::exportToImage( const QString &filePath, const QgsLayoutExporter::ImageExportSettings &settings )
{
  int worldFilePageNo = -1;
  if ( mLayout->referenceMap() )
  {
    worldFilePageNo = mLayout->referenceMap()->page();
  }

  QFileInfo fi( filePath );
  QString path = fi.path();
  QString baseName = fi.baseName();
  QString extension = fi.completeSuffix();

  LayoutDpiRestorer dpiRestorer( mLayout );
  ( void )dpiRestorer;
  mLayout->context().setDpi( settings.dpi );

  QList< int > pages;
  if ( settings.pages.empty() )
  {
    for ( int page = 0; page < mLayout->pageCollection()->pageCount(); ++page )
      pages << page;
  }
  else
  {
    for ( int page : settings.pages )
    {
      if ( page >= 0 && page < mLayout->pageCollection()->pageCount() )
        pages << page;
    }
  }

  for ( int page : qgis::as_const( pages ) )
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

    if ( image.isNull() )
    {
      return MemoryError;
    }

    QString outputFilePath = generateFileName( path, baseName, extension, page );
    if ( !saveImage( image, outputFilePath, extension ) )
    {
      return FileError;
    }

#if 0 //TODO
    if ( page == worldFilePageNo )
    {
      mLayout->georeferenceOutput( outputFilePath, nullptr, bounds, imageDlg.resolution() );

      if ( settings.generateWorldFile )
      {
        // should generate world file for this page
        double a, b, c, d, e, f;
        if ( bounds.isValid() )
          mLayout->computeWorldFileParameters( bounds, a, b, c, d, e, f );
        else
          mLayout->computeWorldFileParameters( a, b, c, d, e, f );

        QFileInfo fi( outputFilePath );
        // build the world file name
        QString outputSuffix = fi.suffix();
        QString worldFileName = fi.absolutePath() + '/' + fi.baseName() + '.'
                                + outputSuffix.at( 0 ) + outputSuffix.at( fi.suffix().size() - 1 ) + 'w';

        writeWorldFile( worldFileName, a, b, c, d, e, f );
      }
    }
#endif
  }
  return Success;
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

QString QgsLayoutExporter::generateFileName( const QString &path, const QString &baseName, const QString &suffix, int page ) const
{
  if ( page == 0 )
  {
    return path + '/' + baseName + '.' + suffix;
  }
  else
  {
    return path + '/' + baseName + '_' + QString::number( page + 1 ) + '.' + suffix;
  }
}

bool QgsLayoutExporter::saveImage( const QImage &image, const QString &imageFilename, const QString &imageFormat )
{
  QImageWriter w( imageFilename, imageFormat.toLocal8Bit().constData() );
  if ( imageFormat.compare( QLatin1String( "tiff" ), Qt::CaseInsensitive ) == 0 || imageFormat.compare( QLatin1String( "tif" ), Qt::CaseInsensitive ) == 0 )
  {
    w.setCompression( 1 ); //use LZW compression
  }
  return w.write( image );
}

