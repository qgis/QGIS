/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalewidget.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsScaleWidget : QWidget
{
%Docstring
A combobox which lets the user select map scale from predefined list
and highlights nearest to current scale value
*
%End

%TypeHeaderCode
#include "qgsscalewidget.h"
%End
  public:

    explicit QgsScaleWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 QgsScaleWidget creates a combobox which lets the user select map scale from predefined list
and highlights nearest to current scale value
*
%End

    void setShowCurrentScaleButton( bool showCurrentScaleButton );
%Docstring
Sets whether to show a button to set the scale to the current scale of the map canvas next to the combobox.

.. note::

   the map canvas must be defined to show the button

.. seealso:: :py:func:`showCurrentScaleButton`

.. seealso:: :py:func:`setMapCanvas`
%End

    bool showCurrentScaleButton();
%Docstring
Returns whether a button to set the scale from map canvas is shown or not.

.. seealso:: :py:func:`setShowCurrentScaleButton`
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
Set the map ``canvas`` associated to the current button.
%End

    QString scaleString() const;
%Docstring
Returns the selected scale as a string, e.g. "1:150".

.. seealso:: :py:func:`setScaleString`
%End

    bool setScaleString( const QString &string );
%Docstring
Set the selected scale from a ``string``, e.g. "1:150".

.. seealso:: :py:func:`scaleString`
%End

    double scale() const;
%Docstring
Returns the selected scale as a double.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`setScale`
%End

    double minScale() const;
%Docstring
Returns the minimum scale, or 0 if no minimum scale set.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
Any scale lower than the minimum scale will automatically be converted to the minimum scale.
Except for 0 which is always allowed.
%End

    static QString toString( double scale );
%Docstring
Helper function to convert a ``scale`` double to scale string.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

The returned string will be rounded (e.g. 1:1000, not 1:1000.345).

.. seealso:: :py:func:`toDouble`
%End

    static double toDouble( const QString &scaleString, bool *ok = 0 );
%Docstring
Helper function to convert a scale ``string`` to double.
The returned value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
If specified, ``ok`` will be set to true if the string was successfully interpreted as a scale.

.. seealso:: :py:func:`toString`
%End

  public slots:

    void setScale( double scale );
%Docstring
Set the selected scale from a double.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`scale`
%End

    void updateScales( const QStringList &scales = QStringList() );
%Docstring
Sets the list of predefined ``scales`` to show in the combobox. List elements
are expected to be valid scale strings, such as "1:1000000".
%End

    void setScaleFromCanvas();
%Docstring
Assigns the current scale from the map canvas, if set.

.. seealso:: :py:func:`setMapCanvas`
%End

    void setMinScale( double scale );
%Docstring
Set the minimum allowed ``scale``. Set to 0 to disable the minimum scale.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
Any scale lower than the minimum scale will automatically be converted to the minimum scale.
Except for 0 which is always allowed.
%End

  signals:

    void scaleChanged( double scale );
%Docstring
Emitted when *user* has finished editing/selecting a new scale.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalewidget.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
