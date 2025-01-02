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

#include "qgsbrowsertreeview.h"
#include "qgsdockwidget.h"
#include "qgsmimedatautils.h"
#include "qgis_gui.h"

class QgsMessageBar;
class QgsBrowserWidget;

/**
 * \ingroup gui
 * \brief A dock widget containing a QgsBrowserWidget for navigating and managing data sources.
 */
class GUI_EXPORT QgsBrowserDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    /**
      * Constructor for QgsBrowserDockWidget
      * \param name name of the widget
      * \param browserModel instance of the (shared) browser model
      * \param parent parent widget
      */
    explicit QgsBrowserDockWidget( const QString &name, QgsBrowserGuiModel *browserModel, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsBrowserDockWidget() override;

    /**
     * Returns a pointer to the QgsBrowserWidget used by the dock widget.
     *
     * \since QGIS 3.22
     */
    QgsBrowserWidget *browserWidget();

    /**
     * Add directory to favorites.
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead.
     */
    Q_DECL_DEPRECATED void addFavoriteDirectory( const QString &favDir, const QString &name = QString() ) SIP_DEPRECATED;

    /**
     * Sets a message \a bar to use alongside the dock widget. Setting this allows items
     * to utilize the message bar to provide non-blocking feedback to users, e.g.
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

    /**
     * Sets the customization for data items based on item's data provider key
     *
     * By default browser model shows all items from all available data items provider and few special
     * items (e.g. Favorites). To customize the behavior, set the filter to not load certain data items.
     * The items that are not based on data item providers (e.g. Favorites, Home) have
     * prefix "special:"
     *
     * Used in the proxy browser model to hide items
     *
     * \since QGIS 3.12
     */
    void setDisabledDataItemsKeys( const QStringList &filter );

  public slots:

    /**
     * Adds the layer corresponding to the specified model \a index.
     *
     * Returns TRUE if the index was successfully intrepreted as a map layer and loaded, or
     * FALSE if the index is not a map layer or could not be loaded.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 - retrieve the QgsLayerItem itself and manually add to project.
     */
    Q_DECL_DEPRECATED bool addLayerAtIndex( const QModelIndex &index ) SIP_DEPRECATED;

    /**
     * Show context menu.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void showContextMenu( QPoint ) SIP_DEPRECATED;

    /**
     * Add current item to favorite.
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead.
     */
    Q_DECL_DEPRECATED void addFavorite() SIP_DEPRECATED;

    /**
     * Add directory from file dialog to favorite.
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead.
     */
    Q_DECL_DEPRECATED void addFavoriteDirectory() SIP_DEPRECATED;

    /**
     * Remove from favorite.
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 - use the methods in QgsBrowserModel instead.
     */
    Q_DECL_DEPRECATED void removeFavorite() SIP_DEPRECATED;

    /**
     * Refresh the browser model and view.
    */
    void refresh();

    /**
     * Show/hide filter widget.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void showFilterWidget( bool visible ) SIP_DEPRECATED;

    /**
     * Enable/disable properties widget.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void enablePropertiesWidget( bool enable ) SIP_DEPRECATED;

    /**
     * Sets filter syntax.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void setFilterSyntax( QAction * ) SIP_DEPRECATED;

    /**
     * Sets filter case sensitivity.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void setCaseSensitive( bool caseSensitive ) SIP_DEPRECATED;

    /**
     * Apply filter to the model.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void setFilter() SIP_DEPRECATED;

    /**
     * Sets the selection to \a index and expand it.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void setActiveIndex( const QModelIndex &index ) SIP_DEPRECATED;

    /**
     * Update project home directory.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void updateProjectHome() SIP_DEPRECATED;

    /**
     * Add selected layers to the project
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
    */
    Q_DECL_DEPRECATED void addSelectedLayers() SIP_DEPRECATED;

    /**
     * Show the layer properties.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
    */
    Q_DECL_DEPRECATED void showProperties() SIP_DEPRECATED;

    /**
     * Hide current item.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
    */
    Q_DECL_DEPRECATED void hideItem() SIP_DEPRECATED;

    /**
     * Toggle fast scan
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED void toggleFastScan() SIP_DEPRECATED;

    /**
     * Selection has changed.
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0 -- this method is not intended for public use.
     */
    Q_DECL_DEPRECATED void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected ) SIP_DEPRECATED;

    /**
     * Splitter has been moved.
     *
     * \deprecated QGIS 3.40. No longer used.
     */
    Q_DECL_DEPRECATED void splitterMoved() SIP_DEPRECATED;

  signals:
    //! Emitted when a file needs to be opened
    void openFile( const QString &fileName, const QString &fileTypeHint = QString() );
    //! Emitted when drop uri list needs to be handled
    void handleDropUriList( const QgsMimeDataUtils::UriList &uris );
    //! Connections changed in the browser
    void connectionsChanged();

  private:
    QgsBrowserWidget *mWidget = nullptr;
};

#endif // QGSBROWSERDOCKWIDGET_H
