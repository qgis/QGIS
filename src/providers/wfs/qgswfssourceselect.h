/***************************************************************************
                              qgswfssourceselect.h
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSSOURCESELECT_H
#define QGSWFSSOURCESELECT_H

#include "ui_qgswfssourceselectbase.h"
#include "qgshelp.h"
#include "qgswfscapabilities.h"
#include "qgsoapiflandingpagerequest.h"
#include "qgsoapifcollection.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"

#include <QItemDelegate>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class QgsProjectionSelectionDialog;
class QgsWfsCapabilities;
class QgsSubsetStringEditorInterface;

class QgsWFSItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsWFSItemDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent ) {}

    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

class QgsWFSSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsWFSSourceSelectBase
{
    Q_OBJECT

  public:
    QgsWFSSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );
    ~QgsWFSSourceSelect() override;

    void reset() override;

  private:
    QgsWFSSourceSelect(); //default constructor is forbidden
    QgsProjectionSelectionDialog *mProjectionSelector = nullptr;

    /**
     * Stores the available CRS for a server connections.
     * The first string is the typename, the corresponding list
     * stores the CRS for the typename in the form 'EPSG:XXXX'
    */
    QMap<QString, QStringList> mAvailableCRS;
    std::unique_ptr<QgsWfsCapabilities> mCapabilities;
    std::unique_ptr<QgsOapifLandingPageRequest> mOAPIFLandingPage;
    std::unique_ptr<QgsOapifCollectionsRequest> mOAPIFCollections;
    QString mUri; // data source URI
    QgsWFSItemDelegate *mItemDelegate = nullptr;
    QStandardItemModel *mModel = nullptr;
    QSortFilterProxyModel *mModelProxy = nullptr;
    QPushButton *mBuildQueryButton = nullptr;
    QgsWfsCapabilities::Capabilities mCaps;
    QModelIndex mSQLIndex;
    QgsSubsetStringEditorInterface *mSQLComposerDialog = nullptr;
    QString mVersion;

    /**
     * Returns the best suited CRS from a list of authority ids
     *
     * 1. project CRS if contained in the list and the project is not empty
     * 2. the first entry in the list else
     *
     * \returns the authority id of the crs or an empty string in case of error
    */
    QString getPreferredCrs( const QList<QString> &crsList ) const;

    void showHelp();

  public slots:

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;
    void addButtonClicked() override;

  private slots:
    void addEntryToServerList();
    void modifyEntryOfServerList();
    void deleteEntryOfServerList();
    void connectToServer();
    void buildQuery( const QModelIndex &index );
    void changeCRS();
    void changeCRSFilter();
    void cmbConnections_activated( int index );
    void capabilitiesReplyFinished();
    void oapifLandingPageReplyFinished();
    void oapifCollectionsReplyFinished();
    void btnSave_clicked();
    void btnLoad_clicked();
    void treeWidgetItemDoubleClicked( const QModelIndex &index );
    void treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous );
    void buildQueryButtonClicked();
    void filterChanged( const QString &text );
    void updateSql();

    void populateConnectionList();
    void changeConnection();
    void startOapifLandingPageRequest();
    void startOapifCollectionsRequest( const QString &url );
    void resizeTreeViewAfterModelFill();
    bool isOapif() const { return mVersion == QLatin1String( "OGC_API_FEATURES" ); }
};

#endif
