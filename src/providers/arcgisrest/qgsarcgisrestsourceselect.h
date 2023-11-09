/***************************************************************************
    qgsarcgisrestsourceselect.h
    ---------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARCGISRESTSOURCESELECT_H
#define QGSARCGISRESTSOURCESELECT_H

#define SIP_NO_FILE

#include "ui_qgsarcgisservicesourceselectbase.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsbrowserproxymodel.h"

#include <QItemDelegate>

class QgsBrowserModel;


class QStandardItemModel;
class QSortFilterProxyModel;
class QgsProjectionSelectionDialog;
class QgsOwsConnection;
class QgsMapCanvas;

class QgsArcGisRestBrowserProxyModel : public QgsBrowserProxyModel
{
    Q_OBJECT

  public:

    explicit QgsArcGisRestBrowserProxyModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    void setConnectionName( const QString &name );
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    QString mConnectionName;
};

/**
 * Base class for listing ArcGis REST layers available from a remote service.
 */
class QgsArcGisRestSourceSelect : public QgsAbstractDataSourceWidget, protected Ui::QgsArcGisServiceSourceSelectBase
{
    Q_OBJECT

  public:

    //! Constructor
    QgsArcGisRestSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Destructor
    ~QgsArcGisRestSourceSelect() override;

  protected:

    QgsBrowserGuiModel *mBrowserModel = nullptr;
    QgsArcGisRestBrowserProxyModel *mProxyModel = nullptr;

    QPushButton *mBuildQueryButton = nullptr;
    QButtonGroup *mImageEncodingGroup = nullptr;

    //! Updates the UI for the list of available image encodings from the specified list.
    void populateImageEncodings( const QString &supportedFormats );
    //! Returns the selected image encoding.
    QString getSelectedImageEncoding() const;
    void showEvent( QShowEvent *event ) override;

  private:
    void populateConnectionList();

  public slots:

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;

  private slots:
    void addEntryToServerList();
    void deleteEntryOfServerList();
    void modifyEntryOfServerList();
    void addButtonClicked() override;
    void buildQueryButtonClicked();
    void updateCrsLabel();
    void updateImageEncodings();
    void connectToServer();
    void disconnectFromServer();
    void filterChanged( const QString &text );
    void cmbConnections_activated( int index );
    void showHelp();
    void treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous );
    void btnSave_clicked();
    void btnLoad_clicked();
    void onRefresh();

    void refreshModel( const QModelIndex &index );

  private:

    QString indexToUri( const QModelIndex &proxyIndex, QString &layerName, Qgis::ArcGisRestServiceType &serviceType, const QgsRectangle &extent = QgsRectangle() );

    QString mConnectedService;
};


#endif // QGSARCGISRESTSOURCESELECT_H
