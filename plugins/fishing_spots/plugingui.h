/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#include <pluginguibase.uic.h>
//
// QT includes
//
#include <qstring.h>
#include <qobject.h>
#include <qurl.h>
#include <qhttp.h>

/**
@author Tim Sutton
*/
class PluginGui : public PluginGuiBase
{
Q_OBJECT
public:
    PluginGui();
    PluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    void check();

    
protected slots:

  void slotStateChanged(int state);
  void slotResponseHeaderReceived ( const QHttpResponseHeader & resp );
  void slotRequestFinished ( int id, bool error );
  void slotTimeOut();
  
signals:

  void finished();

signals:
   void drawRasterLayer(QString);
   void drawVectorrLayer(QString,QString,QString);

private:

  QUrl mQUrl;
  int mTimeOutInt;
  QHttp mQhttp;
  int mSetHostIdInt;
  int mHeaderIdInt;
  int mGetIdInt;
  QString mRequestQString;
  QHttpResponseHeader mQHttpResponseHeader;

  void requestHeadFinished(int id);
  void requestGetFinished(int id);
  void finish();
};

#endif
