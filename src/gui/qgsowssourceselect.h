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
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgsguiutils.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"

#include <QStringList>
#include <QPushButton>
#include <QNetworkRequest>
#include "qgis_gui.h"

class QgsDataProvider;
class QButtonGroup;
class QgsTreeWidgetItem;
class QDomDocument;
class QDomElement;


/**
 * \ingroup gui
 * \brief  Dialog to create connections and add layers WCS etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add
 * layers from the WCS server to the map canvas.
 */
class GUI_EXPORT QgsOWSSourceSelect : public QgsAbstractDataSourceWidget, protected Ui::QgsOWSSourceSelectBase
{
    Q_OBJECT

  public:
    //! Formats supported by provider
    struct SupportedFormat
    {
      QString format;
      QString label;
    };

    //! Constructor
    QgsOWSSourceSelect( const QString &service, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    ~QgsOWSSourceSelect() override;

  public slots:

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;

  protected slots:
    //! show whatever error is exposed.
    void showError( const QString &title, const QString &format, const QString &error );

    //! Sets status message to theMessage
    void showStatusMessage( const QString &message );

  protected:

    /**
     * List of image formats (encodings) supported by provider
     * \returns list of format/label pairs
     */
    virtual QList<QgsOWSSourceSelect::SupportedFormat> providerFormats();

    //! List of formats supported for currently selected layer item(s)
    virtual QStringList selectedLayersFormats();

    //! Server CRS supported for currently selected layer item(s)
    virtual QStringList selectedLayersCrses();

    //! List of times (temporalDomain timePosition/timePeriod for currently selected layer item(s)
    virtual QStringList selectedLayersTimes();

    //virtual QStringList layerCRS( int id );

    //! Populate the connection list combo box
    void populateConnectionList();

    //! Populate supported formats
    void populateFormats();

    //! Clear previously set formats
    void clearFormats();

    //! Sets supported CRSs
    void populateCrs();

    //! Clear CRSs
    void clearCrs();

    //! Populate times
    void populateTimes();

    //! Clear times
    void clearTimes();

    //! Connection name
    QString connName();

    //! Connection info (uri)
    QString connectionInfo();

    //! Sets the server connection combo box to that stored in the config file.
    void setConnectionListPosition();

    //! Add a few example servers to the list.
    void addDefaultServers();

    //! Service name
    QString mService;

    /**
     * \brief Populate the layer list.
     *
     * \retval false if the layers could not be retrieved or parsed
     */
    virtual void populateLayerList();

    /**
     * create an item including possible parents
     * \note not available in Python bindings
     */
    QgsTreeWidgetItem *createItem( int id,
                                   const QStringList &names,
                                   QMap<int, QgsTreeWidgetItem *> &items,
                                   int &layerAndStyleCount,
                                   const QMap<int, int> &layerParents,
                                   const QMap<int, QStringList> &layerParentNames ) SIP_FACTORY SIP_SKIP;

    //! Returns a textual description for the authority id
    QString descriptionForAuthId( const QString &authId );

    //! layer name derived from latest layer selection (updated as long it's not edited manually)
    QString mLastLayerName;

    QMap<QString, QString> mCrsNames;

    void addWmsListRow( const QDomElement &item, int row );
    void addWmsListItem( const QDomElement &el, int row, int column );

    virtual void enableLayersForCrs( QTreeWidgetItem *item );

    //! Returns currently selected format
    QString selectedFormat();

    //! Returns currently selected Crs
    QString selectedCrs();

    //! Returns currently selected time
    QString selectedTime();

    //! Returns currently selected cache load control
    QNetworkRequest::CacheLoadControl selectedCacheLoadControl();

    QList<QTreeWidgetItem *> mCurrentSelection;
    QTableWidgetItem *mCurrentTileset = nullptr;

    //! Name for selected connection
    QString mConnName;

    //! Connection info for selected connection
    QString mConnectionInfo;

    //! URI for selected connection
    QgsDataSourceUri mUri;

  private slots:

    //! Opens the create connection dialog to build a new connection
    void mNewButton_clicked();
    //! Opens a dialog to edit an existing connection
    void mEditButton_clicked();
    //! Deletes the selected connection
    void mDeleteButton_clicked();
    //! Saves connections to the file
    void mSaveButton_clicked();
    //! Loads connections from the file
    void mLoadButton_clicked();

    /**
     * Connects to the database using the stored connection parameters.
     * Once connected, available layers are displayed.
     */
    void mConnectButton_clicked();

    void searchFinished();

    //! Opens the Spatial Reference System dialog.
    void mChangeCRSButton_clicked();

    //! Signaled when a layer selection is changed.
    virtual void mLayersTreeWidget_itemSelectionChanged();

    //! Stores the selected datasource whenerver it is changed
    void mConnectionsComboBox_activated( int );

    //! Add some default wms servers to the list
    void mAddDefaultButton_clicked();


  private:
    //! Selected CRS
    QString mSelectedCRS;

    //! Common CRSs for selected layers
    QSet<QString> mSelectedLayersCRSs;

    //! Supported formats
    QList<SupportedFormat> mProviderFormats;

    //! Map mime type labels to supported formats
    QMap<QString, QString> mMimeLabelMap;

  private slots:
    void mSearchButton_clicked();
    void mSearchTableWidget_itemSelectionChanged();
    void mTilesetsTableWidget_itemClicked( QTableWidgetItem *item );
    void mLayerUpButton_clicked();
    void mLayerDownButton_clicked();
    virtual void updateButtons();
};

#endif // QGSOWSSOURCESELECT_H
