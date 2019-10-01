/***************************************************************************
                         qgsmeshlayerutils.cpp
                         --------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshlayerutils.h"
#include "qgsmeshtimesettings.h"
#include "qgstriangularmesh.h"

#include <limits>
#include <QTime>
#include <QDateTime>

///@cond PRIVATE

QVector<double> QgsMeshLayerUtils::calculateMagnitudes( const QgsMeshDataBlock &block )
{
  Q_ASSERT( QgsMeshDataBlock::ActiveFlagInteger != block.type() );
  int count = block.count();
  QVector<double> ret( count );

  for ( int i = 0; i < count; ++i )
  {
    double mag = block.value( i ).scalar();
    ret[i] = mag;
  }
  return ret;
}

void QgsMeshLayerUtils::boundingBoxToScreenRectangle( const QgsMapToPixel &mtp,
    const QSize &outputSize,
    const QgsRectangle &bbox,
    int &leftLim,
    int &rightLim,
    int &topLim,
    int &bottomLim )
{
  QgsPointXY ll = mtp.transform( bbox.xMinimum(), bbox.yMinimum() );
  QgsPointXY ur = mtp.transform( bbox.xMaximum(), bbox.yMaximum() );
  topLim = std::max( int( ur.y() ), 0 );
  bottomLim = std::min( int( ll.y() ), outputSize.height() - 1 );
  leftLim = std::max( int( ll.x() ), 0 );
  rightLim = std::min( int( ur.x() ), outputSize.width() - 1 );
}

static void lamTol( double &lam )
{
  const static double eps = 1e-6;
  if ( ( lam < 0.0 ) && ( lam > -eps ) )
  {
    lam = 0.0;
  }
}

static bool E3T_physicalToBarycentric( const QgsPointXY &pA, const QgsPointXY &pB, const QgsPointXY &pC, const QgsPointXY &pP,
                                       double &lam1, double &lam2, double &lam3 )
{
  if ( pA == pB || pA == pC || pB == pC )
    return false; // this is not a valid triangle!

  // Compute vectors
  QgsVector v0( pC - pA );
  QgsVector v1( pB - pA );
  QgsVector v2( pP - pA );

  // Compute dot products
  double dot00 = v0 * v0;
  double dot01 = v0 * v1;
  double dot02 = v0 * v2;
  double dot11 = v1 * v1;
  double dot12 = v1 * v2;

  // Compute barycentric coordinates
  double invDenom = 1.0 / ( dot00 * dot11 - dot01 * dot01 );
  lam1 = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
  lam2 = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;
  lam3 = 1.0 - lam1 - lam2;

  // Apply some tolerance to lam so we can detect correctly border points
  lamTol( lam1 );
  lamTol( lam2 );
  lamTol( lam3 );

  // Return if POI is outside triangle
  if ( ( lam1 < 0 ) || ( lam2 < 0 ) || ( lam3 < 0 ) )
  {
    return false;
  }

  return true;
}

double QgsMeshLayerUtils::interpolateFromVerticesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val1, double val2, double val3, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return lam1 * val3 + lam2 * val2 + lam3 * val1;
}

double QgsMeshLayerUtils::interpolateFromFacesData( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
    double val, const QgsPointXY &pt )
{
  double lam1, lam2, lam3;
  if ( !E3T_physicalToBarycentric( p1, p2, p3, pt, lam1, lam2, lam3 ) )
    return std::numeric_limits<double>::quiet_NaN();

  return val;
}

QVector<double> QgsMeshLayerUtils::interpolateFromFacesData( QVector<double> valuesOnFaces, QgsMesh *nativeMesh,
    QgsTriangularMesh *triangularMesh,
    QgsMeshDataBlock *active,
    QgsMeshRendererScalarSettings::DataInterpolationMethod method )
{
  Q_UNUSED( triangularMesh )

  assert( nativeMesh );
  assert( active );
  assert( method == QgsMeshRendererScalarSettings::NeighbourAverage );

  // assuming that native vertex count = triangular vertex count
  assert( nativeMesh->vertices.size() == triangularMesh->vertices().size() );
  int vertexCount = triangularMesh->vertices().size();

  QVector<double> res( vertexCount, 0.0 );
  // for face datasets do simple average of the valid values of all faces that contains this vertex
  QVector<int> count( vertexCount, 0 );

  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    if ( active->active( i ) )
    {
      double val = valuesOnFaces[ i ];
      if ( !std::isnan( val ) )
      {
        // assign for all vertices
        const auto &face = nativeMesh->faces.at( i );
        for ( int j = 0; j < face.size(); ++j )
        {
          int vertexIndex = face[j];
          res[vertexIndex] += val;
          count[vertexIndex] += 1;
        }
      }
    }
  }

  for ( int i = 0; i < vertexCount; ++i )
  {
    if ( count.at( i ) > 0 )
    {
      res[i] = res[i] / double( count.at( i ) );
    }
    else
    {
      res[i] = std::numeric_limits<double>::quiet_NaN();
    }
  }

  return res;
}

QgsRectangle QgsMeshLayerUtils::triangleBoundingBox( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 )
{
  // p1
  double xMin = p1.x();
  double xMax = p1.x();
  double yMin = p1.y();
  double yMax = p1.y();

  //p2
  xMin = ( ( xMin < p2.x() ) ? xMin : p2.x() );
  xMax = ( ( xMax > p2.x() ) ? xMax : p2.x() );
  yMin = ( ( yMin < p2.y() ) ? yMin : p2.y() );
  yMax = ( ( yMax > p2.y() ) ? yMax : p2.y() );

  // p3
  xMin = ( ( xMin < p3.x() ) ? xMin : p3.x() );
  xMax = ( ( xMax > p3.x() ) ? xMax : p3.x() );
  yMin = ( ( yMin < p3.y() ) ? yMin : p3.y() );
  yMax = ( ( yMax > p3.y() ) ? yMax : p3.y() );

  QgsRectangle bbox( xMin,  yMin,  xMax,  yMax );
  return bbox;
}

QString QgsMeshLayerUtils::formatTime( double hours, const QgsMeshTimeSettings &settings )
{
  QString ret;
  if ( settings.useAbsoluteTime() )
  {
    QString format( settings.absoluteTimeFormat() );
    QDateTime dateTime( settings.absoluteTimeReferenceTime() );
    int seconds = static_cast<int>( hours * 3600.0 );
    dateTime = dateTime.addSecs( seconds );
    ret = dateTime.toString( format );
    if ( ret.isEmpty() ) // error
      ret = dateTime.toString();
  }
  else
  {
    QString format( settings.relativeTimeFormat() );
    format = format.trimmed();
    hours = hours + settings.relativeTimeOffsetHours();
    int totalHours = static_cast<int>( hours );

    if ( format == QStringLiteral( "hh:mm:ss.zzz" ) )
    {
      int ms = static_cast<int>( hours * 3600.0 * 1000 );
      int seconds = ms / 1000;
      int z = ms % 1000;
      int m = seconds / 60;
      int s = seconds % 60;
      int h = m / 60;
      m = m % 60;
      ret = QStringLiteral( "%1:%2:%3.%4" ).
            arg( h, 2, 10, QLatin1Char( '0' ) ).
            arg( m, 2, 10, QLatin1Char( '0' ) ).
            arg( s, 2, 10, QLatin1Char( '0' ) ).
            arg( z, 3, 10, QLatin1Char( '0' ) );
    }
    else if ( format == QStringLiteral( "hh:mm:ss" ) )
    {
      int seconds = static_cast<int>( hours * 3600.0 );
      int m = seconds / 60;
      int s = seconds % 60;
      int h = m / 60;
      m = m % 60;
      ret = QStringLiteral( "%1:%2:%3" ).
            arg( h, 2, 10, QLatin1Char( '0' ) ).
            arg( m, 2, 10, QLatin1Char( '0' ) ).
            arg( s, 2, 10, QLatin1Char( '0' ) );

    }
    else if ( format == QStringLiteral( "d hh:mm:ss" ) )
    {
      int seconds = static_cast<int>( hours * 3600.0 );
      int m = seconds / 60;
      int s = seconds % 60;
      int h = m / 60;
      m = m % 60;
      int d = totalHours / 24;
      h = totalHours % 24;
      ret = QStringLiteral( "%1 d %2:%3:%4" ).
            arg( d ).
            arg( h, 2, 10, QLatin1Char( '0' ) ).
            arg( m, 2, 10, QLatin1Char( '0' ) ).
            arg( s, 2, 10, QLatin1Char( '0' ) );
    }
    else if ( format == QStringLiteral( "d hh" ) )
    {
      int d = totalHours / 24;
      int h = totalHours % 24;
      ret = QStringLiteral( "%1 d %2" ).
            arg( d ).
            arg( h );
    }
    else if ( format == QStringLiteral( "d" ) )
    {
      int d = totalHours / 24;
      ret = QStringLiteral( "%1" ).arg( d );
    }
    else if ( format == QStringLiteral( "ss" ) )
    {
      int seconds = static_cast<int>( hours * 3600.0 );
      ret = QStringLiteral( "%1" ).arg( seconds );
    }
    else     // "hh"
    {
      ret = QStringLiteral( "%1" ).arg( hours );
    }
  }
  return ret;
}

///@endcond
