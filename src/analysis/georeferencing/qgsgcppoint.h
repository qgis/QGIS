/***************************************************************************
     qgsgcppoint.h
     --------------------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGCPPOINT_H
#define QGSGCPPOINT_H

#include "qgis_analysis.h"
#include "qgscoordinatereferencesystem.h"

class QgsCoordinateTransformContext;

/**
 * \ingroup analysis
 * \brief Contains properties of a ground control point (GCP).
 *
 * \since QGIS 3.26
*/
class ANALYSIS_EXPORT QgsGcpPoint
{
  public:

    //! Coordinate point types
    enum class PointType
    {
      Source, //!< Source point
      Destination, //!< Destination point
    };

    /**
     * Constructor for QgsGcpPoint.
     *
     * \param sourcePoint source coordinates. This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     * \param destinationPoint destination coordinates
     * \param destinationPointCrs CRS of destination point
     * \param enabled whether the point is currently enabled
     */
    QgsGcpPoint( const QgsPointXY &sourcePoint, const QgsPointXY &destinationPoint,
                 const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled = true );

    /**
     * Returns the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see setSourcePoint()
     */
    QgsPointXY sourcePoint() const { return mSourcePoint; }

    /**
     * Sets the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see sourcePoint()
     */
    void setSourcePoint( QgsPointXY point ) { mSourcePoint = point; }

    /**
     * Returns the destination coordinates.
     *
     * \see setDestinationPoint()
     */
    QgsPointXY destinationPoint() const { return mDestinationPoint; }

    /**
     * Sets the destination coordinates.
     *
     * \see destinationPoint()
     */
    void setDestinationPoint( QgsPointXY point ) { mDestinationPoint = point; }

    /**
     * Returns the CRS of the destination point.
     *
     * \see setDestinationPointCrs()
     */
    QgsCoordinateReferenceSystem destinationPointCrs() const;

    /**
     * Sets the \a crs of the destination point.
     *
     * \see destinationPointCrs()
     */
    void setDestinationPointCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the destionationPoint() transformed to the given target CRS.
     */
    QgsPointXY transformedDestinationPoint( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const;

    /**
     * Returns TRUE if the point is currently enabled.
     *
     * \see setEnabled()
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets whether the point is currently enabled.
     *
     * \see isEnabled()
     */
    void setEnabled( bool enabled ) { mEnabled = enabled; }

    // TODO c++20 - replace with = default
    bool operator==( const QgsGcpPoint &other ) const
    {
      return mEnabled == other.mEnabled
             && mSourcePoint == other.mSourcePoint
             && mDestinationPoint == other.mDestinationPoint
             && mDestinationCrs == other.mDestinationCrs;
    }

    bool operator!=( const QgsGcpPoint &other ) const
    {
      return !( *this == other );
    }

  private:

    QgsPointXY mSourcePoint;
    QgsPointXY mDestinationPoint;
    QgsCoordinateReferenceSystem mDestinationCrs;
    bool mEnabled = true;

};

#endif //QGSGCPPOINT_H
