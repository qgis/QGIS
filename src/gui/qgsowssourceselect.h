/***************************************************************************
    qgsowssourceselect.h  -  selector for WMS,WFS,WCS layers
                             -------------------
    begin                : 3 April 2005
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    generalized          : (C) 2012 Radim Blazek, based on qgsowsconnection.h

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSSOURCESELECT_H
#define QGSOWSSOURCESELECT_H
#include "ui_qgsowssourceselectbase.h"
#include "qgsdatasourceuri.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgsdataprovider.h"

#include <QStringList>
#include <QPushButton>

class QgisApp;
class QgsDataProvider;
class QButtonGroup;
class QgsNumericSortTreeWidgetItem;
class QDomDocument;
class QDomElement;

/** Formats supported by provider */
struct QgsOWSSupportedFormat
{
  QString format;
  QString label;
};

/*!
 * \brief  Dialog to create connections and add layers from WMS, WFS, WCS etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add
 * layers from the WMS server to the map canvas.
 */
class GUI_EXPORT QgsOWSSourceSelect : public QDialog, public Ui::QgsOWSSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsOWSSourceSelect( QString service, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags, bool managerMode = false, bool embeddedMode = false );
    //! Destructor
    ~QgsOWSSourceSelect();

  public slots:

    //! Opens the create connection dialog to build a new connection
    void on_mNewButton_clicked();
    //! Opens a dialog to edit an existing connection
    void on_mEditButton_clicked();
    //! Deletes the selected connection
    void on_mDeleteButton_clicked();
    //! Saves connections to the file
    void on_mSaveButton_clicked();
    //! Loads connections from the file
    void on_mLoadButton_clicked();

    /*! Connects to the database using the stored connection parameters.
    * Once connected, available layers are displayed.
    */
    void on_mConnectButton_clicked();

    //! Determines the layers the user selected
    virtual void addClicked();

    void searchFinished();

    //! Opens the Spatial Reference System dialog.
    void on_mChangeCRSButton_clicked();

    //! Signaled when a layer selection is changed.
    virtual void on_mLayersTreeWidget_itemSelectionChanged();

    //! Set status message to theMessage
    void showStatusMessage( QString const &theMessage );

    //! show whatever error is exposed.
    void showError( QString const &theTitle, QString const &theFormat, QString const &theError );

    //! Stores the selected datasource whenerver it is changed
    void on_mConnectionsComboBox_activated( int );

    //! Add some default wms servers to the list
    void on_mAddDefaultButton_clicked();

    void on_mDialogButtonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  signals:
    void addRasterLayer( QString const & rasterLayerPath,
                         QString const & baseName,
                         QString const & providerKey );
    void connectionsChanged();

  protected:
    /**
     * List of image formats (encodings) supported by provider
     * @return list of format/label pairs
     */
    virtual QList<QgsOWSSupportedFormat> providerFormats();

    //! List of formats supported for currently selected layer item(s)
    virtual QStringList selectedLayersFormats();

    //! Server CRS supported for currently selected layer item(s)
    virtual QStringList selectedLayersCRSs();

    //virtual QStringList layerCRS( int id );

    //! Populate the connection list combo box
    void populateConnectionList();

    //! Populate supported formats
    void populateFormats();

    //! Clear previously set formats
    void clearFormats();

    //! Set supported CRSs
    void populateCRS();

    //! Connection name
    QString connName();

    //! Connection info (uri)
    QString connectionInfo();

    //! Set the server connection combo box to that stored in the config file.
    void setConnectionListPosition();

    //! Add a few example servers to the list.
    void addDefaultServers();

    //! Service name
    QString mService;

    //! Connections manager mode
    bool mManagerMode;

    //! Embedded mode, without 'Close'
    bool mEmbeddedMode;


    /**
     * \brief Populate the layer list.
     *
     * \retval false if the layers could not be retrieved or parsed
     */
    virtual void populateLayerList( );

    //! create an item including possible parents
    QgsNumericSortTreeWidgetItem *createItem( int id,
        const QStringList &names,
        QMap<int, QgsNumericSortTreeWidgetItem *> &items,
        int &layerAndStyleCount,
        const QMap<int, int> &layerParents,
        const QMap<int, QStringList> &layerParentNames );

    //! Returns a textual description for the authority id
    QString descriptionForAuthId( QString authId );

    //! layer name derived from latest layer selection (updated as long it's not edited manually)
    QString mLastLayerName;

    //! The widget that controls the image format radio buttons
    QButtonGroup *mImageFormatGroup;

    QPushButton *mAddButton;

    QMap<QString, QString> mCrsNames;

    void addWMSListRow( const QDomElement& item, int row );
    void addWMSListItem( const QDomElement& el, int row, int column );

    virtual void enableLayersForCrs( QTreeWidgetItem *item );

    //! Returns currently selected format
    QString selectedFormat();

    //! Returns currently selected Crs
    QString selectedCRS();

    QList<QTreeWidgetItem*> mCurrentSelection;
    QTableWidgetItem* mCurrentTileset;

    //! Name for selected connection
    QString mConnName;

    //! Cnnection info for selected connection
    QString mConnectionInfo;

    //! URI for selected connection
    QgsDataSourceURI mUri;

  private:
    //! Selected CRS
    QString mSelectedCRS;

    //! Common CRSs for selected layers
    QSet<QString> mSelectedLayersCRSs;

    //! Supported formats
    QList<QgsOWSSupportedFormat> mProviderFormats;

    //! Map mime type labels to supported formats
    QMap<QString, QString> mMimeLabelMap;

  private slots:
    void on_mSearchButton_clicked();
    void on_mAddWMSButton_clicked();
    void on_mSearchTableWidget_itemSelectionChanged();
    void on_mTilesetsTableWidget_itemClicked( QTableWidgetItem *item );
    void on_mLayerUpButton_clicked();
    void on_mLayerDownButton_clicked();
    virtual void updateButtons();
};

#endif // QGSOWSSOURCESELECT_H
