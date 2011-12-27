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
class QgsRasterTransparency;
class QgsRasterViewPort;

class QgsRasterRenderer
{
  public:

    struct RasterPartInfo
    {
      int currentCol;
      int currentRow;
      int nCols;
      int nRows;
      int nColsPerPart;
      int nRowsPerPart;
      int nPartsPerDimension;
      void* data;
    };

    QgsRasterRenderer( QgsRasterDataProvider* provider, QgsRasterResampler* resampler = 0 );
    virtual ~QgsRasterRenderer();
    virtual void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel ) = 0;

    bool usesTransparency() const;

    void setOpacity( double opacity ) { mOpacity = opacity; }
    double opacity() const { return mOpacity; }

    void setRasterTransparency( QgsRasterTransparency* t ) { mRasterTransparency = t; }
    const QgsRasterTransparency* rasterTransparency() const { return mRasterTransparency; }

    void setAlphaBand( int band ) { mAlphaBand = band; }
    int alphaBand() const { return mAlphaBand; }

  protected:
    inline double readValue( void *data, QgsRasterDataProvider::DataType type, int index );

    void startRasterRead( int bandNumber, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double& oversampling );
    bool readNextRasterPart( int bandNumber, QgsRasterViewPort* viewPort, int& nCols, int& nRows, void** rasterData, int& topLeftCol, int& topLeftRow );
    void stopRasterRead( int bandNumber );


    QgsRasterDataProvider* mProvider;
    QgsRasterResampler* mResampler;
    QMap<int, RasterPartInfo> mRasterPartInfos;

    /**Global alpha value (0-1)*/
    double mOpacity;
    /**Raster transparency per color or value. Overwrites global alpha value*/
    QgsRasterTransparency* mRasterTransparency;
    /**Read alpha value from band. Is combined with value from raster transparency / global alpha value.
        Default: -1 (not set)*/
    int mAlphaBand;

  private:
    /**Remove part into and release memory*/
    void removePartInfo( int bandNumer );
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
