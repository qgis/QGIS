/***************************************************************************
    qgswmssourceselect.h  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSSOURCESELECT_H
#define QGSWMSSOURCESELECT_H
#include "ui_qgswmssourceselectbase.h"
#include "qgsdatasourceuri.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgswmsprovider.h"

#include <QStringList>
#include <QPushButton>

class QgisApp;
class QgsWmsProvider;
class QButtonGroup;
class QgsNumericSortTreeWidgetItem;
class QDomDocument;
class QDomElement;
class QgsWmsCapabilities;

/*!
 * \brief   Dialog to create connections and add layers from WMS, etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add
 * layers from the WMS server to the map canvas.
 */
class QgsWMSSourceSelect : public QDialog, private Ui::QgsWMSSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsWMSSourceSelect( QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags, bool managerMode = false, bool embeddedMode = false );
    //! Destructor
    ~QgsWMSSourceSelect();

  public slots:

    //! Opens the create connection dialog to build a new connection
    void on_btnNew_clicked();
    //! Opens a dialog to edit an existing connection
    void on_btnEdit_clicked();
    //! Deletes the selected connection
    void on_btnDelete_clicked();
    //! Saves connections to the file
    void on_btnSave_clicked();
    //! Loads connections from the file
    void on_btnLoad_clicked();

    /** Connects to the database using the stored connection parameters.
    * Once connected, available layers are displayed.
    */
    void on_btnConnect_clicked();

    //! Determines the layers the user selected
    void addClicked();

    void searchFinished();

    //! Opens the Spatial Reference System dialog.
    void on_btnChangeSpatialRefSys_clicked();

    //! Signaled when a layer selection is changed.
    void on_lstLayers_itemSelectionChanged();

    //! Set status message to theMessage
    void showStatusMessage( QString const &theMessage );

    //! show whatever error is exposed by the QgsWmsProvider.
    void showError( QgsWmsProvider *wms );

    //! Stores the selected datasource whenerver it is changed
    void on_cmbConnections_activated( int );

    //! Add some default wms servers to the list
    void on_btnAddDefault_clicked();

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    //! Populate the connection list combo box
    void populateConnectionList();

    //! Connection name
    QString connName();

    //! Set the server connection combo box to that stored in the config file.
    void setConnectionListPosition();

    //! Add a few example servers to the list.
    void addDefaultServers();

    //! Connections manager mode
    bool mManagerMode;

    //! Embedded mode, without 'Close'
    bool mEmbeddedMode;

    //! Selected CRS
    QString mCRS;

    //! Default CRS
    QString mDefaultCRS;

    //! Common CRSs for selected layers
    QSet<QString> mCRSs;

    //! Supported formats
    //QStringList mFormats;
    QVector<QgsWmsSupportedFormat> mFormats;

    //! Labels for supported formats
    //QStringList mLabels;

    //! Map mime types to supported formats
    QMap<QString, int> mMimeMap;


    // Clear layers list, crs, encodings ...
    void clear();

    /**
     * \brief Populate the layer list - private for now.
     *
     * \retval false if the layers could not be retrieved or parsed -
     *         see mWmsProvider->errorString() for more info
     */
    bool populateLayerList( const QgsWmsCapabilities& capabilities );

    //! create an item including possible parents
    QgsNumericSortTreeWidgetItem *createItem( int id,
        const QStringList &names,
        QMap<int, QgsNumericSortTreeWidgetItem *> &items,
        int &layerAndStyleCount,
        const QMap<int, int> &layerParents,
        const QMap<int, QStringList> &layerParentNames );

    //! Returns a textual description for the authority id
    QString descriptionForAuthId( QString authId );

    //! Keeps the layer order list up-to-date with changed layers and styles
    void updateLayerOrderTab( const QStringList& newLayerList, const QStringList& newStyleList, const QStringList &newTitleList );

    //! Name for selected connection
    QString mConnName;

    //! URI for selected connection
    QgsDataSourceURI mUri;

    //! layer name derived from latest layer selection (updated as long it's not edited manually)
    QString mLastLayerName;

    //! The widget that controls the image format radio buttons
    QButtonGroup *mImageFormatGroup;

    QPushButton *mAddButton;

    QMap<QString, QString> mCrsNames;

    void addWMSListRow( const QDomElement& item, int row );
    void addWMSListItem( const QDomElement& el, int row, int column );

    void applySelectionConstraints( QTreeWidgetItem *item );
    void collectNamedLayers( QTreeWidgetItem *item, QStringList &layers, QStringList &styles, QStringList &titles );
    void enableLayersForCrs( QTreeWidgetItem *item );

    void collectSelectedLayers( QStringList &layers, QStringList &styles, QStringList &titles );
    QString selectedImageEncoding();

    QList<QTreeWidgetItem*> mCurrentSelection;
    QTableWidgetItem* mCurrentTileset;

    QList<QgsWmtsTileLayer> mTileLayers;

  signals:
    void addRasterLayer( QString const & rasterLayerPath,
                         QString const & baseName,
                         QString const & providerKey );
    void connectionsChanged();
  private slots:
    void on_btnSearch_clicked();
    void on_btnAddWMS_clicked();
    void on_tableWidgetWMSList_itemSelectionChanged();
    void on_lstTilesets_itemClicked( QTableWidgetItem *item );
    void on_mLayerUpButton_clicked();
    void on_mLayerDownButton_clicked();
    void updateButtons();
};


#endif // QGSWMSSOURCESELECT_H
