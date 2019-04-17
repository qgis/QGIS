/***************************************************************************
                         qgslayoutsize.h
                         ---------------
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

#ifndef QGSLAYOUTSIZE_H
#define QGSLAYOUTSIZE_H

#include "qgis_core.h"
#include "qgsunittypes.h"
#include <QSizeF>


/**
 * \ingroup core
 * \class QgsLayoutSize
 * \brief This class provides a method of storing sizes, consisting of a width and height,
 * for use in QGIS layouts. Measurement units are stored alongside the size.
 *
 * \see QgsLayoutMeasurementConverter
 * \note This class does not inherit from QSizeF since QSizeF includes methods which should not apply to sizes
 * with units. For instance, the + and - operators would mislead users of this class to believe that
 * addition of two QgsLayoutSize with different unit types would automatically convert units. Instead,
 * all unit conversion must be handled by a QgsLayoutMeasurementConverter so that conversion between
 * paper and screen units can be correctly performed.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutSize
{
  public:

    /**
     * Constructor for QgsLayoutSize.
     * \param width width
     * \param height height
     * \param units units for width and height
    */
    QgsLayoutSize( double width, double height, QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );

    /**
     * Constructor for QgsLayoutSize.
    */
    explicit QgsLayoutSize( QSizeF size, QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );

    /**
     * Constructor for an empty layout size
     * \param units units for measurement
    */
    explicit QgsLayoutSize( QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );

    /**
     * Sets new \a width and \a height for the size.
     * \see setWidth()
     * \see setHeight()
     * \see setUnits()
    */
    void setSize( const double width, const double height ) { mWidth = width; mHeight = height; }

    /**
     * Returns the width of the size.
     * \see setWidth()
     * \see height()
    */
    double width() const { return mWidth; }

    /**
     * Sets the \a width for the size.
     * \see width()
     * \see setHeight()
    */
    void setWidth( const double width ) { mWidth = width; }

    /**
     * Returns the height of the size.
     * \see setHeight()
     * \see width()
    */
    double height() const { return mHeight; }

    /**
     * Sets the \a height for the size.
     * \see height()
     * \see setWidth()
    */
    void setHeight( const double height ) { mHeight = height; }

    /**
     * Returns the units for the size.
     * \see setUnits()
    */
    QgsUnitTypes::LayoutUnit units() const { return mUnits; }

    /**
     * Sets the \a units for the size. Does not alter the stored width or height,
     * ie. no conversion is done.
     * \see units()
    */
    void setUnits( const QgsUnitTypes::LayoutUnit units ) { mUnits = units; }

    /**
     * Tests whether the size is empty, ie both its width and height
     * are zero.
     * \returns TRUE if size is empty
    */
    bool isEmpty() const;

    /**
     * Converts the layout size to a QSizeF. The unit information is discarded
     * during this operation.
     * \returns QSizeF with same dimensions as layout size
    */
    QSizeF toQSizeF() const;

    /**
     * Encodes the layout size to a string
     * \see decodeSize()
    */
    QString encodeSize() const;

    /**
     * Decodes a size from a \a string.
     * \see encodeSize()
    */
    static QgsLayoutSize decodeSize( const QString &string );

    bool operator==( const QgsLayoutSize &other ) const;
    bool operator!=( const QgsLayoutSize &other ) const;

    /**
     * Multiplies the width and height by a scalar value.
     */
    QgsLayoutSize operator*( double v ) const;

    /**
     * Multiplies the width and height by a scalar value.
     */
    QgsLayoutSize operator*=( double v );

    /**
     * Divides the width and height by a scalar value.
     */
    QgsLayoutSize operator/( double v ) const;

    /**
     * Divides the width and height by a scalar value.
     */
    QgsLayoutSize operator/=( double v );

  private:

    double mWidth = 0.0;
    double mHeight = 0.0;
    QgsUnitTypes::LayoutUnit mUnits = QgsUnitTypes::LayoutMillimeters;

};

#endif // QGSLAYOUTSIZE_H
