/***************************************************************************
                         qgsrasterdrawer.h
                         -------------------
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

#ifndef QGSRASTERDRAWER_H
#define QGSRASTERDRAWER_H

//#include "qgsrasterdataprovider.h"
#include "qgsrasterface.h"

#include <QMap>

class QPainter;
class QImage;
class QgsMapToPixel;
class QgsRasterResampler;
class QgsRasterProjector;
class QgsRasterTransparency;
class QgsRasterViewPort;

class QDomElement;

class QgsRasterDrawer
{
  public:
    //Stores information about reading of a raster band. Columns and rows are in unsampled coordinates
    struct RasterPartInfo
    {
      int currentCol;
      int currentRow;
      int nCols;
      int nRows;
      int nColsPerPart;
      int nRowsPerPart;
      void* data; //data (can be in oversampled/undersampled resolution)
      QgsRasterProjector* prj; //raster projector (or 0 if no reprojection is done)
    };

    QgsRasterDrawer( QgsRasterFace* input );
    ~QgsRasterDrawer();

    void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel );

  protected:
    //inline double readValue( void *data, QgsRasterFace::DataType type, int index );

    /**Start reading of raster band. Raster data can then be retrieved by calling readNextRasterPart until it returns false.
      @param bandNumer number of raster band to read
      @param viewPort describes raster position on screen
     */
    void startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel );
    /**Fetches next part of raster data
       @param nCols number of columns on output device
       @param nRows number of rows on output device
       @param nColsRaster number of raster columns (different to nCols if oversamplingX != 1.0)
       @param nRowsRaster number of raster rows (different to nRows if oversamplingY != 0)*/
    //bool readNextRasterPart( int bandNumber, QgsRasterViewPort* viewPort,
    //                           int& nCols, int& nRows,
    //                           int& nColsRaster, int& nRowsRaster,
    //                           void** rasterData, int& topLeftCol, int& topLeftRow );
    bool readNextRasterPart( int bandNumber, QgsRasterViewPort* viewPort,
                             int& nCols, int& nRows,
                             void** rasterData,
                             int& topLeftCol, int& topLeftRow );
    /**Draws raster part
      @param topLeftCol Left position relative to left border of viewport
      @param topLeftRow Top position relative to top border of viewport*/
    void drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow ) const;
    void stopRasterRead( int bandNumber );

    QgsRasterFace* mInput;
    QMap<int, RasterPartInfo> mRasterPartInfos;

  private:
    /**Remove part into and release memory*/
    void removePartInfo( int bandNumer );
};
/*
inline double QgsRasterDrawer::readValue( void *data, QgsRasterFace::DataType type, int index )
{
  if ( !mProvider )
  {
    return 0;
  }

  if ( !data )
  {
    return mProvider->noDataValue();
  }

  switch ( type )
  {
    case QgsRasterFace::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterFace::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterFace::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterFace::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterFace::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterFace::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterFace::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  return mProvider->noDataValue();
}
*/

#endif // QGSRASTERDRAWER_H
