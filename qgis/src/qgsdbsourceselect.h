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
#ifdef WIN32
#include "qgsdbsourceselectbase.h"
#else
#include "qgsdbsourceselectbase.uic.h"
#endif

class QListBoxItem;
class QgisApp;
/*! \class QgsDbSourceSelect
 * \brief Dialog to create connections and add tables from PostgresQL.
 *
 * This dialog allows the user to define and save connection information
 * for PostGIS enabled PostgresQL databases. The user can then connect and add 
 * tables from the database to the map canvas.
 */
class QgsDbSourceSelect : public QgsDbSourceSelectBase 
{
  Q_OBJECT
 public:
    //! Constructor
    QgsDbSourceSelect(QgisApp *app=0, QWidget *parent = 0, const char *name = 0);
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
    /*! Connects to the database using the stored connection parameters. 
    * Once connected, available layers are displayed.
    */
    void dbConnect();
    //! String list containing the selected tables
    QStringList selectedTables();
    //! Connection info (database, host, user, password)
    QString connInfo();
    //! Add the layer selected when user double-clicks the mouse
    void addLayer(QListBoxItem *item);
    public slots:
      void setSql(QListViewItem *);
 private:
    QString m_connInfo;
    QStringList m_selectedTables;
    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;
};


#endif // QGSDBSOURCESELECT_H
