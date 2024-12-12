/***************************************************************************
    qgsstacsourceselect.h
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACSOURCESELECT_H
#define QGSSTACSOURCESELECT_H

#include "ui_qgsstacsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"
#include "qgsmimedatautils.h"
#include "qobjectuniqueptr.h"

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QUrl>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsStacSearchParametersDialog;
class QgsStacItemListModel;
class QgsStacController;
class QgsRubberBand;

class GUI_EXPORT QgsStacSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsStacSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsStacSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );
    //! Destructor
    ~QgsStacSourceSelect() override;

    void hideEvent( QHideEvent *e ) override;
    void showEvent( QShowEvent *e ) override;

    void addButtonClicked() override;

  private slots:

    //! Connects using current connection
    void btnConnect_clicked();
    //! Opens the create connection dialog to build a new connection
    void btnNew_clicked();
    //! Opens a dialog to edit an existing connection
    void btnEdit_clicked();
    //! Deletes the selected connection
    void btnDelete_clicked();
    //! Saves connections to the file
    void btnSave_clicked();
    //! Loads connections from the file
    void btnLoad_clicked();
    //! Stores the selected datasource whenerver it is changed
    void cmbConnections_currentTextChanged( const QString &text );

    //! Called when the STAC Catalog request finishes
    void onStacObjectRequestFinished( int requestId, QString error );

    //! Called when the request for list of collections is finished
    void onCollectionsRequestFinished( int requestId, QString error );

    //! Called when search requests finish
    void onItemCollectionRequestFinished( int requestId, QString error );

    //! Opens the filter parameters dialog
    void openSearchParametersDialog();

    //! Called when scrolling to fetch next page when scrolled to the end
    void onItemsViewScroll( int value );

    //! Called when double clicking a result item
    void onItemDoubleClicked( const QModelIndex &index );

    //! Enables Add Layers button based on current item, updates rubber bands
    void onCurrentItemChanged( const QModelIndex &current, const QModelIndex &previous );

  private:
    //! Perform the actual search request
    void search();
    void populateConnectionList();
    void setConnectionListPosition();
    void showHelp();
    void updateFilterPreview();
    void fetchNextResultPage();

    void showItemsContextMenu( QPoint point );

    void highlightFootprint( const QModelIndex &index );
    void showFootprints( bool enable );
    void loadUri( const QgsMimeDataUtils::Uri &uri );

    QString mCollectionsUrl;
    QString mSearchUrl;
    QUrl mNextPageUrl;

    QgsStacController *mStac = nullptr;
    QgsStacItemListModel *mItemsModel = nullptr;
    QgsStacSearchParametersDialog *mParametersDialog = nullptr;
    QObjectUniquePtr<QgsRubberBand> mCurrentItemBand;
    QVector<QgsRubberBand *> mRubberBands;
};

///@endcond
#endif // QGSSTACSOURCESELECT_H
