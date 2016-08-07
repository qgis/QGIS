/***************************************************************************
                         qgssurface.h
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

#include "qgsabstractgeometry.h"
#include "qgsrectangle.h"

class QgsPolygonV2;

/** \ingroup core
 * \class QgsSurface
 */
class CORE_EXPORT QgsSurface: public QgsAbstractGeometry
{
  public:

    virtual QgsPolygonV2* surfaceToPolygon() const = 0;

    /** Returns the minimal bounding box for the geometry
     */
    virtual QgsRectangle boundingBox() const override
    {
      if ( mBoundingBox.isNull() )
      {
        mBoundingBox = calculateBoundingBox();
      }
      return mBoundingBox;
    }

  protected:

    virtual void clearCache() const override { mBoundingBox = QgsRectangle(); mCoordinateSequence.clear(); QgsAbstractGeometry::clearCache(); }

    mutable QgsCoordinateSequence mCoordinateSequence;
    mutable QgsRectangle mBoundingBox;
};

#endif // QGSSURFACEV2_H
