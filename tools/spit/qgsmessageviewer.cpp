/***************************************************************************
                          qgsmessageviewer.cpp  -  description
                             -------------------
    begin                : Tue Dec 23 2003
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

#include "qgsmessageviewer.h"

QgsMessageViewer::QgsMessageViewer(QWidget *parent, const char *name): QgsMessageViewerBase(parent, name){}
QgsMessageViewer::~QgsMessageViewer(){}
void QgsMessageViewer::setMessage(QString message){
  txtMessage->setText(message);
}
