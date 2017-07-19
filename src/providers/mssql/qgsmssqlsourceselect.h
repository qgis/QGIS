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
#include "qgsguiutils.h"
#include "qgsdbfilterproxymodel.h"
#include "qgsmssqltablemodel.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>

class QPushButton;
class QStringList;
class QgsMssqlGeomColumnTypeThread;
class QgisApp;

class QgsMssqlSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsMssqlSourceSelectDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};




/** \class QgsMssqlSourceSelect
 * \brief Dialog to create connections and add tables from MSSQL.
 *
 * This dialog allows the user to define and save connection information
 * for MSSQL databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsMssqlSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsDbSourceSelectBase
{
    Q_OBJECT

  public:

    //! static function to delete a connection
    static void deleteConnection( const QString &key );

    //! Constructor
    QgsMssqlSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    ~QgsMssqlSourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connectionInfo();

  signals:
    void addGeometryColumn( const QgsMssqlLayerProperty & );

  public slots:

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;

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
    void on_mSearchTableEdit_textChanged( const QString &text );
    void on_mSearchColumnComboBox_currentIndexChanged( const QString &text );
    void on_mSearchModeComboBox_currentIndexChanged( const QString &text );
    void setSql( const QModelIndex &index );
    //! Store the selected database
    void on_cmbConnections_activated( int );
    void setLayerType( const QgsMssqlLayerProperty &layerProperty );
    void on_mTablesTreeView_clicked( const QModelIndex &index );
    void on_mTablesTreeView_doubleClicked( const QModelIndex &index );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void on_buttonBox_helpRequested() { QgsHelp::openHelp( QStringLiteral( "working_with_vector/supported_data.html#mssql-spatial-layers" ) ); }

    void columnThreadFinished();


  private:
    typedef QPair<QString, QString> geomPair;
    typedef QList<geomPair> geomCol;

    // queue another query for the thread
    void addSearchGeometryColumn( const QString &connectionName, const QgsMssqlLayerProperty &layerProperty, bool estimateMetadata );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type );
    // The column labels
    QStringList mColumnLabels;
    // Our thread for doing long running queries
    QgsMssqlGeomColumnTypeThread *mColumnTypeThread = nullptr;
    QString mConnInfo;
    QStringList mSelectedTables;
    bool mUseEstimatedMetadata;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsMssqlTableModel mTableModel;
    QgsDatabaseFilterProxyModel mProxyModel;

    QPushButton *mBuildQueryButton = nullptr;
    QPushButton *mAddButton = nullptr;

    void finishList();
};

#endif // QGSMSSQLSOURCESELECT_H
