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

#include "qgsconnectiondialogbase.h"

class QgsConnectionDialog : public QgsConnectionDialogBase
{
 public:

    QgsConnectionDialog(QWidget* parent = 0, QString connName=QString::null, bool modal = true, WFlags fl = 0);
    ~QgsConnectionDialog();
    void testConnection();
    void saveConnection();
    void helpInfo();
};

#endif
