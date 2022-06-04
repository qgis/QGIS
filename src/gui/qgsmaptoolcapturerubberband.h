/***************************************************************************
    qgsmaptoolcapturerubberband.h  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2022
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCAPTURERUBBERBAND_H
#define QGSMAPTOOLCAPTURERUBBERBAND_H


#include "qgsgeometryrubberband.h"


class QgsMapToolCaptureRubberBand;


#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Class that reprensents a rubber band that can be linear or circular.
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsMapToolCaptureRubberBand: public QgsGeometryRubberBand
{
  public:
    //! Constructor
    QgsMapToolCaptureRubberBand( QgsMapCanvas *mapCanvas, QgsWkbTypes::GeometryType geomType = QgsWkbTypes::LineGeometry );

    /**
     * Returns the curve defined by the rubber band, or NULLPTR if no curve is defined.
     *
     * The caller takes onwnership of the returned object.
     */
    QgsCurve *curve() SIP_FACTORY;

    /**
     * Returns if the curve defined by the rubber band is complete :
     * has more than 2 points for circular string and more than 1 point for linear string
     */
    bool curveIsComplete() const;

    /**
     * Resets the rubber band with the specified geometry type
     * that must be line geometry or polygon geometry.
     * \a firstPolygonPoint is the first point that will be used to render the polygon rubber band (if \a geomType is PolygonGeometry)
     */
    void reset( QgsWkbTypes::GeometryType geomType = QgsWkbTypes::LineGeometry, QgsWkbTypes::Type stringType = QgsWkbTypes::LineString, const QgsPoint &firstPolygonPoint = QgsPoint() );

    //! Sets the geometry type of the rubberband without removing already existing points
    void setRubberBandGeometryType( QgsWkbTypes::GeometryType geomType );

    //! Adds point to the rubber band
    void addPoint( const QgsPoint &point, bool doUpdate = true );

    //! Moves the last point to the \a point position
    void movePoint( const QgsPoint &point );

    //! Moves the point with \a index to the \a point position
    void movePoint( int index, const QgsPoint &point );

    //! Returns the points count in the rubber band (except the first point if polygon)
    int pointsCount();

    //! Returns the type of the curve (linear string or circular string)
    QgsWkbTypes::Type stringType() const;

    //! Sets the type of the curve (linear string or circular string)
    void setStringType( const QgsWkbTypes::Type &type );

    //! Returns the last point of the rubber band
    QgsPoint lastPoint() const;

    //! Returns the point of the rubber band at position from end
    QgsPoint pointFromEnd( int posFromEnd ) const;

    //! Removes the last point of the rrubber band
    void removeLastPoint();

  private:
    QgsWkbTypes::Type mStringType = QgsWkbTypes::LineString;

    void setGeometry( QgsAbstractGeometry *geom ) override;
    void updateCurve();

    QgsCurve *createLinearString();
    QgsCurve *createCircularString();

    QgsPointSequence mPoints;
    QgsPoint mFirstPolygonPoint;
};

/// @endcond

#endif
