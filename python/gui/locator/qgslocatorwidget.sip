/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocatorwidget.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLocatorWidget : QWidget
{
%Docstring
 A special locator widget which allows searching for matching results from a QgsLocator
 and presenting them to users for selection.
.. seealso:: QgsLocator
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslocatorwidget.h"
%End
  public:

    QgsLocatorWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsLocatorWidget.
%End

    QgsLocator *locator();
%Docstring
 Returns a pointer to the locator utilized by this widget.
 :rtype: QgsLocator
%End

  public slots:

    void search( const QString &string );
%Docstring
 Triggers the locator widget to focus, open and start searching for a specified ``string``.
%End

  protected:

    virtual bool eventFilter( QObject *obj, QEvent *event );


};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/locator/qgslocatorwidget.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
