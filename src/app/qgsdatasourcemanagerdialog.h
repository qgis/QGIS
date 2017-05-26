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
#include "qgshelp.h"

class QgsBrowserDockWidget;
class QgsRasterLayer;
class QgsMapCanvas;

namespace Ui
{
  class QgsDataSourceManagerDialog;
}

class QgsDataSourceManagerDialog : public QgsOptionsDialogBase
{
    Q_OBJECT

  public:
    explicit QgsDataSourceManagerDialog( QgsMapCanvas *mapCanvas, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    ~QgsDataSourceManagerDialog();

  public slots:

    //! Sync current page with the leftbar list
    void setCurrentPage( int index );

    //! For signal forwarding to QgisApp
    void rasterLayerAdded( QString const &uri, QString const &baseName, QString const &providerKey );
    void vectorLayerAdded( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    void vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    void on_buttonBox_helpRequested() { QgsHelp::openHelp( QStringLiteral( "TODO_PLACEHOLDER.html" ) ); }

  signals:
    //! For signal forwarding to QgisApp
    void addRasterLayer( QString const &uri, QString const &baseName, QString const &providerKey );
    void addRasterLayer( );
    void addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );
    //! Replace the selected layer by a vector layer defined by uri, layer name, data source uri
    void replaceSelectedVectorLayer( const QString &oldId, const QString &uri, const QString &layerName, const QString &provider );
    void addWfsLayer( const QString &uri, const QString &typeName );
    void addAfsLayer( const QString &uri, const QString &typeName );
    void addVectorLayers( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );
    void showProgress( int progress, int totalSteps );
    void showStatusMessage( const QString &message );
    void addDatabaseLayers( QStringList const &layerPathList, QString const &providerKey );

  private:
    //! Return the dialog from the provider
    QDialog *providerDialog( QString const providerKey, QString const providerName, QString const icon, QString title = QString( ) );
    void addDbProviderDialog( QString const providerKey, QString const providerName, QString const icon, QString title = QString( ) );
    void addRasterProviderDialog( QString const providerKey, QString const providerName, QString const icon, QString title = QString( ) );
    Ui::QgsDataSourceManagerDialog *ui;
    QgsBrowserDockWidget *mBrowserWidget = nullptr;
    //! Map canvas
    QgsMapCanvas *mMapCanvas = nullptr;
    int mPreviousCurrentRow;
};

#endif // QGSDATASOURCEMANAGERDIALOG_H
