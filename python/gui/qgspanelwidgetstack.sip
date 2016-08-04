/**
 * A stack widget to manage panels in the interface. Handles the open and close events
 * for added panels.
 * Any widgets that want to have a non blocking panel based interface should use this
 * class to manage the panels.
 */
class QgsPanelWidgetStack: public QWidget
{
%TypeHeaderCode
#include "qgspanelwidgetstack.h"
%End
  public:

    /**
      * A stack widget to manage panels in the interface. Handles the open and close events
      * for added panels.
      * @param parent
      */
    QgsPanelWidgetStack( QWidget* parent = nullptr );

    /**
     * Adds the main widget to the stack and selects it for the user
     * The main widget can not be closed and only the showPanel signal is attached
     * to handle children widget opening panels.
     * @param panel The panel to set as the first widget in the stack.
     */
    void addMainPanel( QgsPanelWidget* panel );

    /**
     * The main widget that is set in the stack.  The main widget can not be closed
     * and doesn't display a back button.
     * @return The main QgsPanelWidget that is active in the stack.
     */
    QgsPanelWidget* mainWidget();

    /**
     * Removes the main widget from the stack and transfers ownsership to the
     * caller.
     * @return The main widget that is set in the stack.
     */
    QgsPanelWidget* takeMainWidget();

    /**
     * Clear the stack of all widgets. Unless the panels autoDelete is set to false
     * the widget will be deleted.
     */
    void clear();


  public slots:
    /**
     * Accept the current active widget in the stack.
     *
     * Calls the panelAccepeted signal on the active widget.
     */
    void acceptCurrentPanel();

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
