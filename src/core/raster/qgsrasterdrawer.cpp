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
//#include "qgsrasterresampler.h"
#include "qgsrasterprojector.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsmaptopixel.h"

//resamplers
//#include "qgsbilinearrasterresampler.h"
//#include "qgscubicrasterresampler.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QPainter>

QgsRasterDrawer::QgsRasterDrawer( QgsRasterInterface* input ): mInput( input )
{
}

QgsRasterDrawer::~QgsRasterDrawer()
{
  //remove remaining memory in partinfos
  /*
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.begin();
  for ( ; partIt != mRasterPartInfos.end(); ++partIt )
  {
    CPLFree( partIt.value().data );
  }
  */
}

void QgsRasterDrawer::draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel )
{
  QgsDebugMsg( "Entered" );
  if ( !p || !mInput || !viewPort || !theQgsMapToPixel )
  {
    return;
  }

  // last pipe filter has only 1 band
  int bandNumber = 1;
  startRasterRead( bandNumber, viewPort, theQgsMapToPixel );

  //number of cols/rows in output pixels
  int nCols = 0;
  int nRows = 0;
  //number of raster cols/rows with oversampling
  //int nRasterCols = 0;
  //int nRasterRows = 0;
  //shift to top left point for the raster part
  int topLeftCol = 0;
  int topLeftRow = 0;

  // We know that the output data type of last pipe filter is QImage data
  //QgsRasterDataProvider::DataType rasterType = ( QgsRasterDataProvider::DataType )mProvider->dataType( mGrayBand );

  void* rasterData;

  // readNextRasterPart calcs and resets  nCols, nRows, topLeftCol, topLeftRow
  while ( readNextRasterPart( bandNumber, viewPort, nCols, nRows,
                              &rasterData, topLeftCol, topLeftRow ) )
  {
    //create image
    //QImage img( nRasterCols, nRasterRows, QImage::Format_ARGB32_Premultiplied );

    // TODO: the exact format should be read from input
    QImage img(( uchar * ) rasterData, nCols, nRows, QImage::Format_ARGB32_Premultiplied );
    drawImage( p, viewPort, img, topLeftCol, topLeftRow );

    // QImage does not delete data block passed to constructor
    free( rasterData );
  }
}

void QgsRasterDrawer::startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel )
{
  if ( !viewPort || !mapToPixel || !mInput )
  {
    return;
  }

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //split raster into small portions if necessary
  RasterPartInfo pInfo;
  pInfo.nCols = viewPort->drawableAreaXDim;
  pInfo.nRows = viewPort->drawableAreaYDim;

  //effective oversampling factors are different to global one because of rounding
  //oversamplingX = (( double )pInfo.nCols * oversampling ) / viewPort->drawableAreaXDim;
  //oversamplingY = (( double )pInfo.nRows * oversampling ) / viewPort->drawableAreaYDim;

  // TODO : we dont know oversampling (grid size) here - how to get totalMemoryUsage ?
  //int totalMemoryUsage = pInfo.nCols * oversamplingX * pInfo.nRows * oversamplingY * mInput->dataTypeSize( bandNumber );
  int totalMemoryUsage = pInfo.nCols * pInfo.nRows * mInput->dataTypeSize( bandNumber );
  int parts = totalMemoryUsage / 100000000 + 1;
  int nPartsPerDimension = sqrt( parts );
  pInfo.nColsPerPart = pInfo.nCols / nPartsPerDimension;
  pInfo.nRowsPerPart = pInfo.nRows / nPartsPerDimension;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  pInfo.data = 0;
  pInfo.prj = 0;
  mRasterPartInfos.insert( bandNumber, pInfo );
}

bool QgsRasterDrawer::readNextRasterPart( int bandNumber, QgsRasterViewPort* viewPort,
    int& nCols, int& nRows, void** rasterData, int& topLeftCol, int& topLeftRow )
{
  if ( !viewPort )
  {
    return false;
  }

  //get partinfo
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt == mRasterPartInfos.end() )
  {
    return false;
  }

  RasterPartInfo& pInfo = partIt.value();

  //remove last data block
  // TODO: data are released somewhere else (check)
  //free( pInfo.data );
  pInfo.data = 0;
  delete pInfo.prj;
  pInfo.prj = 0;

  //already at end
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow == pInfo.nRows )
  {
    return false;
  }

  //read data block
  nCols = qMin( pInfo.nColsPerPart, pInfo.nCols - pInfo.currentCol );
  nRows = qMin( pInfo.nRowsPerPart, pInfo.nRows - pInfo.currentRow );
  int typeSize = mInput->dataTypeSize( bandNumber ) / 8;

  //get subrectangle
  QgsRectangle viewPortExtent = viewPort->mDrawnExtent;
  double xmin = viewPortExtent.xMinimum() + pInfo.currentCol / ( double )pInfo.nCols * viewPortExtent.width();
  double xmax = viewPortExtent.xMinimum() + ( pInfo.currentCol + nCols ) / ( double )pInfo.nCols * viewPortExtent.width();
  double ymin = viewPortExtent.yMaximum() - ( pInfo.currentRow + nRows ) / ( double )pInfo.nRows * viewPortExtent.height();
  double ymax = viewPortExtent.yMaximum() - pInfo.currentRow / ( double )pInfo.nRows * viewPortExtent.height();
  QgsRectangle blockRect( xmin, ymin, xmax, ymax );

  pInfo.data = mInput->block( bandNumber, blockRect, nCols, nRows );

  *rasterData = pInfo.data;
  topLeftCol = pInfo.currentCol;
  topLeftRow = pInfo.currentRow;

  pInfo.currentCol += nCols;
  if ( pInfo.currentCol == pInfo.nCols && pInfo.currentRow + nRows == pInfo.nRows ) //end of raster
  {
    pInfo.currentRow = pInfo.nRows;
  }
  else if ( pInfo.currentCol == pInfo.nCols ) //start new row
  {
    pInfo.currentCol = 0;
    pInfo.currentRow += pInfo.nRowsPerPart;
  }

  return true;
}

void QgsRasterDrawer::stopRasterRead( int bandNumber )
{
  removePartInfo( bandNumber );
}

void QgsRasterDrawer::removePartInfo( int bandNumber )
{
  QMap<int, RasterPartInfo>::iterator partIt = mRasterPartInfos.find( bandNumber );
  if ( partIt != mRasterPartInfos.end() )
  {
    RasterPartInfo& pInfo = partIt.value();
    //CPLFree( pInfo.data );
    free( pInfo.data );
    delete pInfo.prj;
    mRasterPartInfos.remove( bandNumber );
  }
}

void QgsRasterDrawer::drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow ) const
{
  if ( !p || !viewPort )
  {
    return;
  }

  //top left position in device coords
  QPoint tlPoint = QPoint( viewPort->topLeftPoint.x() + topLeftCol, viewPort->topLeftPoint.y() + topLeftRow );

  p->drawImage( tlPoint, img );
}

