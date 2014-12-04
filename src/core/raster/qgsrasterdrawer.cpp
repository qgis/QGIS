/***************************************************************************
                         qgsrasterdrawer.cpp
                         ---------------------
    begin                : June 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsrasterdrawer.h"
#include "qgsrasteriterator.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"
#include <QImage>
#include <QPainter>
#include <QPrinter>

QgsRasterDrawer::QgsRasterDrawer( QgsRasterIterator* iterator ): mIterator( iterator )
{
}

QgsRasterDrawer::~QgsRasterDrawer()
{
}

void QgsRasterDrawer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  QgsDebugMsg( "Entered" );
  if ( !p || !mIterator || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  // last pipe filter has only 1 band
  int bandNumber = 1;
  mIterator->startRasterRead( bandNumber, viewPort->mWidth, viewPort->mHeight, viewPort->mDrawnExtent );

  //number of cols/rows in output pixels
  int nCols = 0;
  int nRows = 0;
  //shift to top left point for the raster part
  int topLeftCol = 0;
  int topLeftRow = 0;

  // We know that the output data type of last pipe filter is QImage data

  QgsRasterBlock *block;

  // readNextRasterPart calcs and resets  nCols, nRows, topLeftCol, topLeftRow
  while ( mIterator->readNextRasterPart( bandNumber, nCols, nRows,
                                         &block, topLeftCol, topLeftRow ) )
  {
    if ( !block )
    {
      QgsDebugMsg( "Cannot get block" );
      continue;
    }

    QImage img = block->image();

    // Because of bug in Acrobat Reader we must use "white" transparent color instead
    // of "black" for PDF. See #9101.
    QPrinter *printer = dynamic_cast<QPrinter *>( p->device() );
    if ( printer && printer->outputFormat() == QPrinter::PdfFormat )
    {
      QgsDebugMsg( "PdfFormat" );

      img = img.convertToFormat( QImage::Format_ARGB32 );
      QRgb transparentBlack = qRgba( 0, 0, 0, 0 );
      QRgb transparentWhite = qRgba( 255, 255, 255, 0 );
      for ( int x = 0; x < img.width(); x++ )
      {
        for ( int y = 0; y < img.height(); y++ )
        {
          if ( img.pixel( x, y ) == transparentBlack )
          {
            img.setPixel( x, y, transparentWhite );
          }
        }
      }
    }

    drawImage( p, viewPort, img, topLeftCol, topLeftRow, theQgsMapToPixel );

    delete block;
  }
}

void QgsRasterDrawer::drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow, const QgsMapToPixel* theQgsMapToPixel ) const
{
  if ( !p || !viewPort )
  {
    return;
  }

  //top left position in device coords
  QPoint tlPoint = QPoint( viewPort->mTopLeftPoint.x() + topLeftCol, viewPort->mTopLeftPoint.y() + topLeftRow );
  p->save();
  p->setRenderHint( QPainter::Antialiasing, false );

  // Blending problem was reported with PDF output if background color has alpha < 255
  // in #7766, it seems to be a bug in Qt, setting a brush with alpha 255 is a workaround
  // which should not harm anything
  p->setBrush( QBrush( QColor( Qt::white ), Qt::NoBrush ) );

  int w = theQgsMapToPixel->mapWidth();
  int h = theQgsMapToPixel->mapHeight();

  if ( theQgsMapToPixel ) {
    // both viewPort and image sizes are dependent on scale
    double cx = w/2.0;
    double cy = h/2.0;

    QgsDebugMsg( QString("XXX img w:%1 h:%2").arg(img.width()).arg(img.height()) );

    p->translate( cx, cy );
    p->rotate( theQgsMapToPixel->mapRotation() );
    p->translate( -cx, -cy );
  }

  if ( w && h && img.width() && img.height() )
  {
    double irat = img.width() / img.height(); // input ratio
    double orat = w / h; // output ratio
    if ( irat != orat )  {
      QgsDebugMsg( QString( "map paint DIFFERENT aspect ratio: img %1,%2  item %3,%4" ).arg( img.width() ).arg( img.height() ).arg( w ).arg( h ) );
#if 0
      QImage scaledImage;
      if ( orat > 1 ) scaledImage = img.scaledToWidth(w);
      else scaledImage = img.scaledToHeight(h);
      // TODO: clip image ? see src/gui/qgsmapcanvasmap.cpp
      int tX = (w-scaledImage.width())/2.0;
      int tY = (h-scaledImage.height())/2.0;
      int fX = 0;
      int fY = 0;
      int fW = w;
      int fH = h;
      p->drawImage(tX, tY, scaledImage, fX, fY, fW, fH);
      p->restore()
      // p->resetTransform(); // could be needed if the painter is reused
      return;
#endif
    }
  }
  p->drawImage( tlPoint, img );

#if 0
  // For debugging:
  QRectF br = QRectF(tlPoint, img.size());
  QPointF c = br.center();
  double rad = std::max(br.width(),br.height())/10;
  p->drawRoundedRect( br, rad, rad );
  p->drawLine( QLineF(br.x(), br.y(), br.x()+br.width(), br.y()+br.height()) );
  p->drawLine( QLineF(br.x()+br.width(), br.y(), br.x(), br.y()+br.height()) );

  double nw = br.width()*0.5; double nh = br.height()*0.5;
  br = QRectF(c-QPointF(nw/2,nh/2), QSize(nw, nh));
  p->drawRoundedRect( br, rad, rad );

  nw = br.width()*0.5; nh = br.height()*0.5;
  br = QRectF(c-QPointF(nw/2,nh/2), QSize(nw, nh));
  p->drawRoundedRect( br, rad, rad );
#endif

  p->restore();
}

