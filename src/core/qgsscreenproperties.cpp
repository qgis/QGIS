/***************************************************************************
     qgsscreenproperties.cpp
     ---------------
    Date                 : June 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscreenproperties.h"
#include "qgsrendercontext.h"

#include <QScreen>

QgsScreenProperties::QgsScreenProperties() = default;

QgsScreenProperties::QgsScreenProperties( const QScreen *screen )
{
  if ( screen )
  {
    mValid = true;
    mDevicePixelRatio = screen->devicePixelRatio();
    mPhysicalDpi = screen->physicalDotsPerInch();
  }
}

void QgsScreenProperties::setDevicePixelRatio( double ratio )
{
  mValid = true;
  mDevicePixelRatio = ratio;
}

void QgsScreenProperties::setPhysicalDpi( double dpi )
{
  mValid = true;
  mPhysicalDpi = dpi;
}

void QgsScreenProperties::updateRenderContextForScreen( QgsRenderContext &context ) const
{
  if ( !mValid )
    return;

  context.setScaleFactor( mPhysicalDpi / 25.4 );
  context.setDevicePixelRatio( mDevicePixelRatio );
}
