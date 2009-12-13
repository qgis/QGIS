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
#include "ui_qgsspatialitesourceselectbase.h"

#include "qgisgui.h"
#include "qgsspatialitefilterproxymodel.h"
#include "qgsspatialitetablemodel.h"
#include "qgscontexthelp.h"

extern "C"
{
#include <spatialite/sqlite3.h>
}

#include <QThread>

#include <vector>
#include <list>
#include <utility>

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QFileDialog>

class QStringList;
class QTableWidgetItem;
class QgisApp;
class QPushButton;

/*! \class QgsSpatiaLiteSourceSelect
 * \brief Dialog to create connections and add tables from SpatiaLite.
 *
 * This dialog allows the user to define and save connection information
 * for SpatiaLite/SQLite databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsSpatiaLiteSourceSelect: public QDialog, private Ui::QgsSpatiaLiteSourceSelectBase
{
    Q_OBJECT

  public:

    //! Constructor
    QgsSpatiaLiteSourceSelect( QgisApp * app, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsSpatiaLiteSourceSelect() {}
    //! Populate the connection list combo box
    void populateConnectionList();
    //! Determines the tables the user selected and closes the dialog
    void addTables();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (DB-path)
    QString connectionInfo();
    // Store the selected database
    void dbChanged();

  public slots:
    /*! Connects to the database using the stored connection parameters.
     * Once connected, available layers are displayed.
     */
    void on_btnConnect_clicked();
    void addClicked();
    //! Opens the create connection dialog to build a new connection
    void on_btnNew_clicked();
    //! Deletes the selected connection
    void on_btnDelete_clicked();
    void on_mSearchOptionsButton_clicked();
    void on_mSearchTableEdit_textChanged( const QString & text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString & text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString & text );
    void on_cmbConnections_activated( int );
    void setLayerType( QString table, QString column, QString type );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString & regexp );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    enum columns
    {
      dbssType = 0,
      dbssDetail,
      dbssSql,
      dbssColumns,
    };

    typedef std::pair < QString, QString > geomPair;
    typedef std::list < geomPair > geomCol;

    /**Checks if geometry_columns_auth table exists*/
    bool checkGeometryColumnsAuth( sqlite3 * handle );

    /**Checks if views_geometry_columns table exists*/
    bool checkViewsGeometryColumns( sqlite3 * handle );

    /**Checks if virts_geometry_columns table exists*/
    bool checkVirtsGeometryColumns( sqlite3 * handle );

    /**Checks if this layer has been declared HIDDEN*/
    bool isDeclaredHidden( sqlite3 * handle, QString table, QString geom );

    /**cleaning well-formatted SQL strings*/
    QString quotedValue( QString value ) const;

    /**Inserts information about the spatial tables into mTableModel*/
    bool getTableInfo( sqlite3 * handle );

    // SpatiaLite DB open / close
    sqlite3 *openSpatiaLiteDb( QString path );
    void closeSpatiaLiteDb( sqlite3 * handle );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the table and column data into a single string
    // useful for display to the user
    QString fullDescription( QString table, QString column, QString type );
    // The column labels
    QStringList mColumnLabels;
    QString mSqlitePath;
    QStringList m_selectedTables;
    // Storage for the range of layer type icons
    QMap < QString, QPair < QString, QIcon > >mLayerIcons;
    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;
    //! Model that acts as datasource for mTableTreeWidget
    QgsSpatiaLiteTableModel mTableModel;
    QgsSpatiaLiteFilterProxyModel mProxyModel;

    QPushButton *mAddButton;
};

#endif // QGSSPATIALITESOURCESELECT_H
