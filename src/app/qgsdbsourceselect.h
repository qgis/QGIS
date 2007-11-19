/***************************************************************************
                          qgdbsourceselect.h  -  description
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
/* $Id$ */
#ifndef QGSDBSOURCESELECT_H
#define QGSDBSOURCESELECT_H
#include "ui_qgsdbsourceselectbase.h"
#include "qgisgui.h"
extern "C"
{
#include <libpq-fe.h>
}

#include <QThread>

#include <vector>
#include <list>
#include <utility>

#include <QMap>
#include <QPair>
#include <QIcon>

class QStringList;
class QTableWidgetItem;
class QgsGeomColumnTypeThread;
class QgisApp;
/*! \class QgsDbSourceSelect
 * \brief Dialog to create connections and add tables from PostgresQL.
 *
 * This dialog allows the user to define and save connection information
 * for PostGIS enabled PostgresQL databases. The user can then connect and add 
 * tables from the database to the map canvas.
 */
class QgsDbSourceSelect : public QDialog, private Ui::QgsDbSourceSelectBase 
{
  Q_OBJECT
 public:

    //! Constructor
    QgsDbSourceSelect(QgisApp *app, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    //! Destructor
    ~QgsDbSourceSelect();
    //! Opens the create connection dialog to build a new connection
    void addNewConnection();
    //! Opens a dialog to edit an existing connection
    void editConnection();
    //! Deletes the selected connection
    void deleteConnection();
    //! Populate the connection list combo box
    void populateConnectionList();
    //! Determines the tables the user selected and closes the dialog
    void addTables();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connInfo();
    //! Return the name of the selected encoding (e.g. UTf-8, ISO-8559-1, etc/)
    QString encoding();
    // Store the selected database
    void dbChanged();
    // Utility function to construct the query for finding out the
    // geometry type of a column
    static QString makeGeomQuery(QString schema, QString table, QString column);

    public slots:
    /*! Connects to the database using the stored connection parameters. 
    * Once connected, available layers are displayed.
    */
      void on_btnConnect_clicked();
      void on_btnAdd_clicked();
      void on_btnNew_clicked();
      void on_btnEdit_clicked();
      void on_btnDelete_clicked();
      void on_lstTables_itemDoubleClicked(QTableWidgetItem *);
      void setSql(QTableWidgetItem *);
      void on_btnHelp_clicked();
      void on_cmbConnections_activated(int);
      void setLayerType(QString schema, QString table, QString column,
                        QString type);
 private:
    enum columns {
	dbssImport=0,
	dbssType,
	dbssImportAs,
	dbssDetail,
	dbssSql,
	dbssColumns,
    };

    typedef std::pair<QString, QString> geomPair;
    typedef std::list<geomPair > geomCol;

    bool getGeometryColumnInfo(PGconn *pd, 
			       geomCol& details, 
                               bool searchGeometryColumnsOnly,
                               bool searchPublicOnly);
    // Set the position of the database connection list to the last
    // used one. 
    void setConnectionListPosition();
    // Show the context help for the dialog
    void showHelp();
    // update 'import as column'
    void updateImportAsInfo(int row, const QString &type);
    // Combine the schema, table and column data into a single string
    // useful for display to the user
    QString fullDescription(QString schema, QString table, QString column);
    // The column labels
    QStringList mColumnLabels;
    // Our thread for doing long running queries
    QgsGeomColumnTypeThread* mColumnTypeThread;
    QString m_connInfo;
    QStringList m_selectedTables;
    // Storage for the range of layer type icons
    QMap<QString, QPair<QString, QIcon> > mLayerIcons;
    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;
    PGconn *pd;
    static const int context_id = 939347163;
};


// Perhaps this class should be in its own file??
//
// A class that determines the geometry type of a given database 
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsGeomColumnTypeThread : public QThread
{
  Q_OBJECT
 public:

  void setConnInfo(QString s);
  void setGeometryColumn(QString schema, QString table, QString column);

  // These functions get the layer types and pass that information out
  // by emitting the setLayerType() signal. The getLayerTypes()
  // function does the actual work, but use the run() function if you
  // want the work to be done as a separate thread from the calling
  // process. 
  virtual void run() { getLayerTypes(); }
  void getLayerTypes();

  signals:
  void setLayerType(QString schema, QString table, QString column,
                    QString type);

 private:
  QString mConnInfo;
  std::vector<QString> schemas, tables, columns;
};

#endif // QGSDBSOURCESELECT_H
