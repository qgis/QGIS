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
#include "qgsoptionsdialogbase.h"
#include "qgsguiutils.h"
#include "qgsmimedatautils.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QgsBrowserDockWidget;
class QgsRasterLayer;
class QgsMapCanvas;

namespace Ui
{
  class QgsDataSourceManagerDialog;
}

/** \ingroup gui
 * The QgsDataSourceManagerDialog class embeds the browser panel and all
 * the provider dialogs.
 * The dialog does not handle layer addition directly but emits signals that
 * need to be forwarded to the QGIS application to be handled.
 */
class GUI_EXPORT QgsDataSourceManagerDialog : public QgsOptionsDialogBase
{
    Q_OBJECT

  public:
    explicit QgsDataSourceManagerDialog( QgsMapCanvas *mapCanvas, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    ~QgsDataSourceManagerDialog();

    /**
     * @brief openPage open a given page in the dialog
     * @param pageName the page name, usually the provider name or "browser" (for the browser panel)
     *        or "ogr" (vector layers) or "raster" (raster layers)
     */
    void openPage( QString pageName );

  public slots:

    //! Sync current page with the leftbar list
    void setCurrentPage( int index );

    //! A raster layer was added: for signal forwarding to QgisApp
    void rasterLayerAdded( QString const &uri, QString const &baseName, QString const &providerKey );
    //! A vector layer was added: for signal forwarding to QgisApp
    void vectorLayerAdded( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    //! One or more vector layer were added: for signal forwarding to QgisApp
    void vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    //! Reset current page to previously selected page
    void setPreviousPage();

  signals:
    //! Emitted when a raster layer was selected for addition: for signal forwarding to QgisApp
    void addRasterLayer( QString const &uri, QString const &baseName, QString const &providerKey );
    //! Emitted when the user wants to select a raster layer: for signal forwarding to QgisApp
    void addRasterLayer( );
    //! Emitted when a vector layer was selected for addition: for signal forwarding to QgisApp
    void addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    //! Replace the selected layer by a vector layer defined by uri, layer name, data source uri
    void replaceSelectedVectorLayer( const QString &oldId, const QString &uri, const QString &layerName, const QString &provider );
    //! Emitted when a WFS layer was selected for addition: for signal forwarding to QgisApp
    void addWfsLayer( const QString &uri, const QString &typeName );
    //! Emitted when a AFS layer was selected for addition: for signal forwarding to QgisApp
    void addAfsLayer( const QString &uri, const QString &typeName );
    //! Emitted when a one or more layer were selected for addition: for signal forwarding to QgisApp
    void addVectorLayers( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    //! Emitted when the dialog is busy: for signal forwarding to QgisApp
    void showProgress( int progress, int totalSteps );
    //! Emitted when a status message needs to be shown: for signal forwarding to QgisApp
    void showStatusMessage( const QString &message );
    //! Emitted when a DB layer was selected for addition: for signal forwarding to QgisApp
    void addDatabaseLayers( QStringList const &layerPathList, QString const &providerKey );
    //! Emitted when a file needs to be opened
    void openFile( const QString & );
    //! Emitted when drop uri list needs to be handled from the browser
    void handleDropUriList( const QgsMimeDataUtils::UriList & );
    //! Update project home directory
    void updateProjectHome();

  private:
    //! Return the dialog from the provider
    QDialog *providerDialog( QString const providerKey, QString const providerName, QString const icon, QString title = QString( ) );
    void addDbProviderDialog( QString const providerKey, QString const providerName, QString const icon, QString title = QString( ) );
    void addRasterProviderDialog( QString const providerKey, QString const providerName, QString const icon, QString title = QString( ) );
    Ui::QgsDataSourceManagerDialog *ui;
    QgsBrowserDockWidget *mBrowserWidget = nullptr;
    //! Map canvas
    QgsMapCanvas *mMapCanvas = nullptr;
    int mPreviousRow;
    QStringList mPageNames;
};

#endif // QGSDATASOURCEMANAGERDIALOG_H
