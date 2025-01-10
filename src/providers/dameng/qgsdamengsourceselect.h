/***************************************************************************
                          qgdmsourceselect.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : ( C ) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGSOURCESELECT_H
#define QGSDAMENGSOURCESELECT_H

#include "qgsguiutils.h"
#include "qgsdatasourceuri.h"
#include "qgsdamengtablemodel.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdbsourceselect.h"
#include "qgsdamengconn.h"

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>

class QPushButton;
class QgsDamengGeomColumnTypeThread;
class QgisApp;
class QgsDamengSourceSelect;
class QgsProxyProgressTask;
class QgsDamengTableModel;

class QgsDamengSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsDamengSourceSelectDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
};


/**
 * \class QgsDamengSourceSelect
 * \brief Dialog to create connections and add tables from Dameng.
 *
 * This dialog allows the user to define and save connection information
 * for DAMENG enabled Dameng databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsDamengSourceSelect : public QgsAbstractDbSourceSelect
{
    Q_OBJECT

  public:
    //! Constructor
    QgsDamengSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Standalone );

    ~QgsDamengSourceSelect() override;
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info ( database, host, user, password )
    QString connectionInfo( bool expandAuthCfg = true );
    //! Data source URI
    QgsDataSourceUri dataSourceUri();

  signals:
    void addGeometryColumn( const QgsDamengLayerProperty & );
    //void progressMessage( const QString & );

  public slots:
    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;
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
    void setSql( const QModelIndex &index );
    //! Store the selected database
    void setLayerType( const QgsDamengLayerProperty &layerProperty );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void columnThreadFinished();

    void reset() override;

  private:
    typedef QPair<QString, QString> geomPair;
    typedef QList<geomPair> geomCol;

    // queue another query for the thread
    //void addSearchGeometryColumn( const QgsDamengLayerProperty &layerProperty );

    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type );
    // The column labels
    QStringList mColumnLabels;
    // Our thread for doing long running queries
    QgsDamengGeomColumnTypeThread *mColumnTypeThread = nullptr;
    QgsProxyProgressTask *mColumnTypeTask = nullptr;

    QgsDataSourceUri mDataSrcUri;
    QStringList mSelectedTables;
    bool mUseEstimatedMetadata = false;
    // Storage for the range of layer type icons
    QMap< QString, QPair<QString, QIcon> > mLayerIcons;

    //! Model that acts as datasource for mTableTreeWidget
    QgsDamengTableModel *mTableModel = nullptr;

    void finishList();

    void showHelp();
};

#endif // QGSDAMENGSOURCESELECT_H
