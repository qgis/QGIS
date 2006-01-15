/***************************************************************************
                          qgsmessageviewer.cpp  -  description
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

#include "qgsmessageviewer.h"


QgsMessageViewer::QgsMessageViewer(QWidget *parent, Qt::WFlags fl)
: QDialog(parent, fl)
{
  setupUi(this);
}

QgsMessageViewer::~QgsMessageViewer()
{
}

void QgsMessageViewer::setTextFormat(Qt::TextFormat f)
{
  txtMessage->setTextFormat(f);
}

void QgsMessageViewer::setMessage(const QString& msg)
{
  txtMessage->setText(msg);
}

void QgsMessageViewer::appendMessage(const QString& msg)
{
  txtMessage->append(msg);
}
