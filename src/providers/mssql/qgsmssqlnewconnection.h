/***************************************************************************
                          qgsmssqlnewconnection.h  -  description
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMSSQLNEWCONNECTION_H
#define QGSMSSQLNEWCONNECTION_H
#include "ui_qgsmssqlnewconnectionbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"


/**
 * \class QgsMssqlNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for an MSSQL database
 */
class QgsMssqlNewConnection : public QDialog, private Ui::QgsMssqlNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMssqlNewConnection( QWidget *parent = nullptr, const QString &connName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Tests the connection using the parameters supplied
    bool testConnection( const QString &testDatabase = QString() );

    /**
     * \brief List all databases found for the given server.
     */
    void listDatabases();
  public slots:
    void accept() override;
    void btnListDatabase_clicked();
    void btnConnect_clicked();
    void cb_trustedConnection_clicked();
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();

};

#endif //  QGSMSSQLNEWCONNECTION_H
