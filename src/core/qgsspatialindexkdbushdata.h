/***************************************************************************
                             qgsspatialindexkdbushdata.h
                             -----------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALINDEXKDBUSHDATA_H
#define QGSSPATIALINDEXKDBUSHDATA_H

#include "qgsfeatureid.h"
#include "qgspointxy.h"

/**
 * \class QgsSpatialIndexKDBushData
 * \ingroup core
 *
 * A container for data stored inside a QgsSpatialIndexKDBush index.
 *
 * \since QGIS 3.4
*/
class CORE_EXPORT QgsSpatialIndexKDBushData
{
  public:

    /**
     * Constructor for QgsSpatialIndexKDBushData, for a feature with the
     * given \a id and \a x, \a y coordinate.
     */
    QgsSpatialIndexKDBushData( QgsFeatureId id, double x, double y )
      : coords( std::make_pair( x, y ) )
      , id( id )
    {}

    /**
     * Pair of coordinate data.
     * \note Not available in Python bindings.
     */
    std::pair<double, double> coords SIP_SKIP;

    /**
     * Returns the indexed point.
     */
    QgsPointXY point() const
    {
      return QgsPointXY( coords.first, coords.second );
    }

    //! Feature ID
    QgsFeatureId id;
};

#endif // QGSSPATIALINDEXKDBUSHDATA_H
