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


/**
 * \class QgsPgSourceSelect
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
    void progressMessage( const QString & );

  public slots:
    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;
    void buildQuery();

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
    void mSearchGroupBox_toggled( bool );
    void mSearchTableEdit_textChanged( const QString &text );
    void mSearchColumnComboBox_currentIndexChanged( const QString &text );
    void mSearchModeComboBox_currentIndexChanged( const QString &text );
    void cmbConnections_currentIndexChanged( const QString &text );
    void setSql( const QModelIndex &index );
    //! Store the selected database
    void setLayerType( const QgsPostgresLayerProperty &layerProperty );
    void mTablesTreeView_clicked( const QModelIndex &index );
    void mTablesTreeView_doubleClicked( const QModelIndex &index );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

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
    bool mUseEstimatedMetadata = false;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsPgTableModel mTableModel;
    QgsDatabaseFilterProxyModel mProxyModel;

    QPushButton *mBuildQueryButton = nullptr;

    void finishList();

    void showHelp();
};

#endif // QGSPGSOURCESELECT_H
