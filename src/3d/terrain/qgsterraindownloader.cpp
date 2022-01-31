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
#include "qgscoordinatetransform.h"
#include "qgsgdalutils.h"


QgsTerrainDownloader::QgsTerrainDownloader( const QgsCoordinateTransformContext &transformContext )
{
  setDataSource( defaultDataSource() );

  // the whole world is projected to a square:
  // X going from 180 W to 180 E
  // Y going from ~85 N to ~85 S  (=atan(sinh(pi)) ... to get a square)
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), transformContext );
  ct.setBallparkTransformsAreAppropriate( true );
  const QgsPointXY topLeftLonLat( -180, 180.0 / M_PI * std::atan( std::sinh( M_PI ) ) );
  const QgsPointXY bottomRightLonLat( 180, 180.0 / M_PI * std::atan( std::sinh( -M_PI ) ) );
  const QgsPointXY topLeft = ct.transform( topLeftLonLat );
  const QgsPointXY bottomRight = ct.transform( bottomRightLonLat );
  mXSpan = ( bottomRight.x() - topLeft.x() );
}

QgsTerrainDownloader::~QgsTerrainDownloader() = default;

QgsTerrainDownloader::DataSource QgsTerrainDownloader::defaultDataSource()
{
  // using terrain tiles stored on AWS and listed within Registry of Open Data on AWS
  // see https://registry.opendata.aws/terrain-tiles/
  //
  // tiles are generated using a variety of sources (SRTM, ETOPO1 and more detailed data for some countries)
  // for more details and attribution see https://github.com/tilezen/joerd/blob/master/docs/data-sources.md

  DataSource ds;
  ds.uri = "https://s3.amazonaws.com/elevation-tiles-prod/terrarium/{z}/{x}/{y}.png";
  ds.zMin = 0;
  ds.zMax = 15;
  return ds;
}

void QgsTerrainDownloader::setDataSource( const QgsTerrainDownloader::DataSource &ds )
{
  mDataSource = ds;
  const QString uri = QString( "type=xyz&url=%1&zmin=%2&zmax=%3" ).arg( mDataSource.uri ).arg( mDataSource.zMin ).arg( mDataSource.zMax );
  mOnlineDtm.reset( new QgsRasterLayer( uri, "terrarium", "wms" ) );
}


void QgsTerrainDownloader::adjustExtentAndResolution( double mupp, const QgsRectangle &extentOrig, QgsRectangle &extent, int &res )
{
  const double xMin = floor( extentOrig.xMinimum() / mupp ) * mupp;
  const double xMax = ceil( extentOrig.xMaximum() / mupp ) * mupp;

  const double yMin = floor( extentOrig.yMinimum() / mupp ) * mupp;
  const double yMax = ceil( extentOrig.yMaximum() / mupp ) * mupp;

  extent = QgsRectangle( xMin, yMin, xMax, yMax );
  res = round( ( xMax - xMin ) / mupp );
}


double QgsTerrainDownloader::findBestTileResolution( double requestedMupp )
{
  int zoom = 0;
  for ( ; zoom <= 15; ++zoom )
  {
    const double tileMupp = mXSpan / ( 256 * ( 1 << zoom ) );
    if ( tileMupp <= requestedMupp )
      break;
  }

  if ( zoom > 15 ) zoom = 15;
  const double finalMupp = mXSpan / ( 256 * ( 1 << zoom ) );
  return finalMupp;
}


void QgsTerrainDownloader::tileImageToHeightMap( const QImage &img, QByteArray &heightMap )
{
  // for description of the "terrarium" format:
  // https://github.com/tilezen/joerd/blob/master/docs/formats.md

  // assuming ARGB premultiplied but with alpha 255
  const QRgb *rgb = reinterpret_cast<const QRgb *>( img.constBits() );
  const int count = img.width() * img.height();
  heightMap.resize( sizeof( float ) * count );
  float *hData = reinterpret_cast<float *>( heightMap.data() );
  for ( int i = 0; i < count; ++i )
  {
    const QRgb c = rgb[i];
    if ( qAlpha( c ) == 255 )
    {
      const float h = qRed( c ) * 256 + qGreen( c ) + qBlue( c ) / 256.f - 32768;
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
  if ( !mOnlineDtm || !mOnlineDtm->isValid() )
  {
    QgsDebugMsg( "missing a valid data source" );
    return QByteArray();
  }

  QgsRectangle extentTr = extentOrig;
  if ( destCrs != mOnlineDtm->crs() )
  {
    // if in different CRS - need to reproject extent and resolution
    QgsCoordinateTransform ct( destCrs, mOnlineDtm->crs(), context );
    ct.setBallparkTransformsAreAppropriate( true );
    extentTr = ct.transformBoundingBox( extentOrig );
  }

  const double requestedMupp = extentTr.width() / res;
  const double finalMupp = findBestTileResolution( requestedMupp );

  // adjust extent to match native resolution of terrain tiles

  QgsRectangle extent;
  const int resOrig = res;
  adjustExtentAndResolution( finalMupp, extentTr, extent, res );

  // request tile

  QgsRasterBlock *b = mOnlineDtm->dataProvider()->block( 1, extent, res, res );
  const QImage img = b->image();
  delete b;
  if ( !tmpFilenameImg.isEmpty() )
    img.save( tmpFilenameImg );

  // convert to height data

  QByteArray heightMap;
  tileImageToHeightMap( img, heightMap );

  // prepare source/destination datasets for resampling

  const gdal::dataset_unique_ptr hSrcDS( QgsGdalUtils::createSingleBandMemoryDataset( GDT_Float32, extent, res, res, mOnlineDtm->crs() ) );
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

  const CPLErr err = GDALRasterIO( GDALGetRasterBand( hSrcDS.get(), 1 ), GF_Write, 0, 0, res, res, heightMap.data(), res, res, GDT_Float32, 0, 0 );
  if ( err != CE_None )
  {
    QgsDebugMsg( "failed to write heightmap data to GDAL dataset" );
    return QByteArray();
  }

  // resample to the desired extent + resolution
  QgsGdalUtils::resampleSingleBandRaster( hSrcDS.get(), hDstDS.get(), GRA_Bilinear,
                                          context.calculateCoordinateOperation( mOnlineDtm->crs(), destCrs ).toUtf8().constData() );

  QByteArray heightMapOut;
  heightMapOut.resize( resOrig * resOrig * sizeof( float ) );
  char *data = heightMapOut.data();

  // read the data back

  const CPLErr err2 = GDALRasterIO( GDALGetRasterBand( hDstDS.get(), 1 ), GF_Read, 0, 0, resOrig, resOrig, data, resOrig, resOrig, GDT_Float32, 0, 0 );
  if ( err2 != CE_None )
  {
    QgsDebugMsg( "failed to read heightmap data from GDAL dataset" );
    return QByteArray();
  }

  return heightMapOut;
}
