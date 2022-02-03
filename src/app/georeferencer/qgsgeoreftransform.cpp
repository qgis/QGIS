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

#include <cmath>

#include <cassert>
#include <limits>

QgsGeorefTransform::QgsGeorefTransform( const QgsGeorefTransform &other )
{
  selectTransformParametrisation( other.mTransformParametrisation );
}

QgsGeorefTransform::QgsGeorefTransform( TransformMethod parametrisation )
{
  selectTransformParametrisation( parametrisation );
}

QgsGeorefTransform::QgsGeorefTransform() = default;

QgsGeorefTransform::~QgsGeorefTransform() = default;

QgsGeorefTransform::TransformMethod QgsGeorefTransform::transformParametrisation() const
{
  return mTransformParametrisation;
}

void QgsGeorefTransform::selectTransformParametrisation( TransformMethod parametrisation )
{
  if ( parametrisation != mTransformParametrisation )
  {
    mGeorefTransformImplementation.reset( QgsGcpTransformerInterface::create( parametrisation ) );
    mParametersInitialized = false;
    mTransformParametrisation = parametrisation;
  }
}

void QgsGeorefTransform::loadRaster( const QString &fileRaster )
{
  mRasterChangeCoords.loadRaster( fileRaster );
}

QgsPointXY QgsGeorefTransform::toSourceCoordinate( const QgsPointXY &pixel ) const
{
  return mRasterChangeCoords.toXY( pixel );
}

bool QgsGeorefTransform::providesAccurateInverseTransformation() const
{
  return ( mTransformParametrisation == TransformMethod::Linear
           || mTransformParametrisation == TransformMethod::Helmert
           || mTransformParametrisation == TransformMethod::PolynomialOrder1 );
}

bool QgsGeorefTransform::parametersInitialized() const
{
  return mParametersInitialized;
}

QgsGcpTransformerInterface *QgsGeorefTransform::clone() const
{
  std::unique_ptr< QgsGeorefTransform > res( new QgsGeorefTransform( *this ) );
  res->updateParametersFromGcps( mSourceCoordinates, mDestinationCoordinates, mInvertYAxis );
  return res.release();
}

bool QgsGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates, bool invertYAxis )
{
  mSourceCoordinates = sourceCoordinates;
  mDestinationCoordinates = destinationCoordinates;
  mInvertYAxis = invertYAxis;

  if ( !mGeorefTransformImplementation )
  {
    return false;
  }
  if ( sourceCoordinates.size() != destinationCoordinates.size() ) // Defensive sanity check
  {
    throw ( std::domain_error( "Internal error: GCP mapping is not one-to-one" ) );
  }
  if ( sourceCoordinates.size() < minimumGcpCount() )
  {
    return false;
  }
  if ( mRasterChangeCoords.hasExistingGeoreference() )
  {
    const QVector<QgsPointXY> sourcePixelCoordinates = mRasterChangeCoords.getPixelCoords( sourceCoordinates );
    mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGcps( sourcePixelCoordinates, destinationCoordinates, invertYAxis );
  }
  else
  {
    mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGcps( sourceCoordinates, destinationCoordinates, invertYAxis );
  }
  return mParametersInitialized;
}

int QgsGeorefTransform::minimumGcpCount() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->minimumGcpCount() : 0;
}

QgsGcpTransformerInterface::TransformMethod QgsGeorefTransform::method() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->method() : TransformMethod::InvalidTransform;
}

GDALTransformerFunc QgsGeorefTransform::GDALTransformer() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->GDALTransformer() : nullptr;
}

void *QgsGeorefTransform::GDALTransformerArgs() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->GDALTransformerArgs() : nullptr;
}

bool QgsGeorefTransform::transformRasterToWorld( const QgsPointXY &raster, QgsPointXY &world )
{
  // flip y coordinate due to different CS orientation
  const QgsPointXY raster_flipped( raster.x(), -raster.y() );
  return transformPrivate( raster_flipped, world, false );
}

bool QgsGeorefTransform::transformWorldToRaster( const QgsPointXY &world, QgsPointXY &raster )
{
  const bool success = transformPrivate( world, raster, true );
  // flip y coordinate due to different CS orientation
  raster.setY( -raster.y() );
  return success;
}

bool QgsGeorefTransform::transform( const QgsPointXY &src, QgsPointXY &dst, bool rasterToWorld )
{
  return rasterToWorld ? transformRasterToWorld( src, dst ) : transformWorldToRaster( src, dst );
}

bool QgsGeorefTransform::getLinearOriginScale( QgsPointXY &origin, double &scaleX, double &scaleY ) const
{
  if ( transformParametrisation() != TransformMethod::Linear )
  {
    return false;
  }
  if ( !mGeorefTransformImplementation || !parametersInitialized() )
  {
    return false;
  }
  QgsLinearGeorefTransform *transform = dynamic_cast<QgsLinearGeorefTransform *>( mGeorefTransformImplementation.get() );
  return transform && transform->getOriginScale( origin, scaleX, scaleY );
}

bool QgsGeorefTransform::getOriginScaleRotation( QgsPointXY &origin, double &scaleX, double &scaleY, double &rotation ) const
{

  if ( mTransformParametrisation == TransformMethod::Linear )
  {
    rotation = 0.0;
    QgsLinearGeorefTransform *transform = dynamic_cast<QgsLinearGeorefTransform *>( mGeorefTransformImplementation.get() );
    return transform && transform->getOriginScale( origin, scaleX, scaleY );
  }
  else if ( mTransformParametrisation == TransformMethod::Helmert )
  {
    double scale;
    QgsHelmertGeorefTransform *transform = dynamic_cast<QgsHelmertGeorefTransform *>( mGeorefTransformImplementation.get() );
    if ( !transform || ! transform->getOriginScaleRotation( origin, scale, rotation ) )
    {
      return false;
    }
    scaleX = scale;
    scaleY = scale;
    return true;
  }
  return false;
}


bool QgsGeorefTransform::transformPrivate( const QgsPointXY &src, QgsPointXY &dst, bool inverseTransform ) const
{
  // Copy the source coordinate for inplace transform
  double x = src.x();
  double y = src.y();

  if ( !QgsGcpTransformerInterface::transform( x, y, inverseTransform ) )
    return false;

  dst.setX( x );
  dst.setY( y );
  return true;
}


