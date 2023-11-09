/***************************************************************************
                        qgsvertexid.cpp
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvertexid.h"
#include "qgsabstractgeometry.h"

bool QgsVertexId::isValid( const QgsAbstractGeometry *geom ) const
{
  return ( part >= 0 && part < geom->partCount() ) &&
         ( ring < geom->ringCount( part ) ) &&
         ( vertex < 0 || vertex < geom->vertexCount( part, ring ) );
}
