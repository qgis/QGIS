/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspanelwidgetstack.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPanelWidgetStack : QWidget
{
%Docstring
A stack widget to manage panels in the interface. Handles the open and close events
for added panels.
Any widgets that want to have a non blocking panel based interface should use this
class to manage the panels.
%End

%TypeHeaderCode
#include "qgspanelwidgetstack.h"
%End
  public:

    QgsPanelWidgetStack( QWidget *parent = 0 );
%Docstring
A stack widget to manage panels in the interface. Handles the open and close events
for added panels.

:param parent:
%End

    void setMainPanel( QgsPanelWidget *panel );
%Docstring
Adds the main panel widget to the stack and selects it for the user
The main widget can not be closed and only the showPanel signal is attached
to handle children widget opening panels.

:param panel: The panel to set as the first widget in the stack.

.. note::

   a stack can have only one main panel. Any existing main panel
should be removed by first calling takeMainPanel().

.. seealso:: :py:func:`mainPanel`

.. seealso:: :py:func:`takeMainPanel`
%End

    QgsPanelWidget *mainPanel();
%Docstring
The main panel widget that is set in the stack. The main widget can not be closed
and doesn't display a back button.

:return: The main QgsPanelWidget that is active in the stack.

.. seealso:: :py:func:`setMainPanel`
%End

    QgsPanelWidget *takeMainPanel();
%Docstring
Removes the main panel widget from the stack and transfers ownsership to the
caller.

:return: The main widget that is set in the stack.

.. note::

   Calling this will clear out any current stacked panels by accepting
each panel in turn.

.. seealso:: :py:func:`mainPanel`

.. seealso:: :py:func:`setMainPanel`
%End

    void clear();
%Docstring
Clear the stack of all widgets. Unless the panels autoDelete is set to false
the widget will be deleted.
%End

    QgsPanelWidget *currentPanel();
%Docstring
Returns the panel currently shown in the stack.

.. versionadded:: 3.0
%End

  public slots:

    void acceptCurrentPanel();
%Docstring
Accept the current active widget in the stack.

Calls the panelAccepeted signal on the active widget.

.. seealso:: :py:func:`acceptAllPanels`
%End

    void acceptAllPanels();
%Docstring
Accepts all panel widgets open in the stack in turn until until only the mainPanel()
remains.

.. seealso:: :py:func:`acceptCurrentPanel`

.. versionadded:: 3.0
%End

    void showPanel( QgsPanelWidget *panel );
%Docstring
Show a panel in the stack widget. Will connect to the panels showPanel event to handle
nested panels. Auto switches the the given panel for the user.

:param panel: The panel to show.
%End

    void closePanel( QgsPanelWidget *panel );
%Docstring
Closes the panel in the widget. Will also delete the widget.
This slot is normally auto connected to panelAccepted when a panel is shown.

:param panel: The panel to close.
%End

  protected:

    virtual void mouseReleaseEvent( QMouseEvent *e );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspanelwidgetstack.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
