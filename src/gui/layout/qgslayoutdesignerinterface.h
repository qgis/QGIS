/***************************************************************************
    qgslayoutdesignerinterface.h
     ---------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTDESIGNERINTERFACE_H
#define QGSLAYOUTDESIGNERINTERFACE_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QObject>

class QgsLayout;
class QgsLayoutView;
class QgsLayoutItem;
class QgsMessageBar;
class QgsMasterLayoutInterface;
class QMenu;
class QDockWidget;
class QToolBar;

/**
 * \ingroup gui
 * \class QgsLayoutDesignerInterface
 * A common interface for layout designer dialogs and widgets.
 *
 * Provides a common interface and stable API for layout designer dialogs and widgets.
 * This interface can be used by plugins and scripts to interact with
 * open layout designer dialogs.
 *
 * \note Layout designer dialogs are transitory. They are created only on demand
 * (when a user opens the dialog) and are deleted as soon as the user closes the dialog.
 * There can be multiple designer dialogs open at any one time, and each is a separate
 * instance of the dialog and QgsLayoutDesignerInterface. Accordingly, plugins must
 * take care to react to newly created designer dialogs and apply their customizations
 * to all newly created dialogs. This can be done by listening for the QgisInterface::layoutDesignerOpened
 * signal. Plugins must also listen for the QgisInterface::layoutDesignerWillBeClosed
 * signal and gracefully cleanup any customizations before the designer dialog is
 * deleted.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutDesignerInterface: public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutDesignerInterface.
     */
    QgsLayoutDesignerInterface( QObject *parent SIP_TRANSFERTHIS = nullptr )
      : QObject( parent )
    {}

    /**
     * Returns the current layout displayed in the designer.
     * \see view()
     */
    virtual QgsLayout *layout() = 0;

    /**
     * Returns the master layout displayed in the designer.
     * \see layout()
     */
    virtual QgsMasterLayoutInterface *masterLayout() = 0;

    /**
     * Returns the layout view utilized by the designer.
     * \see layout()
     */
    virtual QgsLayoutView *view() = 0;

    /**
     * Returns the designer's message bar.
     */
    virtual QgsMessageBar *messageBar() = 0;

    /**
     * Selects the specified \a items.
     */
    virtual void selectItems( const QList< QgsLayoutItem * > &items ) = 0;

    /**
     * Toggles whether the atlas preview mode should be \a enabled in the designer.
     *
     * \see atlasPreviewModeEnabled()
     * \since QGIS 3.4
     */
    virtual void setAtlasPreviewEnabled( bool enabled ) = 0;

    /**
     * Returns whether the atlas preview mode is enabled in the designer.
     *
     * \see setAtlasPreviewEnabled()
     * \since QGIS 3.4
     */
    virtual bool atlasPreviewEnabled() const = 0;

    /**
     * Shows the configuration widget for the specified layout \a item.
     *
     * If \a bringPanelToFront is true, then the item properties panel will be automatically
     * shown and raised to the top of the interface.
     *
     * \since QGIS 3.4
     */
    virtual void showItemOptions( QgsLayoutItem *item, bool bringPanelToFront = true ) = 0;

    // Menus and toolbars

    /**
     * Returns a reference to the designer's "Layout" menu.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see editMenu()
     * \see viewMenu()
     * \see itemsMenu()
     * \see atlasMenu()
     * \see reportMenu()
     * \see settingsMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *layoutMenu() = 0;

    /**
     * Returns a reference to the designer's "Edit" menu.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutMenu()
     * \see viewMenu()
     * \see itemsMenu()
     * \see atlasMenu()
     * \see reportMenu()
     * \see settingsMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *editMenu() = 0;

    /**
     * Returns a reference to the designer's "View" menu.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutMenu()
     * \see editMenu()
     * \see itemsMenu()
     * \see atlasMenu()
     * \see reportMenu()
     * \see settingsMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *viewMenu() = 0;

    /**
     * Returns a reference to the designer's "Items" menu.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutMenu()
     * \see editMenu()
     * \see viewMenu()
     * \see atlasMenu()
     * \see reportMenu()
     * \see settingsMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *itemsMenu() = 0;

    /**
     * Returns a reference to the designer's "Atlas" menu.
     *
     * Note that this may not exist or may be hidden if the designer is in report mode.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutMenu()
     * \see editMenu()
     * \see viewMenu()
     * \see itemsMenu()
     * \see reportMenu()
     * \see settingsMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *atlasMenu() = 0;

    /**
     * Returns a reference to the designer's "Report" menu.
     *
     * Note that this may not exist or may be hidden if the designer is not in report mode.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutMenu()
     * \see editMenu()
     * \see viewMenu()
     * \see itemsMenu()
     * \see atlasMenu()
     * \see settingsMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *reportMenu() = 0;

    /**
     * Returns a reference to the designer's "Settings" menu.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutMenu()
     * \see editMenu()
     * \see viewMenu()
     * \see itemsMenu()
     * \see atlasMenu()
     * \see reportMenu()
     *
     * \since QGIS 3.4
     */
    virtual QMenu *settingsMenu() = 0;

    /**
     * Returns a reference to the designer's "Layout" toolbar.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see navigationToolbar()
     * \see actionsToolbar()
     * \see atlasToolbar()
     *
     * \since QGIS 3.4
     */
    virtual QToolBar *layoutToolbar() = 0;

    /**
     * Returns a reference to the designer's "Navigation" toolbar.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutToolbar()
     * \see actionsToolbar()
     * \see atlasToolbar()
     *
     * \since QGIS 3.4
     */
    virtual QToolBar *navigationToolbar() = 0;

    /**
     * Returns a reference to the designer's "Actions" toolbar.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutToolbar()
     * \see navigationToolbar()
     * \see atlasToolbar()
     *
     * \since QGIS 3.4
     */
    virtual QToolBar *actionsToolbar() = 0;

    /**
     * Returns a reference to the designer's "Atlas" toolbar.
     *
     * Note that this toolbar may not exist or may be hidden if the
     * designer is in report mode.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see layoutToolbar()
     * \see navigationToolbar()
     * \see actionsToolbar()
     *
     * \since QGIS 3.4
     */
    virtual QToolBar *atlasToolbar() = 0;

    /**
     * Adds a \a dock widget to the layout designer, in the specified dock \a area.
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see removeDockWidget()
     *
     * \since QGIS 3.4
     */
    virtual void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dock ) = 0;

    /**
     * Removes the specified \a dock widget from layout designer (without deleting it).
     *
     * \note See class documentation for notes regarding handling customization of designer dialogs.
     *
     * \see addDockWidget()
     *
     * \since QGIS 3.4
     */
    virtual void removeDockWidget( QDockWidget *dock ) = 0;

  public slots:

    /**
     * Closes the layout designer.
     */
    virtual void close() = 0;

    /**
     * Toggles whether or not the rulers should be \a visible in the designer.
     *
     * \since QGIS 3.4
     */
    virtual void showRulers( bool visible ) = 0;


};

#endif // QGSLAYOUTDESIGNERINTERFACE_H
