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

.. seealso:: :py:class:`QgsLayoutMeasurementConverter`

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

:param width: width
:param height: height
:param units: units for width and height
%End

    explicit QgsLayoutSize( const QSizeF size, const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
Constructor for QgsLayoutSize.
%End

    explicit QgsLayoutSize( const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
Constructor for an empty layout size

:param units: units for measurement
%End

    void setSize( const double width, const double height );
%Docstring
Sets new ``width`` and ``height`` for the size.

.. seealso:: :py:func:`setWidth`

.. seealso:: :py:func:`setHeight`

.. seealso:: :py:func:`setUnits`
%End

    double width() const;
%Docstring
Returns the width of the size.

.. seealso:: :py:func:`setWidth`

.. seealso:: :py:func:`height`
%End

    void setWidth( const double width );
%Docstring
Sets the ``width`` for the size.

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`setHeight`
%End

    double height() const;
%Docstring
Returns the height of the size.

.. seealso:: :py:func:`setHeight`

.. seealso:: :py:func:`width`
%End

    void setHeight( const double height );
%Docstring
Sets the ``height`` for the size.

.. seealso:: :py:func:`height`

.. seealso:: :py:func:`setWidth`
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
Returns the units for the size.

.. seealso:: :py:func:`setUnits`
%End

    void setUnits( const QgsUnitTypes::LayoutUnit units );
%Docstring
Sets the ``units`` for the size. Does not alter the stored width or height,
ie. no conversion is done.

.. seealso:: :py:func:`units`
%End

    bool isEmpty() const;
%Docstring
Tests whether the size is empty, ie both its width and height
are zero.

:return: true if size is empty
%End

    QSizeF toQSizeF() const;
%Docstring
Converts the layout size to a QSizeF. The unit information is discarded
during this operation.

:return: QSizeF with same dimensions as layout size
%End

    QString encodeSize() const;
%Docstring
Encodes the layout size to a string

.. seealso:: :py:func:`decodeSize`
%End

    static QgsLayoutSize decodeSize( const QString &string );
%Docstring
Decodes a size from a ``string``.

.. seealso:: :py:func:`encodeSize`
%End

    bool operator==( const QgsLayoutSize &other ) const;
    bool operator!=( const QgsLayoutSize &other ) const;

    QgsLayoutSize operator*( const double v ) const;

    QgsLayoutSize operator*=( const double v );

    QgsLayoutSize operator/( const double v ) const;

    QgsLayoutSize operator/=( const double v );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsize.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
