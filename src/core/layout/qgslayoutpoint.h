/***************************************************************************
                         qgslayoutpoint.h
                         ----------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTPOINT_H
#define QGSLAYOUTPOINT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsconfig.h"
#include <QPointF>

/**
 * \ingroup core
 * \class QgsLayoutPoint
 * \brief This class provides a method of storing points, consisting of an x and y coordinate,
 * for use in QGIS layouts. Measurement units are stored alongside the position.
 *
 * \see QgsLayoutMeasurementConverter
 * \note This class does not inherit from QPointF since QPointF includes methods which should not apply
 * to positions with units. For instance, the + and - operators would mislead users of this class
 * to believe that addition of two QgsLayoutPoints with different unit types would automatically convert
 * units. Instead, all unit conversion must be handled by a QgsLayoutMeasurementConverter so that
 * conversion between paper and screen units can be correctly performed.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutPoint
{
  public:

    /**
     * Constructor for QgsLayoutPoint.
    */
    QgsLayoutPoint( double x, double y, Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Constructor for QgsLayoutPoint.
    */
    explicit QgsLayoutPoint( QPointF point, Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Constructor for an empty point, where both x and y are set to 0.
     * \param units units for measurement
    */
    explicit QgsLayoutPoint( Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Sets new x and y coordinates for the point.
     * \see setX()
     * \see setY()
     * \see setUnits()
    */
    void setPoint( const double x, const double y ) { mX = x; mY = y; }

    /**
     * Returns x coordinate of point.
     * \see setX()
     * \see y()
    */
    double x() const { return mX; }

    /**
     * Sets the x coordinate of point.
     * \see x()
     * \see setY()
    */
    void setX( const double x )
    {
#ifdef QGISDEBUG
      Q_ASSERT_X( !std::isnan( x ), "QgsLayoutPoint", "Layout point with NaN coordinates created" );
#endif
      mX = x;
    }

    /**
     * Returns y coordinate of point.
     * \see setY()
     * \see x()
    */
    double y() const { return mY; }

    /**
     * Sets y coordinate of point.
     * \see y()
     * \see setX()
    */
    void setY( const double y )
    {
#ifdef QGISDEBUG
      Q_ASSERT_X( !std::isnan( y ), "QgsLayoutPoint", "Layout point with NaN coordinates created" );
#endif
      mY = y;
    }

    /**
     * Returns the units for the point.
     * \see setUnits()
    */
    Qgis::LayoutUnit units() const { return mUnits; }

    /**
     * Sets the \a units for the point. Does not alter the stored coordinates,
     * ie. no conversion is done.
     * \see units()
    */
    void setUnits( const Qgis::LayoutUnit units ) { mUnits = units; }

    /**
     * Tests whether the position is null, ie both its x and y coordinates
     * are zero.
     * \returns TRUE if point is null
    */
    bool isNull() const;

    /**
     * Converts the layout point to a QPointF. The unit information is discarded
     * during this operation.
     * \returns QPointF with same x and y coordinates as layout point
    */
    QPointF toQPointF() const;

    /**
     * Encodes the layout point to a string
     * \see decodePoint()
    */
    QString encodePoint() const;

    /**
     * Decodes a point from a \a string.
     * \see encodePoint()
    */
    static QgsLayoutPoint decodePoint( const QString &string );

    bool operator==( const QgsLayoutPoint &other ) const;
    bool operator!=( const QgsLayoutPoint &other ) const;

    /**
     * Multiplies the x and y by a scalar value.
     */
    QgsLayoutPoint operator*( double v ) const;

    /**
     * Multiplies the x and y by a scalar value.
     */
    QgsLayoutPoint operator*=( double v );

    /**
     * Divides the x and y by a scalar value.
     */
    QgsLayoutPoint operator/( double v ) const;

    /**
     * Divides the x and y by a scalar value.
     */
    QgsLayoutPoint operator/=( double v );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayoutPoint: %1, %2 %3 >" ).arg( sipCpp->x() ).arg( sipCpp->y() ).arg( QgsUnitTypes::toAbbreviatedString( sipCpp->units() ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    double mX = 0.0;
    double mY = 0.0;
    Qgis::LayoutUnit mUnits = Qgis::LayoutUnit::Millimeters;

};

#endif // QGSLAYOUTPOINT_H
