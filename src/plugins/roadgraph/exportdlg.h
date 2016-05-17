/***************************************************************************
  exportdlg.h
  --------------------------------------
  Date                 : 2010-11-29
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef ROADGRAPH_EXPORTDLG_H
#define ROADGRAPH_EXPORTDLG_H

#include <QDialog>

// forward declaration QT-classes
class QComboBox;

// forward declaration Qgis-classes

//forward declaration RoadGraph plugins classes
class QgsVectorLayer;

/**
@author Sergey Yakushev
*/
/**
* \class RgSettingsDlg
* \brief implement of export dialog
*/
class RgExportDlg : public QDialog
{
    Q_OBJECT
  public:
    RgExportDlg( QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr );
    ~RgExportDlg();

  public:
    QgsVectorLayer* mapLayer() const;

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();

  private:
    QComboBox *mcbLayers;
};
#endif
