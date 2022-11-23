/***************************************************************************
    qgsdatasourcemanagerdialog.h - datasource manager dialog

    ---------------------
    begin                : May 19, 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATASOURCEMANAGERDIALOG_H
#define QGSDATASOURCEMANAGERDIALOG_H

#include <QList>
#include <QDialog>
#include "ui_qgsdatasourcemanagerdialog.h"
#include "qgsoptionsdialogbase.h"
#include "qgsguiutils.h"
#include "qgsmimedatautils.h"
#include "qgshelp.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QgsBrowserDockWidget;
class QgsRasterLayer;
class QgsMapCanvas;
class QgsAbstractDataSourceWidget;
class QgsLayerMetadataSearchWidget;
class QgsBrowserGuiModel;
class QgsMessageBar;

/**
 * \ingroup gui
 * \brief The QgsDataSourceManagerDialog class embeds the browser panel and all
 * the provider dialogs.
 * The dialog does not handle layer addition directly but emits signals that
 * need to be forwarded to the QGIS application to be handled.
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsDataSourceManagerDialog : public QgsOptionsDialogBase, private Ui::QgsDataSourceManagerDialog
{
    Q_OBJECT

  public:

    /**
     * QgsDataSourceManagerDialog constructor
      * \param browserModel instance of the (shared) browser model
      * \param parent the object
      * \param canvas a pointer to the map canvas
      * \param fl window flags
      */
    explicit QgsDataSourceManagerDialog( QgsBrowserGuiModel *browserModel, QWidget *parent = nullptr, QgsMapCanvas *canvas = nullptr, Qt::WindowFlags fl = Qt::Window );
    ~QgsDataSourceManagerDialog() override;

    /**
     * Open a given page in the dialog
     * \param pageName the page name, usually the provider name or "browser" (for the browser panel)
     *        or "ogr" (vector layers) or "raster" (raster layers)
     */
    void openPage( const QString &pageName );

    //! Returns the dialog's message bar
    QgsMessageBar *messageBar() const;

  public slots:

    /**
     * Raise, unminimize and activate this window.
     *
     * \since QGIS 3.28
     */
    void activate();

    //! Sync current page with the leftbar list
    void setCurrentPage( int index );

    // TODO: use this with an internal source select dialog instead of forwarding the whole raster selection to app

    //! A raster layer was added: for signal forwarding to QgisApp
    void rasterLayerAdded( QString const &uri, QString const &baseName, QString const &providerKey );

    /**
     * One or more raster layer were added: for signal forwarding to QgisApp
     * \since QGIS 3.20
     */
    void rasterLayersAdded( const QStringList &layersList );
    //! A vector layer was added: for signal forwarding to QgisApp
    void vectorLayerAdded( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    //! One or more vector layer were added: for signal forwarding to QgisApp
    void vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    //! Reset current page to previously selected page
    void setPreviousPage();
    //! Refresh the browser view
    void refresh();

    /**
     * Resets the interface of the datasource manager after reopening the dialog.
     *
     * Will clear the selection of embedded all source selection widgets.
     *
     * \since QGIS 3.10
     */
    void reset();

  protected:
    void showEvent( QShowEvent *event ) override;

  signals:

    /**
     * Emitted when a one or more layer were selected for addition: for signal forwarding to QgisApp
     * \since QGIS 3.20
     */
    void addRasterLayers( const QStringList &layersList );
    //! Emitted when a raster layer was selected for addition: for signal forwarding to QgisApp
    void addRasterLayer( const QString &uri, const QString &baseName, const QString &providerKey );

    //! Emitted when a vector layer was selected for addition: for signal forwarding to QgisApp
    void addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );

    /**
     * Emitted when a mesh layer was selected for addition: for signal forwarding to QgisApp
     * \since QGIS 3.4
     */
    void addMeshLayer( const QString &uri, const QString &baseName, const QString &providerKey );

    /**
     * Emitted when a vector tile layer was selected for addition: for signal forwarding to QgisApp
     * \since QGIS 3.14
     */
    void addVectorTileLayer( const QString &uri, const QString &baseName );

    /**
     * Emitted when a point cloud layer was selected for addition: for signal forwarding to QgisApp
     * \since QGIS 3.18
     */
    void addPointCloudLayer( const QString &pointCloudLayerPath, const QString &baseName, const QString &providerKey );

    //! Replace the selected layer by a vector layer defined by uri, layer name, data source uri
    void replaceSelectedVectorLayer( const QString &oldId, const QString &uri, const QString &layerName, const QString &provider );
    //! Emitted when a one or more layer were selected for addition: for signal forwarding to QgisApp
    void addVectorLayers( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );

    //! Emitted when a status message needs to be shown: for signal forwarding to QgisApp
    void showStatusMessage( const QString &message );
    //! Emitted when a DB layer was selected for addition: for signal forwarding to QgisApp
    void addDatabaseLayers( const QStringList &layerPathList, const QString &providerKey );
    //! Emitted when a file needs to be opened
    void openFile( const QString &fileName, const QString &fileTypeHint = QString() );
    //! Emitted when drop uri list needs to be handled from the browser
    void handleDropUriList( const QgsMimeDataUtils::UriList & );
    //! Update project home directory
    void updateProjectHome();

    /**
     * Emitted when a connection has changed inside the provider dialogs
     * This signal is normally forwarded to the application to notify other
     * browsers that they need to refresh their connections list
     */
    void connectionsChanged();

    /**
     * One or more provider connections have changed and the
     * dialogs should be refreshed
     */
    void providerDialogsRefreshRequested();

  private:
    void addProviderDialog( QgsAbstractDataSourceWidget *dlg, const QString &providerKey, const QString &providerName, const QString &text, const QIcon &icon, const QString &toolTip = QString() );
    void removeProviderDialog( const QString &providerName );
    void makeConnections( QgsAbstractDataSourceWidget *dlg, const QString &providerKey );
    Ui::QgsDataSourceManagerDialog *ui = nullptr;
    QgsBrowserDockWidget *mBrowserWidget = nullptr;
    QgsLayerMetadataSearchWidget *mLayerMetadataSearchWidget = nullptr;
    int mPreviousRow;
    QStringList mPageProviderKeys;
    QStringList mPageProviderNames;
    // Map canvas
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsBrowserGuiModel *mBrowserModel = nullptr;

};

#endif // QGSDATASOURCEMANAGERDIALOG_H
