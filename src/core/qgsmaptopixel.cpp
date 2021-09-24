/***************************************************************************
                qgsmaptopixel.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaptopixel.h"

#include <QPoint>
#include <QTextStream>
#include <QVector>
#include <QTransform>

#include "qgslogger.h"
#include "qgspointxy.h"


QgsMapToPixel::QgsMapToPixel( double mapUnitsPerPixel,
                              double xc,
                              double yc,
                              int width,
                              int height,
                              double rotation )
  : mValid( true )
  , mMapUnitsPerPixel( mapUnitsPerPixel )
  , mWidth( width )
  , mHeight( height )
  , mRotation( rotation )
  , mXCenter( xc )
  , mYCenter( yc )
  , mXMin( xc - ( mWidth * mMapUnitsPerPixel / 2.0 ) )
  , mYMin( yc - ( mHeight * mMapUnitsPerPixel / 2.0 ) )
{
  Q_ASSERT( mapUnitsPerPixel > 0 );
  updateMatrix();
}

QgsMapToPixel::QgsMapToPixel( double mapUnitsPerPixel )
  : mValid( true )
  , mMapUnitsPerPixel( mapUnitsPerPixel )
  , mWidth( 0 )
  , mHeight( 0 )
  , mXCenter( 0 )
  , mYCenter( 0 )
{
  updateMatrix();
}

QgsMapToPixel QgsMapToPixel::fromScale( double scale, QgsUnitTypes::DistanceUnit mapUnits, double dpi )
{
  const double metersPerPixel = 25.4 / dpi / 1000.0;
  const double mapUnitsPerPixel = metersPerPixel * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::DistanceMeters, mapUnits );
  return QgsMapToPixel( mapUnitsPerPixel * scale );
}

QgsMapToPixel::QgsMapToPixel()
{
  updateMatrix();
}

bool QgsMapToPixel::updateMatrix()
{
  const QTransform newMatrix = transform();

  // https://github.com/qgis/QGIS/issues/20856
  if ( !newMatrix.isInvertible() )
    return false;

  mMatrix = newMatrix;
  return true;
}

void QgsMapToPixel::setMapUnitsPerPixel( double mapUnitsPerPixel )
{
  mValid = true;

  const double oldUnits = mMapUnitsPerPixel;
  mMapUnitsPerPixel = mapUnitsPerPixel;
  if ( !updateMatrix() )
  {
    mMapUnitsPerPixel = oldUnits;
  }
}

void QgsMapToPixel::setMapRotation( double degrees, double cx, double cy )
{
  mValid = true;

  const double oldRotation = mRotation;
  const double oldXCenter = mXCenter;
  const double oldYCenter = mYCenter;
  const double oldWidth = mWidth;

  mRotation = degrees;
  mXCenter = cx;
  mYCenter = cy;
  if ( mWidth < 0 )
  {
    // set width not that we can compute it
    mWidth = ( ( mXCenter - mXMin ) * 2 ) / mMapUnitsPerPixel;
  }

  if ( !updateMatrix() )
  {
    mRotation = oldRotation;
    mXCenter = oldXCenter;
    mYCenter = oldYCenter;
    mWidth = oldWidth;
  }
}

void QgsMapToPixel::setParameters( double mapUnitsPerPixel,
                                   double xc,
                                   double yc,
                                   int width,
                                   int height,
                                   double rotation,
                                   bool *ok )
{
  mValid = true;

  const double oldMUPP = mMapUnitsPerPixel;
  const double oldXCenter = mXCenter;
  const double oldYCenter = mYCenter;
  const double oldWidth = mWidth;
  const double oldHeight = mHeight;
  const double oldRotation = mRotation;
  const double oldXMin = mXMin;
  const double oldYMin = mYMin;

  mMapUnitsPerPixel = mapUnitsPerPixel;
  mXCenter = xc;
  mYCenter = yc;
  mWidth = width;
  mHeight = height;
  mRotation = rotation;
  mXMin = xc - ( mWidth * mMapUnitsPerPixel / 2.0 );
  mYMin = yc - ( mHeight * mMapUnitsPerPixel / 2.0 );

  if ( !updateMatrix() )
  {
    mMapUnitsPerPixel = oldMUPP;
    mXCenter = oldXCenter;
    mYCenter = oldYCenter;
    mWidth = oldWidth;
    mHeight = oldHeight;
    mRotation = oldRotation;
    mXMin = oldXMin;
    mYMin = oldYMin;
    *ok = false;
  }
  else
  {
    *ok = true;
  }
}

void QgsMapToPixel::setParameters( double mapUnitsPerPixel,
                                   double xc,
                                   double yc,
                                   int width,
                                   int height,
                                   double rotation )
{
  mValid = true;
  bool ok;
  setParameters( mapUnitsPerPixel, xc, yc, width, height, rotation, &ok );
}

QString QgsMapToPixel::showParameters() const
{
  QString rep;
  QTextStream( &rep ) << "Map units/pixel: " << mMapUnitsPerPixel
                      << " center: " << mXCenter << ',' << mYCenter
                      << " rotation: " << mRotation
                      << " size: " << mWidth << 'x' << mHeight;
  return rep;
}

QTransform QgsMapToPixel::transform() const
{
  // NOTE: operations are done in the reverse order in which
  //       they are configured, so translation to geographical
  //       center happens first, then scaling, then rotation
  //       and finally translation to output viewport center

  const double rotation = mapRotation();
  if ( qgsDoubleNear( rotation, 0.0 ) )
  {
    //no rotation, return a simplified matrix
    return QTransform::fromScale( 1.0 / mMapUnitsPerPixel, -1.0 / mMapUnitsPerPixel )
           .translate( -mXMin, - ( mYMin + mHeight * mMapUnitsPerPixel ) );
  }
  else
  {
    const double cy = mapHeight() / 2.0;
    const double cx = mapWidth() / 2.0;
    return QTransform::fromTranslate( cx, cy )
           .rotate( rotation )
           .scale( 1 / mMapUnitsPerPixel, -1 / mMapUnitsPerPixel )
           .translate( -mXCenter, -mYCenter );
  }
}

