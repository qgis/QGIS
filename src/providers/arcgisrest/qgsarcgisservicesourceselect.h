/***************************************************************************
    qgsarcgisservicesourceselect.h
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

#ifndef QGSARCGISSERVICESOURCESELECT_H
#define QGSARCGISSERVICESOURCESELECT_H

#define SIP_NO_FILE

#include "ui_qgsarcgisservicesourceselectbase.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsabstractdatasourcewidget.h"

#include <QItemDelegate>

class QStandardItemModel;
class QSortFilterProxyModel;
class QgsProjectionSelectionDialog;
class QgsOwsConnection;
class QgsMapCanvas;

/**
 * Base class for listing ArcGis layers available from a remote service.
 */
class QgsArcGisServiceSourceSelect : public QgsAbstractDataSourceWidget, protected Ui::QgsArcGisServiceSourceSelectBase
{
    Q_OBJECT

  public:
    //! Whether the dialog is for a map service or a feature service
    enum ServiceType { MapService, FeatureService };

    //! Constructor
    QgsArcGisServiceSourceSelect( const QString &serviceName, ServiceType serviceType, QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Destructor
    ~QgsArcGisServiceSourceSelect() override;

  protected:
    QString mServiceName;
    ServiceType mServiceType;
    QgsProjectionSelectionDialog *mProjectionSelector = nullptr;
    //  Available CRS for a server connection, key=typename, value=list("EPSG:XXXX")
    QMap<QString, QStringList> mAvailableCRS;
    QStandardItemModel *mModel = nullptr;
    QSortFilterProxyModel *mModelProxy = nullptr;
    QPushButton *mBuildQueryButton = nullptr;
    QButtonGroup *mImageEncodingGroup = nullptr;
    QgsRectangle mCanvasExtent;
    QgsCoordinateReferenceSystem mCanvasCrs;

    //! To be implemented in the child class. Called when a new connection is initiated.
    virtual bool connectToService( const QgsOwsConnection &connection ) = 0;
    //! May be implemented in child classes for services which support customized queries.
    virtual void buildQuery( const QgsOwsConnection &, const QModelIndex & ) {}
    //! To be implemented in the child class. Constructs an URI for the specified service layer.
    virtual QString getLayerURI( const QgsOwsConnection &connection,
                                 const QString &layerTitle,
                                 const QString &layerName,
                                 const QString &crs = QString(),
                                 const QString &filter = QString(),
                                 const QgsRectangle &bBox = QgsRectangle() ) const = 0;
    //! Updates the UI for the list of available image encodings from the specified list.
    void populateImageEncodings( const QStringList &availableEncodings );
    //! Returns the selected image encoding.
    QString getSelectedImageEncoding() const;

  private:
    void populateConnectionList();

    //! A layer is added from the dialog
    virtual void addServiceLayer( QString uri, QString typeName ) = 0;

    /**
     * Returns the best suited CRS from a set of authority ids
       1. project CRS if contained in the set
       2. WGS84 if contained in the set
       3. the first entry in the set else
    \returns the authority id of the crs or an empty string in case of error*/
    QString getPreferredCrs( const QSet<QString> &crsSet ) const;

    /**
     * Store a pointer to map canvas to retrieve extent and CRS
     * Used to select an appropriate CRS and possibly to retrieve data only in the current extent
     */
    QgsMapCanvas *mMapCanvas = nullptr;


  public slots:

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;

  private slots:
    void addEntryToServerList();
    void deleteEntryOfServerList();
    void modifyEntryOfServerList();
    void addButtonClicked() override;
    void buildQueryButtonClicked();
    void changeCrs();
    void changeCrsFilter();
    void connectToServer();
    void filterChanged( const QString &text );
    void cmbConnections_activated( int index );
    void showHelp();
    void treeWidgetItemDoubleClicked( const QModelIndex &index );
    void treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous );
};

/**
 * Item delegate with tweaked sizeHint.
 */
class QgsAbstractDataSourceWidgetItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    //! Constructor
    QgsAbstractDataSourceWidgetItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) { }
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

#endif // QGSARCGISSERVICESOURCESELECT_H
