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
#include "ui_qgsdbsourceselectbase.h"
#include "qgsguiutils.h"
#include "qgsdbfilterproxymodel.h"
#include "qgsspatialitetablemodel.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsspatialiteconnection.h"

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
// class QgsSpatiaLiteSourceSelect: public QDialog, private Ui::QgsDbSourceSelectBase
class QgsSpatiaLiteSourceSelect: public QgsAbstractDataSourceWidget, private Ui::QgsDbSourceSelectBase
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
     * \see collectSelectedTables()
     * \since QGIS 1.8
     */
    void addTables();

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from classes using SpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see SpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString getDatabaseUri() {  return mTableModel.getDatabaseUri(); }

    /**
     * Store the selected database
     * \note
     *  - Remember which database was selected
     *  - in 'SpatiaLite/connections/selected'
     * \see collectSelectedTables()
     * \since QGIS 1.8
     */
    void dbChanged();
    void setSpatialiteDbInfo( SpatialiteDbInfo *spatialiteDbInfo, bool setConnectionInfo = false );

  public slots:

    /**
     * Connects to the database using the stored connection parameters.
     * Once connected, available layers are displayed.
     */
    void on_btnConnect_clicked();
    void buildQuery();
    void addClicked();

    /**
     * Calls UpdateLayerStatistics for a Spatialite Database
     *  - for selected Layers ar the whole Database
     * \note
     *  - Uses the SpatialiteDbInfo interface
     *  -> UpdateLayerStatistics()
     * \see SpatialiteDbInfo::isDbSpatialite()
     * \since QGIS 3.0
     */
    void updateStatistics();

    /**
     * Select File-name for new Database
     *  - for use with SpatialiteDbInfo::createDatabase
     * \note
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite45: also with Styles/Raster/VectorCoveragesTable's
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \see SpatialiteDbInfo::createDatabase
     * \see cmbDbCreateOption
     * \since QGIS 3.0
     */
    void on_btnSave_clicked();
    //! Opens the create connection dialog to build a new connection
    void on_btnNew_clicked();
    //! Deletes the selected connection
    void on_btnDelete_clicked();
    void on_mSearchGroupBox_toggled( bool );
    void on_mSearchTableEdit_textChanged( const QString &text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString &text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString &text );
    void setSql( const QModelIndex &index );
    void on_cmbConnections_activated( int );
    void on_mTablesTreeView_clicked( const QModelIndex &index );
    void on_mTablesTreeView_doubleClicked( const QModelIndex &index );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void on_buttonBox_helpRequested() { QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#spatialite-layers" ) ); }

    /**
     * Load selected Database-Source
     *  - load Database an fill the QgsSpatiaLiteTableModel with the result
     * \note
     *  - only the first entry of 'paths' will be used
     * \param paths absolute path to file(s) to load
     * \param providerKey
     * \see on_btnConnect_clicked
     * \see addDatabaseLayers
     * \see QgisApp::openLayer
     * \since QGIS 3.0
     */
    void addDatabaseSource( QStringList const &paths, QString const &providerKey = QStringLiteral( "spatialite" ) );
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
     * \see addDatabaseSource
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
     * \see on_btnConnect_clicked
     * \since QGIS 1.8
     */
    QString mSqlitePath;

    /**
     * Function to collect selected layers
     * \note
     *  - fills m_selectedTables with uri
     *  - fills m_selectedLayers with LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * \see addTables()
     * \see updateStatistics()
     * \since QGIS 3.0
     */
    int collectSelectedTables();

    /**
     * List of selected Tabels
     * \note
     *  - as LayerName LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * \see collectSelectedTables()
     * \since QGIS 3.0
     */
    QStringList m_selectedLayers;

    /**
     * List of extra Sql-Query for selected Layers
     * \note
     *  - as extra Sql-Query
     * \see collectSelectedTables()
     * \since QGIS 1.8
     */
    QStringList m_selectedLayersSql;

    /**
     * Add a list of database layers to the map
     * - to fill a Map of QgsVectorLayers and/or QgsRasterLayers
     * -> can be for both QgsSpatiaLiteProvider or QgsOgr/GdalProvider
     * \note
     * - requested by User to add to main QGis
     * - collectSelectedTables must be called beforhand
     * -> emit addDatabaseLayers for each supported Provider
     * \param saSelectedLayers formatted as 'table_name(geometry_name)'
     * \returns amount of Uris entries
     * \see getSelectedLayersUris
     * \see collectSelectedTables()
     * \since QGIS 3.0
     */
    int addDbMapLayers() const {  return mTableModel.addDbMapLayers( m_selectedLayers, m_selectedLayersSql );  }

    /**
     * Map of valid Selected Layers requested by the User
     * - only Uris that created a valid QgsVectorLayer/QgsRasterLayer
     * -> filled during addDatabaseLayers
     * -> called through the SpatialiteDbInfo stored in QgsSpatiaLiteTableModel
     * \note
     * - Key: LayerName formatted as 'table_name(geometry)'
     * - Value: Uris dependent on provider
     * - can be for both QgsSpatiaLiteProvider or QgsGdalProvider
     * \returns mSelectedLayersUris  Map of LayerNames and  valid Layer-Uris entries
     * \see QgsSpatiaLiteTableModel:;getSpatialiteDbInfo
     * \see addDatabaseLayers
     * \since QGIS 3.0
     */
    QMap<QString, QString> getSelectedLayersUris() const { return mTableModel.getSelectedLayersUris(); }

    /**
     * Storage for the range of layer type icons
     * \note
     *  - fills m_selectedTables with uri
     *  - fills m_selectedLayers with LayerName formatted as 'table_name(geometry_name)' or 'table_name'
     * \see addTables()
     * \see updateStatistics()
     * \since QGIS 3.0
     */
    QMap < QString, QPair < QString, QIcon > >mLayerIcons;

    /**
     * Model that acts as datasource for mTableTreeWidget
     * \note
     *  - filled by values contained in SpatialiteDbInfo
     * \since QGIS 1.8
     */
    QgsSpatiaLiteTableModel mTableModel;

    /**
     * QgsDatabaseFilterProxyMode
     * \note
     *  - does what ever needs to be done
     * \since QGIS 1.8
     */
    QgsDatabaseFilterProxyModel mProxyModel;

    /**
     * Creates extra Sql-Query for selected Layers
     * \note
     *  - retrieves Table and Geometry Names with Sql from TableModel
     *  -> which know the needed index-number
     *  TODO for QGIS 3.0
     *  - TableModel should retrieve the Url from SpatialiteDbInfo
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
     *  - calls setSql
     *  TODO for QGIS 3.0
     *  - TableModel should the Layer-Type (Raster/Vector)
     *  - TableModel should the Provider-Type (Spatialite/Gdal/Oge)
     * \see buildQuery
     * \see setSql
     * \since QGIS 1.8
     */
    QPushButton *mBuildQueryButton = nullptr;

    /**
     * Add selected Entries to Qgis
     * \note
     *  - calls addTables
     * \see addClicked
     * \see addTables
     * \since QGIS 1.8
     */
    QPushButton *mAddButton = nullptr;

    /**
     * Button Interface for UpdateLayerStatitics
     * \note
     *  - UpdateLayerStatitics is done through SpatialiteDbInfo
     * \see updateStatistics
     * \since QGIS 1.8
     */
    QPushButton *mStatsButton = nullptr;

    /**
     * Will contain values to represent option to create a new Database
     *  - for use with SpatialiteDbInfo::createDatabase
     * \note
     * - SpatialMetadata::Spatialite40: InitSpatialMetadata only
     * - SpatialMetadata::Spatialite45: also with Styles/Raster/VectorCoveragesTable's
     * - SpatialMetadata::SpatialiteGpkg: using spatialite 'gpkgCreateBaseTables' function
     * - SpatialMetadata::SpatialiteMBTiles: creating a view-base MbTiles file with grid tables
     * \returns true if the Database was created
     * \see mDbCreateOption
     * \see SpatialiteDbInfo::createDatabase
     * \since QGIS 3.0
     */
    QComboBox *cmbDbCreateOption = nullptr;

    /**
     * QgsProviderRegistry::WidgetMode
     * \note
     *  - does what ever needs to be done
     * \since QGIS 1.8
     */
    QgsProviderRegistry::WidgetMode mWidgetMode = QgsProviderRegistry::WidgetMode::None;
};

#endif // QGSSPATIALITESOURCESELECT_H
