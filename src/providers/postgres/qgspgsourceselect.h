/***************************************************************************
                          qgpgsourceselect.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPGSOURCESELECT_H
#define QGSPGSOURCESELECT_H

#include "ui_qgsdbsourceselectbase.h"
#include "qgsguiutils.h"
#include "qgsdatasourceuri.h"
#include "qgsdbfilterproxymodel.h"
#include "qgspgtablemodel.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>

class QPushButton;
class QStringList;
class QgsGeomColumnTypeThread;
class QgisApp;
class QgsPgSourceSelect;

class QgsPgSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsPgSourceSelectDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
};


/** \class QgsPgSourceSelect
 * \brief Dialog to create connections and add tables from PostgresQL.
 *
 * This dialog allows the user to define and save connection information
 * for PostGIS enabled PostgreSQL databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsPgSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsDbSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsPgSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    ~QgsPgSourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connectionInfo( bool expandAuthCfg = true );
    //! Data source URI
    QgsDataSourceUri dataSourceUri();

  signals:
    void addGeometryColumn( const QgsPostgresLayerProperty & );
    void progress( int, int );
    void progressMessage( const QString & );

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
    void on_cmbConnections_currentIndexChanged( const QString &text );
    void setSql( const QModelIndex &index );
    //! Store the selected database
    void setLayerType( const QgsPostgresLayerProperty &layerProperty );
    void on_mTablesTreeView_clicked( const QModelIndex &index );
    void on_mTablesTreeView_doubleClicked( const QModelIndex &index );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void on_buttonBox_helpRequested() { QgsHelp::openHelp( QStringLiteral( "working_with_vector/supported_data.html#postgis-layers" ) ); }

    void columnThreadFinished();

  private:
    typedef QPair<QString, QString> geomPair;
    typedef QList<geomPair> geomCol;

    // queue another query for the thread
    void addSearchGeometryColumn( const QgsPostgresLayerProperty &layerProperty );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type );
    // The column labels
    QStringList mColumnLabels;
    // Our thread for doing long running queries
    QgsGeomColumnTypeThread *mColumnTypeThread = nullptr;
    QgsDataSourceUri mDataSrcUri;
    QStringList mSelectedTables;
    bool mUseEstimatedMetadata;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsPgTableModel mTableModel;
    QgsDatabaseFilterProxyModel mProxyModel;

    QPushButton *mBuildQueryButton = nullptr;
    QPushButton *mAddButton = nullptr;

    void finishList();
};

#endif // QGSPGSOURCESELECT_H
