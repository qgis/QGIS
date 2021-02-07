/***************************************************************************
    qgsgcptransformer.cpp
     --------------------------------------
    Date                 : 18-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgcptransformer.h"

#include <gdal.h>
#include <gdal_alg.h>

#include "qgsleastsquares.h"

#include <cmath>

#include <cassert>
#include <limits>

//
// QgsLinearGeorefTransform
//

bool QgsLinearGeorefTransform::getOriginScale( QgsPointXY &origin, double &scaleX, double &scaleY ) const
{
  origin = mParameters.origin;
  scaleX = mParameters.scaleX;
  scaleY = mParameters.scaleY;
  return true;
}

bool QgsLinearGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &pixelCoords )
{
  if ( mapCoords.size() < minimumGcpCount() )
    return false;
  QgsLeastSquares::linear( mapCoords, pixelCoords, mParameters.origin, mParameters.scaleX, mParameters.scaleY );
  return true;
}

int QgsLinearGeorefTransform::minimumGcpCount() const
{
  return 2;
}

GDALTransformerFunc QgsLinearGeorefTransform::GDALTransformer() const
{
  return QgsLinearGeorefTransform::linearTransform;
}

void *QgsLinearGeorefTransform::GDALTransformerArgs() const
{
  return ( void * )&mParameters;
}

int QgsLinearGeorefTransform::linearTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  Q_UNUSED( z )
  LinearParameters *t = static_cast<LinearParameters *>( pTransformerArg );
  if ( !t )
    return false;

  if ( !bDstToSrc )
  {
    for ( int i = 0; i < nPointCount; ++i )
    {
      x[i] = x[i] * t->scaleX + t->origin.x();
      y[i] = -y[i] * t->scaleY + t->origin.y();
      panSuccess[i] = true;
    }
  }
  else
  {
    // Guard against division by zero
    if ( std::fabs( t->scaleX ) < std::numeric_limits<double>::epsilon() ||
         std::fabs( t->scaleY ) < std::numeric_limits<double>::epsilon() )
    {
      for ( int i = 0; i < nPointCount; ++i )
      {
        panSuccess[i] = false;
      }
      return false;
    }
    for ( int i = 0; i < nPointCount; ++i )
    {
      x[i] = ( x[i] - t->origin.x() ) / t->scaleX;
      y[i] = ( y[i] - t->origin.y() ) / ( -t->scaleY );
      panSuccess[i] = true;
    }
  }

  return true;
}

//
// QgsHelmertGeorefTransform
//
bool QgsHelmertGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &pixelCoords )
{
  if ( mapCoords.size() < minimumGcpCount() )
    return false;

  QgsLeastSquares::helmert( mapCoords, pixelCoords, mHelmertParameters.origin, mHelmertParameters.scale, mHelmertParameters.angle );
  return true;
}

int QgsHelmertGeorefTransform::minimumGcpCount() const
{
  return 2;
}


GDALTransformerFunc QgsHelmertGeorefTransform::GDALTransformer() const
{
  return QgsHelmertGeorefTransform::helmert_transform;
}

void *QgsHelmertGeorefTransform::GDALTransformerArgs() const
{
  return ( void * )&mHelmertParameters;
}

bool QgsHelmertGeorefTransform::getOriginScaleRotation( QgsPointXY &origin, double &scale, double &rotation ) const
{
  origin = mHelmertParameters.origin;
  scale = mHelmertParameters.scale;
  rotation = mHelmertParameters.angle;
  return true;
}

int QgsHelmertGeorefTransform::helmert_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  Q_UNUSED( z )
  HelmertParameters *t = static_cast<HelmertParameters *>( pTransformerArg );
  if ( !t )
    return false;

  double a = std::cos( t->angle ), b = std::sin( t->angle ), x0 = t->origin.x(), y0 = t->origin.y(), s = t->scale;
  if ( !bDstToSrc )
  {
    a *= s;
    b *= s;
    for ( int i = 0; i < nPointCount; ++i )
    {
      double xT = x[i], yT = y[i];
      // Because rotation parameters where estimated in a CS with negative y-axis ^= down.
      // we need to apply the rotation matrix and a change of base:
      // |cos a,-sin a| |1, 0|   | std::cos a,  std::sin a|
      // |sin a, std::cos a| |0,-1| = | std::sin a, -cos a|
      x[i] = x0 + ( a * xT + b * yT );
      y[i] = y0 + ( b * xT - a * yT );
      panSuccess[i] = true;
    }
  }
  else
  {
    // Guard against division by zero
    if ( std::fabs( s ) < std::numeric_limits<double>::epsilon() )
    {
      for ( int i = 0; i < nPointCount; ++i )
      {
        panSuccess[i] = false;
      }
      return false;
    }
    a /= s;
    b /= s;
    for ( int i = 0; i < nPointCount; ++i )
    {
      double xT = x[i], yT = y[i];
      xT -= x0;
      yT -= y0;
      // | std::cos a,  std::sin a |^-1   |cos a,  std::sin a|
      // | std::sin a, -cos a |    = |sin a, -cos a|
      x[i] = a * xT + b * yT;
      y[i] = b * xT - a * yT;
      panSuccess[i] = true;
    }
  }
  return true;
}

//
// QgsGDALGeorefTransform
//

QgsGDALGeorefTransform::QgsGDALGeorefTransform( bool useTPS, unsigned int polynomialOrder )
  : mPolynomialOrder( std::min( 3u, polynomialOrder ) )
  , mIsTPSTransform( useTPS )
{
  mGDALTransformer     = nullptr;
  mGDALTransformerArgs = nullptr;
}

QgsGDALGeorefTransform::~QgsGDALGeorefTransform()
{
  destroyGdalArgs();
}

bool QgsGDALGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &pixelCoords )
{
  assert( mapCoords.size() == pixelCoords.size() );
  if ( mapCoords.size() != pixelCoords.size() )
    return false;
  int n = mapCoords.size();

  GDAL_GCP *GCPList = new GDAL_GCP[n];
  for ( int i = 0; i < n; i++ )
  {
    GCPList[i].pszId = new char[20];
    snprintf( GCPList[i].pszId, 19, "gcp%i", i );
    GCPList[i].pszInfo = nullptr;
    GCPList[i].dfGCPPixel = pixelCoords[i].x();
    GCPList[i].dfGCPLine  = -pixelCoords[i].y();
    GCPList[i].dfGCPX = mapCoords[i].x();
    GCPList[i].dfGCPY = mapCoords[i].y();
    GCPList[i].dfGCPZ = 0;
  }
  destroyGdalArgs();

  if ( mIsTPSTransform )
    mGDALTransformerArgs = GDALCreateTPSTransformer( n, GCPList, false );
  else
    mGDALTransformerArgs = GDALCreateGCPTransformer( n, GCPList, mPolynomialOrder, false );

  for ( int i = 0; i < n; i++ )
  {
    delete [] GCPList[i].pszId;
  }
  delete [] GCPList;

  return nullptr != mGDALTransformerArgs;
}

int QgsGDALGeorefTransform::minimumGcpCount() const
{
  if ( mIsTPSTransform )
    return 1;
  else
    return ( ( mPolynomialOrder + 2 ) * ( mPolynomialOrder + 1 ) ) / 2;
}

GDALTransformerFunc QgsGDALGeorefTransform::GDALTransformer() const
{
  // Fail if no arguments were calculated through updateParametersFromGCP
  if ( !mGDALTransformerArgs )
    return nullptr;

  if ( mIsTPSTransform )
    return GDALTPSTransform;
  else
    return GDALGCPTransform;
}

void *QgsGDALGeorefTransform::GDALTransformerArgs() const
{
  return mGDALTransformerArgs;
}

void QgsGDALGeorefTransform::destroyGdalArgs()
{
  if ( mGDALTransformerArgs )
  {
    if ( mIsTPSTransform )
      GDALDestroyTPSTransformer( mGDALTransformerArgs );
    else
      GDALDestroyGCPTransformer( mGDALTransformerArgs );
  }
}

//
// QgsProjectiveGeorefTransform
//

QgsProjectiveGeorefTransform::QgsProjectiveGeorefTransform()
  : mParameters()
{}

bool QgsProjectiveGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &pixelCoords )
{
  if ( mapCoords.size() < minimumGcpCount() )
    return false;

  // HACK: flip y coordinates, because georeferencer and gdal use different conventions
  QVector<QgsPointXY> flippedPixelCoords;
  flippedPixelCoords.reserve( pixelCoords.size() );
  for ( const QgsPointXY &coord : pixelCoords )
  {
    flippedPixelCoords << QgsPointXY( coord.x(), -coord.y() );
  }

  QgsLeastSquares::projective( mapCoords, flippedPixelCoords, mParameters.H );

  // Invert the homography matrix using adjoint matrix
  double *H = mParameters.H;

  double adjoint[9];
  adjoint[0] = H[4] * H[8] - H[5] * H[7];
  adjoint[1] = -H[1] * H[8] + H[2] * H[7];
  adjoint[2] = H[1] * H[5] - H[2] * H[4];

  adjoint[3] = -H[3] * H[8] + H[5] * H[6];
  adjoint[4] = H[0] * H[8] - H[2] * H[6];
  adjoint[5] = -H[0] * H[5] + H[2] * H[3];

  adjoint[6] = H[3] * H[7] - H[4] * H[6];
  adjoint[7] = -H[0] * H[7] + H[1] * H[6];
  adjoint[8] = H[0] * H[4] - H[1] * H[3];

  double det = H[0] * adjoint[0] + H[3] * adjoint[1] + H[6] * adjoint[2];

  if ( std::fabs( det ) < 1024.0 * std::numeric_limits<double>::epsilon() )
  {
    mParameters.hasInverse = false;
  }
  else
  {
    mParameters.hasInverse = true;
    double oo_det = 1.0 / det;
    for ( int i = 0; i < 9; i++ )
    {
      mParameters.Hinv[i] = adjoint[i] * oo_det;
    }
  }
  return true;
}

int QgsProjectiveGeorefTransform::minimumGcpCount() const
{
  return 4;
}

GDALTransformerFunc QgsProjectiveGeorefTransform::GDALTransformer() const
{
  return QgsProjectiveGeorefTransform::projectiveTransform;
}

void *QgsProjectiveGeorefTransform::GDALTransformerArgs() const
{
  return ( void * )&mParameters;
}

int QgsProjectiveGeorefTransform::projectiveTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  Q_UNUSED( z )
  ProjectiveParameters *t = static_cast<ProjectiveParameters *>( pTransformerArg );
  if ( !t )
    return false;

  double *H = nullptr;
  if ( !bDstToSrc )
  {
    H = t->H;
  }
  else
  {
    if ( !t->hasInverse )
    {
      for ( int i = 0; i < nPointCount; ++i )
      {
        panSuccess[i] = false;
      }
      return false;
    }
    H = t->Hinv;
  }


  for ( int i = 0; i < nPointCount; ++i )
  {
    double Z = x[i] * H[6] + y[i] * H[7] + H[8];
    // Projects to infinity?
    if ( std::fabs( Z ) < 1024.0 * std::numeric_limits<double>::epsilon() )
    {
      panSuccess[i] = false;
      continue;
    }
    double X = ( x[i] * H[0] + y[i] * H[1] + H[2] ) / Z;
    double Y = ( x[i] * H[3] + y[i] * H[4] + H[5] ) / Z;

    x[i] = X;
    y[i] = Y;

    panSuccess[i] = true;
  }

  return true;
}
