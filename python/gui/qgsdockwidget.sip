/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdockwidget.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsDockWidget : QDockWidget
{
%Docstring
QgsDockWidget subclass with more fine-grained control over how the widget is closed or opened.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsdockwidget.h"
%End
  public:

    explicit QgsDockWidget( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags flags = 0 );
%Docstring
Constructor for QgsDockWidget.

:param parent: parent widget
:param flags: window flags
%End

    explicit QgsDockWidget( const QString &title, QWidget *parent /TransferThis/ = 0, Qt::WindowFlags flags = 0 );
%Docstring
Constructor for QgsDockWidget.

:param title: dock title
:param parent: parent widget
:param flags: window flags
%End

    bool isUserVisible() const;
%Docstring
Returns true if the dock is both opened and raised to the front (ie not hidden by
any other tabs.

.. seealso:: :py:func:`setUserVisible`
%End

  public slots:

    void setUserVisible( bool visible );
%Docstring
Sets the dock widget as visible to a user, ie both shown and raised to the front.

:param visible: set to true to show the dock to the user, or false to hide the dock.
When setting a dock as user visible, the dock will be opened (if it is not already
opened) and raised to the front.
When setting as hidden, the following logic is used:
- hiding a dock which is open but not raised (ie hidden by another tab) will have no
effect, and the dock will still be opened and hidden by the other tab
- hiding a dock which is open and raised (ie, user visible) will cause the dock to
be closed
- hiding a dock which is closed has no effect and raises no signals

.. seealso:: :py:func:`isUserVisible`
%End

  protected:

    virtual void closeEvent( QCloseEvent * );

    virtual void showEvent( QShowEvent *event );


  signals:

    void closed();
%Docstring
Emitted when dock widget is closed.

.. seealso:: :py:func:`closedStateChanged`

.. seealso:: :py:func:`opened`
%End

    void closedStateChanged( bool wasClosed );
%Docstring
Emitted when dock widget is closed (or opened).

:param wasClosed: will be true if dock widget was closed, or false if dock widget was opened

.. seealso:: :py:func:`closed`

.. seealso:: :py:func:`openedStateChanged`
%End

    void opened();
%Docstring
Emitted when dock widget is opened.

.. seealso:: :py:func:`openedStateChanged`

.. seealso:: :py:func:`closed`
%End

    void openedStateChanged( bool wasOpened );
%Docstring
Emitted when dock widget is opened (or closed).

:param wasOpened: will be true if dock widget was opened, or false if dock widget was closed

.. seealso:: :py:func:`closedStateChanged`

.. seealso:: :py:func:`opened`
%End

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdockwidget.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
