/***************************************************************************
                          qgspgnewconnection.h  -  description
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
#ifndef QGSPGNEWCONNECTION_H
#define QGSPGNEWCONNECTION_H
#include "ui_qgspgnewconnectionbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"
#include "qgsauthconfigselect.h"

/** \class QgsPgNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a PostgreSQL database
 */
class QgsPgNewConnection : public QDialog, private Ui::QgsPgNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsPgNewConnection( QWidget *parent = nullptr, const QString& connName = QString::null, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsPgNewConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
  public slots:
    void accept() override;
    void on_btnConnect_clicked();
    void on_cb_geometryColumnsOnly_clicked();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    QgsAuthConfigSelect * mAuthConfigSelect;
};

#endif //  QGSPGNEWCONNECTIONBASE_H
