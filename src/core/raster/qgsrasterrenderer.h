/***************************************************************************
                         qgsrasterrenderer.h
                         -------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRENDERER_H
#define QGSRASTERRENDERER_H

#include "qgsrasterdataprovider.h"

class QPainter;
class QgsMapToPixel;
class QgsRasterResampler;
class QgsRasterProjector;
class QgsRasterTransparency;
class QgsRasterViewPort;

class QgsRasterRenderer
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

    QgsRasterRenderer( QgsRasterDataProvider* provider );
    virtual ~QgsRasterRenderer();
    virtual void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel ) = 0;

    bool usesTransparency( QgsCoordinateReferenceSystem& srcSRS, QgsCoordinateReferenceSystem& dstSRS ) const;

    void setOpacity( double opacity ) { mOpacity = opacity; }
    double opacity() const { return mOpacity; }

    void setRasterTransparency( QgsRasterTransparency* t ) { mRasterTransparency = t; }
    const QgsRasterTransparency* rasterTransparency() const { return mRasterTransparency; }

    void setAlphaBand( int band ) { mAlphaBand = band; }
    int alphaBand() const { return mAlphaBand; }

    void setInvertColor( bool invert ) { mInvertColor = invert; }
    bool invertColor() const { return mInvertColor; }

    /**Set resampler for zoomed in scales. Takes ownership of the object*/
    void setZoomedInResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedInResampler() const { return mZoomedInResampler; }

    /**Set resampler for zoomed out scales. Takes ownership of the object*/
    void setZoomedOutResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedOutResampler() const { return mZoomedOutResampler; }

    void setMaxOversampling( double os ) { mMaxOversampling = os; }
    double maxOversampling() const { return mMaxOversampling; }

  protected:
    inline double readValue( void *data, QgsRasterDataProvider::DataType type, int index );

    /**Start reading of raster band. Raster data can then be retrieved by calling readNextRasterPart until it returns false.
      @param bandNumer number of raster band to read
      @param viewPort describes raster position on screen
      @param oversamplingX out: oversampling rate in x-direction
      @param oversamplingY out: oversampling rate in y-direction*/
    void startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double& oversamplingX, double& oversamplingY );
    /**Fetches next part of raster data
       @param nCols number of columns on output device
       @param nRows number of rows on output device
       @param nColsRaster number of raster columns (different to nCols if oversamplingX != 1.0)
       @param nRowsRaster number of raster rows (different to nRows if oversamplingY != 0)*/
    bool readNextRasterPart( int bandNumber, double oversamplingX, double oversamplingY, QgsRasterViewPort* viewPort, int& nCols, int& nRows,
                             int& nColsRaster, int& nRowsRaster, void** rasterData, int& topLeftCol, int& topLeftRow );
    /**Draws raster part
      @param topLeftCol Left position relative to left border of viewport
      @param topLeftRow Top position relative to top border of viewport*/
    void drawImage( QPainter* p, QgsRasterViewPort* viewPort, const QImage& img, int topLeftCol, int topLeftRow,
                    int nCols, int nRows, double oversamplingX, double oversamplingY ) const;
    void stopRasterRead( int bandNumber );


    QgsRasterDataProvider* mProvider;
    /**Resampler used if screen resolution is higher than raster resolution (zoomed in). 0 means no resampling (nearest neighbour)*/
    QgsRasterResampler* mZoomedInResampler;
    /**Resampler used if raster resolution is higher than raster resolution (zoomed out). 0 mean no resampling (nearest neighbour)*/
    QgsRasterResampler* mZoomedOutResampler;
    QMap<int, RasterPartInfo> mRasterPartInfos;

    /**Global alpha value (0-1)*/
    double mOpacity;
    /**Raster transparency per color or value. Overwrites global alpha value*/
    QgsRasterTransparency* mRasterTransparency;
    /**Read alpha value from band. Is combined with value from raster transparency / global alpha value.
        Default: -1 (not set)*/
    int mAlphaBand;

    bool mInvertColor;

    /**Maximum boundary for oversampling (to avoid too much data traffic). Default: 2.0*/
    double mMaxOversampling;

  private:
    /**Remove part into and release memory*/
    void removePartInfo( int bandNumer );
    void projectImage( const QImage& srcImg, QImage& dstImage, QgsRasterProjector* prj ) const;
};

inline double QgsRasterRenderer::readValue( void *data, QgsRasterDataProvider::DataType type, int index )
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
    case QgsRasterDataProvider::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterDataProvider::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterDataProvider::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterDataProvider::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterDataProvider::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterDataProvider::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterDataProvider::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  return mProvider->noDataValue();
}

#endif // QGSRASTERRENDERER_H
