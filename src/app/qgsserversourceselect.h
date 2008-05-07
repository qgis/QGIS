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
#include "ui_qgsserversourceselectbase.h"
#include "qgisgui.h"

#include <vector>
#include <map>
#include <set>

class QgisApp;
class QgsWmsProvider;
class QButtonGroup;
/*!
 * \brief   Dialog to create connections and add layers from WMS, etc.
 *
 * This dialog allows the user to define and save connection information
 * for WMS servers, etc.
 *
 * The user can then connect and add 
 * layers from the WMS server to the map canvas.
 */
class QgsServerSourceSelect : public QDialog, private Ui::QgsServerSourceSelectBase 
{
  Q_OBJECT
  
public:

    //! Constructor
    QgsServerSourceSelect(QgisApp *app, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    //! Destructor
    ~QgsServerSourceSelect();
    //! Populate the connection list combo box
    void populateConnectionList();

    //! Connection name
    QString connName();

    //! Connection info (uri)
    QString connInfo();

    //! Connection Proxy Host
    QString connProxyHost();

    //! Connection Proxy Port
    int connProxyPort();

    //! Connection Proxy User
    QString connProxyUser();

    //! Connection Proxy Pass
    QString connProxyPass();

    //! String list containing the selected layers
    QStringList selectedLayers();

    //! String list containing the visual styles selected for the selected layers - this corresponds with the output from selectedLayers()
    QStringList selectedStylesForSelectedLayers();

    //! String containing the MIME type of the preferred image encoding
    QString selectedImageEncoding();

    //! String containing the selected WMS-format CRS
    QString selectedCrs();

    //! Stores which server is now selected.
    void serverChanged();

    //! Set the server connection combo box to that stored in the config file. 
    void setConnectionListPosition();

public slots:

    //! Opens the create connection dialog to build a new connection
    void on_btnNew_clicked();
    //! Opens a dialog to edit an existing connection
    void on_btnEdit_clicked();
    //! Deletes the selected connection
    void on_btnDelete_clicked();

    /*! Connects to the database using the stored connection parameters. 
    * Once connected, available layers are displayed.
    */
    void on_btnConnect_clicked();

    //! Determines the layers the user selected and closes the dialog
    void on_btnAdd_clicked();

    //! Opens the Spatial Reference System dialog.
    void on_btnChangeSpatialRefSys_clicked();

    //! Opens help application
    void on_btnHelp_clicked();

    //! Signaled when a layer selection is changed.  Ensures that only one style is selected per layer.
    void on_lstLayers_itemSelectionChanged();

    //! Set status message to theMessage
    void showStatusMessage(QString const & theMessage);

    //! show whatever error is exposed by the QgsWmsProvider.
    void showError(QgsWmsProvider * wms);

    //! Stores the selected datasource whenerver it is changed
    void on_cmbConnections_activated(int);

    //! Add some default wms servers to the list
    void on_btnAddDefault_clicked();

private:

    //! Add a few example servers to the list.
    void addDefaultServers();

    /**
     * \brief Populate the layer list - private for now.
     *
     * \retval FALSE if the layers could not be retreived or parsed - 
     *         see mWmsProvider->errorString() for more info
     */
    bool populateLayerList(QgsWmsProvider* wmsProvider);

    //! Populate the image encoding button group - private for now.
    void populateImageEncodingGroup(QgsWmsProvider* wmsProvider);

    //! Returns the common CRSs for the selected layers.
    std::set<QString> crsForSelection();

    //! Returns a textual description for the EPSG number
    QString descriptionForEpsg(long epsg);

    //! Name for selected connection
    QString m_connName;

    //! URI for selected connection
    QString m_connInfo;

    //! Proxy Host for selected connection
    QString m_connProxyHost;

    //! Proxy Port for selected connection
    int m_connProxyPort;

    //! Proxy User for selected connection
    QString m_connProxyUser;

    //! Proxy Pass for selected connection
    QString m_connProxyPass;

    QStringList m_selectedLayers;
    QStringList m_selectedStylesForSelectedLayers;
    long m_Epsg;

    std::map<QString, QString> m_selectedStyleIdForLayer;

    //! The mime type, the text to use in the button and a unique number
    QMap<QString, QPair<QString, int> > m_PotentialFormats;

    //! Pointer to the qgis application mainwindow
    QgisApp *qgisApp;

    //! The widget that controls the image format radio buttons
    QButtonGroup* m_imageFormatGroup;
    QHBoxLayout* m_imageFormatLayout;

    //! The WMS provider that retreives information for this dialog
    QgsWmsProvider * mWmsProvider;

    static const int context_id = 710979116;
};


#endif // QgsServerSourceSelect_H
