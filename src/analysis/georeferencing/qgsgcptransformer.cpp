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


bool QgsGcpTransformerInterface::transform( double &x, double &y, bool inverseTransform ) const
{
  GDALTransformerFunc t = GDALTransformer();
  // Fail if no transformer function was returned
  if ( !t )
    return false;

  double z = 0.0;
  int success = 0;

  // Call GDAL transform function
  ( *t )( GDALTransformerArgs(), inverseTransform ? 1 : 0, 1,  &x, &y, &z, &success );
  if ( !success )
    return false;

  return true;
}

QString QgsGcpTransformerInterface::methodToString( QgsGcpTransformerInterface::TransformMethod method )
{
  switch ( method )
  {
    case QgsGcpTransformerInterface::TransformMethod::Linear:
      return QObject::tr( "Linear" );
    case QgsGcpTransformerInterface::TransformMethod::Helmert:
      return QObject::tr( "Helmert" );
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder1:
      return QObject::tr( "Polynomial 1" );
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2:
      return QObject::tr( "Polynomial 2" );
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder3:
      return QObject::tr( "Polynomial 3" );
    case QgsGcpTransformerInterface::TransformMethod::ThinPlateSpline:
      return QObject::tr( "Thin Plate Spline (TPS)" );
    case QgsGcpTransformerInterface::TransformMethod::Projective:
      return QObject::tr( "Projective" );
    default:
      return QObject::tr( "Not set" );
  }
}

QgsGcpTransformerInterface *QgsGcpTransformerInterface::create( QgsGcpTransformerInterface::TransformMethod method )
{
  switch ( method )
  {
    case TransformMethod::Linear:
      return new QgsLinearGeorefTransform;
    case TransformMethod::Helmert:
      return new QgsHelmertGeorefTransform;
    case TransformMethod::PolynomialOrder1:
      return new QgsGDALGeorefTransform( false, 1 );
    case TransformMethod::PolynomialOrder2:
      return new QgsGDALGeorefTransform( false, 2 );
    case TransformMethod::PolynomialOrder3:
      return new QgsGDALGeorefTransform( false, 3 );
    case TransformMethod::ThinPlateSpline:
      return new QgsGDALGeorefTransform( true, 0 );
    case TransformMethod::Projective:
      return new QgsProjectiveGeorefTransform;
    default:
      return nullptr;
  }
}

QgsGcpTransformerInterface *QgsGcpTransformerInterface::createFromParameters( QgsGcpTransformerInterface::TransformMethod method, const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates )
{
  std::unique_ptr< QgsGcpTransformerInterface > transformer( create( method ) );
  if ( !transformer )
    return nullptr;

  if ( !transformer->updateParametersFromGcps( sourceCoordinates, destinationCoordinates ) )
    return nullptr;

  return transformer.release();
}


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

QgsGcpTransformerInterface *QgsLinearGeorefTransform::clone() const
{
  std::unique_ptr< QgsLinearGeorefTransform > res = std::make_unique< QgsLinearGeorefTransform >();
  res->mParameters = mParameters;
  return res.release();
}

bool QgsLinearGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, bool invertYAxis )
{
  if ( destinationCoordinates.size() < minimumGcpCount() )
    return false;

  mParameters.invertYAxis = invertYAxis;
  QgsLeastSquares::linear( sourceCoordinates, destinationCoordinates, mParameters.origin, mParameters.scaleX, mParameters.scaleY );
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

QgsGcpTransformerInterface::TransformMethod QgsLinearGeorefTransform::method() const
{
  return TransformMethod::Linear;
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
      y[i] = ( t->invertYAxis ? -1 : 1 ) * y[i] * t->scaleY + t->origin.y();
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
      y[i] = ( y[i] - t->origin.y() ) / ( ( t->invertYAxis ? -1 : 1 ) * t->scaleY );
      panSuccess[i] = true;
    }
  }

  return true;
}

//
// QgsHelmertGeorefTransform
//
bool QgsHelmertGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, bool invertYAxis )
{
  if ( destinationCoordinates.size() < minimumGcpCount() )
    return false;

  mHelmertParameters.invertYAxis = invertYAxis;
  QgsLeastSquares::helmert( sourceCoordinates, destinationCoordinates, mHelmertParameters.origin, mHelmertParameters.scale, mHelmertParameters.angle );
  return true;
}

int QgsHelmertGeorefTransform::minimumGcpCount() const
{
  return 2;
}

GDALTransformerFunc QgsHelmertGeorefTransform::GDALTransformer() const
{
  return QgsHelmertGeorefTransform::helmertTransform;
}

void *QgsHelmertGeorefTransform::GDALTransformerArgs() const
{
  return ( void * )&mHelmertParameters;
}

QgsGcpTransformerInterface::TransformMethod QgsHelmertGeorefTransform::method() const
{
  return TransformMethod::Helmert;
}

bool QgsHelmertGeorefTransform::getOriginScaleRotation( QgsPointXY &origin, double &scale, double &rotation ) const
{
  origin = mHelmertParameters.origin;
  scale = mHelmertParameters.scale;
  rotation = mHelmertParameters.angle;
  return true;
}

QgsGcpTransformerInterface *QgsHelmertGeorefTransform::clone() const
{
  std::unique_ptr< QgsHelmertGeorefTransform > res = std::make_unique< QgsHelmertGeorefTransform >();
  res->mHelmertParameters = mHelmertParameters;
  return res.release();
}

int QgsHelmertGeorefTransform::helmertTransform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  Q_UNUSED( z )
  const HelmertParameters *t = static_cast< const HelmertParameters *>( pTransformerArg );
  if ( !t )
    return false;

  double a = std::cos( t->angle );
  double b = std::sin( t->angle );
  const double x0 = t->origin.x();
  const double y0 = t->origin.y();
  const double s = t->scale;
  if ( !bDstToSrc )
  {
    a *= s;
    b *= s;
    for ( int i = 0; i < nPointCount; ++i )
    {
      const double xT = x[i];
      const double yT = y[i];

      if ( t->invertYAxis )
      {
        // Because rotation parameters where estimated in a CS with negative y-axis ^= down.
        // we need to apply the rotation matrix and a change of base:
        // |cos a, -sin a | |1, 0|   | cos a,  sin a|
        // |sin a,  cos a | |0,-1| = | sin a, -cos a|
        x[i] = x0 + ( a * xT + b * yT );
        y[i] = y0 + ( b * xT - a * yT );
      }
      else
      {
        x[i] = x0 + ( a * xT - b * yT );
        y[i] = y0 + ( b * xT + a * yT );
      }
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
      const double xT = x[i] - x0;
      const double yT = y[i] - y0;
      if ( t->invertYAxis )
      {
        // | cos a,  sin a |^-1   |cos a,  sin a|
        // | sin a, -cos a |    = |sin a, -cos a|
        x[i] = a * xT + b * yT;
        y[i] = b * xT - a * yT;
      }
      else
      {
        x[i] = a * xT + b * yT;
        y[i] = -b * xT + a * yT;
      }
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
}

QgsGDALGeorefTransform::~QgsGDALGeorefTransform()
{
  destroyGdalArgs();
}

QgsGcpTransformerInterface *QgsGDALGeorefTransform::clone() const
{
  std::unique_ptr< QgsGDALGeorefTransform > res = std::make_unique< QgsGDALGeorefTransform >( mIsTPSTransform, mPolynomialOrder );
  res->updateParametersFromGcps( mSourceCoords, mDestCoordinates, mInvertYAxis );
  return res.release();
}

bool QgsGDALGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, bool invertYAxis )
{
  mSourceCoords = sourceCoordinates;
  mDestCoordinates = destinationCoordinates;
  mInvertYAxis = invertYAxis;

  assert( sourceCoordinates.size() == destinationCoordinates.size() );
  if ( sourceCoordinates.size() != destinationCoordinates.size() )
    return false;
  const int n = sourceCoordinates.size();

  GDAL_GCP *GCPList = new GDAL_GCP[n];
  for ( int i = 0; i < n; i++ )
  {
    GCPList[i].pszId = new char[20];
    snprintf( GCPList[i].pszId, 19, "gcp%i", i );
    GCPList[i].pszInfo = nullptr;
    GCPList[i].dfGCPPixel = sourceCoordinates[i].x();
    GCPList[i].dfGCPLine  = ( mInvertYAxis ? -1 : 1 ) * sourceCoordinates[i].y();
    GCPList[i].dfGCPX = destinationCoordinates[i].x();
    GCPList[i].dfGCPY = destinationCoordinates[i].y();
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

QgsGcpTransformerInterface::TransformMethod QgsGDALGeorefTransform::method() const
{
  if ( mIsTPSTransform )
    return TransformMethod::ThinPlateSpline;

  switch ( mPolynomialOrder )
  {
    case 1:
      return TransformMethod::PolynomialOrder1;
    case 2:
      return TransformMethod::PolynomialOrder2;
    case 3:
      return TransformMethod::PolynomialOrder3;
  }
  return TransformMethod::InvalidTransform;
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

QgsGcpTransformerInterface *QgsProjectiveGeorefTransform::clone() const
{
  std::unique_ptr< QgsProjectiveGeorefTransform > res = std::make_unique< QgsProjectiveGeorefTransform >();
  res->mParameters = mParameters;
  return res.release();
}

bool QgsProjectiveGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, bool invertYAxis )
{
  if ( destinationCoordinates.size() < minimumGcpCount() )
    return false;

  if ( invertYAxis )
  {
    // HACK: flip y coordinates, because georeferencer and gdal use different conventions
    QVector<QgsPointXY> flippedPixelCoords;
    flippedPixelCoords.reserve( sourceCoordinates.size() );
    for ( const QgsPointXY &coord : sourceCoordinates )
    {
      flippedPixelCoords << QgsPointXY( coord.x(), -coord.y() );
    }

    QgsLeastSquares::projective( flippedPixelCoords, destinationCoordinates, mParameters.H );
  }
  else
  {
    QgsLeastSquares::projective( sourceCoordinates, destinationCoordinates, mParameters.H );
  }

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

  const double det = H[0] * adjoint[0] + H[3] * adjoint[1] + H[6] * adjoint[2];

  if ( std::fabs( det ) < 1024.0 * std::numeric_limits<double>::epsilon() )
  {
    mParameters.hasInverse = false;
  }
  else
  {
    mParameters.hasInverse = true;
    const double oo_det = 1.0 / det;
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

QgsGcpTransformerInterface::TransformMethod QgsProjectiveGeorefTransform::method() const
{
  return TransformMethod::Projective;
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
    const double Z = x[i] * H[6] + y[i] * H[7] + H[8];
    // Projects to infinity?
    if ( std::fabs( Z ) < 1024.0 * std::numeric_limits<double>::epsilon() )
    {
      panSuccess[i] = false;
      continue;
    }
    const double X = ( x[i] * H[0] + y[i] * H[1] + H[2] ) / Z;
    const double Y = ( x[i] * H[3] + y[i] * H[4] + H[5] ) / Z;

    x[i] = X;
    y[i] = Y;

    panSuccess[i] = true;
  }

  return true;
}
