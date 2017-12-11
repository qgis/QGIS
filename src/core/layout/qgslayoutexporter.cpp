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
#include "qgsogrutils.h"
#include <QImageWriter>
#include <QSize>

#include "gdal.h"
#include "cpl_conv.h"

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
  if ( QgsLayoutItemMap *referenceMap = mLayout->referenceMap() )
  {
    worldFilePageNo = referenceMap->page();
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

    if ( page == worldFilePageNo )
    {
      georeferenceOutput( outputFilePath, nullptr, bounds, settings.dpi );

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
        QString worldFileName = fi.absolutePath() + '/' + fi.baseName() + '.'
                                + outputSuffix.at( 0 ) + outputSuffix.at( fi.suffix().size() - 1 ) + 'w';

        writeWorldFile( worldFileName, a, b, c, d, e, f );
      }
    }

  }
  return Success;
}

double *QgsLayoutExporter::computeGeoTransform( const QgsLayoutItemMap *map, const QRectF &region, double dpi ) const
{
  if ( !map )
    map = mLayout->referenceMap();

  if ( !map )
    return nullptr;

  if ( dpi < 0 )
    dpi = mLayout->context().dpi();

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
  double *t = new double[6];
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
  if ( !worldFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
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
  if ( !map )
    map = mLayout->referenceMap();

  if ( !map )
    return false; // no reference map

  if ( dpi < 0 )
    dpi = mLayout->context().dpi();

  double *t = computeGeoTransform( map, exportRegion, dpi );
  if ( !t )
    return false;

  // important - we need to manually specify the DPI in advance, as GDAL will otherwise
  // assume a DPI of 150
  CPLSetConfigOption( "GDAL_PDF_DPI", QString::number( dpi ).toLocal8Bit().constData() );
  gdal::dataset_unique_ptr outputDS( GDALOpen( file.toLocal8Bit().constData(), GA_Update ) );
  if ( outputDS )
  {
    GDALSetGeoTransform( outputDS.get(), t );
#if 0
    //TODO - metadata can be set here, e.g.:
    GDALSetMetadataItem( outputDS, "AUTHOR", "me", nullptr );
#endif
    GDALSetProjection( outputDS.get(), map->crs().toWkt().toLocal8Bit().constData() );
  }
  CPLSetConfigOption( "GDAL_PDF_DPI", nullptr );
  delete[] t;
  return true;
}

void QgsLayoutExporter::computeWorldFileParameters( double &a, double &b, double &c, double &d, double &e, double &f, double dpi ) const
{
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
    dpi = mLayout->context().dpi();

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

