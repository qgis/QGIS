/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalerangewidget.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsScaleRangeWidget : QWidget
{
%Docstring
A widget allowing entry of a range of map scales, e.g. minimum scale and maximum scale.
%End

%TypeHeaderCode
#include "qgsscalerangewidget.h"
%End
  public:

    explicit QgsScaleRangeWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsScaleRangeWidget.
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Sets the map ``canvas`` which will be used for the current scale buttons.
If not set, the buttons are hidden.
%End

    double minimumScale() const;
%Docstring
Returns the selected minimum scale (i.e. most "zoomed out" scale), or 0 if minimum scale is not set.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`
%End

    double maximumScale() const;
%Docstring
Returns the selected maximum scale (i.e. most "zoomed in" scale), or 0 if maximum scale is not set.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`
%End

    void reloadProjectScales();
%Docstring
Call to reload the preset scales from the current project and apply them to the 2 scales combo boxes.
%End

  public slots:

    void setMinimumScale( double scale );
%Docstring
Set the minimum ``scale`` (i.e. most "zoomed out" scale), or 0 to indicate the minimum is not set.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`

.. seealso:: :py:func:`setScaleRange`
%End

    void setMaximumScale( double scale );
%Docstring
Set the maximum ``scale`` (i.e. most "zoomed in" scale), or 0 to indicate the minimum is not set.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`

.. seealso:: :py:func:`setScaleRange`
%End

    void setScaleRange( double min, double max );
%Docstring
Sets the scale range, from ``min`` scale (i.e. most "zoomed out" scale) to ``max`` scale (most "zoomed in" scale).
The scale values indicates the scale denominator, e.g. 1000.0 for a 1:1000 map,
or 0 to indicate not set.

.. seealso:: :py:func:`setMinimumScale`

.. seealso:: :py:func:`setMaximumScale`
%End

  signals:

    void rangeChanged( double min, double max );
%Docstring
Emitted when the scale range set in the widget is changed.
The scale values indicates the scale denominator, e.g. 1000.0 for a 1:1000 map,
or 0 to indicate not set.

.. versionadded:: 2.16
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalerangewidget.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
