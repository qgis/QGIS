/***************************************************************************
                         qgslayoutmeasurement.h
                         --------------------
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

#ifndef QGSLAYOUTMEASUREMENT_H
#define QGSLAYOUTMEASUREMENT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsunittypes.h"

/**
 * \ingroup core
 * \class QgsLayoutMeasurement
 * \brief Provides a method of storing measurements for use in QGIS layouts
 * using a variety of different measurement units.
 * \see QgsLayoutMeasurementConverter
 */
class CORE_EXPORT QgsLayoutMeasurement
{
  public:

    /**
     * Constructor for QgsLayoutMeasurement.
     * \param length measurement length
     * \param units measurement units
    */
    explicit QgsLayoutMeasurement( double length, Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Returns the length of the measurement.
     * \see setLength()
    */
    double length() const { return mLength; }

    /**
     * Sets the \a length of the measurement.
     * \see length()
    */
    void setLength( const double length ) { mLength = length; }

    /**
     * Returns the units for the measurement.
     * \see setUnits()
    */
    Qgis::LayoutUnit units() const { return mUnits; }

    /**
     * Sets the \a units for the measurement. Does not alter the stored length,
     * ie. no length conversion is done.
     * \see units()
    */
    void setUnits( const Qgis::LayoutUnit units ) { mUnits = units; }

    /**
     * Encodes the layout measurement to a string
     * \see decodeMeasurement()
    */
    QString encodeMeasurement() const;

    /**
     * Decodes a measurement from a \a string.
     * \see encodeMeasurement()
    */
    static QgsLayoutMeasurement decodeMeasurement( const QString &string );

    bool operator==( QgsLayoutMeasurement other ) const;
    bool operator!=( QgsLayoutMeasurement other ) const;

    /**
     * Adds a scalar value to the measurement.
     */
    QgsLayoutMeasurement operator+( double v ) const;

    /**
     * Adds a scalar value to the measurement.
     */
    QgsLayoutMeasurement operator+=( double v );

    /**
     * Subtracts a scalar value from the measurement.
     */
    QgsLayoutMeasurement operator-( double v ) const;

    /**
     * Subtracts a scalar value from the measurement.
     */
    QgsLayoutMeasurement operator-=( double v );

    /**
     * Multiplies the measurement by a scalar value.
     */
    QgsLayoutMeasurement operator*( double v ) const;

    /**
     * Multiplies the measurement by a scalar value.
     */
    QgsLayoutMeasurement operator*=( double v );

    /**
     * Divides the measurement by a scalar value.
     */
    QgsLayoutMeasurement operator/( double v ) const;

    /**
     * Divides the measurement by a scalar value.
     */
    QgsLayoutMeasurement operator/=( double v );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayoutMeasurement: %1 %2 >" ).arg( sipCpp->length() ).arg( QgsUnitTypes::toAbbreviatedString( sipCpp->units() ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    double mLength = 0.0;
    Qgis::LayoutUnit mUnits = Qgis::LayoutUnit::Millimeters;

};

#endif // QGSLAYOUTMEASUREMENT_H
