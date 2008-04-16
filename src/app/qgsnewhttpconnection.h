/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
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
#ifndef QGSNEWHTTPCONNECTION_H
#define QGSNEWHTTPCONNECTION_H
#include "ui_qgsnewhttpconnectionbase.h"
#include "qgisgui.h"
/*! 
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class QgsNewHttpConnection : public QDialog, private Ui::QgsNewHttpConnectionBase 
{
  Q_OBJECT
 public:
    //! Constructor
    QgsNewHttpConnection(QWidget *parent = 0, const QString& baseKey = "/Qgis/connections-wms/", const QString& connName = QString::null, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    //! Destructor
    ~QgsNewHttpConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
 public slots:
    //! Saves the connection to ~/.qt/qgisrc
    void saveConnection();
    //! Show context help
    void on_btnHelp_clicked();
 private:
    QString mBaseKey;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    static const int context_id = 308026563;
};

#endif //  QGSNEWHTTPCONNECTION_H
