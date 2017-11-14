/***************************************************************************
    qgspanelwidgetstack.h
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
#ifndef QGSPANELWIDGETSTACK_H
#define QGSPANELWIDGETSTACK_H

#include <QWidget>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QStack>

#include "ui_qgsrenderercontainerbase.h"
#include "qgis_gui.h"

class QgsPanelWidget;

/**
 * \ingroup gui
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
      * \param parent
      */
    QgsPanelWidgetStack( QWidget *parent = nullptr );

    /**
     * Adds the main panel widget to the stack and selects it for the user
     * The main widget can not be closed and only the showPanel signal is attached
     * to handle children widget opening panels.
     * \param panel The panel to set as the first widget in the stack.
     * \note a stack can have only one main panel. Any existing main panel
     * should be removed by first calling takeMainPanel().
     * \see mainPanel()
     * \see takeMainPanel()
     */
    void setMainPanel( QgsPanelWidget *panel );

    /**
     * The main panel widget that is set in the stack. The main widget can not be closed
     * and doesn't display a back button.
     * \returns The main QgsPanelWidget that is active in the stack.
     * \see setMainPanel()
     */
    QgsPanelWidget *mainPanel();

    /**
     * Removes the main panel widget from the stack and transfers ownsership to the
     * caller.
     * \returns The main widget that is set in the stack.
     * \note Calling this will clear out any current stacked panels by accepting
     * each panel in turn.
     * \see mainPanel()
     * \see setMainPanel()
     */
    QgsPanelWidget *takeMainPanel();

    /**
     * Clear the stack of all widgets. Unless the panels autoDelete is set to false
     * the widget will be deleted.
     */
    void clear();

    /**
     * Returns the panel currently shown in the stack.
     * \since QGIS 3.0
     */
    QgsPanelWidget *currentPanel();

  public slots:

    /**
     * Accept the current active widget in the stack.
     *
     * Calls the panelAccepeted signal on the active widget.
     * \see acceptAllPanels()
     */
    void acceptCurrentPanel();

    /**
     * Accepts all panel widgets open in the stack in turn until until only the mainPanel()
     * remains.
     * \see acceptCurrentPanel();
     * \since QGIS 3.0
     */
    void acceptAllPanels();

    /**
     * Show a panel in the stack widget. Will connect to the panels showPanel event to handle
     * nested panels. Auto switches the the given panel for the user.
     * \param panel The panel to show.
     */
    void showPanel( QgsPanelWidget *panel );

    /**
     * Closes the panel in the widget. Will also delete the widget.
     * This slot is normally auto connected to panelAccepted when a panel is shown.
     * \param panel The panel to close.
     */
    void closePanel( QgsPanelWidget *panel );

  protected:

    void mouseReleaseEvent( QMouseEvent *e ) override;

  private:
    void updateBreadcrumb();
    QStack<QString> mTitles;
};


#endif // QGSPANELWIDGETSTACK_H
