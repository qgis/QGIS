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
#include "qgis_sip.h"
#include "qgis.h"
#include <QList>
#include <QWidget>
#include <QMimeData>
#include <QString>
#include <QMenu>

class QgsDataItem;
class QgsMessageBar;
class QgsLayerItem;
class QgsBrowserTreeView;

/**
 * \class QgsDataItemGuiContext
 * \ingroup gui
 *
 * \brief Encapsulates the context in which a QgsDataItem is shown within the application GUI.
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
    QgsMessageBar *messageBar() const;

    /**
     * Sets the associated message \a bar.
     *
     * This bar can be used to provide non-blocking feedback to users.
     *
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the associated view.
     *
     * \see setView()
     * \since QGIS 3.28
     */
    QgsBrowserTreeView *view() const;

    /**
     * Sets the associated \a view.
     *
     * \see view()
     * \since QGIS 3.28
     */
    void setView( QgsBrowserTreeView *view );

  private:

    QgsMessageBar *mMessageBar = nullptr;

    QgsBrowserTreeView *mView = nullptr;
};

Q_DECLARE_METATYPE( QgsDataItemGuiContext );

/**
 * \class QgsDataItemGuiProvider
 * \ingroup gui
 *
 * \brief Abstract base class for providers which affect how QgsDataItem items behave
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
     * Returns the provider's precedence to use when populating context menus via calls to populateContextMenu().
     *
     * Providers which return larger values will be called AFTER other providers when the menu is being populated.
     * This allows them to nicely insert their corresponding menu items in the desired location with respect to
     * existing items added by other providers.
     *
     * The default implementation returns 0.
     *
     * \since QGIS 3.22
     */
    virtual int precedenceWhenPopulatingMenus() const;

    /**
     * Sets a new \a name for the item, and returns TRUE if the item was successfully renamed.
     *
     * Items which implement this method should return the QgsDataItem::Rename capability.
     *
     * The default implementation does nothing.
     *
     * \since QGIS 3.10
     */
    virtual bool rename( QgsDataItem *item, const QString &name, QgsDataItemGuiContext context );

    /**
     * Tries to permanently delete map layer representing the given item.
     * Returns TRUE if the layer was successfully deleted.
     *
     * Items which implement this method should return the QgsDataItem::Delete capability.
     *
     * The default implementation does nothing.
     *
     * \since QGIS 3.10
     */
    virtual bool deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context );

    /**
     * Called when a user double clicks on an \a item. Providers should return TRUE
     * if the double-click was handled and do not want other providers to handle the
     * double-click, and to prevent the default double-click behavior for items.
     */
    virtual bool handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext context );

    /**
     * Providers should return TRUE if the drops are allowed (handleDrop() should be
     * implemented in that case as well).
     * \since QGIS 3.10
     */
    virtual bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context );

    /**
     * Called when a user drops on an \a item. Providers should return TRUE
     * if the drop was handled and do not want other providers to handle the
     * drop, and to prevent the default drop behavior for items.
     * \since QGIS 3.10
     */
    virtual bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action );

    /**
     * Creates source widget from data item for QgsBrowserPropertiesWidget
     * By default it returns nullptr.
     * Caller takes responsibility of deleting created.
     *
     * The function is replacement of QgsDataItem::paramWidget()
     *
     * \since QGIS 3.10
     */
    virtual QWidget *createParamWidget( QgsDataItem *item, QgsDataItemGuiContext context ) SIP_FACTORY;

    /**
     * Notify the user showing a \a message with \a title and \a level
     * If the context has a message bar the message will be shown in the message bar
     * else a message dialog will be used.
     *
     * Since QGIS 3.18, the optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. A duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     *
     * \since QGIS 3.16
     */
    static void notify( const QString &title, const QString &message, QgsDataItemGuiContext context, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = -1, QWidget *parent = nullptr );
};

#endif // QGSDATAITEMGUIPROVIDER_H
