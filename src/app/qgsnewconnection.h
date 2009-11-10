/***************************************************************************
                          qgsnewconnection.h  -  description
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
/* $Id$ */
#ifndef QGSNEWCONNECTION_H
#define QGSNEWCONNECTION_H
#include "ui_qgsnewconnectionbase.h"
#include "qgisgui.h"
/*! \class QgsNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a PostgresQl database
 */
class QgsNewConnection : public QDialog, private Ui::QgsNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsNewConnection( QWidget *parent = 0, const QString& connName = QString::null, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsNewConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
    //! Saves the connection to ~/.qt/qgisrc
    void saveConnection();
  public slots:
    void accept();
    void helpClicked();
    void on_btnConnect_clicked();
    void on_cb_geometryColumnsOnly_clicked();
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    static const int context_id = 929865718;
};

#endif //  QGSNEWCONNECTIONBASE_H
