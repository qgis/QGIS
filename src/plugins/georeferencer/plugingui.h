/***************************************************************************
 *   Copyright (C) 2005 by Lars Luthman
 *   larsl@users.sourceforge.net
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

#include <ui_pluginguibase.h>
#include <QDialog>

class QgisInterface;
class QgsPointDialog;

/**
@author Tim Sutton
*/
class QgsGeorefPluginGui : public QDialog, private Ui::QgsGeorefPluginGuiBase
{
Q_OBJECT
public:
    QgsGeorefPluginGui();
    QgsGeorefPluginGui(QgisInterface* theQgisInterface, QWidget* parent = 0, Qt::WFlags fl = 0);
    ~QgsGeorefPluginGui();

    /**Finds the qgis main window
     @return window pointer or 0 in case or error*/
    static QWidget* findMainWindow();
    
public slots:
    void on_pbnClose_clicked();
    void on_pbnDescription_clicked();
    void on_pbnSelectRaster_clicked();
    void on_mArrangeWindowsButton_clicked();
    
private:
    
   QString mProjBehaviour, mProjectCRS;
   int mProjectCRSID;
   QgisInterface* mIface;
   /**dialog to enter reference point*/
   QgsPointDialog* mPointDialog;
   /**Flag if plugin windows have been arranged with button*/
   bool mPluginWindowsArranged;
   QSize origSize;
   QPoint origPos;
};

#endif
