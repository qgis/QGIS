/***************************************************************************
                         qgstoolbuttonaction.h
                         ---------------------
    begin                : October 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTOOLBUTTONACTION_H
#define QGSTOOLBUTTONACTION_H

#include "qgis_gui.h"

#include <QPointer>
#include <QToolButton>
#include <QWidgetAction>

class QMenu;

/**
 * \class QgsToolButtonAction
 * \ingroup gui
 * \brief QWidgetAction for adding toolbuttons/actions to the toolbars which
 * are not children of QMainWindow.
 *
 * QgsToolButtonAction should be used to add buttons with dropdown menus
 * to make them available in the toolbar extension popup.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsToolButtonAction : public QWidgetAction
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsToolButtonAction
     * \param parent parent widget
     */
    explicit QgsToolButtonAction( QObject *parent = nullptr );

    /**
     * Sets whether auto-raise is enabled.
     */
    void setAutoRaise( bool autoRaise );

    /**
     * Sets the popup mode to use for button's popup menu.
     *
     * \see setMenu()
     * \see addActions()
     */
    void setPopupMode( QToolButton::ToolButtonPopupMode mode );

    /**
     * Sets the toolbutton style, e.g. icon only, text only, or text beside/below the icon.
     */
    void setToolButtonStyle( Qt::ToolButtonStyle style );

    /**
     * Associates a \a menu with this tool button. The menu will be shown according to the button's popup mode.
     * Ownership of the menu is not transferred.
     *
     * \see setPopupMode()
     * \see addActions()
     */
    void setMenu( QMenu *menu );

    /**
     * Appends \a actions to the button's list of actions. This is an alternative to setMenu(),
     * the buttons build their popup menu from these actions. If a menu was set with
     * setMenu() it takes precedence.
     *
     * \see setMenu()
     */
    void addActions( const QList<QAction *> &actions );

    /**
     * Sets the default action to \a action
     *
     * The buttons mirror the default action's icon, text, tooltip and state, and
     * clicking a button triggers it.
     *
     * Ownership of the action is not transferred.
     *
     * \see defaultAction()
     */
    void setDefaultAction( QAction *action );

    /**
     * Returns the default action.
     *
     * \see setDefaultAction()
     */
    QAction *defaultAction() const;

  protected:
    QWidget *createWidget( QWidget *parent ) override;

  private:
    void applyState( QToolButton *button ) const;
    void updateButtons();

  private:
    bool mAutoRaise = false;
    Qt::ToolButtonStyle mToolButtonStyle = Qt::ToolButtonStyle::ToolButtonIconOnly;
    QToolButton::ToolButtonPopupMode mPopupMode = QToolButton::ToolButtonPopupMode::DelayedPopup;
    QPointer< QMenu > mMenu;
    QList< QAction * > mButtonActions;
    QPointer< QAction > mDefaultAction;
};

#endif // QGSTOOLBUTTONACTION_H
