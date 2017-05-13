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
.. note::

   the map canvas must be defined to show the button
%End
    bool showCurrentScaleButton();
%Docstring
returns if a button to set the scale from map canvas is shown or not
 :rtype: bool
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
set the map canvas associated to the current button
%End

    QString scaleString();
%Docstring
Function to read the selected scale as text
 :rtype: str
%End
    bool setScaleString( const QString &scaleTxt );
%Docstring
Function to set the selected scale from text
 :rtype: bool
%End
    double scale() const;
%Docstring
Function to read the selected scale as double
 :rtype: float
%End
    void setScale( double scale );
%Docstring
Function to set the selected scale from double
%End
    double minScale() const;
%Docstring
Function to read the min scale
 :rtype: float
%End

    static QString toString( double scale );
%Docstring
 :rtype: str
%End
    static double toDouble( const QString &scaleString, bool *ok = 0 );
%Docstring
Helper function to convert a scale string to double
 :rtype: float
%End

  public slots:
    void updateScales( const QStringList &scales = QStringList() );
%Docstring
updates the list of predefined scales
%End

    void setScaleFromCanvas();
%Docstring
assign the current scale from the map canvas
%End

    void setMinScale( double scale );
%Docstring
Function to set the min scale
%End

  signals:
    void scaleChanged( double scale );
%Docstring
Signal is emitted when *user* has finished editing/selecting a new scale.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsscalewidget.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
