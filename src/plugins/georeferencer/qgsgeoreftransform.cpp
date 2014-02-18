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

#include "qgsgeoreftransform.h"

#include <gdal.h>
#include <gdal_alg.h>

#include "qgsleastsquares.h"

#include <cmath>
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
    QgsLinearGeorefTransform() {}
    ~QgsLinearGeorefTransform() {}

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
    QgsHelmertGeorefTransform() {}
    struct HelmertParameters
    {
      QgsPoint origin;
      double   scale;
      double   angle;
    };

    bool getOriginScaleRotation( QgsPoint &origin, double& scale, double& rotation ) const;
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

/**
 * A planar projective transform, expressed by a homography.
 *
 * Implements model fitting which minimizes algebraic error using total least squares.
 */
class QgsProjectiveGeorefTransform : public QgsGeorefTransformInterface
{
  public:
    QgsProjectiveGeorefTransform() {}
    ~QgsProjectiveGeorefTransform() {}

    bool updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords );
    uint getMinimumGCPCount() const;

    GDALTransformerFunc  GDALTransformer()     const { return QgsProjectiveGeorefTransform::projective_transform; }
    void                *GDALTransformerArgs() const { return ( void * )&mParameters; }
  private:
    struct ProjectiveParameters
    {
      double H[9];        // Homography
      double Hinv[9];     // Inverted homography
      bool   hasInverse;  // Could the inverted homography be calculated?
    } mParameters;

    static int projective_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
                                     double *x, double *y, double *z, int *panSuccess );
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

void QgsGeorefTransform::setRasterChangeCoords( const QString &fileRaster )
{
  mRasterChangeCoords.setRaster( fileRaster );
}

bool QgsGeorefTransform::providesAccurateInverseTransformation() const
{
  return ( mTransformParametrisation == Linear
           || mTransformParametrisation == Helmert
           || mTransformParametrisation == PolynomialOrder1 );
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
  if ( mRasterChangeCoords.hasCrs() )
  {
    std::vector<QgsPoint> pixelCoordsCorrect = mRasterChangeCoords.getPixelCoords( pixelCoords );
    mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGCPs( mapCoords, pixelCoordsCorrect );
    pixelCoordsCorrect.clear();
  }
  else
  {
    mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGCPs( mapCoords, pixelCoords );
  }
  return mParametersInitialized;
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
    case Projective:       return new QgsProjectiveGeorefTransform;
    default:               return NULL;
  }
}

bool QgsGeorefTransform::transformRasterToWorld( const QgsPoint &raster, QgsPoint &world )
{
  // flip y coordinate due to different CS orientation
  QgsPoint raster_flipped( raster.x(), -raster.y() );
  return gdal_transform( raster_flipped, world, 0 );
}

bool QgsGeorefTransform::transformWorldToRaster( const QgsPoint &world, QgsPoint &raster )
{
  bool success = gdal_transform( world, raster, 1 );
  // flip y coordinate due to different CS orientation
  raster.setY( -raster.y() );
  return success;
}

bool QgsGeorefTransform::transform( const QgsPoint &src, QgsPoint &dst, bool rasterToWorld )
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

bool QgsGeorefTransform::getOriginScaleRotation( QgsPoint &origin, double &scaleX, double &scaleY, double& rotation ) const
{

  if ( mTransformParametrisation == Linear )
  {
    rotation = 0.0;
    return dynamic_cast<QgsLinearGeorefTransform *>( mGeorefTransformImplementation )->getOriginScale( origin, scaleX, scaleY );
  }
  else if ( mTransformParametrisation == Helmert )
  {
    double scale;
    if ( ! dynamic_cast<QgsHelmertGeorefTransform*>( mGeorefTransformImplementation )->getOriginScaleRotation( origin, scale, rotation ) )
    {
      return false;
    }
    scaleX = scale;
    scaleY = scale;
    return true;
  }
  return false;
}


bool QgsGeorefTransform::gdal_transform( const QgsPoint &src, QgsPoint &dst, int dstToSrc ) const
{
  GDALTransformerFunc t = GDALTransformer();
  // Fail if no transformer function was returned
  if ( !t )
    return false;

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
  Q_UNUSED( z );
  LinearParameters* t = static_cast<LinearParameters*>( pTransformerArg );
  if ( t == NULL )
  {
    return false;
  }

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
    if ( qAbs( t->scaleX ) < std::numeric_limits<double>::epsilon() ||
         qAbs( t->scaleY ) < std::numeric_limits<double>::epsilon() )
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

bool QgsHelmertGeorefTransform::getOriginScaleRotation( QgsPoint &origin, double& scale, double& rotation ) const
{
  origin = mHelmertParameters.origin;
  scale = mHelmertParameters.scale;
  rotation = mHelmertParameters.angle;
  return true;
}

int QgsHelmertGeorefTransform::helmert_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  Q_UNUSED( z );
  HelmertParameters* t = static_cast<HelmertParameters*>( pTransformerArg );
  if ( t == NULL )
  {
    return false;
  }

  double a = cos( t->angle ), b = sin( t->angle ), x0 = t->origin.x(), y0 = t->origin.y(), s = t->scale;
  if ( !bDstToSrc )
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
      panSuccess[i] = true;
    }
  }
  else
  {
    // Guard against division by zero
    if ( qAbs( s ) < std::numeric_limits<double>::epsilon() )
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
      // | cos a,  sin a |^-1   |cos a,  sin a|
      // | sin a, -cos a |    = |sin a, -cos a|
      x[i] =  a * xT + b * yT;
      y[i] =  b * xT - a * yT;
      panSuccess[i] = true;
    }
  }
  return true;
}

QgsGDALGeorefTransform::QgsGDALGeorefTransform( bool useTPS, unsigned int polynomialOrder ) : mPolynomialOrder( qMin( 3u, polynomialOrder ) ), mIsTPSTransform( useTPS )
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
  if ( mapCoords.size() != pixelCoords.size() )
    return false;
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
  if ( !mGDALTransformerArgs )
    return NULL;

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

bool QgsProjectiveGeorefTransform::updateParametersFromGCPs( const std::vector<QgsPoint> &mapCoords, const std::vector<QgsPoint> &pixelCoords )
{
  if ( mapCoords.size() < getMinimumGCPCount() )
    return false;


  // HACK: flip y coordinates, because georeferencer and gdal use different conventions
  std::vector<QgsPoint> flippedPixelCoords( pixelCoords.size() );
  for ( uint i = 0; i < pixelCoords.size(); i++ )
  {
    flippedPixelCoords[i] = QgsPoint( pixelCoords[i].x(), -pixelCoords[i].y() );
  }

  QgsLeastSquares::projective( mapCoords, flippedPixelCoords, mParameters.H );

  // Invert the homography matrix using adjoint matrix
  double *H = mParameters.H;

  double adjoint[9];
  adjoint[0] =  H[4] * H[8] - H[5] * H[7];
  adjoint[1] = -H[1] * H[8] + H[2] * H[7];
  adjoint[2] =  H[1] * H[5] - H[2] * H[4];

  adjoint[3] = -H[3] * H[8] + H[5] * H[6];
  adjoint[4] =  H[0] * H[8] - H[2] * H[6];
  adjoint[5] = -H[0] * H[5] + H[2] * H[3];

  adjoint[6] =  H[3] * H[7] - H[4] * H[6];
  adjoint[7] = -H[0] * H[7] + H[1] * H[6];
  adjoint[8] =  H[0] * H[4] - H[1] * H[3];

  double det = H[0] * adjoint[0] + H[3] * adjoint[1] + H[6] * adjoint[2];

  if ( qAbs( det ) < 1024.0*std::numeric_limits<double>::epsilon() )
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

uint QgsProjectiveGeorefTransform::getMinimumGCPCount() const
{
  return 4;
}

int QgsProjectiveGeorefTransform::projective_transform( void *pTransformerArg, int bDstToSrc, int nPointCount,
    double *x, double *y, double *z, int *panSuccess )
{
  Q_UNUSED( z );
  ProjectiveParameters* t = static_cast<ProjectiveParameters*>( pTransformerArg );
  if ( t == NULL )
  {
    return false;
  }

  double *H;
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
    if ( qAbs( Z ) < 1024.0*std::numeric_limits<double>::epsilon() )
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
