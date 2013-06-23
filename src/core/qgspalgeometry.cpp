/***************************************************************************
                        qgspalgeometry.cpp  -  description
                        ---------------------------------
   begin                : May 2009
   copyright            : (C) 2009 by Marco Hugentobler
   email                : marco dot hugentobler at karto dot baug dot ethz dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspalgeometry.h"
#include "qgsgeometry.h"
#include "qgsoverlayobject.h"
#include <geos_c.h>

QgsPALGeometry::QgsPALGeometry( QgsOverlayObject* op ): mOverlayObjectPtr( op )
{
}

QgsPALGeometry::QgsPALGeometry(): mOverlayObjectPtr( 0 )
{
}

QgsPALGeometry::~QgsPALGeometry()
{
}

const GEOSGeometry* QgsPALGeometry::getGeosGeometry()
{
  if ( mOverlayObjectPtr )
  {
    if ( mOverlayObjectPtr->geometry() )
    {
      return mOverlayObjectPtr->geometry()->asGeos();
    }
  }
  return 0;
}

