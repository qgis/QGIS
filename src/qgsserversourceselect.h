/***************************************************************************
    qgserversourceselect.h  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
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

#ifndef QGSSERVERSOURCESELECT_H
#define QGSSERVERSOURCESELECT_H
#ifdef WIN32
#include "qgsserversourceselectbase.h"
#else
#include "qgsserversourceselectbase.uic.h"
#endif

#include <vector>
#include <utility>

class QListBoxItem;
class QgisApp;
/*!
 * \brief   Dialog to create connections and add layers from WMS, etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add 
 * layers from the WMS server to the map canvas.
 */
class QgsServerSourceSelect : public QgsServerSourceSelectBase 
{
  Q_OBJECT
  
public:

    //! Constructor
    QgsServerSourceSelect(QgisApp *app=0, QWidget *parent = 0, const char *name = 0);
    //! Destructor
    ~QgsServerSourceSelect();
    //! Opens the create connection dialog to build a new connection
    void addNewConnection();
    //! Opens a dialog to edit an existing connection
    void editConnection();
	//! Deletes the selected connection
	void deleteConnection();
	//! Populate the connection list combo box
	void populateConnectionList();
    /*! Connects to the database using the stored connection parameters. 
    * Once connected, available layers are displayed.
    */
    void serverConnect();
    
    //! Determines the layers the user selected and closes the dialog
    void addLayers();
    
    //! Connection name
    QString connName();
    //! Connection info (uri)
    QString connInfo();
    //! String list containing the selected layers
    QStringList selectedLayers();
 
private:

    QString m_connName;
    QString m_connInfo;
    QStringList m_selectedLayers;
    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;
};


#endif // QgsServerSourceSelect_H
