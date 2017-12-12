/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsuserinputdockwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsUserInputDockWidget : QgsDockWidget
{
%Docstring
 The QgsUserInputDockWidget class is a dock widget that shall be used to display widgets for user inputs.
 It can be used by map tools, plugins, etc.
 Several widgets can be displayed at once, they will be separated by a separator. Widgets will be either layout horizontally or vertically.
 The dock is automatically hidden if it contains no widget.
%End

%TypeHeaderCode
#include "qgsuserinputdockwidget.h"
%End
  public:

    QgsUserInputDockWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsUserInputDockWidget
%End

    void addUserInputWidget( QWidget *widget );
%Docstring
 Add a widget to be displayed in the dock.
 \param widget widget to add. Ownership is not transferred.
%End

  protected:
    virtual void paintEvent( QPaintEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsuserinputdockwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
