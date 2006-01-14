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
#include <QDialog>
extern "C"
{
#include <libpq-fe.h>
}

#include <vector>
#include <utility>

class Q3ListBoxItem;
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
    QgsDbSourceSelect(QgisApp *app);
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
    //! Add the layer selected when user double-clicks the mouse
    void addLayer(Q3ListBoxItem *item);
    //! Return the name of the selected encoding (e.g. UTf-8, ISO-8559-1, etc/)
    QString encoding();
    // Store the selected database
    void dbChanged();
    public slots:
    /*! Connects to the database using the stored connection parameters. 
    * Once connected, available layers are displayed.
    */
      void on_btnConnect_clicked();
      void on_btnAdd_clicked();
      void on_btnNew_clicked();
      void on_btnEdit_clicked();
      void on_btnDelete_clicked();
      void on_lstTables_doubleClicked(Q3ListViewItem *);
      void setSql(Q3ListViewItem *);
      void on_btnHelp_clicked();
 private:

    typedef std::pair<QString, QString> geomPair;
    typedef std::vector<geomPair > geomCol;

    bool getGeometryColumnInfo(PGconn *pd, 
			       geomCol& details);
    // Set the position of the database connection list to the last
    // used one. 
    void setConnectionListPosition();
    // Show the context help for the dialog
    void showHelp();

    QString m_connInfo;
    QStringList m_selectedTables;
    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;
    PGconn *pd;
    static const int context_id = 1244423922;
};


#endif // QGSDBSOURCESELECT_H
