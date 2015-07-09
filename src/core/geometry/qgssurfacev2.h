/***************************************************************************
                         qgssurfacev2.h
                         --------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
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

#ifndef QGSSURFACEV2_H
#define QGSSURFACEV2_H

#include "qgsabstractgeometryv2.h"
#include "qgspointv2.h"

class QgsPolygonV2;

class CORE_EXPORT QgsSurfaceV2: public QgsAbstractGeometryV2
{
  public:
    virtual QgsPointV2 centroid() const = 0;
    virtual QgsPointV2 pointOnSurface() const = 0;
    virtual QgsPolygonV2* surfaceToPolygon() const = 0;
};

#endif // QGSSURFACEV2_H
