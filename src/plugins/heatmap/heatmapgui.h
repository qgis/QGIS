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
#ifndef HeatmapGUI_H
#define HeatmapGUI_H

#include <QDialog>
#include <ui_heatmapguibase.h>

#include "qgsvectorlayer.h"

/**
@author Tim Sutton
*/
class HeatmapGui : public QDialog, private Ui::HeatmapGuiBase
{
    Q_OBJECT
  public:
    HeatmapGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~HeatmapGui();

  private:
    static const int context_id = 0;
    void enableOrDisableOkButton();

  private slots:
    void on_mButtonBox_rejected();
    void on_mButtonBox_accepted();
    void on_mBrowseButton_clicked();

  signals:
    void createRasterOutput( QgsVectorLayer* );

};

#endif
