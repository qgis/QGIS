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

/** \ingroup gui
 * The QgsDataSourceManagerDialog class embeds the browser panel and all
 * the provider dialogs.
 * The dialog does not handle layer addition directly but emits signals that
 * need to be forwarded to the QGIS application to be handled.
 * \since QGIS 3.0
 * @note not available in Python bindings
 */
class GUI_EXPORT QgsDataSourceManagerDialog : public QgsOptionsDialogBase, private Ui::QgsDataSourceManagerDialog
{
    Q_OBJECT

  public:

    /** QgsDataSourceManagerDialog constructor
      * \param parent the object
      * \param canvas a pointer to the map canvas
      * \param fl window flags
      */
    explicit QgsDataSourceManagerDialog( QWidget *parent = nullptr, QgsMapCanvas *canvas = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    ~QgsDataSourceManagerDialog();

    /**
     * @brief openPage open a given page in the dialog
     * \param pageName the page name, usually the provider name or "browser" (for the browser panel)
     *        or "ogr" (vector layers) or "raster" (raster layers)
     */
    void openPage( QString pageName );

  public slots:

    //! Sync current page with the leftbar list
    void setCurrentPage( int index );

    //! A raster layer was added: for signal forwarding to QgisApp
    //! TODO: use this with an internal source select dialog instead of forwarding the whole raster selection to app
    void rasterLayerAdded( QString const &uri, QString const &baseName, QString const &providerKey );
    //! A vector layer was added: for signal forwarding to QgisApp
    void vectorLayerAdded( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    //! One or more vector layer were added: for signal forwarding to QgisApp
    void vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    //! Reset current page to previously selected page
    void setPreviousPage();
    //! Refresh the browser view
    void refresh();

  protected:
    virtual void showEvent( QShowEvent *event ) override;

  signals:
    //! Emitted when a raster layer was selected for addition: for signal forwarding to QgisApp
    void addRasterLayer( const QString &uri, const QString &baseName, const QString &providerKey );
    //! Emitted when the user wants to select a raster layer: for signal forwarding to QgisApp
    void addRasterLayer();
    //! Emitted when a vector layer was selected for addition: for signal forwarding to QgisApp
    void addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    //! Replace the selected layer by a vector layer defined by uri, layer name, data source uri
    void replaceSelectedVectorLayer( const QString &oldId, const QString &uri, const QString &layerName, const QString &provider );
    //! Emitted when a one or more layer were selected for addition: for signal forwarding to QgisApp
    void addVectorLayers( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    //! Emitted when the dialog is busy: for signal forwarding to QgisApp
    void showProgress( int progress, int totalSteps );
    //! Emitted when a status message needs to be shown: for signal forwarding to QgisApp
    void showStatusMessage( const QString &message );
    //! Emitted when a DB layer was selected for addition: for signal forwarding to QgisApp
    void addDatabaseLayers( const QStringList &layerPathList, const QString &providerKey );
    //! Emitted when a file needs to be opened
    void openFile( const QString & );
    //! Emitted when drop uri list needs to be handled from the browser
    void handleDropUriList( const QgsMimeDataUtils::UriList & );
    //! Update project home directory
    void updateProjectHome();
    //! Emitted when a connection has changed inside the provider dialogs
    //! This signal is normally forwarded to the application to notify other
    //! browsers that they need to refresh their connections list
    void connectionsChanged();
    //! One or more provider connections have changed and the
    //! dialogs should be refreshed
    void providerDialogsRefreshRequested();

  private:
    void addProviderDialog( QgsAbstractDataSourceWidget *dlg, const QString &providerKey, const QString &providerName, const QIcon &icon, QString toolTip = QString() );
    void makeConnections( QgsAbstractDataSourceWidget *dlg, const QString &providerKey );
    Ui::QgsDataSourceManagerDialog *ui;
    QgsBrowserDockWidget *mBrowserWidget = nullptr;
    int mPreviousRow;
    QStringList mPageNames;
    // Map canvas
    QgsMapCanvas *mMapCanvas = nullptr;


};

#endif // QGSDATASOURCEMANAGERDIALOG_H
