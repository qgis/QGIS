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

#include <pluginguibase.h>

/**
@author Peter Brewer
*/
class PluginGui : public PluginGuiBase
{
Q_OBJECT;
public:
    PluginGui();
    PluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();
    void setPlacement(QString);
    void setPreferredSize(int);
    void setSnapping(bool);
    void setEnabled(bool);
    void setStyle(QString);
    void setColour(QColor);

    
    
private:
    
signals:
   void drawRasterLayer(QString);
   void drawVectorrLayer(QString,QString,QString);
   void changePlacement(QString);
   void changePreferredSize(int);
   void changeSnapping(bool);
   void changeEnabled(bool);
   void changeStyle(QString);
   void changeColour(QColor);
   void refreshCanvas();
};

#endif
