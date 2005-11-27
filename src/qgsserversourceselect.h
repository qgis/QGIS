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
#include <map>
#include <utility>

class Q3ListBoxItem;
class QgisApp;

class QgsWmsProvider;

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

    //! Signaled when a layer selection is changed.  Ensures that only one style is selected per layer.
    void layerSelectionChanged();

    //! Connection name
    QString connName();
    //! Connection info (uri)
    QString connInfo();

    //! String list containing the selected layers
    QStringList selectedLayers();
    //! String list containing the visual styles selected for the selected layers - this corresponds with the output from selectedLayers()
    QStringList selectedStylesForSelectedLayers();

    //! String containing the MIME type of the preferred image encoding
    QString selectedImageEncoding();

private:

    //! Populate the layer list - private for now.
    void populateLayerList(QgsWmsProvider* wmsProvider);

    //! Populate the image encoding button group - private for now.
    void populateImageEncodingGroup(QgsWmsProvider* wmsProvider);

    QString m_connName;
    QString m_connInfo;
    QStringList m_selectedLayers;
    QStringList m_selectedStylesForSelectedLayers;

    std::map<QString, QString> m_selectedStyleIdForLayer;

    //! What MIME type corresponds to the Button ID in btnGrpImageEncoding?
    std::vector<QString> m_MimeTypeForButtonId;

    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;
};


#endif // QgsServerSourceSelect_H
