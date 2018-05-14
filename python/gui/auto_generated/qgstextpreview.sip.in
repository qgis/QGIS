/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgstextpreview.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsTextPreview : QLabel
{
%Docstring
A widget for previewing text formatting settings.

QgsTextPreview provides a widget for previewing the appearance of text rendered
using QgsTextRenderer. The preview includes all settings contained within
a QgsTextFormat, including shadow, background and buffer.

In order to preview the exact appearance of text which uses sizes in map units,
the scale and map units must be set by calling setScale() and setMapUnits().

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgstextpreview.h"
%End
  public:

    QgsTextPreview( QWidget *parent = 0 );
%Docstring
Constructor for QgsTextPreview

:param parent: parent widget
%End

    virtual void paintEvent( QPaintEvent *e );


    void setFormat( const QgsTextFormat &format );
%Docstring
Sets the text format for previewing in the widget.

:param format: text format

.. seealso:: :py:func:`format`
%End

    QgsTextFormat format() const;
%Docstring
Returns the text format used for previewing text in the widget.

.. seealso:: :py:func:`setFormat`
%End

    void setScale( double scale );
%Docstring
Sets the ``scale`` to use for previewing format sizes in map units.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`scale`

.. seealso:: :py:func:`setMapUnits`
%End

    double scale() const;
%Docstring
Returns the scale used for previewing format sizes in map units.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`setScale`

.. seealso:: :py:func:`mapUnits`
%End

    void setMapUnits( QgsUnitTypes::DistanceUnit unit );
%Docstring
Sets the map unit type for previewing format sizes in map units.

:param unit: map units

.. seealso:: :py:func:`mapUnits`

.. seealso:: :py:func:`setScale`
%End

    QgsUnitTypes::DistanceUnit mapUnits() const;
%Docstring
Returns the map unit type used for previewing format sizes in map units.

.. seealso:: :py:func:`setMapUnits`

.. seealso:: :py:func:`scale`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgstextpreview.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
