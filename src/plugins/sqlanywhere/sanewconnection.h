/***************************************************************************
  sanewconnection.h
  Dialogue box for defining new connections to a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SANEWCONNECTION_H
#define SANEWCONNECTION_H
#include "ui_sanewconnectionbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

/*! \class SaNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a SQL Anywhere database
 */
class SaNewConnection : public QDialog, private Ui::SaNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    SaNewConnection( QWidget *parent = 0, const QString& connName = QString::null, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~SaNewConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
  public slots:
    void accept();
    void on_btnConnect_clicked();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
};

#endif //  SANEWCONNECTIONBASE_H
