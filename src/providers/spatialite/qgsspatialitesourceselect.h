/***************************************************************************
                          qgspatialitesourceselect.h  -  description
                             -------------------
    begin                : Dec 2008
    copyright            : (C) 2008 by Sandro Furieri
    email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSPATIALITESOURCESELECT_H
#define QGSSPATIALITESOURCESELECT_H

#include "qgsguiutils.h"
#include "qgsdbfilterproxymodel.h"
#include "ui_qgsspatialitesourceselectbase.h"
#include "qgsspatialitedbinfomodel.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include <QThread>
#include <QMap>
#include <QList>
#include <QPair>
#include <QIcon>
#include <QFileDialog>

class QStringList;
class QTableWidgetItem;
class QPushButton;

/**
 * \class QgsSpatiaLiteSourceSelect
 * \brief Dialog to create connections and add tables from SpatiaLite.
 *
 * This dialog allows the user to define and save connection information
 * for SpatiaLite/SQLite databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
// class QgsSpatiaLiteSourceSelect: public QgsAbstractDataSourceWidget, private Ui::QgsDbSourceSelectBase
class QgsSpatiaLiteSourceSelect: public QgsAbstractDataSourceWidget, private Ui::QgsSpatialiteSourceSelectBase
{
    Q_OBJECT

  public:

    /* Open file selector to add new connection */
    static bool newConnection( QWidget *parent );

    //! Constructor
    QgsSpatiaLiteSourceSelect( QWidget *parent, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    ~QgsSpatiaLiteSourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();

    /**
     * Determines the tables the user selected and closes the dialog
     * \note
     *  - as url
     * \see collectConnectionsSelectedTables()
     * \since QGIS 1.8
     */
    void addSelectedDbLayers();

    /**
     * Deletes the selected connection
     * \note
     *  - emit connectionsChanged
     * \see onSourceSelectButtons_clicked
     * \see populateConnectionList
     * \since QGIS 1.8
     */
    void connectionDelete( QString connectionString );

    /**
     * Move the selected Item in Tab 'Layer Order' Up/Down inside a Groupe
     * \note
     *  - TODO: resolve selection is lost
     * \see onSourceSelectButtons_clicked
     * \see populateConnectionList
     * \since QGIS 1.8
     */
    void moveLayerOrderUpDown( int iMoveCount = 0 );

    /**
     * Connection info (DB-path) without table and geometry
     *  - Tab 'Connections'
     * - this will be called from classes using QgsSpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString getDatabaseUri() {  return mConnectionsSpatialiteDbInfoModel.getDatabaseUri(); }

    /**
     * Retrieve the current active Tab if the TabWodget
     *  - Tab 'Connections'
     * - this will be called from classes using QgsSpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see QgsSpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    int getCurrentTabIndex() {  return mTabWidget->currentIndex(); }

    /**
     * Index of Tab 'Connections'
     * - use to determine which Tab is active when a Button is pushed
     * \note
     *  - used with  getCurrentTabIndex
    * \see mTabConnections
    * \see getCurrentTabIndex
    * \since QGIS 3.0
    */
    int getTabConnectionsIndex() {  return mTabWidget->indexOf( mTabConnections ); }

    /**
     * Index of Tab 'Layer Order'
     * - use to determine which Tab is active when a Button is pushed
     * \note
     *  - used with  getCurrentTabIndex
    * \see mTabLayerOrder
    * \see getCurrentTabIndex
    * \since QGIS 3.0
    */
    int getTabLayerOrderIndex() {  return mTabWidget->indexOf( mTabLayerOrder ); }

    /**
     * Index of Tab 'Layer Tilesets'
     * - use to determine which Tab is active when a Button is pushed
     * \note
     *  - used with  getCurrentTabIndex
    * \see mTabLayerTilesets
    * \see getCurrentTabIndex
    * \since QGIS 3.0
    */
    int getTabLayerTilesetsIndex() {  return mTabWidget->indexOf( mTabLayerTilesets ); }

    /**
     * Index of Tab 'Server Search'
     * - use to determine which Tab is active when a Button is pushed
     * \note
     *  - used with  getCurrentTabIndex
    * \see mTabServerSearch
    * \see getCurrentTabIndex
    * \since QGIS 3.0
    */
    int getTabServerSearchIndex() {  return mTabWidget->indexOf( mTabServerSearch ); }

    /**
     * Store the selected database
     * \note
     *  - Remember which database was selected
     *  - in 'SpatiaLite/connections/selected'
     * \see collectConnectionsSelectedTables()
     * \since QGIS 1.8
     */
    void dbChanged();
    void setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo, bool setConnectionInfo = false );

    /**
     * Add or Remove Items for 'LayerOrder'
     * \note
     *  - this will call the Table-Model version that does the work
    * \param bRemoveItems are the selected Item to be added or removed
    * \returns iCountItems count of unique-Items added or removed
    * \see onSourceSelectTreeView_SelectionChanged
    * \see QgsSpatialiteDbInfoModel::setLayerOrderData
    * \since QGIS 3.0
    */
    int setLayerOrderData( bool bRemoveItems = false );

  public slots:

    /**
     * CustomContextMenu building for QgsSpatialiteDbInfoItem
     *  - Reacting to a QWidget::customContextMenuRequested event
     *  - Tab 'Connections'
     *  - on right Click Mouse
     * \note
     *  - based on InfoItem-Type and Column-Position, Context Menu
     *  - getCopyCellText will analyse if a Copy/Paste for that Cell should be offered
     * \see spatialiteDbInfoContextMenuRequest
     * \see QgsSpatialiteDbInfoItem::getCopyCellText
     * \since QGIS 1.8
     */
    void spatialiteDbInfoContextMenuRequested( QPoint pointPosition );

    /**
     * CustomContextMenu reaction QgsSpatialiteDbInfoItem
     *  - Tab 'Connections'
     *  - after Menu selection right Click Mouse
     * \note
     *  - based on InfoItem-Type and Column-Position, Context Menu
     *  - default is Copy/Paste for Cells containing non-static information
     *  Since mapCanvas() is read-only, an setExtent cannot be called for movint to Extent/Center
     * \see spatialiteDbInfoContextMenuRequested
     * \see QgsSpatialiteDbInfoItem::getCopyCellText
     * \since QGIS 1.8
     */
    void spatialiteDbInfoContextMenuRequest();

    /**
     * Calls UpdateLayerStatistics for a Spatialite Database
     *  - for selected Layers ar the whole Database
     * \note
     *  - Uses the QgsSpatialiteDbInfo interface
     *  -> UpdateLayerStatistics()
     * \see QgsSpatialiteDbInfo::isDbSpatialite()
     * \since QGIS 3.0
     */
    void onUpdateLayerStatitics();

    void on_mSearchGroupBox_toggled( bool );
    void on_mSearchLineEdit_textChanged( const QString &text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString &text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString &text );
    void setLayerSqlQuery( const QModelIndex &index );
    void on_mConnectionsComboBox_activated( int );

    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void on_mDialogButtonBox_helpRequested() { QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#spatialite-layers" ) ); }

    /**
     * Create and empty Spatialite Database
     *  - supporting also GeoPackage and MbTiles
     * \note
     *  - create and load empty Database
     * \param dbCreateOption representing the Database Type (SpatialMetadata) to be created
     * \param sDbCreateOption absolute path to file to load
     * \see onSourceSelectButtons_clicked
     * \see QgsSpatialiteDbInfo::SpatialMetadata
    * \returns true if created and loaded
     * \since QGIS 3.0
     */
    bool createEmptySpatialiteDb( QgsSpatialiteDbInfo::SpatialMetadata dbCreateOption, QString sDbCreateOption = QString() );

    /**
     * Load single Database-Source
     *  - load Database an fill the QgsSpatiaLiteTableModel with the result
     * \note
     *  - will set the mSqlitePath
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite50: also with Styles/Raster/VectorCoveragesTable's
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \see QgsSpatialiteDbInfo::createDatabase
     * \param path absolute path to file to load
     * \param providerKey
     * \see onSourceSelectButtons_clicked
     * \see addConnectionsDatabaseSource
     * \see mSqlitePath
     * \since QGIS 3.0
     */
    void addConnectionDatabaseSource( QString const &path, QString const &providerKey = QStringLiteral( "spatialite" ) );

    /**
     * Load selected Database-Sources
     *  - load Database an fill the QgsSpatiaLiteTableModel with the result
     * \note
     *  - only the first entry of 'paths' will be used
     * \param paths absolute path to file(s) to load
     * \param providerKey
     * \see addConnectionDatabaseSource
     * \since QGIS 3.0
     */
    void addConnectionsDatabaseSource( QStringList const &paths, QString const &providerKey = QStringLiteral( "spatialite" ) );

    //! Set status message to theMessage
    void showStatusMessage( const QString &message, int iDebug = 0 );

  private slots:

    /**
     * Deals with the different Buttons in the SourceSelect-Dialog
     *  - Reacting to a QPushButton::clicked event
     * \note
     * Based on which Tab is active and which Button has been pressed
     * - a corresponding function, with the gathered information,  will be called
     * Tab 'Connections'
     *  - mConnectionsConnectButton: Connects to the database using the stored connection parameters.
     *  - mConnectionsCreateButton: Opens the create connection dialog to build a new connection
     *  - mConnectionsCreateEmptySpatialiteDbButton: Select File-name for new Database
     *  - mConnectionsDeleteButton
     * Tab 'Layer Order'
     *  - mLayerOrderUpButton:
     *  - mLayerOrderDownButton: both call moveLayerOrderUpDown
     * Outside of Tabs
     *  - mSourceSelectAddLayersButton: Add selected Layers to Map-Canvas
     *  - mSourceSelectUpdateLayerStatiticsButton
     *  - mSourceSelectBuildQueryButton
     * \see addConnectionDatabaseSource
     * \see populateConnectionList
     * \see connectionDelete
     * \see createEmptySpatialiteDb
     * \see moveLayerOrderUpDown
     * \see addSelectedDbLayers
     * \see onUpdateLayerStatitics
     * \see setLayerSqlQuery
     * \since QGIS 3.0
     */
    void onSourceSelectButtons_clicked();

    /**
     * Deals with the different TreeViews in the SourceSelect-Dialog
     *  - Reacting to a QTreeView::clicked event
     * \note
     * Based on which Tab is active and which Button has been pressed
     * - a corresponding function, with the gathered information,  will be called
     * Tab 'Connections'
     *  - mConnectionsTreeView: Displaying the opended Database
     * Tab 'Layer Order'
     *  - mLayerOrderTreeView: Displaying the selected Layers of the opended Database
     * outside of Tabs: none
     * \see on_mConnectButton_clicked
     * \since QGIS 3.0
     */
    void onSourceSelectTreeView_clicked( const QModelIndex &index );

    /**
     * Deals with the different TreeViews in the SourceSelect-Dialog
     *  - Reacting to a QTreeView::doubleClicked event
     * \note
     * Based on which Tab is active and which Button has been pressed
     * - a corresponding function, with the gathered information,  will be called
     * Tab 'Connections'
     *  - mConnectionsTreeView: Displaying the opended Database
     * Tab 'Layer Order'
     *  - mLayerOrderTreeView: Displaying the selected Layers of the opended Database
     * outside of Tabs: none
     * \see on_mConnectButton_clicked
     * \since QGIS 3.0
     */
    void onSourceSelectTreeView_doubleClicked( const QModelIndex &index );

    /**
     * Deals with the different TreeViews in the SourceSelect-Dialog
     *  - Reacting to a QItemSelectionModel::selectionChanged event
     * \note
     * Based on which Tab is active and which Button has been pressed
     * - a corresponding function, with the gathered information,  will be called
     * Tab 'Connections'
     *  - mConnectionsTreeView: Displaying the opended Database
     * Tab 'Layer Order'
     *  - mLayerOrderTreeView: Displaying the selected Layers of the opended Database
     * outside of Tabs: none
     * \see on_mConnectButton_clicked
     * \since QGIS 3.0
     */
    void onSourceSelectTreeView_SelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

  signals:

    /**
     * Emitted when the provider's connections have changed
     * This signal is normally forwarded the app and used to refresh browser items
     * \since QGIS 3.0
     */
    void connectionsChanged();

    /**
     * Emitted when a DB layer has been selected for addition
     * \note
     *  - this event is used to load a DB-Layer during a Drag and Drop
     * \see QgisApp::openLayer
     * \see addConnectionsDatabaseSource
     * \since QGIS 3.0
     */
    void addDatabaseLayers( QStringList const &paths, QString const &providerKey );

  private:

    /**
     * Set the position of the database connection list to the last used one.
     * \note
     *  - from 'SpatiaLite/connections/selected'
     * \see populateConnectionList
     * \since QGIS 1.8
     */
    void setConnectionListPosition();

    /**
     * Combine the table and column data into a single string
     * \note
     *  - useful for display to the user
     * \since QGIS 1.8
     */
    QString fullDescription( const QString &table, const QString &column, const QString &type );

    /**
     * The column labels
     * \note
     *  - not used
     * \since QGIS 1.8
     */
    QStringList mColumnLabels;

    /**
     * Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - retrieved from QgsSpatiaLiteConnection
     * \returns name of connection
     * \see on_mConnectButton_clicked
     * \since QGIS 1.8
     */
    QString mSqlitePath;

    /**
     * Function to collect selected layers
     *  - Tab 'Connections'
     * \note
     *  - fills m_selectedTables with uri
     *  - fills m_selectedLayers with LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * \see addSelectedDbLayers()
     * \see onUpdateLayerStatitics()
     * \since QGIS 3.0
     */
    int collectConnectionsSelectedTables();

    /**
     * List of selected Tabels
     * \note
     *  - as LayerName LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * \see collectConnectionsSelectedTables()
     * \since QGIS 3.0
     */
    QStringList m_selectedLayers;

    /**
     * List of extra Sql-Query for selected Layers
     * \note
     *  - as extra Sql-Query
     * \see collectConnectionsSelectedTables()
     * \since QGIS 1.8
     */
    QStringList m_selectedLayersStyles;

    /**
     * List of extra Sql-Query for selected Layers
     * \note
     *  - as extra Sql-Query
     * \see collectConnectionsSelectedTables()
     * \since QGIS 1.8
     */
    QStringList m_selectedLayersSql;

    /**
     * Add a list of database layers to the map
     *  - Tab 'Connections'
     * - to fill a Map of QgsVectorLayers and/or QgsRasterLayers
     * -> can be for both QgsSpatiaLiteProvider or QgsOgr/GdalProvider
     * \note
     * - requested by User to add to main QGis
     * - collectConnectionsSelectedTables must be called beforhand
     * -> emit addDatabaseLayers for each supported Provider
     * \param saSelectedLayers formatted as 'table_name(geometry_name)'
     * \returns amount of Uris entries
     * \see getSelectedLayersUris
     * \see collectConnectionsSelectedTables()
     * \since QGIS 3.0
     */
    int addConnectionsDbMapLayers() const {  return mConnectionsSpatialiteDbInfoModel.addDbMapLayers( m_selectedLayers, m_selectedLayersStyles, m_selectedLayersSql );  }

    /**
     * Map of valid Selected Layers requested by the User
     *  - Tab 'Connections'
     * - only Uris that created a valid QgsVectorLayer/QgsRasterLayer
     * -> filled during addDatabaseLayers
     * -> called through the QgsSpatialiteDbInfo stored in QgsSpatiaLiteTableModel
     * \note
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: Uris dependent on provider
     * - can be for both QgsSpatiaLiteProvider or QgsGdalProvider
     * \returns mSelectedLayersUris  Map of LayerNames and  valid Layer-Uris entries
     * \see QgsSpatiaLiteTableModel:;getSpatialiteDbInfo
     * \see addDatabaseLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getConnectionsSelectedLayersUris() const { return mConnectionsSpatialiteDbInfoModel.getConnectionsSelectedLayersUris(); }

    /**
     * Storage for the range of layer type icons
     * \note
     *  - fills m_selectedTables with uri
     *  - fills m_selectedLayers with LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * \see addSelectedDbLayers()
     * \see onUpdateLayerStatitics()
     * \since QGIS 3.0
     */
    QMap < QString, QPair < QString, QIcon > >mLayerIcons;

    /**
     * Model that acts as datasource for mConnectionsTreeView
     *  - Tab 'Connections'
     * \note
     * Type: ModelTypeConnections
     *  - filled by values contained in QgsSpatialiteDbInfo
     * \since QGIS 1.8
     */
    QgsSpatialiteDbInfoModel mConnectionsSpatialiteDbInfoModel;

    /**
     * Placeholder for Model RootItem of 'Connections'
     *  - Tab 'Connections'
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addDatabaseEntry
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *mConnectionsRootItem = nullptr;

    /**
     * QgsDatabaseFilterProxyMode for mConnectionsTreeView
     *  - Tab 'Connections'
     * \note
     *  - does what ever needs to be done
     * QgsDatabaseFilterProxyModel, QSortFilterProxyModel
     * \since QGIS 1.8
     */
    QgsDatabaseFilterProxyModel mConnectionsProxyModel;

    /**
     * Model that acts as datasource for mLayerOrderTreeView
     *  - Tab 'Layer Order'
     * \note
     * Type: ModelTypeLayerOrder
     *  - filled by selected Items from mConnectionsTreeView
     * \since QGIS 1.8
     */
    QgsSpatialiteDbInfoModel mLayerOrderSpatialiteDbInfoModel;

    /**
     * Placeholder for Model RootItem of 'Layer Order'
     *  - Tab 'Layer Order'
     * - used as insert point of child Spatial Items when building
     * \note
     * - removes all entries when a new Database is being opened
     * \see setSpatialiteDbInfo
     * \see addDatabaseEntry
     * \since QGIS 3.0
     */
    QgsSpatialiteDbInfoItem *mLayerOrderRootItem = nullptr;

    /**
     * QgsDatabaseFilterProxyMode for mLayerOrderTreeView
     *  - Tab 'Layer Order'
     * \note
     *  - does what ever needs to be done
     * QgsDatabaseFilterProxyModel, QSortFilterProxyModel
     * \since QGIS 1.8
     */
    QgsDatabaseFilterProxyModel mLayerOrderProxyModel;

    /**
     * Creates extra Sql-Query for selected Layers
     * \note
     *  - retrieves Table and Geometry Names with Sql from TableModel
     *  -> which know the needed index-number
     *  TODO for QGIS 3.0
     *  - TableModel should retrieve the Url from QgsSpatialiteDbInfo
     *  -> adding the Sql-Query to that result
     * \see QgsSpatiaLiteTableModel::getTableName
     * \see QgsSpatiaLiteTableModel::getGeometryName
     * \see QgsSpatiaLiteTableModel::getSqlQuery
     * \see m_selectedLayersSql
     * \since QGIS 1.8
     */
    QString layerUriSql( const QModelIndex &index );

    /**
     * Build Sql-Query
     * \note
     *  - calls setLayerSqlQuery
     *  TODO for QGIS 3.0
     *  - TableModel should the Layer-Type (Raster/Vector)
     *  - TableModel should the Provider-Type (Spatialite/Gdal/Oge)
     * \see onSourceSelectButtons_clicked
     * \see setLayerSqlQuery
     * \since QGIS 1.8
     */
    QPushButton *mSourceSelectBuildQueryButton = nullptr;

    /**
     * Add selected Entries to Qgis
     * \note
     *  - calls addSelectedDbLayers
     * \see onSourceSelectButtons_clicked
     * \see addSelectedDbLayers
     * \since QGIS 1.8
     */
    QPushButton *mSourceSelectAddLayersButton = nullptr;

    /**
     * Button Interface for UpdateLayerStatitics
     * \note
     *  - UpdateLayerStatitics is done through QgsSpatialiteDbInfo
     * \see onSourceSelectButtons_clicked
     * \see onUpdateLayerStatitics
     * \since QGIS 1.8
     */
    QPushButton *mSourceSelectUpdateLayerStatiticsButton = nullptr;

    /**
     * Will contain values to represent option to create a new Database
     *  - for use with QgsSpatialiteDbInfo::createDatabase
     * \note
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite50: also with Styles/Raster/VectorCoveragesTable's
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \returns true if the Database was created
     * \see mDbCreateOption
     * \see QgsSpatialiteDbInfo::createDatabase
     * \since QGIS 3.0
     */
    // QComboBox *cmbDbCreateOption = nullptr;

    /**
     * QgsProviderRegistry::WidgetMode
     * \note
     *  - does what ever needs to be done
     * \since QGIS 1.8
     */
    QgsProviderRegistry::WidgetMode mWidgetMode = QgsProviderRegistry::WidgetMode::None;

    /**
     * Spatialite Provider Key
     * - for use with QgsVectorLayer and for building Uri
     * \note
     *  - Spatialite-Geometries
    * \see createDbLayerInfoUri
    * \see addConnectionsDbMapLayers
    * \since QGIS 3.0
    */
    const QString mSpatialiteProviderKey = QStringLiteral( "spatialite50" );

};

#endif // QGSSPATIALITESOURCESELECT_H
