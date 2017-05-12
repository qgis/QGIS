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

  signals:
    void scaleChanged( double scale );
%Docstring
Signal is emitted when *user* has finished editing/selecting a new scale.
%End

  public slots:
    void updateScales( const QStringList &scales = QStringList() );

    void setMinScale( double scale );
%Docstring
 Set the minimum allowed scale.
 Anything scale lower than the minimum scale will automatically
 be converted to the minimum scale.
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
