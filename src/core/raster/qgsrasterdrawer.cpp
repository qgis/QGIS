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

    drawImage( p, viewPort, img, topLeftCol, topLeftRow );

    delete block;
  }
}

void QgsRasterDrawer::drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow ) const
{
  if ( !p || !viewPort )
  {
    return;
  }

  //top left position in device coords
  QPoint tlPoint = QPoint( viewPort->mTopLeftPoint.x() + topLeftCol, viewPort->mTopLeftPoint.y() + topLeftRow );
  p->save();
  p->setRenderHint( QPainter::Antialiasing, false );
  p->drawImage( tlPoint, img );
  p->restore();
}

