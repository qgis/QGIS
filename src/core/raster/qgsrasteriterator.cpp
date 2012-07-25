#include "qgsrasteriterator.h"
#include "qgsrasterinterface.h"
#include "qgsrasterprojector.h"
#include "qgsrasterviewport.h"

QgsRasterIterator::QgsRasterIterator( QgsRasterInterface* input ): mInput( input ),
    mMaximumTileWidth( 2000 ), mMaximumTileHeight( 2000 )
{
}

QgsRasterIterator::~QgsRasterIterator()
{
}

void QgsRasterIterator::startRasterRead( int bandNumber, int nCols, int nRows, const QgsRectangle& extent )
{
  if ( !mInput )
  {
    return;
  }

  mExtent = extent;

  //remove any previous part on that band
  removePartInfo( bandNumber );

  //split raster into small portions if necessary
  RasterPartInfo pInfo;
  pInfo.nCols = nCols;
  pInfo.nRows = nRows;
  pInfo.currentCol = 0;
  pInfo.currentRow = 0;
  pInfo.data = 0;
  pInfo.prj = 0;
  mRasterPartInfos.insert( bandNumber, pInfo );
}

bool QgsRasterIterator::readNextRasterPart( int bandNumber,
    int& nCols, int& nRows,
    void** rasterData,
    int& topLeftCol, int& topLeftRow )
{
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
  nCols = qMin( mMaximumTileWidth, pInfo.nCols - pInfo.currentCol );
  nRows = qMin( mMaximumTileHeight, pInfo.nRows - pInfo.currentRow );

  //get subrectangle
  QgsRectangle viewPortExtent = mExtent;
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
    pInfo.currentRow += nRows;
  }

  return true;
}

void QgsRasterIterator::stopRasterRead( int bandNumber )
{
  removePartInfo( bandNumber );
}

void QgsRasterIterator::removePartInfo( int bandNumber )
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
