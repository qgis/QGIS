/***************************************************************************
                          qgoraclesourceselect.h  -  description
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLESOURCESELECT_H
#define QGSORACLESOURCESELECT_H

#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgsoracleconnpool.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdbsourceselect.h"

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>

class QPushButton;
class QgsOracleColumnTypeTask;
class QgisApp;
class QgsOracleSourceSelect;
class QgsOracleTableModel;

class QgsOracleSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsOracleSourceSelectDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {}

    ~QgsOracleSourceSelectDelegate() override
    {
      setConn( nullptr );
    }

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;

    void setConnectionInfo( const QgsDataSourceUri &connInfo ) { mConnInfo = connInfo; }

  protected:
    void setConn( QgsOracleConn *conn ) const { if ( mConn ) QgsOracleConnPool::instance()->releaseConnection( mConn ); mConn = conn; }

    QgsOracleConn *conn() const
    {
      if ( !mConn )
        setConn( QgsOracleConnPool::instance()->acquireConnection( QgsOracleConn::toPoolName( mConnInfo ) ) );
      return mConn;
    }

  private:
    QgsDataSourceUri mConnInfo;
    //! lazily initialized connection (to detect possible primary keys)
    mutable QgsOracleConn *mConn = nullptr;
};


/**
 * \class QgsOracleSourceSelect
 * \brief Dialog to create connections and add tables from Oracle.
 *
 * This dialog allows the user to define and save connection information
 * for Oracle databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsOracleSourceSelect : public QgsAbstractDbSourceSelect
{
    Q_OBJECT

  public:
    //! Constructor
    QgsOracleSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
    //! Destructor
    ~QgsOracleSourceSelect() override;
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

    /**
     * Connects to the database using the stored connection parameters.
     * Once connected, available layers are displayed.
     */
    void btnConnect_clicked();
    void cbxAllowGeometrylessTables_stateChanged( int );
    //! Opens the create connection dialog to build a new connection
    void btnNew_clicked();
    //! Opens a dialog to edit an existing connection
    void btnEdit_clicked();
    //! Deletes the selected connection
    void btnDelete_clicked();
    //! Saves the selected connections to file
    void btnSave_clicked();
    //! Loads the selected connections from file
    void btnLoad_clicked();
    void cmbConnections_currentIndexChanged( const QString &text );
    //! Store the selected database
    void setLayerType( const QgsOracleLayerProperty &layerProperty );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void columnTaskFinished();

  protected slots:
    void setSql( const QModelIndex &index ) override;

  private:
    typedef QPair<QString, QString> geomPair;
    typedef QList<geomPair> geomCol;

    //! try to load list of tables from local cache
    void loadTableFromCache();

    // queue another query for the task
    void addSearchGeometryColumn( QgsOracleLayerProperty layerProperty );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type );
    // The column labels
    QStringList mColumnLabels;
    // Our task for doing long running queries
    QgsOracleColumnTypeTask *mColumnTypeTask = nullptr;
    QgsDataSourceUri mConnInfo;
    QStringList mSelectedTables;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsOracleTableModel *mTableModel = nullptr;
    QgsOracleSourceSelectDelegate *mTablesTreeDelegate = nullptr;

    QPushButton *mAddButton = nullptr;

    void finishList();
    bool mIsConnected = false;

    void showHelp();

};

#endif // QGSORACLESOURCESELECT_H
