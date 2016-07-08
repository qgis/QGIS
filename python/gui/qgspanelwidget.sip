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

    /**
     * Return the dock mode state.
     * @return True if in dock mode.  If in dock mode the widget
     * will emit the showPanel signal to handle panel opening
     * If false it will open dialogs when openPanel is called.
     */
    bool dockMode();

    /**
     * The the auto delete property on the widget. True by default.
     * When auto delete is enabeld when a panel is removed from the stack
     * it will be deleted.
     * @param autoDelete Enable or disable auto delete on the panel.
     */
    void setAutoDelete( bool autoDelete );

    /**
     * The the auto delete property on the widget. True by default.
     * When auto delete is enabeld when a panel is removed from the stack
     * it will be deleted.
     * @returns The auto delete value for the widget.
     */
    bool autoDelete();

  signals:

    /**
     * Emitted when the panel is accepted by the user.
     * @param panel The panel widget that was accepted.
     * @note This argument is normally raised with emit panelAccepted(this)
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
     * Emitted when the widget state changes.
     * Connect to this to pull any changes off the widget when needed.
     * As panels are non blocking "dialogs" you should listen to this signal
     * to give the user feedback when something changes.
     */
    void widgetChanged();

  public slots:
    /**
     * Open a panel or dialog depending on dock mode setting
     * If dock mode is true this method will emit the showPanel signal
     * for connected slots to handle the open event.
     *
     * If dock mode is false this method will open a dialog
     * and block the user.
     *
     * @param panel The panel widget to open.
     */
    void openPanel( QgsPanelWidget* panel );

    /**
     * Accept the panel. Causes panelAccepted to be emitted.
     * Widgets are normally removed form the interface using the panel manager or the caller.
     */
    void acceptPanel();

  protected:

    /**
     * @brief Overridden key press event to handle the esc event on the widget.
     * @param event The key event
     */
    void keyPressEvent( QKeyEvent* event );
};


/**
 * @brief Wrapper widget for existing widgets which can't have
 * the inheritance tree changed, e.g dialogs.
 *
 * @note Generally you should use the QgsPanelWidget class if you can
 * and only use this wrapper if you can't update your code.
 */
class QgsPanelWidgetWrapper: public QgsPanelWidget
{
%TypeHeaderCode
#include "qgspanelwidget.h"
%End
  public:
    /**
     * @brief Wrapper widget for existing widgets which can't have
     * the inheritance tree changed, e.g dialogs.
     * @param widget The widget to wrap.
     * @param parent The parent widget.
     */
    QgsPanelWidgetWrapper( QWidget* widget, QWidget* parent = nullptr );

    /**
     * Returns the internal widget that is wrapped in this panel.
     * @return The internal widget. Can be nullptr.
     */
    QWidget* widget();
};
