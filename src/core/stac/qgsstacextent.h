/***************************************************************************
    qgsstacextent.h
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACEXTENT_H
#define QGSSTACEXTENT_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsbox3d.h"
#include "qgsrange.h"

/**
 * \ingroup core
 * \brief Class for storing a STAC SpatioTemporal extent
 *
 *  QgsStacExtent contains one overall spatial extent and one overall temporal extent
 *  It is possible to add further refined sub extents to better describe clustered data.
 *
 *  All spatial extents are in WGS84 longitude/latitude and elevation in meters
 *
 * \note Not available in python bindings
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStacExtent
{
  public:
    //! Default constructor for empty extent
    QgsStacExtent() = default;

    //! Sets the overall spatial extent to \a extent
    void setSpatialExtent( QgsBox3D extent );

    //! Sets the overall spatial extent to \a extent
    void setSpatialExtent( QgsRectangle extent );

    //! Adds a more detailed spatial \a extent
    void addDetailedSpatialExtent( QgsBox3D extent );

    //! Adds a more detailed spatial \a extent
    void addDetailedSpatialExtent( QgsRectangle extent );

    //! Returns the overall spatial extent
    QgsBox3D spatialExtent() const;

    //! Sets the overall temporal extent to \a extent
    void setTemporalExtent( QgsDateTimeRange extent );

    //! Adds a more detailed temporal \a extent
    void addDetailedTemporalExtent( QgsDateTimeRange extent );

    //! Returns the overall temporal extent
    QgsDateTimeRange temporalExtent() const;

    //! Returns all detailed spatial sub extents defined
    QVector< QgsBox3D > detailedSpatialExtents() const;

    //! Returns all detailed temporal sub extents defined
    QVector< QgsDateTimeRange > detailedTemporalExtents() const;

    //! Returns TRUE if there are detailed spatial sub extents defined
    bool hasDetailedSpatialExtents() const;

    //! Returns TRUE if there are detailed temporal sub extents defined
    bool hasDetailedTemporalExtents() const;

  private:
    QgsBox3D mSpatialExtent;
    QgsDateTimeRange mTemporalExtent;
    QVector< QgsBox3D > mDetailedSpatialExtents;
    QVector< QgsDateTimeRange > mDetailedTemporalExtents;
};

#endif // QGSSTACEXTENT_H
