/***************************************************************************
  qgsdataitemguiprovider.h
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMGUIPROVIDER_H
#define QGSDATAITEMGUIPROVIDER_H

#include "qgis_gui.h"
#include <QList>

class QString;
class QgsDataItem;
class QMenu;
class QgsMessageBar;

/**
 * \class QgsDataItemGuiContext
 * \ingroup gui
 *
 * Encapsulates the context in which a QgsDataItem is shown within the application GUI.
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsDataItemGuiContext
{
  public:

    /**
     * Constructor for QgsDataItemGuiContext.
     */
    QgsDataItemGuiContext() = default;

    /**
     * Returns the associated message bar.
     *
     * This bar can be used to provide non-blocking feedback to users.
     *
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar();

    /**
     * Sets the associated message \a bar.
     *
     * This bar can be used to provide non-blocking feedback to users.
     *
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

  private:

    QgsMessageBar *mMessageBar = nullptr;
};

/**
 * \class QgsDataItemGuiProvider
 * \ingroup gui
 *
 * Abstract base class for providers which affect how QgsDataItem items behave
 * within the application GUI.
 *
 * Providers must be registered via QgsDataItemGuiProviderRegistry.
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsDataItemGuiProvider
{
  public:

    virtual ~QgsDataItemGuiProvider() = default;

    /**
     * Returns the provider's name.
     */
    virtual QString name() = 0;

    /**
     * Called when the given context \a menu is being populated for the given \a item, allowing the provider
     * to add its own actions and submenus to the context menu. Additionally,
     * providers could potentially alter menus and actions added by other providers
     * if desired, or use standard QMenu API to insert their items and submenus into
     * the desired location within the context menu.
     *
     * The \a selectedItems list contains a list of ALL currently selected items within the browser view.
     * Subclasses can utilize this list in order to create actions which operate on multiple items
     * at once, e.g. to allow deletion of multiple layers from a database at once.
     *
     * When creating a context menu, this method is called for EVERY QgsDataItemGuiProvider
     * within the QgsDataItemGuiProviderRegistry. It is the QgsDataItemGuiProvider subclass'
     * responsibility to test the \a item and \a selectedItems for their properties and classes and decide what actions
     * (if any) are appropriate to add to the context \a menu.
     *
     * Care must be taken to correctly parent newly created sub menus and actions to the
     * provided \a menu to avoid memory leaks.
     *
     * The \a context argument gives the wider context under which the context menu is being shown,
     * and contains accessors for useful objects like the application message bar.
     *
     * The base class method has no effect.
     */
    virtual void populateContextMenu( QgsDataItem *item, QMenu *menu,
                                      const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context );

    /**
     * Called when a user double clicks on an \a item. Providers should return true
     * if the double-click was handled and do not want other providers to handle the
     * double-click, and to prevent the default double-click behavior for items.
     */
    virtual bool handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext context );

};

#endif // QGSDATAITEMGUIPROVIDER_H
