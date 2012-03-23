/***************************************************************************
                          qgmssqlsourceselect.h  -  description
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMSSQLSOURCESELECT_H
#define QGSMSSQLSOURCESELECT_H

#include "ui_qgsdbsourceselectbase.h"
#include "qgisgui.h"
#include "qgsdbfilterproxymodel.h"
#include "qgsmssqltablemodel.h"
#include "qgscontexthelp.h"

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>
#include <QThread>

class QPushButton;
class QStringList;
class QgsGeomColumnTypeThread;
class QgisApp;
class QgsPgSourceSelect;

class QgsMssqlSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT;

  public:
    QgsMssqlSourceSelectDelegate( QObject *parent = NULL )
        : QItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
};

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsMssqlGeomColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsMssqlGeomColumnTypeThread( QString connectionName, bool useEstimatedMetadata );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    virtual void run();

  signals:
    void setLayerType( QgsMssqlLayerProperty layerProperty );

  public slots:
    void addGeometryColumn( QgsMssqlLayerProperty layerProperty );
    void stop();

  private:
    QgsMssqlGeomColumnTypeThread() {}

    QString mConnectionName;
    bool mUseEstimatedMetadata;
    bool mStopped;
    QList<QgsMssqlLayerProperty> layerProperties;
};


/*! \class QgsMssqlSourceSelect
 * \brief Dialog to create connections and add tables from MS SQL.
 *
 * This dialog allows the user to define and save connection information
 * for MS SQL databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsMssqlSourceSelect : public QDialog, private Ui::QgsDbSourceSelectBase
{
    Q_OBJECT

  public:

    //! static function to delete a connection
    static void deleteConnection( QString key );

    //! Constructor
    QgsMssqlSourceSelect( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags, bool managerMode = false, bool embeddedMode = false );
    //! Destructor
    ~QgsMssqlSourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connectionInfo();

  signals:
    void addDatabaseLayers( QStringList const & layerPathList, QString const & providerKey );
    void connectionsChanged();
    void addGeometryColumn( QgsMssqlLayerProperty );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addTables();
    void buildQuery();

    /*! Connects to the database using the stored connection parameters.
    * Once connected, available layers are displayed.
    */
    void on_btnConnect_clicked();
    void on_cbxAllowGeometrylessTables_stateChanged( int );
    //! Opens the create connection dialog to build a new connection
    void on_btnNew_clicked();
    //! Opens a dialog to edit an existing connection
    void on_btnEdit_clicked();
    //! Deletes the selected connection
    void on_btnDelete_clicked();
    //! Saves the selected connections to file
    void on_btnSave_clicked();
    //! Loads the selected connections from file
    void on_btnLoad_clicked();
    void on_mSearchTableEdit_textChanged( const QString & text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString & text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString & text );
    void setSql( const QModelIndex& index );
    //! Store the selected database
    void on_cmbConnections_activated( int );
    void setLayerType( QgsMssqlLayerProperty layerProperty );
    void on_mTablesTreeView_clicked( const QModelIndex &index );
    void on_mTablesTreeView_doubleClicked( const QModelIndex &index );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString& regexp );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void columnThreadFinished();

  private:
    typedef QPair<QString, QString> geomPair;
    typedef QList<geomPair> geomCol;

    //! Connections manager mode
    bool mManagerMode;

    //! Embedded mode, without 'Close'
    bool mEmbeddedMode;

    // queue another query for the thread
    void addSearchGeometryColumn( QString connectionName, QgsMssqlLayerProperty layerProperty );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( QString schema, QString table, QString column, QString type );
    // The column labels
    QStringList mColumnLabels;
    // Our thread for doing long running queries
    QgsMssqlGeomColumnTypeThread* mColumnTypeThread;
    QString mConnInfo;
    QStringList mSelectedTables;
    bool mUseEstimatedMetadata;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsMssqlTableModel mTableModel;
    QgsDbFilterProxyModel mProxyModel;

    QPushButton *mBuildQueryButton;
    QPushButton *mAddButton;

    void finishList();
};

#endif // QGSMSSQLSOURCESELECT_H
