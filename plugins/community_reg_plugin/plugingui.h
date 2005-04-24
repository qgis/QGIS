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
#ifndef QGSCOMMUNTYREGPLUGINGUI_H
#define QGSCOMMUNTYREGPLUGINGUI_H

#include <pluginguibase.h>
class QUrl;
class  QHttp;
/**
@author Tim Sutton
*/
class QgsCommunityRegPluginGui : public QgsCommunityRegPluginGuiBase
{
Q_OBJECT
public:
    QgsCommunityRegPluginGui();
    QgsCommunityRegPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsCommunityRegPluginGui();
public slots:
    void pbnOK_clicked();
    void pbnCancel_clicked();
    void submit();
    void submitDone( bool error );
    void pbnGetCoords_clicked();
private:
    QHttp * mConnection;  
    QHttp * mHttp;
signals:
   void drawRasterLayer(QString);
   void drawVectorrLayer(QString,QString,QString);
};

#endif
