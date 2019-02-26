/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspanelwidget.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsPanelWidget : QWidget
{
%Docstring
Base class for any widget that can be shown as a inline panel
%End

%TypeHeaderCode
#include "qgspanelwidget.h"
%End
  public:

    QgsPanelWidget( QWidget *parent = 0 );
%Docstring
Base class for any widget that can be shown as a inline panel

:param parent: Parent widget.
%End

    void setPanelTitle( const QString &panelTitle );
%Docstring
Set the title of the panel when shown in the interface.

:param panelTitle: The panel title.
%End

    QString panelTitle();
%Docstring
The title of the panel.

:return: The title pf the panel.
%End

    void connectChildPanels( const QList<QgsPanelWidget *> &panels );
%Docstring
Connect the given sub panel widgets showPanel signals to this current panels
main showPanel event to bubble up to the user.

Use this method if you have children widgets that need to show a panel to the user.

:param panels: A list of panel widgets to connect.
%End

    void connectChildPanel( QgsPanelWidget *panel );
%Docstring
Connect the given sub panel widgets showPanel signals to this current panels
main showPanel event to bubble up to the user.

Use this method if you have children widgets that need to show a panel to the user.

:param panel: The panel to connect.
%End

    virtual void setDockMode( bool dockMode );
%Docstring
Set the widget in dock mode which tells the widget to emit panel
widgets and not open dialogs

:param dockMode: ``True`` to enable dock mode.
%End

    bool dockMode();
%Docstring
Returns the dock mode state.

:return: ``True`` if in dock mode.  If in dock mode the widget
         will emit the showPanel signal to handle panel opening
         If ``False`` it will open dialogs when openPanel is called.
%End

    void setAutoDelete( bool autoDelete );
%Docstring
The the auto delete property on the widget. ``True`` by default.
When auto delete is enabled when a panel is removed from the stack
it will be deleted.

:param autoDelete: Enable or disable auto delete on the panel.
%End

    bool autoDelete();
%Docstring
The the auto delete property on the widget. ``True`` by default.
When auto delete is enabled when a panel is removed from the stack
it will be deleted.

:return: The auto delete value for the widget.
%End

    static QgsPanelWidget *findParentPanel( QWidget *widget );
%Docstring
Traces through the parents of a widget to find if it is contained within a QgsPanelWidget
widget.

:param widget: widget which may be contained within a panel widget

:return: parent panel widget if found, otherwise ``None``

.. versionadded:: 3.0
%End

  signals:

    void panelAccepted( QgsPanelWidget *panel );
%Docstring
Emitted when the panel is accepted by the user.

:param panel: The panel widget that was accepted.

.. note::

   This argument is normally raised with emit panelAccepted(this)
   so that callers can retrieve the widget easier in calling code.

.. note::

   this is emitted only when this panel is accepted, and is not emitted for
   child panels. For example, if this panel opens a second stacked panel, then this panel
   will not emit panelAccepted when the second panel is accepted.
%End

    void showPanel( QgsPanelWidget *panel );
%Docstring
Emit when you require a panel to be show in the interface.

:param panel: The panel widget to show.

.. note::

   If you are connected to this signal you should also connect
   given panels showPanel signal as they can be nested.
%End

    void widgetChanged();
%Docstring
Emitted when the widget state changes.
Connect to this to pull any changes off the widget when needed.
As panels are non blocking "dialogs" you should listen to this signal
to give the user feedback when something changes.
%End

  public slots:

    void openPanel( QgsPanelWidget *panel );
%Docstring
Open a panel or dialog depending on dock mode setting
If dock mode is ``True`` this method will emit the showPanel signal
for connected slots to handle the open event.

If dock mode is ``False`` this method will open a dialog
and block the user.

:param panel: The panel widget to open.
%End

    void acceptPanel();
%Docstring
Accept the panel. Causes panelAccepted to be emitted.
Widgets are normally removed form the interface using the panel manager or the caller.
%End

  protected:

    virtual void keyPressEvent( QKeyEvent *event );

%Docstring
Overridden key press event to handle the esc event on the widget.

:param event: The key event
%End

};


class QgsPanelWidgetWrapper: QgsPanelWidget
{
%Docstring
Wrapper widget for existing widgets which can't have
the inheritance tree changed, e.g dialogs.

.. note::

   Generally you should use the QgsPanelWidget class if you can
   and only use this wrapper if you can't update your code.
%End

%TypeHeaderCode
#include "qgspanelwidget.h"
%End
  public:

    QgsPanelWidgetWrapper( QWidget *widget, QWidget *parent = 0 );
%Docstring
Wrapper widget for existing widgets which can't have
the inheritance tree changed, e.g dialogs.

:param widget: The widget to wrap.
:param parent: The parent widget.
%End

    QWidget *widget();
%Docstring
Returns the internal widget that is wrapped in this panel.

:return: The internal widget. Can be ``None``.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspanelwidget.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
