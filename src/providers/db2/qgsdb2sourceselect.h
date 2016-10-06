/***************************************************************************
  qgsdb2sourceselect.h
      dialog to select DB2 layer(s) and add to the map canvas
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#ifndef QGSDB2SOURCESELECT_H
#define QGSDB2SOURCESELECT_H

#include "ui_qgsdbsourceselectbase.h"
#include "qgisgui.h"
#include "qgsdbfilterproxymodel.h"
#include "qgsdb2tablemodel.h"
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

class QgsDb2SourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsDb2SourceSelectDelegate( QObject *parent = NULL )
        : QItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsDb2GeomColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsDb2GeomColumnTypeThread( QString connectionName, bool useEstimatedMetadata );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    virtual void run() override;

  signals:
    void setLayerType( QgsDb2LayerProperty layerProperty );

  public slots:
    void addGeometryColumn( QgsDb2LayerProperty layerProperty );
    void stop();

  private:
    QgsDb2GeomColumnTypeThread() {}

    QString mConnectionName;
    bool mUseEstimatedMetadata;
    bool mStopped;
    QList<QgsDb2LayerProperty> layerProperties;
};


/** \class QgsDb2SourceSelect
 * \brief Dialog to create connections and add tables from Db2.
 *
 * This dialog allows the user to define and save connection information
 * for Db2 databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsDb2SourceSelect : public QDialog, private Ui::QgsDbSourceSelectBase
{
    Q_OBJECT

  public:

    //! static function to delete a connection
    static void deleteConnection( QString key );

    //! Constructor
    QgsDb2SourceSelect( QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags, bool managerMode = false, bool embeddedMode = false );
    //! Destructor
    ~QgsDb2SourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connectionInfo();

  signals:
    void addDatabaseLayers( QStringList const & layerPathList, QString const & providerKey );
    void connectionsChanged();
    void addGeometryColumn( QgsDb2LayerProperty );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addTables();
    void buildQuery();

    /** Connects to the database using the stored connection parameters.
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
    void on_mSearchGroupBox_toggled( bool );
    void on_mSearchTableEdit_textChanged( const QString & text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString & text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString & text );
    void setSql( const QModelIndex& index );
    //! Store the selected database
    void on_cmbConnections_activated( int );
    void setLayerType( QgsDb2LayerProperty layerProperty );
    void on_mTablesTreeView_clicked( const QModelIndex &index );
    void on_mTablesTreeView_doubleClicked( const QModelIndex &index );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
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
    void addSearchGeometryColumn( QString connectionName, QgsDb2LayerProperty layerProperty, bool estimateMetadata );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( QString schema, QString table, QString column, QString type );
    // The column labels
    QStringList mColumnLabels;
    // Our thread for doing long running queries
    QgsDb2GeomColumnTypeThread* mColumnTypeThread;
    QString mConnInfo;
    QStringList mSelectedTables;
    bool mUseEstimatedMetadata;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsDb2TableModel mTableModel;
    QgsDbFilterProxyModel mProxyModel;

    QPushButton *mBuildQueryButton;
    QPushButton *mAddButton;

    void finishList();
};

#endif // QGSDb2SOURCESELECT_H
