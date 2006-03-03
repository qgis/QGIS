/***************************************************************************
                          qgsconnectiondialog.h  -  description
                             -------------------
    begin                : Thu Dec 10 2003
    copyright            : (C) 2003 by Denis Antipov
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONNECTIONDIALOG_H
#define QGSCONNECTIONDIALOG_H

// $Id$

#include "ui_qgsconnectiondialogbase.h"
#include "qgisgui.h"

class QgsConnectionDialog : public QDialog, private Ui::QgsConnectionDialogBase
{
  Q_OBJECT
 public:

    QgsConnectionDialog(QWidget *parent = 0, const QString& connName = QString::null, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    ~QgsConnectionDialog();
    void testConnection();
    void saveConnection();
    void helpInfo();

public slots:

  void on_btnOk_clicked()      { saveConnection(); }
  void on_btnCancel_clicked()  { done(1); }
  void on_btnHelp_clicked()    { helpInfo(); }
  void on_btnConnect_clicked() { testConnection(); }
};

#endif
