/***************************************************************************
                         qgslayoutmeasurementconverter.h
                         -------------------------------
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

#ifndef QGSLAYOUTMEASUREMENTCONVERTER_H
#define QGSLAYOUTMEASUREMENTCONVERTER_H

#include "qgis_core.h"
#include "qgsunittypes.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutsize.h"
#include "qgslayoutpoint.h"
#include <QSizeF>
#include <QPointF>


/**
 * \ingroup core
 * \class QgsLayoutMeasurementConverter
 * \brief This class provides a method of converting QgsLayoutMeasurements from
 * one unit to another. Conversion to or from pixel units utilizes a specified
 * dots per inch (DPI) property for the converter. Converters default to using
 * 300 DPI.
 * \see QgsLayoutMeasurement
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutMeasurementConverter
{
  public:

    /**
     * Constructor for QgsLayoutMeasurementConverter.
     */
    QgsLayoutMeasurementConverter() = default;

    /**
     * Sets the dots per inch (\a dpi) for the measurement converter. This is used
     * when converting measurements to and from pixels.
     * \see dpi()
    */
    void setDpi( const double dpi ) { mDpi = dpi; }

    /**
     * Returns the Dots per inch (DPI) of the measurement converter. This is used
     * when converting measurements to and from pixels.
     * \see setDpi()
    */
    double dpi() const { return mDpi; }

    /**
     * Converts a measurement from one unit to another.
     * \param measurement measurement to convert
     * \param targetUnits units to convert measurement into
     * \returns measurement converted to target units
    */
    QgsLayoutMeasurement convert( QgsLayoutMeasurement measurement, QgsUnitTypes::LayoutUnit targetUnits ) const;

    /**
     * Converts a layout size from one unit to another.
     * \param size layout size to convert
     * \param targetUnits units to convert size into
     * \returns size converted to target units
    */
    QgsLayoutSize convert( const QgsLayoutSize &size, QgsUnitTypes::LayoutUnit targetUnits ) const;

    /**
     * Converts a layout point from one unit to another.
     * \param point layout point to convert
     * \param targetUnits units to convert point into
     * \returns point converted to target units
    */
    QgsLayoutPoint convert( const QgsLayoutPoint &point, QgsUnitTypes::LayoutUnit targetUnits ) const;

  private:

    double mDpi = 300.0;

    double convertToMillimeters( QgsLayoutMeasurement measurement ) const;
    double convertToCentimeters( QgsLayoutMeasurement measurement ) const;
    double convertToMeters( QgsLayoutMeasurement measurement ) const;
    double convertToInches( QgsLayoutMeasurement measurement ) const;
    double convertToFeet( QgsLayoutMeasurement measurement ) const;
    double convertToPoints( QgsLayoutMeasurement measurement ) const;
    double convertToPicas( QgsLayoutMeasurement measurement ) const;
    double convertToPixels( QgsLayoutMeasurement measurement ) const;

};

#endif // QGSLAYOUTMEASUREMENTCONVERTER_H
