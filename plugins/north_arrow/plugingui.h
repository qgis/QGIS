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
@author Tim Sutton
*/
class QgsNorthArrowPluginGui : public QgsNorthArrowPluginGuiBase
{
Q_OBJECT
public:
    QgsNorthArrowPluginGui();
    QgsNorthArrowPluginGui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsNorthArrowPluginGui();
    void pbnOK_clicked();
    void pbnCancel_clicked();

private:
    void rotatePixmap(int theRotationInt);
    void paintEvent( QPaintEvent * );//overloads qwidget
    void resizeEvent(QResizeEvent *); //overloads qwidget
signals:
   //void drawRasterLayer(QString);
   //void drawVectorrLayer(QString,QString,QString);
   void rotationChanged(int);
   void changePlacement(QString);
   // enable NorthArrow
   void enableNorthArrow(bool);

public slots:
    void setRotation(int);
    void setPlacement(QString thePlacementQString);
    void setEnabled(bool);
    
private slots:
    //overides function byt the same name created in .ui
    void spinSize_valueChanged( int theInt);
    //overides function byt the same name created in .ui
    void sliderRotation_valueChanged( int theInt);
};

#endif
