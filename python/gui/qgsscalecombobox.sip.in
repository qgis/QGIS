/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalecombobox.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsScaleComboBox : QComboBox
{
%Docstring
A combobox which lets the user select map scale from predefined list
and highlights nearest to current scale value
*
%End

%TypeHeaderCode
#include "qgsscalecombobox.h"
%End
  public:

    QgsScaleComboBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsScaleComboBox.
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

    static double toDouble( const QString &string, bool *ok = 0 );
%Docstring
Helper function to convert a scale ``string`` to double.
The returned value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
If specified, ``ok`` will be set to true if the string was successfully interpreted as a scale.

.. seealso:: :py:func:`toString`
%End

  signals:

    void scaleChanged( double scale );
%Docstring
Emitted when *user* has finished editing/selecting a new scale.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

  public slots:

    void updateScales( const QStringList &scales = QStringList() );
%Docstring
Sets the list of predefined ``scales`` to show in the combobox. List elements
are expected to be valid scale strings, such as "1:1000000".
%End

    void setScale( double scale );
%Docstring
Set the selected scale from a double.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`scale`
%End

    void setMinScale( double scale );
%Docstring
Set the minimum allowed ``scale``. Set to 0 to disable the minimum scale.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
Any scale lower than the minimum scale will automatically be converted to the minimum scale.
Except for 0 which is always allowed.
%End

  protected:
    virtual void showPopup();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalecombobox.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
