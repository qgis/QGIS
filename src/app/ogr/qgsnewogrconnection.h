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
/* $Id:$ */

#ifndef QGSNEWOGRCONNECTION_H
#define QGSNEWOGRCONNECTION_H
#include "ui_qgsnewogrconnectionbase.h"
#include "qgisgui.h"

/*! \class QgsNewOgrConnection
 * \brief Dialog to allow the user to define, test and save connection
 * information for OGR databases
 */
class QgsNewOgrConnection : public QDialog, private Ui::QgsNewOgrConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsNewOgrConnection( QWidget *parent = 0, const QString& connType = QString::null, const QString& connName = QString::null, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsNewOgrConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
    //! Saves the connection to ~/.qt/qgisrc
    void saveConnection();
    //! Display the context help
    void helpInfo();
  public slots:
    void accept();
    void help();
    void on_btnConnect_clicked();
  private:
    static const int context_id = 63428984;
};

#endif //  QGSNEWOGRCONNECTIONBASE_H
