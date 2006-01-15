/***************************************************************************
                          qgsmessageviewer.h  -  description
                             -------------------
    begin                : Wed Jun 4 2003
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
#ifndef QGSMESSAGEVIEWER_H
#define QGSMESSAGEVIEWER_H

#include "ui_qgsmessageviewer.h"
#include "qgisgui.h"

class QgsMessageViewer: public QDialog, private Ui::QgsMessageViewer
{
  public:
    QgsMessageViewer(QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    ~QgsMessageViewer();
    void setTextFormat(Qt::TextFormat f);
    void setMessage(const QString& msg);
    void appendMessage(const QString& msg);
};

#endif
