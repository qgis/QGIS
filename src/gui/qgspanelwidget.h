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
#include <QStack>
#include "qgis_gui.h"

class QMenu;

/**
 * \ingroup gui
 * \brief Base class for any widget that can be shown as a inline panel
 */
class GUI_EXPORT QgsPanelWidget : public QWidget
{
    Q_OBJECT
  public:
    /**
     * \brief Base class for any widget that can be shown as a inline panel
     * \param parent Parent widget.
     */
    QgsPanelWidget( QWidget *parent = nullptr );

    /**
     * Set the title of the panel when shown in the interface.
     * \param panelTitle The panel title.
     */
    void setPanelTitle( const QString &panelTitle ) { mPanelTitle = panelTitle; }

    /**
     * The title of the panel.
     * \returns The title pf the panel.
     */
    QString panelTitle() { return mPanelTitle; }

    /**
    * Connect the given sub panel widgets showPanel signals to this current panels
    * main showPanel event to bubble up to the user.
    *
    * Use this method if you have children widgets that need to show a panel to the user.
    * \param panels A list of panel widgets to connect.
    */
    void connectChildPanels( const QList<QgsPanelWidget *> &panels );

    /**
     * Connect the given sub panel widgets showPanel signals to this current panels
     * main showPanel event to bubble up to the user.
     *
     * Use this method if you have children widgets that need to show a panel to the user.
     * \param panel The panel to connect.
     */
    void connectChildPanel( QgsPanelWidget *panel );

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * \param dockMode TRUE to enable dock mode.
     */
    virtual void setDockMode( bool dockMode );

    /**
     * Returns TRUE if the size constraints and hints for the panel widget should be
     * applied to the parent QgsPanelWidgetStack which this panel is shown in.
     *
     * The default behavior is to return FALSE.
     *
     * \since QGIS 3.20
     */
    virtual bool applySizeConstraintsToStack() const;

    /**
     * Returns the dock mode state.
     * \returns TRUE if in dock mode.  If in dock mode the widget
     * will emit the showPanel signal to handle panel opening
     * If FALSE it will open dialogs when openPanel is called.
     */
    bool dockMode() { return mDockMode; }

    /**
     * The the auto delete property on the widget. TRUE by default.
     * When auto delete is enabled when a panel is removed from the stack
     * it will be deleted.
     * \param autoDelete Enable or disable auto delete on the panel.
     */
    void setAutoDelete( bool autoDelete ) { mAutoDelete = autoDelete; }

    /**
     * The the auto delete property on the widget. TRUE by default.
     * When auto delete is enabled when a panel is removed from the stack
     * it will be deleted.
     * \returns The auto delete value for the widget.
     */
    bool autoDelete() { return mAutoDelete; }

    /**
     * Traces through the parents of a widget to find if it is contained within a QgsPanelWidget
     * widget.
     * \param widget widget which may be contained within a panel widget
     * \returns parent panel widget if found, otherwise NULLPTR
     */
    static QgsPanelWidget *findParentPanel( QWidget *widget );

    /**
     * Returns the (translated) tooltip text to use for the menu button for this panel.
     *
     * This is only used when the panel returns a menuButtonMenu().
     *
     * \since QGIS 3.12
     */
    virtual QString menuButtonTooltip() const;

    /**
     * Returns the menu to use for the menu button for this panel, or NULLPTR if
     * no menu button is required.
     *
     * \since QGIS 3.12
     */
    virtual QMenu *menuButtonMenu();

  signals:

    /**
     * Emitted when the panel is accepted by the user.
     * \param panel The panel widget that was accepted.
     * \note This argument is normally raised with emit panelAccepted(this)
     * so that callers can retrieve the widget easier in calling code.
     * \note this is emitted only when this panel is accepted, and is not emitted for
     * child panels. For example, if this panel opens a second stacked panel, then this panel
     * will not emit panelAccepted when the second panel is accepted.
     */
    void panelAccepted( QgsPanelWidget *panel );

    /**
     * Emit when you require a panel to be show in the interface.
     * \param panel The panel widget to show.
     * \note If you are connected to this signal you should also connect
     * given panels showPanel signal as they can be nested.
     */
    void showPanel( QgsPanelWidget *panel );

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
     * If dock mode is TRUE this method will emit the showPanel signal
     * for connected slots to handle the open event.
     *
     * If dock mode is FALSE this method will open a dialog
     * and block the user.
     *
     * \param panel The panel widget to open.
     */
    void openPanel( QgsPanelWidget *panel );

    /**
     * Accept the panel. Causes panelAccepted to be emitted.
     * Widgets are normally removed form the interface using the panel manager or the caller.
     */
    void acceptPanel();

  protected:
    /**
     * \brief Overridden key press event to handle the esc event on the widget.
     * \param event The key event
     */
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    bool mAutoDelete = true;
    QString mPanelTitle;
    bool mDockMode = false;
};


/**
 * \ingroup gui
 * \brief Wrapper widget for existing widgets which can't have
 * the inheritance tree changed, e.g dialogs.
 *
 * \note Generally you should use the QgsPanelWidget class if you can
 * and only use this wrapper if you can't update your code.
 */
class GUI_EXPORT QgsPanelWidgetWrapper : public QgsPanelWidget
{
    Q_OBJECT
  public:
    /**
     * \brief Wrapper widget for existing widgets which can't have
     * the inheritance tree changed, e.g dialogs.
     * \param widget The widget to wrap.
     * \param parent The parent widget.
     */
    QgsPanelWidgetWrapper( QWidget *widget, QWidget *parent = nullptr );

    /**
     * Returns the internal widget that is wrapped in this panel.
     * \returns The internal widget. Can be NULLPTR.
     */
    QWidget *widget() { return mWidget; }

  private:
    QWidget *mWidget = nullptr;
};

#endif // QGSPANELWIDGET_H
