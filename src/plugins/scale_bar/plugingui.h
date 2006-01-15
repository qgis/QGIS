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
#ifndef QGSSCALEBARPLUGINGUI_H
#define QGSSCALEBARPLUGINGUI_H

#include <ui_pluginguibase.h>
#include <QDialog>

/**
@author Peter Brewer
*/
class QgsScaleBarPluginGui : public QDialog, private Ui::QgsScaleBarPluginGuiBase
{
Q_OBJECT;
public:
    QgsScaleBarPluginGui();
    QgsScaleBarPluginGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsScaleBarPluginGui();
    void setPlacement(QString);
    void setPreferredSize(int);
    void setSnapping(bool);
    void setEnabled(bool);
    void setStyle(QString);
    void setColour(QColor);

    
   //accessor for getting a pointer to the size spin widget
   QSpinBox * getSpinSize();

public slots:
    void on_pbnOK_clicked();
    void on_pbnCancel_clicked();
    void on_pbnChangeColour_clicked();
    
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
