/***************************************************************************
   qgshanasourceselect.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANASOURCESELECT_H
#define QGSHANASOURCESELECT_H

#include "qgsdatasourceuri.h"
#include "qgshanacolumntypethread.h"
#include "qgshelp.h"
#include "qgsproviderregistry.h"
#include "qgsguiutils.h"
#include "qgsabstractdbsourceselect.h"

#include <QMap>
#include <QPair>
#include <QIcon>
#include <QItemDelegate>
#include <QString>

class QgsProxyProgressTask;
class QgisApp;

class QgsHanaSourceSelectDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsHanaSourceSelectDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {}

    QWidget *createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &option,
      const QModelIndex &index ) const override;
    void setModelData(
      QWidget *editor,
      QAbstractItemModel *model,
      const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
};

/**
 * \class QgsHanaSourceSelect
 * \brief Dialog to create connections and add tables from SAP HANA.
 *
 * This dialog allows the user to define and save connection information
 * for SAP HANA databases. The user can then connect and add
 * tables from the database to the map canvas.
 */
class QgsHanaSourceSelect : public QgsAbstractDbSourceSelect
{
    Q_OBJECT

  public:

    //! static function to delete a connection
    static void deleteConnection( const QString &key );

    //! Constructor
    QgsHanaSourceSelect(
      QWidget *parent = nullptr,
      Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
      QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    ~QgsHanaSourceSelect() override;
    //! Populate the connection list combo box
    void populateConnectionList();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connectionInfo();

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
    //! Store the selected database
    void cmbConnections_activated( int );
    void setLayerType( const QgsHanaLayerProperty &layerProperty );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void columnThreadFinished();

  protected slots:
    void setSql( const QModelIndex &index ) override;
    void treeviewDoubleClicked( const QModelIndex &index ) override;

  private:
    // Set the position of the database connection list to the last
    // used one.
    void setConnectionListPosition();
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription(
      const QString &schema,
      const QString &table,
      const QString &column,
      const QString &type );
    void finishList();
    void showHelp();

    QString mConnectionName;
    QString mConnectionInfo;
    // A thread for detecting geometry types
    std::unique_ptr<QgsHanaColumnTypeThread> mColumnTypeThread;
    std::unique_ptr<QgsProxyProgressTask> mColumnTypeTask;
    QStringList mSelectedTables;
    //! Model that acts as datasource for mTableTreeWidget
    QgsHanaTableModel *mTableModel = nullptr;
};

#endif // QGSHANASOURCESELECT_H
