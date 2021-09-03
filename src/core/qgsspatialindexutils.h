/***************************************************************************
  qgsgenericspatialindex.h
  ------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALINDEXUTILS_H
#define QGSSPATIALINDEXUTILS_H

#include "qgis_core.h"
#define SIP_NO_FILE

class QgsRectangle;

///@cond PRIVATE
// forward declaration
namespace SpatialIndex
{
  class IStorageManager;
  class ISpatialIndex;
  class Region;
  class Point;

  namespace StorageManager
  {
    class IBuffer;
  }
}
///@endcond

/**
 * \ingroup core
 * \class QgsSpatialIndexUtils
 *
 * \brief Contains utility functions for working with spatial indexes.
 *
 * \note Not available in Python bindings.
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsSpatialIndexUtils
{
  public:

    /**
     * Converts a QGIS \a rectangle to a SpatialIndex region.
     */
    static SpatialIndex::Region rectangleToRegion( const QgsRectangle &rectangle );

};

#endif // QGSSPATIALINDEXUTILS_H
