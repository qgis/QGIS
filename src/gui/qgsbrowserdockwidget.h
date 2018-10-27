/***************************************************************************
    qgsbrowserdockwidget.h
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
#ifndef QGSBROWSERDOCKWIDGET_H
#define QGSBROWSERDOCKWIDGET_H

#include "ui_qgsbrowserdockwidgetbase.h"
#include "ui_qgsbrowserlayerpropertiesbase.h"
#include "ui_qgsbrowserdirectorypropertiesbase.h"
#include "ui_qgsbrowserpropertiesdialogbase.h"

#include "qgsdataitem.h"
#include "qgsbrowsertreeview.h"
#include "qgsdockwidget.h"
#include "qgsbrowserdockwidget_p.h"
#include "qgis_gui.h"
#include <QSortFilterProxyModel>

class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserProxyModel;
class QgsMessageBar;

/**
 * \ingroup gui
 * The QgsBrowserDockWidget class
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsBrowserDockWidget : public QgsDockWidget, private Ui::QgsBrowserDockWidgetBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserDockWidget
      * \param name name of the widget
      * \param browserModel instance of the (shared) browser model
      * \param parent parent widget
      */
    explicit QgsBrowserDockWidget( const QString &name, QgsBrowserModel *browserModel, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsBrowserDockWidget() override;

    /**
     * Add directory to favorites.
     * \deprecated will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead
     */
    Q_DECL_DEPRECATED void addFavoriteDirectory( const QString &favDir, const QString &name = QString() ) SIP_DEPRECATED;

    /**
     * Sets a message \a bar to use alongside the dock widget. Setting this allows items
     * to utilise the message bar to provide non-blocking feedback to users, e.g.
     * success or failure of actions.
     *
     * \see messageBar()
     *
     * \since QGIS 3.6
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the dock.
     *
     * \see setMessageBar()
     *
     * \since QGIS 3.6
     */
    QgsMessageBar *messageBar();

  public slots:

    /**
     * Adds the layer corresponding to the specified model \a index.
     *
     * Returns true if the index was successfully intrepreted as a map layer and loaded, or
     * false if the index is not a map layer or could not be loaded.
     */
    bool addLayerAtIndex( const QModelIndex &index );

    //! Show context menu
    void showContextMenu( QPoint );

    /**
     * Add current item to favorite.
     * \deprecated will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead
     */
    Q_DECL_DEPRECATED void addFavorite() SIP_DEPRECATED;

    /**
     * Add directory from file dialog to favorite.
     * \deprecated will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead
     */
    Q_DECL_DEPRECATED void addFavoriteDirectory() SIP_DEPRECATED;

    /**
     * Remove from favorite.
     * \deprecated will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead
     */
    Q_DECL_DEPRECATED void removeFavorite() SIP_DEPRECATED;

    //! Refresh browser view model (and view)
    void refresh();

    //! Show/hide filter widget
    void showFilterWidget( bool visible );
    //! Enable/disable properties widget
    void enablePropertiesWidget( bool enable );
    //! Sets filter syntax
    void setFilterSyntax( QAction * );
    //! Sets filter case sensitivity
    void setCaseSensitive( bool caseSensitive );
    //! Apply filter to the model
    void setFilter();
    //! Update project home directory
    void updateProjectHome();

    //! Add selected layers to the project
    void addSelectedLayers();
    //! Show the layer properties
    void showProperties();
    //! Hide current item
    void hideItem();

    /**
     * Toggle fast scan
     * \deprecated will be removed in QGIS 4.0
     */
    Q_DECL_DEPRECATED void toggleFastScan() SIP_DEPRECATED;

    // TODO QGIS 4.0: make these private

    //! Selection has changed
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //! Splitter has been moved
    void splitterMoved();

  signals:
    //! Emitted when a file needs to be opened
    void openFile( const QString &fileName, const QString &fileTypeHint = QString() );
    //! Emitted when drop uri list needs to be handled
    void handleDropUriList( const QgsMimeDataUtils::UriList & );
    //! Connections changed in the browser
    void connectionsChanged();

  protected:
    //! Show event override
    void showEvent( QShowEvent *event ) override;

  private slots:
    void itemDoubleClicked( const QModelIndex &index );

  private:
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

    QgsDockBrowserTreeView *mBrowserView = nullptr;
    QgsBrowserModel *mModel = nullptr;
    QgsBrowserProxyModel *mProxyModel = nullptr;
    QString mInitPath;
    bool mPropertiesWidgetEnabled;
    // height fraction
    float mPropertiesWidgetHeight;

    QgsMessageBar *mMessageBar = nullptr;

};



#endif // QGSBROWSERDOCKWIDGET_H
