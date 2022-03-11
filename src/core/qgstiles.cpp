/***************************************************************************
  qgstiles.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiles.h"

#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgssettings.h"

QgsTileMatrix QgsTileMatrix::fromWebMercator( int zoomLevel )
{
  constexpr double z0xMin = -20037508.3427892;
  constexpr double z0yMax =  20037508.3427892;

  return fromCustomDef( zoomLevel, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), QgsPointXY( z0xMin, z0yMax ), 2 * z0yMax );
}

QgsTileMatrix QgsTileMatrix::fromCustomDef( int zoomLevel, const QgsCoordinateReferenceSystem &crs,
    const QgsPointXY &z0TopLeftPoint, double z0Dimension, int z0MatrixWidth, int z0MatrixHeight )
{
  // Square extent calculation
  double z0xMin = z0TopLeftPoint.x();
  double z0yMax = z0TopLeftPoint.y();
  double z0xMax = z0xMin + z0MatrixWidth * z0Dimension;
  double z0yMin = z0yMax - z0MatrixHeight * z0Dimension;

  // Constant for scale denominator calculation
  constexpr double TILE_SIZE = 256.0;
  constexpr double PIXELS_TO_M = 2.8 / 10000.0;  // WMS/WMTS define "standardized rendering pixel size" as 0.28mm
  const double unitToMeters = QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), QgsUnitTypes::DistanceMeters );
  // Scale denominator calculation
  const double scaleDenom0 = ( z0Dimension / TILE_SIZE ) * ( unitToMeters / PIXELS_TO_M );

  int numTiles = static_cast<int>( pow( 2, zoomLevel ) ); // assuming we won't ever go over 30 zoom levels

  QgsTileMatrix tm;
  tm.mCrs = crs;
  tm.mZoomLevel = zoomLevel;
  tm.mMatrixWidth = z0MatrixWidth * numTiles;
  tm.mMatrixHeight = z0MatrixHeight * numTiles;
  tm.mTileXSpan = ( z0xMax - z0xMin ) / tm.mMatrixWidth;
  tm.mTileYSpan = ( z0yMax - z0yMin ) / tm.mMatrixHeight;
  tm.mExtent = QgsRectangle( z0xMin, z0yMin, z0xMax, z0yMax );
  tm.mScaleDenom = scaleDenom0 / pow( 2, zoomLevel );
  return tm;
}

QgsTileMatrix QgsTileMatrix::fromTileMatrix( const int zoomLevel, const QgsTileMatrix &tileMatrix )
{
  QgsTileMatrix tm;
  int numTiles = static_cast<int>( pow( 2, zoomLevel ) ); // assuming we won't ever go over 30 zoom levels
  int aZoomLevel = tileMatrix.zoomLevel();
  int aNumTiles = static_cast<int>( pow( 2, aZoomLevel ) );
  int aMatrixWidth = tileMatrix.matrixWidth();
  int aMatrixHeight = tileMatrix.matrixHeight();
  QgsRectangle aExtent = tileMatrix.extent();
  tm.mCrs = tileMatrix.crs();
  tm.mZoomLevel = zoomLevel;
  tm.mMatrixWidth = aMatrixWidth * numTiles / aNumTiles;
  tm.mMatrixHeight = aMatrixHeight * numTiles / aNumTiles;
  tm.mTileXSpan = aExtent.width() / tm.mMatrixWidth;
  tm.mTileYSpan = aExtent.height() / tm.mMatrixHeight;
  tm.mExtent = aExtent;
  tm.mScaleDenom = tileMatrix.scale() * pow( 2, aZoomLevel )  / pow( 2, zoomLevel );
  return tm;
}

QgsRectangle QgsTileMatrix::tileExtent( QgsTileXYZ id ) const
{
  double xMin = mExtent.xMinimum() + mTileXSpan * id.column();
  double xMax = xMin + mTileXSpan;
  double yMax = mExtent.yMaximum() - mTileYSpan * id.row();
  double yMin = yMax - mTileYSpan;
  return QgsRectangle( xMin, yMin, xMax, yMax );
}

QgsPointXY QgsTileMatrix::tileCenter( QgsTileXYZ id ) const
{
  double x = mExtent.xMinimum() + mTileXSpan * id.column() + mTileXSpan / 2;
  double y = mExtent.yMaximum() - mTileYSpan * id.row() - mTileYSpan / 2;
  return QgsPointXY( x, y );
}

QgsTileRange QgsTileMatrix::tileRangeFromExtent( const QgsRectangle &r ) const
{
  double x0 = std::clamp( r.xMinimum(), mExtent.xMinimum(), mExtent.xMaximum() );
  double y0 = std::clamp( r.yMinimum(), mExtent.yMinimum(), mExtent.yMaximum() );
  double x1 = std::clamp( r.xMaximum(), mExtent.xMinimum(), mExtent.xMaximum() );
  double y1 = std::clamp( r.yMaximum(), mExtent.yMinimum(), mExtent.yMaximum() );
  if ( x0 >= x1 || y0 >= y1 )
    return QgsTileRange();   // nothing to display

  double tileX1 = ( x0 - mExtent.xMinimum() ) / mTileXSpan;
  double tileX2 = ( x1 - mExtent.xMinimum() ) / mTileXSpan;
  double tileY1 = ( mExtent.yMaximum() - y1 ) / mTileYSpan;
  double tileY2 = ( mExtent.yMaximum() - y0 ) / mTileYSpan;

  QgsDebugMsgLevel( QStringLiteral( "Tile range of edges [%1,%2] - [%3,%4]" ).arg( tileX1 ).arg( tileY1 ).arg( tileX2 ).arg( tileY2 ), 2 );

  // figure out tile range from zoom
  int startColumn = std::clamp( static_cast<int>( floor( tileX1 ) ), 0, mMatrixWidth - 1 );
  int endColumn = std::clamp( static_cast<int>( floor( tileX2 ) ), 0, mMatrixWidth - 1 );
  int startRow = std::clamp( static_cast<int>( floor( tileY1 ) ), 0, mMatrixHeight - 1 );
  int endRow = std::clamp( static_cast<int>( floor( tileY2 ) ), 0, mMatrixHeight - 1 );
  return QgsTileRange( startColumn, endColumn, startRow, endRow );
}

QPointF QgsTileMatrix::mapToTileCoordinates( const QgsPointXY &mapPoint ) const
{
  double dx = mapPoint.x() - mExtent.xMinimum();
  double dy = mExtent.yMaximum() - mapPoint.y();
  return QPointF( dx / mTileXSpan, dy / mTileYSpan );
}

//
// QgsTileMatrixSet
//

void QgsTileMatrixSet::addGoogleCrs84QuadTiles( int minimumZoom, int maximumZoom )
{
  if ( maximumZoom < minimumZoom )
    std::swap( minimumZoom, maximumZoom );

  for ( int zoom = minimumZoom; zoom <= maximumZoom; ++zoom )
  {
    addMatrix( QgsTileMatrix::fromWebMercator( zoom ) );
  }
}

QgsTileMatrix QgsTileMatrixSet::tileMatrix( int zoom ) const
{
  return mTileMatrices.value( zoom );
}

void QgsTileMatrixSet::addMatrix( const QgsTileMatrix &matrix )
{
  mTileMatrices.insert( matrix.zoomLevel(), matrix );
}

int QgsTileMatrixSet::minimumZoom() const
{
  int res = -1;
  for ( auto it = mTileMatrices.constBegin(); it != mTileMatrices.constEnd(); ++it )
  {
    if ( res == -1 || it->zoomLevel() < res )
      res = it->zoomLevel();
  }
  return res;
}

int QgsTileMatrixSet::maximumZoom() const
{
  int res = -1;
  for ( auto it = mTileMatrices.constBegin(); it != mTileMatrices.constEnd(); ++it )
  {
    if ( res == -1 || it->zoomLevel() > res )
      res = it->zoomLevel();
  }
  return res;
}

void QgsTileMatrixSet::dropMatricesOutsideZoomRange( int minimumZoom, int maximumZoom )
{
  for ( auto it = mTileMatrices.begin(); it != mTileMatrices.end(); )
  {
    if ( it->zoomLevel() < minimumZoom || it->zoomLevel() > maximumZoom )
    {
      it = mTileMatrices.erase( it );
    }
    else
    {
      ++it;
    }
  }
}

QgsCoordinateReferenceSystem QgsTileMatrixSet::crs() const
{
  if ( mTileMatrices.empty() )
    return QgsCoordinateReferenceSystem();

  return mTileMatrices.value( minimumZoom() ).crs();
}

double QgsTileMatrixSet::scaleToZoom( double scale ) const
{
  int zoomUnder = -1;
  int zoomOver = -1;
  double scaleUnder = 0;
  double scaleOver = 0;

  switch ( mScaleToTileZoomMethod )
  {
    case Qgis::ScaleToTileZoomLevelMethod::MapBox:
    {
      // TODO: it seems that map scale is double (is that because of high-dpi screen?)
      // (this TODO was taken straight from QgsVectorTileUtils::scaleToZoom!)
      scale *= 2;
      break;
    }
    case Qgis::ScaleToTileZoomLevelMethod::Esri:
      break;
  }

  for ( auto it = mTileMatrices.constBegin(); it != mTileMatrices.constEnd(); ++it )
  {
    if ( it->scale() > scale && ( zoomUnder == -1 || zoomUnder < it->zoomLevel() ) )
    {
      zoomUnder = it->zoomLevel();
      scaleUnder = it->scale();
    }
    if ( it->scale() < scale && ( zoomOver == -1 || zoomOver > it->zoomLevel() ) )
    {
      zoomOver = it->zoomLevel();
      scaleOver = it->scale();
    }
  }

  if ( zoomUnder < 0 )
    return zoomOver;
  if ( zoomOver < 0 )
    return zoomUnder;
  else
    return ( scaleUnder - scale ) / ( scaleUnder - scaleOver ) * ( zoomOver - zoomUnder ) + zoomUnder;
}

int QgsTileMatrixSet::scaleToZoomLevel( double scale ) const
{
  int tileZoom = 0;
  switch ( mScaleToTileZoomMethod )
  {
    case Qgis::ScaleToTileZoomLevelMethod::MapBox:
      tileZoom = static_cast<int>( round( scaleToZoom( scale ) ) );
      break;
    case Qgis::ScaleToTileZoomLevelMethod::Esri:
      tileZoom = static_cast<int>( floor( scaleToZoom( scale ) ) );
      break;
  }

  return std::clamp( tileZoom, minimumZoom(), maximumZoom() );
}

bool QgsTileMatrixSet::readXml( const QDomElement &element, QgsReadWriteContext & )
{
  mTileMatrices.clear();

  mScaleToTileZoomMethod = qgsEnumKeyToValue( element.attribute( QStringLiteral( "scaleToZoomMethod" ) ), Qgis::ScaleToTileZoomLevelMethod::MapBox );

  const QDomNodeList children = element.childNodes();
  for ( int i = 0; i < children.size(); i++ )
  {
    const QDomElement matrixElement = children.at( i ).toElement();

    QgsTileMatrix matrix;
    matrix.mZoomLevel = matrixElement.attribute( QStringLiteral( "zoomLevel" ) ).toInt();
    matrix.mMatrixWidth = matrixElement.attribute( QStringLiteral( "matrixWidth" ) ).toInt();
    matrix.mMatrixHeight = matrixElement.attribute( QStringLiteral( "matrixHeight" ) ).toInt();
    matrix.mExtent = QgsRectangle(
                       matrixElement.attribute( QStringLiteral( "xMin" ) ).toDouble(),
                       matrixElement.attribute( QStringLiteral( "yMin" ) ).toDouble(),
                       matrixElement.attribute( QStringLiteral( "xMax" ) ).toDouble(),
                       matrixElement.attribute( QStringLiteral( "yMax" ) ).toDouble()
                     );

    matrix.mScaleDenom = matrixElement.attribute( QStringLiteral( "scale" ) ).toDouble();
    matrix.mTileXSpan = matrixElement.attribute( QStringLiteral( "tileXSpan" ) ).toDouble();
    matrix.mTileYSpan = matrixElement.attribute( QStringLiteral( "tileYSpan" ) ).toDouble();
    matrix.mCrs.readXml( matrixElement );
    addMatrix( matrix );
  }
  return true;
}

QDomElement QgsTileMatrixSet::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement setElement = document.createElement( QStringLiteral( "matrixSet" ) );
  setElement.setAttribute( QStringLiteral( "scaleToZoomMethod" ), qgsEnumValueToKey( mScaleToTileZoomMethod ) );

  for ( auto it = mTileMatrices.constBegin(); it != mTileMatrices.constEnd(); ++it )
  {
    QDomElement matrixElement = document.createElement( QStringLiteral( "matrix" ) );
    matrixElement.setAttribute( QStringLiteral( "zoomLevel" ), it->zoomLevel() );
    matrixElement.setAttribute( QStringLiteral( "matrixWidth" ), it->matrixWidth() );
    matrixElement.setAttribute( QStringLiteral( "matrixHeight" ), it->matrixHeight() );

    matrixElement.setAttribute( QStringLiteral( "xMin" ), qgsDoubleToString( it->mExtent.xMinimum() ) );
    matrixElement.setAttribute( QStringLiteral( "xMax" ), qgsDoubleToString( it->mExtent.xMaximum() ) );
    matrixElement.setAttribute( QStringLiteral( "yMin" ), qgsDoubleToString( it->mExtent.yMinimum() ) );
    matrixElement.setAttribute( QStringLiteral( "yMax" ), qgsDoubleToString( it->mExtent.yMaximum() ) );

    matrixElement.setAttribute( QStringLiteral( "scale" ), qgsDoubleToString( it->scale() ) );
    matrixElement.setAttribute( QStringLiteral( "tileXSpan" ), qgsDoubleToString( it->mTileXSpan ) );
    matrixElement.setAttribute( QStringLiteral( "tileYSpan" ), qgsDoubleToString( it->mTileYSpan ) );

    it->crs().writeXml( matrixElement, document );
    setElement.appendChild( matrixElement );
  }
  return setElement;
}
