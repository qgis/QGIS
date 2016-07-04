/***************************************************************************
    qgspanelwidget.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPANELWIDGET_H
#define QGSPANELWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QStack>

#include "ui_qgsrenderercontainerbase.h"


/** \ingroup gui
 * @brief Base class for any widget that can be shown as a inline panel
 */
class GUI_EXPORT QgsPanelWidget : public QWidget
{
    Q_OBJECT
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
    void setPanelTitle( QString panelTitle ) { mPanelTitle = panelTitle; }

    /**
     * The title of the panel.
     * @return The title pf the panel.
     */
    QString panelTitle() { return mPanelTitle; }

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
    bool dockMode() { return mDockMode; }

    /**
     * The the auto delete property on the widget. True by default.
     * When auto delete is enabeld when a panel is removed from the stack
     * it will be deleted.
     * @param autoDelete Enable or disable auto delete on the panel.
     */
    void setAutoDelete( bool autoDelete ) { mAutoDelete = autoDelete; }

    /**
     * The the auto delete property on the widget. True by default.
     * When auto delete is enabeld when a panel is removed from the stack
     * it will be deleted.
     * @returns The auto delete value for the widget.
     */
    bool autoDelete() { return mAutoDelete; }

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

  private:
    bool mAutoDelete;
    QString mPanelTitle;
    bool mDockMode;

};


/** \ingroup gui
 * @brief Wrapper widget for existing widgets which can't have
 * the inheritance tree changed, e.g dialogs.
 *
 * @note Generally you should use the QgsPanelWidget class if you can
 * and only use this wrapper if you can't update your code.
 */
class GUI_EXPORT QgsPanelWidgetWrapper: public QgsPanelWidget
{
    Q_OBJECT
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
    QWidget* widget() { return mWidget; }

  private:
    QWidget* mWidget;

};


/** \ingroup gui
 * A stack widget to manage panels in the interface. Handles the open and close events
 * for added panels.
 * Any widgets that want to have a non blocking panel based interface should use this
 * class to manage the panels.
 */
class GUI_EXPORT QgsPanelWidgetStack : public QWidget, private Ui::QgsRendererWidgetContainerBase
{
    Q_OBJECT
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
  private:
    void updateBreadcrumb();
    QStack<QString> mTitles;
};


#endif // QGSPANELWIDGET_H
