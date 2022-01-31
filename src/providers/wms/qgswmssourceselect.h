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
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgswmsprovider.h"
#include "qgsabstractdatasourcewidget.h"

#include <QStringList>
#include <QPushButton>

class QButtonGroup;
class QgsTreeWidgetItem;
class QDomDocument;
class QDomElement;
class QgsWmsCapabilities;
class QgsWmsInterpretationComboBox;

/*!
 * \brief   Dialog to create connections and add layers from WMS, etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add
 * layers from the WMS server to the map canvas.
 */
class QgsWMSSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsWMSSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsWMSSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;

    //! Determines the layers the user selected
    void addButtonClicked() override;

    void reset() override;

  private slots:

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

    /**
     * Connects to the database using the stored connection parameters.
     * Once connected, available layers are displayed.
     */
    void btnConnect_clicked();

    //! Opens the Spatial Reference System dialog.
    void crsSelectorChanged( const QgsCoordinateReferenceSystem &crs );

    //! Signaled when a layer selection is changed.
    void lstLayers_itemSelectionChanged();

    //! Sets status message to theMessage
    void showStatusMessage( QString const &message );

    //! show whatever error is exposed by the QgsWmsProvider.
    void showError( QgsWmsProvider *wms );

    //! Stores the selected datasource whenerver it is changed
    void cmbConnections_activated( int );

    //! filter the layers
    void filterLayers( const QString &searchText );

    //! Filters the tiles sets
    void filterTiles( const QString &searchText );

  private:
    //! Populate the connection list combo box
    void populateConnectionList();

    //! Connection name
    QString connName();

    //! Sets the server connection combo box to that stored in the config file.
    void setConnectionListPosition();

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
     * \returns FALSE if the layers could not be retrieved or parsed -
     *         see mWmsProvider->errorString() for more info
     */
    bool populateLayerList( const QgsWmsCapabilities &capabilities );

    //! create an item including possible parents
    QgsTreeWidgetItem *createItem( int id,
                                   const QStringList &names,
                                   QMap<int, QgsTreeWidgetItem *> &items,
                                   int &layerAndStyleCount,
                                   const QMap<int, int> &layerParents,
                                   const QMap<int, QStringList> &layerParentNames );

    //! Returns a textual description for the authority id
    QString descriptionForAuthId( const QString &authId );

    //! Keeps the layer order list up-to-date with changed layers and styles
    void updateLayerOrderTab( const QStringList &newLayerList, const QStringList &newStyleList, const QStringList &newTitleList );

    //! Name for selected connection
    QString mConnName;

    //! URI for selected connection
    QgsDataSourceUri mUri;

    //! The widget that controls the image format radio buttons
    QButtonGroup *mImageFormatGroup = nullptr;

    QMap<QString, QString> mCrsNames;

    void applySelectionConstraints( QTreeWidgetItem *item );
    void collectNamedLayers( QTreeWidgetItem *item, QStringList &layers, QStringList &styles, QStringList &titles );
    void enableLayersForCrs( QTreeWidgetItem *item );

    void collectSelectedLayers( QStringList &layers, QStringList &styles, QStringList &titles );

    /**
     * Collects the available dimensions from the WMS layers and adds them
     * to the passed \a uri.
     */
    void collectDimensions( QStringList &layers, QgsDataSourceUri &uri );

    QString selectedImageEncoding();

    QList<QTreeWidgetItem *> mCurrentSelection;
    QTableWidgetItem *mCurrentTileset = nullptr;

    QList<QgsWmtsTileLayer> mTileLayers;

    //! Stores all the layers properties from the service capabilities.
    QVector<QgsWmsLayerProperty> mLayerProperties;

    // save the current status of the layer true
    QMap<QTreeWidgetItem *, bool> mTreeInitialExpand = QMap<QTreeWidgetItem *, bool>();

    QgsWmsInterpretationComboBox *mInterpretationCombo = nullptr;

  private slots:
    void lstTilesets_itemClicked( QTableWidgetItem *item );
    void mLayerUpButton_clicked();
    void mLayerDownButton_clicked();
    void updateButtons();
    void showHelp();
};

/*!
 * \brief ComboBox to select interpretation to convert image to a single band raster
 */
class QgsWmsInterpretationComboBox : public QComboBox
{
  public:
    //! Constructor
    QgsWmsInterpretationComboBox( QWidget *parent = nullptr );

    /**
     * Sets the interpretation from its key \a interpretationKey, default if interpretationKey is empty
     */
    void setInterpretation( const QString &interpretationKey );

    //! Returns the key of the current selected interpretation, returns empty string if the current is default
    QString interpretation() const;
};

#endif // QGSWMSSOURCESELECT_H
