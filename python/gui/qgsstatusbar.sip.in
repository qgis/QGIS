/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsstatusbar.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsStatusBar : QWidget
{
%Docstring
A proxy widget for QStatusBar.

Unlike QStatusBar, QgsStatusBar allows finer control of widget placement, including
the option to locate permanent widgets on the left side of the bar.

QgsStatusBar is designed to be embedded into an existing
window's QStatusBar, as a permanent widget. This allows reuse of the special QStatusBar handling
for resize grips and other platform specific status bar tweaks.

Instead of adding child widgets and showing messages directly in the window's status bar,
these widgets (and messages) should instead be added into the embedded QgsStatusBar.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsstatusbar.h"
%End
  public:

    enum Anchor
    {
      AnchorLeft,
      AnchorRight,
    };

    QgsStatusBar( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsStatusBar.
%End

    void addPermanentWidget( QWidget *widget /Transfer/, int stretch = 0, Anchor anchor = AnchorRight );
%Docstring
Adds the given ``widget`` permanently to this status bar, reparenting the widget if it isn't already a child
of this object.

The ``stretch`` parameter is used to compute a suitable size for the given widget as the status bar
grows and shrinks. The default stretch factor is 0, i.e giving the widget a minimum of space.

The ``anchor`` parameter controls which side of the status bar the widget should be anchored to.
%End

    void removeWidget( QWidget *widget );
%Docstring
Removes a ``widget`` from the status bar. Ownership of the widget remains unchanged, and the
widget itself is not deleted.
%End

    QString currentMessage() const;
%Docstring
Returns the current message shown in the status bar.

.. seealso:: :py:func:`showMessage`
%End

  public slots:

    void showMessage( const QString &message, int timeout = 0 );
%Docstring
Displays the given ``message`` for the specified number of milli-seconds (``timeout``).
If ``timeout`` is 0 (default), the message remains displayed until the clearMessage()
slot is called or until the showMessage() slot is called again to change the message.

.. seealso:: :py:func:`clearMessage`

.. seealso:: :py:func:`currentMessage`
%End

    void clearMessage();
%Docstring
Removes any temporary message being shown.

.. seealso:: :py:func:`showMessage`
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsstatusbar.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
