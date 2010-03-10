/***************************************************************************
    qgsgeoreftransform.cpp - Encapsulates GCP-based parameter estimation and
    reprojection for different transformation models.
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
/* $Id */

#include "qgsgeoreftransform.h"

#include <gdal.h>
#include <gdal_alg.h>

#include "qgsleastsquares.h"

#include <cmath>
using std::abs;
using std::cos;
using std::sin;
using std::pow;

#include <cassert>
#include <limits>

/**
 * A simple transform which is paremetrized by a translation and anistotropic scale.
 */
class QgsLinearGeorefTransform : public QgsGeorefTransformInterface
{
  public:
    QgsLinearGeorefTransform()  { }
    ~QgsLinearGeorefTransform() { }

    bool getOriginScale( QgsPoint &origin, double &scaleX, double &scaleY ) const;

    bool updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords );
    uint getMinimumGCPCount() const;

    GDALTransformerFunc  GDALTransformer()     const { return QgsLinearGeorefTransform::linear_transform; }
    void                *GDALTransformerArgs() const { return ( void * )&mParameters; }
  private:
    struct LinearParameters
    {
      QgsPoint origin;
      double scaleX, scaleY;
    } mParameters;

    static int linear_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                 double *x, double *y, double *z, int *panSuccess );
};

/**
 * 2-dimensional helmert transform, parametrised by isotropic scale, rotation angle and translation.
 */
class QgsHelmertGeorefTransform : public QgsGeorefTransformInterface
{
  public:
    struct HelmertParameters
    {
      QgsPoint origin;
      double   scale;
      double   angle;
    };

    bool updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords );
    uint getMinimumGCPCount() const;

    GDALTransformerFunc  GDALTransformer()     const;
    void                *GDALTransformerArgs() const;
  private:
    HelmertParameters mHelmertParameters;

    static int helmert_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                  double *x, double *y, double *z, int *panSuccess );
};

/**
 * Interface to gdal thin plate splines and 1st/2nd/3rd order polynomials.
 */
class QgsGDALGeorefTransform : public QgsGeorefTransformInterface
{
  public:
    QgsGDALGeorefTransform( bool useTPS, unsigned int polynomialOrder );
    ~QgsGDALGeorefTransform();

    bool updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords );
    uint getMinimumGCPCount() const;

    GDALTransformerFunc  GDALTransformer()     const;
    void                *GDALTransformerArgs() const;
  private:
    void destroy_gdal_args();

    const int  mPolynomialOrder;
    const bool mIsTPSTransform;

    GDALTransformerFunc  mGDALTransformer;
    void                *mGDALTransformerArgs;
};

QgsGeorefTransform::QgsGeorefTransform( const QgsGeorefTransform &other )
{
  mTransformParametrisation = InvalidTransform;
  mGeorefTransformImplementation = NULL;
  selectTransformParametrisation( other.mTransformParametrisation );
}

QgsGeorefTransform::QgsGeorefTransform( TransformParametrisation parametrisation )
{
  mTransformParametrisation = InvalidTransform;
  mGeorefTransformImplementation = NULL;
  selectTransformParametrisation( parametrisation );
}

QgsGeorefTransform::QgsGeorefTransform()
{
  mTransformParametrisation = InvalidTransform;
  mGeorefTransformImplementation = NULL;
  mParametersInitialized = false;
}

QgsGeorefTransform::~QgsGeorefTransform()
{
  delete mGeorefTransformImplementation;
}

QgsGeorefTransform::TransformParametrisation QgsGeorefTransform::transformParametrisation() const
{
  return mTransformParametrisation;
}

void QgsGeorefTransform::selectTransformParametrisation( TransformParametrisation parametrisation )
{
  if ( parametrisation != mTransformParametrisation )
  {
    delete mGeorefTransformImplementation;
    mGeorefTransformImplementation = QgsGeorefTransform::createImplementation( parametrisation );
    mParametersInitialized = false;
    mTransformParametrisation = parametrisation;
  }
}

bool QgsGeorefTransform::parametersInitialized() const
{
  return mParametersInitialized;
}

bool QgsGeorefTransform::updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords )
{
  if ( !mGeorefTransformImplementation )
  {
    return false;
  }
  if ( mapCoords.size() != pixelCoords.size() ) // Defensive sanity check
  {
    throw( std::domain_error( "Internal error: GCP mapping is not one-to-one" ) );
  }
  if ( mapCoords.size() < getMinimumGCPCount() )
  {
    return false;
  }
  return mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGCPs( mapCoords, pixelCoords );
}

uint QgsGeorefTransform::getMinimumGCPCount() const
{
  if ( !mGeorefTransformImplementation )
  {
    return 0u;
  }
  return mGeorefTransformImplementation->getMinimumGCPCount();
}

GDALTransformerFunc QgsGeorefTransform::GDALTransformer() const
{
  if ( !mGeorefTransformImplementation )
  {
    return NULL;
  }
  return mGeorefTransformImplementation->GDALTransformer();
}

void* QgsGeorefTransform::GDALTransformerArgs() const
{
  if ( !mGeorefTransformImplementation )
  {
    return NULL;
  }
  return mGeorefTransformImplementation->GDALTransformerArgs();
}

QgsGeorefTransformInterface *QgsGeorefTransform::createImplementation( TransformParametrisation parametrisation )
{
  switch ( parametrisation )
  {
    case Linear:           return new QgsLinearGeorefTransform;
    case Helmert:          return new QgsHelmertGeorefTransform;
    case PolynomialOrder1: return new QgsGDALGeorefTransform( false, 1 );
    case PolynomialOrder2: return new QgsGDALGeorefTransform( false, 2 );
    case PolynomialOrder3: return new QgsGDALGeorefTransform( false, 3 );
    case ThinPlateSpline:  return new QgsGDALGeorefTransform( true, 0 );
    default:               return NULL;
      break;
  }
}

bool QgsGeorefTransform::transformRasterToWorld( const QgsPoint &raster, QgsPoint &world ) const
{
  // flip y coordinate due to different CS orientation
  QgsPoint raster_flipped( raster.x(), -raster.y() );
  return gdal_transform( raster_flipped, world, 0 );
}

bool QgsGeorefTransform::transformWorldToRaster( const QgsPoint &world, QgsPoint &raster ) const
{
  bool success = gdal_transform( world, raster, 1 );
  // flip y coordinate due to different CS orientation
  raster.setY( -raster.y() );
  return success;
}

bool QgsGeorefTransform::transform( const QgsPoint &src, QgsPoint &dst, bool rasterToWorld ) const
{
  return rasterToWorld ? transformRasterToWorld( src, dst ) : transformWorldToRaster( src, dst );
}

bool QgsGeorefTransform::getLinearOriginScale( QgsPoint &origin, double &scaleX, double &scaleY ) const
{
  if ( transformParametrisation() != Linear )
  {
    return false;
  }
  if ( !mGeorefTransformImplementation || !parametersInitialized() )
  {
    return false;
  }
  return dynamic_cast<QgsLinearGeorefTransform *>( mGeorefTransformImplementation )->getOriginScale( origin, scaleX, scaleY );
}


bool QgsGeorefTransform::gdal_transform( const QgsPoint &src, QgsPoint &dst, int dstToSrc ) const
{
  GDALTransformerFunc t = GDALTransformer();
  // Fail if no transformer function was returned
  if ( !t ) return false;

  // Copy the source coordinate for inplace transform
  double x = src.x();
  double y = src.y();
  double z = 0.0;
  int success;

  // Call GDAL transform function
  ( *t )( GDALTransformerArgs(), dstToSrc, 1,  &x, &y, &z, &success );
  if ( !success )
    return false;

  dst.setX( x );
  dst.setY( y );
  return true;
}


bool QgsLinearGeorefTransform::getOriginScale( QgsPoint &origin, double &scaleX, double &scaleY ) const
{
  origin = mParameters.origin;
  scaleX = mParameters.scaleX;
  scaleY = mParameters.scaleY;
  return true;
}

bool QgsLinearGeorefTransform::updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords )
{
  if ( mapCoords.size() < getMinimumGCPCount() )
    return false;
  QgsLeastSquares::linear( mapCoords, pixelCoords, mParameters.origin, mParameters.scaleX, mParameters.scaleY );
  return true;
}

uint QgsLinearGeorefTransform::getMinimumGCPCount() const
{
  return 2;
}

int QgsLinearGeorefTransform::linear_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  LinearParameters* t = static_cast<LinearParameters*>( pTransformerArg );
  if ( t == NULL )
  {
    return FALSE;
  }

  if ( bDstToSrc == FALSE )
  {
    for ( int i = 0; i < nPointCount; ++i )
    {
      x[i] = x[i] * t->scaleX + t->origin.x();
      y[i] = -y[i] * t->scaleY + t->origin.y();
      panSuccess[i] = TRUE;
    }
  }
  else
  {
    // Guard against division by zero
    if ( abs( t->scaleX ) < std::numeric_limits<double>::epsilon() ||
         abs( t->scaleY ) < std::numeric_limits<double>::epsilon() )
    {
      for ( int i = 0; i < nPointCount; ++i )
      {
        panSuccess[i] = FALSE;
      }
      return FALSE;
    }
    for ( int i = 0; i < nPointCount; ++i )
    {
      x[i] = ( x[i] - t->origin.x() ) / t->scaleX;
      y[i] = ( y[i] - t->origin.y() ) / ( -t->scaleY );
      panSuccess[i] = TRUE;
    }
  }

  return TRUE;
}

bool QgsHelmertGeorefTransform::updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords )
{
  if ( mapCoords.size() < getMinimumGCPCount() )
    return false;

  QgsLeastSquares::helmert( mapCoords, pixelCoords, mHelmertParameters.origin, mHelmertParameters.scale, mHelmertParameters.angle );
  return true;
}

uint QgsHelmertGeorefTransform::getMinimumGCPCount() const
{
  return 2;
}


GDALTransformerFunc QgsHelmertGeorefTransform::GDALTransformer() const
{
  return QgsHelmertGeorefTransform::helmert_transform;
}

void *QgsHelmertGeorefTransform::GDALTransformerArgs() const
{
  return ( void* )&mHelmertParameters;
}


int QgsHelmertGeorefTransform::helmert_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  HelmertParameters* t = static_cast<HelmertParameters*>( pTransformerArg );
  if ( t == NULL )
  {
    return FALSE;
  }

  double a = cos( t->angle ), b = sin( t->angle ), x0 = t->origin.x(), y0 = t->origin.y(), s = t->scale;
  if ( bDstToSrc == FALSE )
  {
    a *= s;
    b *= s;
    for ( int i = 0; i < nPointCount; ++i )
    {
      double xT = x[i], yT = y[i];
      // Because rotation parameters where estimated in a CS with negative y-axis ^= down.
      // we need to apply the rotation matrix and a change of base:
      // |cos a,-sin a| |1, 0|   | cos a,  sin a|
      // |sin a, cos a| |0,-1| = | sin a, -cos a|
      x[i] = x0 + ( a * xT + b * yT );
      y[i] = y0 + ( b * xT - a * yT );
      panSuccess[i] = TRUE;
    }
  }
  else
  {
    // Guard against division by zero
    if ( abs( s ) < std::numeric_limits<double>::epsilon() )
    {
      for ( int i = 0; i < nPointCount; ++i )
      {
        panSuccess[i] = FALSE;
      }
      return FALSE;
    }
    a /= s;
    b /= s;
    for ( int i = 0; i < nPointCount; ++i )
    {
      double xT = x[i], yT = y[i];
      xT -= x0;
      yT -= y0;
      // | cos a,  sin a |^-1   |cos a,  sin a|
      // | sin a, -cos a |    = |sin a, -cos a|
      x[i] =  a * xT + b * yT;
      y[i] =  b * xT - a * yT;
      panSuccess[i] = TRUE;
    }
  }
  return TRUE;
}

QgsGDALGeorefTransform::QgsGDALGeorefTransform( bool useTPS, unsigned int polynomialOrder ) : mPolynomialOrder( std::min( 3u, polynomialOrder ) ), mIsTPSTransform( useTPS )
{
  mGDALTransformer     = NULL;
  mGDALTransformerArgs = NULL;
}

QgsGDALGeorefTransform::~QgsGDALGeorefTransform()
{
  destroy_gdal_args();
}

bool QgsGDALGeorefTransform::updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords )
{
  assert( mapCoords.size() == pixelCoords.size() );
  if ( mapCoords.size() != pixelCoords.size() ) return false;
  int n = mapCoords.size();

  GDAL_GCP *GCPList = new GDAL_GCP[n];
  for ( int i = 0; i < n; i++ )
  {
    GCPList[i].pszId = new char[20];
    snprintf( GCPList[i].pszId, 19, "gcp%i", i );
    GCPList[i].pszInfo = NULL;
    GCPList[i].dfGCPPixel =  pixelCoords[i].x();
    GCPList[i].dfGCPLine  = -pixelCoords[i].y();
    GCPList[i].dfGCPX = mapCoords[i].x();
    GCPList[i].dfGCPY = mapCoords[i].y();
    GCPList[i].dfGCPZ = 0;
  }
  destroy_gdal_args();

  if ( mIsTPSTransform )
    mGDALTransformerArgs = GDALCreateTPSTransformer( n, GCPList, false );
  else
    mGDALTransformerArgs = GDALCreateGCPTransformer( n, GCPList, mPolynomialOrder, false );

  for ( int i = 0; i < n; i++ )
  {
    delete [] GCPList[i].pszId;
  }
  delete [] GCPList;
  return NULL != mGDALTransformerArgs;
}

uint QgsGDALGeorefTransform::getMinimumGCPCount() const
{
  if ( mIsTPSTransform )
    return 1;
  else
    return (( mPolynomialOrder + 2 )*( mPolynomialOrder + 1 ) ) / 2;
}

GDALTransformerFunc QgsGDALGeorefTransform::GDALTransformer() const
{
  // Fail if no arguments were calculated through updateParametersFromGCP
  if ( !mGDALTransformerArgs ) return NULL;

  if ( mIsTPSTransform )
    return GDALTPSTransform;
  else
    return GDALGCPTransform;
}

void *QgsGDALGeorefTransform::GDALTransformerArgs() const
{
  return mGDALTransformerArgs;
}

void QgsGDALGeorefTransform::destroy_gdal_args()
{
  if ( mGDALTransformerArgs )
  {
    if ( mIsTPSTransform )
      GDALDestroyTPSTransformer( mGDALTransformerArgs );
    else
      GDALDestroyGCPTransformer( mGDALTransformerArgs );
  }
}
