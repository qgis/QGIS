/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   Copyright (C) 2004 by Gary Sherman                                    *
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

#include "ui_qgsdelimitedtextpluginguibase.h"

#include <QDialog>
class QgisIface;

/**
 * \class QgsDelimitedTextPluginGui
 */
class QgsDelimitedTextPluginGui : public QDialog, private Ui::QgsDelimitedTextPluginGuiBase
{
  Q_OBJECT
  public:
    QgsDelimitedTextPluginGui();
    QgsDelimitedTextPluginGui( QgisIface * _qI, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsDelimitedTextPluginGui();
    public slots:
    void on_pbnOK_clicked();
    void on_pbnHelp_clicked();
    void on_btnBrowseForFile_clicked();
    void on_pbnParse_clicked();
    void updateFieldLists();
    void getOpenFileName();
    void enableButtons();
    void help();

  private:
    QgisIface * qI;
    static const int context_id = -1033030847;
signals:
    void drawRasterLayer(QString);
    void drawVectorLayer(QString,QString,QString);
};

#endif
