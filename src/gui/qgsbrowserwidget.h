/***************************************************************************
    qgsbrowserwidget.h
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSERWIDGET_H
#define QGSBROWSERWIDGET_H

#include "ui_qgsbrowserwidgetbase.h"
#include "qgsmimedatautils.h"
#include "qgspanelwidget.h"
#include "qgis_gui.h"

class QgsBrowserGuiModel;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserProxyModel;
class QgsMessageBar;
class QgsDataItemGuiContext;

class QModelIndex;
class QItemSelection;

/**
 * \ingroup gui
 * \brief A widget showing a browser tree view along with toolbar and toggleable properties pane.
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsBrowserWidget : public QgsPanelWidget, private Ui::QgsBrowserWidgetBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserWidget
      * \param browserModel instance of the (shared) browser model
      * \param parent parent widget
      */
    explicit QgsBrowserWidget( QgsBrowserGuiModel *browserModel, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsBrowserWidget() override;

    /**
     * Sets a message \a bar to use alongside the widget. Setting this allows items
     * to utilize the message bar to provide non-blocking feedback to users, e.g.
     * success or failure of actions.
     *
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget.
     *
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar();

    /**
     * Sets the customization for data items based on item's data provider key
     *
     * By default browser model shows all items from all available data items provider and few special
     * items (e.g. Favorites). To customize the behavior, set the filter to not load certain data items.
     * The items that are not based on data item providers (e.g. Favorites, Home) have
     * prefix "special:"
     *
     * Used in the proxy browser model to hide items
     */
    void setDisabledDataItemsKeys( const QStringList &filter );

  public slots:

    /**
     * Update project home directory
     * \note Not available in Python bindings.
     */
    void updateProjectHome() SIP_SKIP;

    /**
     * Sets the selection to \a index and expands it.
     *
     * \note Not available in Python bindings.
     */
    void setActiveIndex( const QModelIndex &index ) SIP_SKIP;

    /**
     * Refreshes the browser model and view.
     */
    void refresh();

    // keep the stable API slim for now!
#ifndef SIP_RUN

  signals:
    //! Emitted when a file needs to be opened
    void openFile( const QString &fileName, const QString &fileTypeHint = QString() );
    //! Emitted when drop uri list needs to be handled
    void handleDropUriList( const QgsMimeDataUtils::UriList & );
    //! Connections changed in the browser
    void connectionsChanged();

#endif

  protected:
    void showEvent( QShowEvent *event ) override;

  private slots:
    void itemDoubleClicked( const QModelIndex &index );
    void onOptionsChanged();

    //! Show context menu
    void showContextMenu( QPoint );

    //! Show/hide filter widget
    void showFilterWidget( bool visible );
    //! Enable/disable properties widget
    void enablePropertiesWidget( bool enable );

    void propertiesWidgetToggled( bool enabled );

    //! Sets filter syntax
    void setFilterSyntax( QAction * );
    //! Sets filter case sensitivity
    void setCaseSensitive( bool caseSensitive );
    //! Apply filter to the model
    void setFilter();

    //! Add selected layers to the project
    void addSelectedLayers();
    //! Show the layer properties
    void showProperties();
    //! Hide current item
    void hideItem();

  private:
    //! Selection has changed
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //! Splitter has been moved
    void splitterMoved();
    //! Refresh the model
    void refreshModel( const QModelIndex &index );
    //! Add a layer
    void addLayer( QgsLayerItem *layerItem );
    //! Clear the properties widget
    void clearPropertiesWidget();
    //! Sets the properties widget
    void setPropertiesWidget();

    //! Count selected items
    int selectedItemsCount();
    //! Settings prefix (the object name)
    QString settingsSection() { return objectName().toLower(); }

    QgsDataItemGuiContext createContext();

    QgsDockBrowserTreeView *mBrowserView = nullptr;
    QgsBrowserGuiModel *mModel = nullptr;
    QgsBrowserProxyModel *mProxyModel = nullptr;
    QString mInitPath;

    QgsMessageBar *mMessageBar = nullptr;
    QStringList mDisabledDataItemsKeys;

    friend class QgsBrowserDockWidget;
};

#endif // QGSBROWSERWIDGET_H
