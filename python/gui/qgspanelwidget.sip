/**
 * @brief Base class for any widget that can be shown as a inline panel
 */
class QgsPanelWidget : public QWidget
{
%TypeHeaderCode
#include "qgspanelwidget.h"
%End
  public:
    /**
     * @brief Base class for any widget that can be shown as a inline panel
     * @param parent Parent widget.
     */
    QgsPanelWidget( QWidget *parent = 0 );

    /**
     * Set the title of the panel when shown in the interface.
     * @param panelTitle The panel title.
     */
    void setPanelTitle( QString panelTitle );

    /**
     * The title of the panel.
     * @return The title pf the panel.
     */
    QString panelTitle();

    /**
    * Connect the given sub panel widgets showPanel signals to this current panels
    * main showPanel event to bubble up to the user.
    *
    * Use this method if you have children widgets that need to show a panel to the user.
    * @param panels A list of panel widgets to connect.
    */
    void connectChildPanels( QList<QgsPanelWidget*> panels );

    /**
     * Connect the given sub panel widgets showPanel signals to this current panels
     * main showPanel event to bubble up to the user.
     *
     * Use this method if you have children widgets that need to show a panel to the user.
     * @param panel The panel to connect.
     */
    void connectChildPanel( QgsPanelWidget* panel );

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * @param dockMode True to enable dock mode.
     */
    virtual void setDockMode( bool dockMode );
  signals:

    /**
     * Emiited when the panel is accpeted by the user.
     * @param panel The panel widget that was accepted.
     * @note This argument is normally raised with emit panelAccpeted(this)
     * so that callers can retrive the widget easier in calling code.
     */
    void panelAccepted( QgsPanelWidget* panel );

    /**
     * Emit when you require a panel to be show in the interface.
     * @param panel The panel widget to show.
     * @note If you are connected to this signal you should also connect
     * given panels showPanel signal as they can be nested.
     */
    void showPanel( QgsPanelWidget* panel );

    /**
     * Emiited when the widget state changes.
     * Connect to this to pull any changes off the widget when needed.
     * As panels are non blocking "dialogs" you should listen to this signal
     * to give the user feedback when something changes.
     */
    void widgetChanged();

  public slots:

    /**
     * Accept the panel. Causes panelAccepted to be emiited.
     * Widgets are normally removed form the interface using the panel manager or the caller.
     */
    void acceptPanel();

  protected:

    /**
     * @brief Overriden key press event to handle the esc event on the widget.
     * @param event The key event
     */
    void keyPressEvent( QKeyEvent* event );
};


/**
 * A non model panel page that can get shown to the user.  Page will contain a close button and trigger the
 * internal widgets acceptPanel() on close.
 */
class QgsPanelWidgetPage : public QgsPanelWidget
{
%TypeHeaderCode
#include "qgspanelwidget.h"
%End
  public:

    /**
      * A non model panel page that can get shown to the user.
     * @param widget The internal widget to show in the page.
     * @param parent THe parent widget.
     */
    QgsPanelWidgetPage( QgsPanelWidget* widget, QWidget* parent = nullptr );

    ~QgsPanelWidgetPage();

    void setTitle( QString title );
};


/**
 * A stack widget to manage panels in the interface. Handles the open and close events
 * for added panels.
 * Any widgets that want to have a non blocking panel based interface should use this
 * class to manage the panels.
 */
class QgsPanelWidgetStackWidget : public QStackedWidget
{
%TypeHeaderCode
#include "qgspanelwidget.h"
%End
  public:

    /**
      * A stack widget to manage panels in the interface. Handles the open and close events
      * for added panels.
      * @param parent
      */
    QgsPanelWidgetStackWidget( QWidget* parent = nullptr );

    void connectPanels( QList<QgsPanelWidget*> panels );

    void connectPanel( QgsPanelWidget* panel );

    /**
     * Adds the main widget to the stack and selects it for the user
     * The main widget can not be closed and only the showPanel signal is attached
     * to handle children widget opening panels.
     * @param panel The panel to set as the first widget in the stack.
     */
    void addMainPanel( QgsPanelWidget* panel );

  public slots:
    /**
     * Show a panel in the stack widget. Will connect to the panels showPanel event to handle
     * nested panels. Auto switches the the given panel for the user.
     * @param panel The panel to show.
     */
    void showPanel( QgsPanelWidget* panel );

    /**
     * Closes the panel in the widget. Will also delete the widget.
     * This slot is normally auto connected to panelAccepted when a panel is shown.
     * @param panel The panel to close.
     */
    void closePanel( QgsPanelWidget* panel );
};
