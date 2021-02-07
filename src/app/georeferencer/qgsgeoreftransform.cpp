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
  mTransformParametrisation = InvalidTransform;
  mGeorefTransformImplementation = nullptr;
  selectTransformParametrisation( other.mTransformParametrisation );
}

QgsGeorefTransform::QgsGeorefTransform( TransformParametrisation parametrisation )
{
  mTransformParametrisation = InvalidTransform;
  mGeorefTransformImplementation = nullptr;
  selectTransformParametrisation( parametrisation );
}

QgsGeorefTransform::QgsGeorefTransform()
{
  mTransformParametrisation = InvalidTransform;
  mGeorefTransformImplementation = nullptr;
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

bool QgsGeorefTransform::updateParametersFromGcps( const QVector<QgsPointXY> &mapCoords, const QVector<QgsPointXY> &pixelCoords )
{
  if ( !mGeorefTransformImplementation )
  {
    return false;
  }
  if ( mapCoords.size() != pixelCoords.size() ) // Defensive sanity check
  {
    throw ( std::domain_error( "Internal error: GCP mapping is not one-to-one" ) );
  }
  if ( mapCoords.size() < minimumGcpCount() )
  {
    return false;
  }
  if ( mRasterChangeCoords.hasCrs() )
  {
    QVector<QgsPointXY> pixelCoordsCorrect = mRasterChangeCoords.getPixelCoords( pixelCoords );
    mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGcps( mapCoords, pixelCoordsCorrect );
    pixelCoordsCorrect.clear();
  }
  else
  {
    mParametersInitialized = mGeorefTransformImplementation->updateParametersFromGcps( mapCoords, pixelCoords );
  }
  return mParametersInitialized;
}

int QgsGeorefTransform::minimumGcpCount() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->minimumGcpCount() : 0;
}

GDALTransformerFunc QgsGeorefTransform::GDALTransformer() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->GDALTransformer() : nullptr;
}

void *QgsGeorefTransform::GDALTransformerArgs() const
{
  return mGeorefTransformImplementation ? mGeorefTransformImplementation->GDALTransformerArgs() : nullptr;
}

QgsGcpTransformerInterface *QgsGeorefTransform::createImplementation( TransformParametrisation parametrisation )
{
  switch ( parametrisation )
  {
    case Linear:
      return new QgsLinearGeorefTransform;
    case Helmert:
      return new QgsHelmertGeorefTransform;
    case PolynomialOrder1:
      return new QgsGDALGeorefTransform( false, 1 );
    case PolynomialOrder2:
      return new QgsGDALGeorefTransform( false, 2 );
    case PolynomialOrder3:
      return new QgsGDALGeorefTransform( false, 3 );
    case ThinPlateSpline:
      return new QgsGDALGeorefTransform( true, 0 );
    case Projective:
      return new QgsProjectiveGeorefTransform;
    default:
      return nullptr;
  }
}

bool QgsGeorefTransform::transformRasterToWorld( const QgsPointXY &raster, QgsPointXY &world )
{
  // flip y coordinate due to different CS orientation
  QgsPointXY raster_flipped( raster.x(), -raster.y() );
  return gdal_transform( raster_flipped, world, 0 );
}

bool QgsGeorefTransform::transformWorldToRaster( const QgsPointXY &world, QgsPointXY &raster )
{
  bool success = gdal_transform( world, raster, 1 );
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
  if ( transformParametrisation() != Linear )
  {
    return false;
  }
  if ( !mGeorefTransformImplementation || !parametersInitialized() )
  {
    return false;
  }
  QgsLinearGeorefTransform *transform = dynamic_cast<QgsLinearGeorefTransform *>( mGeorefTransformImplementation );
  return transform && transform->getOriginScale( origin, scaleX, scaleY );
}

bool QgsGeorefTransform::getOriginScaleRotation( QgsPointXY &origin, double &scaleX, double &scaleY, double &rotation ) const
{

  if ( mTransformParametrisation == Linear )
  {
    rotation = 0.0;
    QgsLinearGeorefTransform *transform = dynamic_cast<QgsLinearGeorefTransform *>( mGeorefTransformImplementation );
    return transform && transform->getOriginScale( origin, scaleX, scaleY );
  }
  else if ( mTransformParametrisation == Helmert )
  {
    double scale;
    QgsHelmertGeorefTransform *transform = dynamic_cast<QgsHelmertGeorefTransform *>( mGeorefTransformImplementation );
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


bool QgsGeorefTransform::gdal_transform( const QgsPointXY &src, QgsPointXY &dst, int dstToSrc ) const
{
  GDALTransformerFunc t = GDALTransformer();
  // Fail if no transformer function was returned
  if ( !t )
    return false;

  // Copy the source coordinate for inplace transform
  double x = src.x();
  double y = src.y();
  double z = 0.0;
  int success = 0;

  // Call GDAL transform function
  ( *t )( GDALTransformerArgs(), dstToSrc, 1,  &x, &y, &z, &success );
  if ( !success )
    return false;

  dst.setX( x );
  dst.setY( y );
  return true;
}


