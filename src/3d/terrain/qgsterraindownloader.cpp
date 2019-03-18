/***************************************************************************
  qgsterraindownloader.cpp
  --------------------------------------
  Date                 : March 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsterraindownloader.h"

#include "qgslogger.h"
#include "qgsrasterlayer.h"

#include "qgsgdalutils.h"


QgsTerrainDownloader::QgsTerrainDownloader()
{
  QString uri = "type=xyz&url=http://s3.amazonaws.com/elevation-tiles-prod/terrarium/{z}/{x}/{y}.png&zmax=15&zmin=0";
  onlineDtm.reset( new QgsRasterLayer( uri, "terrarium", "wms" ) );

  // the whole world is projected to a square:
  // X going from 180 W to 180 E
  // Y going from ~85 N to ~85 S  (=atan(sinh(pi)) ... to get a square)
  Q_NOWARN_DEPRECATED_PUSH
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  Q_NOWARN_DEPRECATED_POP
  QgsPointXY topLeftLonLat( -180, 180.0 / M_PI * std::atan( std::sinh( M_PI ) ) );
  QgsPointXY bottomRightLonLat( 180, 180.0 / M_PI * std::atan( std::sinh( -M_PI ) ) );
  QgsPointXY topLeft = ct.transform( topLeftLonLat );
  QgsPointXY bottomRight = ct.transform( bottomRightLonLat );
  mXSpan = ( bottomRight.x() - topLeft.x() );
}

QgsTerrainDownloader::~QgsTerrainDownloader() = default;


void QgsTerrainDownloader::adjustExtentAndResolution( double mupp, const QgsRectangle &extentOrig, QgsRectangle &extent, int &res )
{
  double xMin = floor( extentOrig.xMinimum() / mupp ) * mupp;
  double xMax = ceil( extentOrig.xMaximum() / mupp ) * mupp;

  double yMin = floor( extentOrig.yMinimum() / mupp ) * mupp;
  double yMax = ceil( extentOrig.yMaximum() / mupp ) * mupp;

  extent = QgsRectangle( xMin, yMin, xMax, yMax );
  res = round( ( xMax - xMin ) / mupp );
}


double QgsTerrainDownloader::findBestTileResolution( double requestedMupp )
{
  int zoom = 0;
  for ( ; zoom <= 15; ++zoom )
  {
    double tileMupp = mXSpan / ( 256 * ( 1 << zoom ) );
    if ( tileMupp <= requestedMupp )
      break;
  }

  if ( zoom > 15 ) zoom = 15;
  double finalMupp = mXSpan / ( 256 * ( 1 << zoom ) );
  return finalMupp;
}


void QgsTerrainDownloader::tileImageToHeightMap( const QImage &img, QByteArray &heightMap )
{
  // assuming ARGB premultiplied but with alpha 255
  const QRgb *rgb = reinterpret_cast<const QRgb *>( img.constBits() );
  int count = img.width() * img.height();
  heightMap.resize( sizeof( float ) * count );
  float *hData = reinterpret_cast<float *>( heightMap.data() );
  for ( int i = 0; i < count; ++i )
  {
    QRgb c = rgb[i];
    if ( qAlpha( c ) == 255 )
    {
      float h = qRed( c ) * 256 + qGreen( c ) + qBlue( c ) / 256.f - 32768;
      *hData++ = h;
    }
    else
    {
      *hData++ = std::numeric_limits<float>::quiet_NaN();
    }
  }
}


QByteArray QgsTerrainDownloader::getHeightMap( const QgsRectangle &extentOrig, int res, const QgsCoordinateReferenceSystem &destCrs, const QgsCoordinateTransformContext &context, QString tmpFilenameImg, QString tmpFilenameTif )
{
  QgsRectangle extentTr = extentOrig;
  if ( destCrs != onlineDtm->crs() )
  {
    // if in different CRS - need to reproject extent and resolution
    QgsCoordinateTransform ct( destCrs, onlineDtm->crs(), context );
    extentTr = ct.transformBoundingBox( extentOrig );
  }

  double requestedMupp = extentTr.width() / res;
  double finalMupp = findBestTileResolution( requestedMupp );

  // adjust extent to match native resolution of terrain tiles

  QgsRectangle extent;
  int resOrig = res;
  adjustExtentAndResolution( finalMupp, extentTr, extent, res );

  // request tile

  QgsRasterBlock *b = onlineDtm->dataProvider()->block( 1, extent, res, res );
  QImage img = b->image();
  delete b;
  if ( !tmpFilenameImg.isEmpty() )
    img.save( tmpFilenameImg );

  // convert to height data

  QByteArray heightMap;
  tileImageToHeightMap( img, heightMap );

  // prepare source/destination datasets for resampling

  gdal::dataset_unique_ptr hSrcDS( QgsGdalUtils::createSingleBandMemoryDataset( GDT_Float32, extent, res, res, onlineDtm->crs() ) );
  gdal::dataset_unique_ptr hDstDS;
  if ( !tmpFilenameTif.isEmpty() )
    hDstDS = QgsGdalUtils::createSingleBandTiffDataset( tmpFilenameTif, GDT_Float32, extentOrig, resOrig, resOrig, destCrs );
  else
    hDstDS = QgsGdalUtils::createSingleBandMemoryDataset( GDT_Float32, extentOrig, resOrig, resOrig, destCrs );

  if ( !hSrcDS || !hDstDS )
  {
    QgsDebugMsg( "failed to create GDAL dataset for heightmap" );
    return QByteArray();
  }

  CPLErr err = GDALRasterIO( GDALGetRasterBand( hSrcDS.get(), 1 ), GF_Write, 0, 0, res, res, heightMap.data(), res, res, GDT_Float32, 0, 0 );
  if ( err != CE_None )
  {
    QgsDebugMsg( "failed to write heightmap data to GDAL dataset" );
    return QByteArray();
  }

  // resample to the desired extent + resolution

  QgsGdalUtils::resampleSingleBandRaster( hSrcDS.get(), hDstDS.get(), GRA_Bilinear );

  QByteArray heightMapOut;
  heightMapOut.resize( resOrig * resOrig * sizeof( float ) );
  char *data = heightMapOut.data();

  // read the data back

  CPLErr err2 = GDALRasterIO( GDALGetRasterBand( hDstDS.get(), 1 ), GF_Read, 0, 0, resOrig, resOrig, data, resOrig, resOrig, GDT_Float32, 0, 0 );
  if ( err2 != CE_None )
  {
    QgsDebugMsg( "failed to read heightmap data from GDAL dataset" );
    return QByteArray();
  }

  return heightMapOut;
}
