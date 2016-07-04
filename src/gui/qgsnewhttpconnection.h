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
#ifndef QGSNEWHTTPCONNECTION_H
#define QGSNEWHTTPCONNECTION_H
#include "ui_qgsnewhttpconnectionbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"
#include "qgsauthconfigselect.h"

/** \ingroup gui
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class GUI_EXPORT QgsNewHttpConnection : public QDialog, private Ui::QgsNewHttpConnectionBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsNewHttpConnection( QWidget *parent = nullptr, const QString& baseKey = "/Qgis/connections-wms/", const QString& connName = QString::null, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsNewHttpConnection();
  public slots:
    //! Saves the connection to ~/.qt/qgisrc
    void accept() override;

    void on_txtName_textChanged( const QString & );

    void on_txtUrl_textChanged( const QString & );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    QString mBaseKey;
    QString mCredentialsBaseKey;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    QgsAuthConfigSelect * mAuthConfigSelect;
};

#endif //  QGSNEWHTTPCONNECTION_H
