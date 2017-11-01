/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsize.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutSize
{
%Docstring
 This class provides a method of storing sizes, consisting of a width and height,
 for use in QGIS layouts. Measurement units are stored alongside the size.

.. seealso:: QgsLayoutMeasurementConverter
.. versionadded:: 3.0
.. note::

   This class does not inherit from QSizeF since QSizeF includes methods which should not apply to sizes
 with units. For instance, the + and - operators would mislead users of this class to believe that
 addition of two QgsLayoutSize with different unit types would automatically convert units. Instead,
 all unit conversion must be handled by a QgsLayoutMeasurementConverter so that conversion between
 paper and screen units can be correctly performed.
%End

%TypeHeaderCode
#include "qgslayoutsize.h"
%End
  public:

    QgsLayoutSize( const double width, const double height, const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
 Constructor for QgsLayoutSize.
 \param width width
 \param height height
 \param units units for width and height
%End

    explicit QgsLayoutSize( const QSizeF size, const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
 Constructor for QgsLayoutSize.
%End

    explicit QgsLayoutSize( const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
 Constructor for an empty layout size
 \param units units for measurement
%End

    void setSize( const double width, const double height );
%Docstring
 Sets new ``width`` and ``height`` for the size.
.. seealso:: setWidth()
.. seealso:: setHeight()
.. seealso:: setUnits()
%End

    double width() const;
%Docstring
 Returns the width of the size.
.. seealso:: setWidth()
.. seealso:: height()
 :rtype: float
%End

    void setWidth( const double width );
%Docstring
 Sets the ``width`` for the size.
.. seealso:: width()
.. seealso:: setHeight()
%End

    double height() const;
%Docstring
 Returns the height of the size.
.. seealso:: setHeight()
.. seealso:: width()
 :rtype: float
%End

    void setHeight( const double height );
%Docstring
 Sets the ``height`` for the size.
.. seealso:: height()
.. seealso:: setWidth()
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
 Returns the units for the size.
.. seealso:: setUnits()
 :rtype: QgsUnitTypes.LayoutUnit
%End

    void setUnits( const QgsUnitTypes::LayoutUnit units );
%Docstring
 Sets the ``units`` for the size. Does not alter the stored width or height,
 ie. no conversion is done.
.. seealso:: units()
%End

    bool isEmpty() const;
%Docstring
 Tests whether the size is empty, ie both its width and height
 are zero.
 :return: true if size is empty
 :rtype: bool
%End

    QSizeF toQSizeF() const;
%Docstring
 Converts the layout size to a QSizeF. The unit information is discarded
 during this operation.
 :return: QSizeF with same dimensions as layout size
 :rtype: QSizeF
%End

    QString encodeSize() const;
%Docstring
 Encodes the layout size to a string
.. seealso:: decodeSize()
 :rtype: str
%End

    static QgsLayoutSize decodeSize( const QString &string );
%Docstring
 Decodes a size from a ``string``.
.. seealso:: encodeSize()
 :rtype: QgsLayoutSize
%End

    bool operator==( const QgsLayoutSize &other ) const;
    bool operator!=( const QgsLayoutSize &other ) const;
%Docstring
 :rtype: bool
%End

    QgsLayoutSize operator*( const double v ) const;
%Docstring
 Multiplies the width and height by a scalar value.
 :rtype: QgsLayoutSize
%End

    QgsLayoutSize operator*=( const double v );
%Docstring
 Multiplies the width and height by a scalar value.
 :rtype: QgsLayoutSize
%End

    QgsLayoutSize operator/( const double v ) const;
%Docstring
 Divides the width and height by a scalar value.
 :rtype: QgsLayoutSize
%End

    QgsLayoutSize operator/=( const double v );
%Docstring
 Divides the width and height by a scalar value.
 :rtype: QgsLayoutSize
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsize.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
