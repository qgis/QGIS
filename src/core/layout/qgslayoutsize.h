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
#include "qgis.h"
#include "qgsconfig.h"
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
    QgsLayoutSize( double width, double height, Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Constructor for QgsLayoutSize.
    */
    explicit QgsLayoutSize( QSizeF size, Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Constructor for an empty layout size
     * \param units units for measurement
    */
    explicit QgsLayoutSize( Qgis::LayoutUnit units = Qgis::LayoutUnit::Millimeters );

    /**
     * Sets new \a width and \a height for the size.
     * \see setWidth()
     * \see setHeight()
     * \see setUnits()
    */
    void setSize( const double width, const double height )
    {
      mWidth = width;
      mHeight = height;
#ifdef QGISDEBUG
      Q_ASSERT_X( !std::isnan( width ) && !std::isnan( height ), "QgsLayoutSize", "Layout size with NaN dimensions created" );
#endif
    }

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
    void setWidth( const double width )
    {
      mWidth = width;
#ifdef QGISDEBUG
      Q_ASSERT_X( !std::isnan( width ), "QgsLayoutSize", "Layout size with NaN dimensions created" );
#endif
    }

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
    void setHeight( const double height )
    {
      mHeight = height;
#ifdef QGISDEBUG
      Q_ASSERT_X( !std::isnan( height ), "QgsLayoutSize", "Layout size with NaN dimensions created" );
#endif
    }

    /**
     * Returns the units for the size.
     * \see setUnits()
    */
    Qgis::LayoutUnit units() const { return mUnits; }

    /**
     * Sets the \a units for the size. Does not alter the stored width or height,
     * ie. no conversion is done.
     * \see units()
    */
    void setUnits( const Qgis::LayoutUnit units ) { mUnits = units; }

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

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayoutSize: %1 x %2 %3 >" ).arg( sipCpp->width() ).arg( sipCpp->height() ).arg( QgsUnitTypes::toAbbreviatedString( sipCpp->units() ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    double mWidth = 0.0;
    double mHeight = 0.0;
    Qgis::LayoutUnit mUnits = Qgis::LayoutUnit::Millimeters;

};

#endif // QGSLAYOUTSIZE_H
