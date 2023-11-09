/***************************************************************************
  qgsnullpainterdevice.cpp
  --------------------------------------
  Date                 : December 2021
  Copyright            : (C) 2013 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnullpainterdevice.h"


QgsNullPaintDevice::QgsNullPaintDevice()
{
  mPaintEngine = std::make_unique<QgsNullPaintEngine>();
}

QPaintEngine *QgsNullPaintDevice::paintEngine() const
{
  return mPaintEngine.get();
}

int QgsNullPaintDevice::metric( PaintDeviceMetric metric ) const
{
  switch ( metric )
  {
    case QPaintDevice::PdmWidth:
      return mSize.width();
    case QPaintDevice::PdmHeight:
      return mSize.height();
    case QPaintDevice::PdmWidthMM:
      return mSize.width();
    case QPaintDevice::PdmHeightMM:
      return mSize.height();
    case QPaintDevice::PdmNumColors:
      return std::numeric_limits<int>::max();
    case QPaintDevice::PdmDepth:
      return 32;
    case QPaintDevice::PdmDpiX:
    case QPaintDevice::PdmDpiY:
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
      return mDpi;
    case QPaintDevice::PdmDevicePixelRatio:
      return 1;
    case QPaintDevice::PdmDevicePixelRatioScaled:
      return 1;
  }
  return 0;
}
