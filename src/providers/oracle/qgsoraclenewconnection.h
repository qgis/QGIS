/***************************************************************************
                          qgsoraclenewconnection.h  -  description
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLENEWCONNECTION_H
#define QGSORACLENEWCONNECTION_H
#include "ui_qgsoraclenewconnectionbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

/*! \class QgsOracleNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a Oracle database
 */
class QgsOracleNewConnection : public QDialog, private Ui::QgsOracleNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsOracleNewConnection( QWidget *parent = 0, const QString& connName = QString::null, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsOracleNewConnection();
  public slots:
    void accept();
    void on_btnConnect_clicked();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
};

#endif //  QGSORACLENEWCONNECTIONBASE_H
