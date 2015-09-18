/***************************************************************************
                          qgsnewogrconnection.h  -  description
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWOGRCONNECTION_H
#define QGSNEWOGRCONNECTION_H
#include "ui_qgsnewogrconnectionbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

/** \class QgsNewOgrConnection
 * \brief Dialog to allow the user to define, test and save connection
 * information for OGR databases
 */
class QgsNewOgrConnection : public QDialog, private Ui::QgsNewOgrConnectionBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsNewOgrConnection( QWidget *parent = 0, const QString& connType = QString::null, const QString& connName = QString::null, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsNewOgrConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
  public slots:
    void accept() override;
    void on_btnConnect_clicked();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    QString mOriginalConnName;
};

#endif //  QGSNEWOGRCONNECTIONBASE_H
