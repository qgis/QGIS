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
    QMap<QString, QString> mExtensionMap;
    void enableOrDisableOkButton();

  private slots:
    void on_mButtonBox_accepted();
    void on_mButtonBox_rejected();
    void on_mButtonBox_helpRequested();
    void on_mBrowseButton_clicked(); // Function to open the file dialog
    void on_mOutputRasterLineEdit_editingFinished();

  signals:
    /*
     * Signal: createRaster
     * Params:
     *         QgsVectorLayer* -> Input point layer
     *         int             -> Buffer distance
     *         float           -> Decay ratio
     *         QString         -> Output filename
     *         QString         -> Output Format Short Name
     */
    void createRaster( QgsVectorLayer*, int, float, QString, QString );

};

#endif
