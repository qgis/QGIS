/***************************************************************************
                          qgsmessageviewer.h  -  description
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

#include <qwidget.h>
#include <qstring.h>
#include <qtextedit.h>
#include "qgsmessageviewerbase.h"
 
class QgsMessageViewer: public QgsMessageViewerBase{
  public:
  QgsMessageViewer(QWidget *parent=0, const char *name=0);
  ~QgsMessageViewer();
  void setMessage(QString message);
};
