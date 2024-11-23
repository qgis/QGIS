/***************************************************************************
                         qgsdxpaintdevice.cpp
                         --------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfpaintdevice.h"
#include "qgsdxfpaintengine.h"
#include "qgspoint.h"

QgsDxfPaintDevice::QgsDxfPaintDevice( QgsDxfExport *dxf )
{
  mPaintEngine = new QgsDxfPaintEngine( this, dxf );
}

QgsDxfPaintDevice::~QgsDxfPaintDevice()
{
  delete mPaintEngine;
}

QPaintEngine *QgsDxfPaintDevice::paintEngine() const
{
  return mPaintEngine;
}

int QgsDxfPaintDevice::metric( PaintDeviceMetric metric ) const
{
  // NOLINTBEGIN(bugprone-branch-clone)
  switch ( metric )
  {
    case QPaintDevice::PdmWidth:
      return mDrawingSize.width();
    case QPaintDevice::PdmHeight:
      return mDrawingSize.height();
    case QPaintDevice::PdmWidthMM:
      return mDrawingSize.width();
    case QPaintDevice::PdmHeightMM:
      return mDrawingSize.height();
    case QPaintDevice::PdmNumColors:
      return std::numeric_limits<int>::max();
    case QPaintDevice::PdmDepth:
      return 32;
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmDpiY:
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
      return 96;
    case QPaintDevice::PdmDevicePixelRatio:
      return 1;
    case QPaintDevice::PdmDevicePixelRatioScaled:
      return 1;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
    case PdmDevicePixelRatioF_EncodedA:
      return 1;
    case PdmDevicePixelRatioF_EncodedB:
      return 1;
#endif
  }
  // NOLINTEND(bugprone-branch-clone)
  return 0;
}

double QgsDxfPaintDevice::widthScaleFactor() const
{
  if ( !mDrawingSize.isValid() || mRectangle.isEmpty() )
  {
    return 1.0;
  }

  const double widthFactor = mRectangle.width() / mDrawingSize.width();
  const double heightFactor = mRectangle.height() / mDrawingSize.height();
  return ( widthFactor + heightFactor ) / 2.0;
}

QPointF QgsDxfPaintDevice::dxfCoordinates( QPointF pt ) const
{
  if ( !mDrawingSize.isValid() || mRectangle.isEmpty() )
  {
    return QPointF( pt.x(), pt.y() );
  }

  const double x = mRectangle.left() + pt.x() * ( mRectangle.width() / mDrawingSize.width() );
  const double y = mRectangle.bottom() - pt.y() * ( mRectangle.height() / mDrawingSize.height() );
  return QPointF( x, y );
}

void QgsDxfPaintDevice::setLayer( const QString &layer )
{
  if ( mPaintEngine )
  {
    mPaintEngine->setLayer( layer );
  }
}

void QgsDxfPaintDevice::setShift( QPointF shift )
{
  if ( mPaintEngine )
  {
    mPaintEngine->setShift( shift );
  }
}


