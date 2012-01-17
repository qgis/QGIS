/***************************************************************************
 *Copyright (C) 2008 Paolo L. Scala, Barbara Rita Barricelli, Marco Padula *
 * CNR, Milan Unit (Information Technology),                               *
 * Construction Technologies Institute.\n";                                *
 *                                                                         *
 * email : Paolo L. Scala <scala@itc.cnr.it>                               *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef dxf2shpConverterGUI_H
#define dxf2shpConverterGUI_H

#include <QDialog>
#include <ui_dxf2shpconvertergui.h>

/**
  @author Tim Sutton
  */
class dxf2shpConverterGui: public QDialog, private Ui::dxf2shpConverterGui
{
    Q_OBJECT

  public:
    dxf2shpConverterGui( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~dxf2shpConverterGui();

  private:
    void getInputFileName();
    void getOutputFileName();
    void getOutputDir();

    void saveState();
    void restoreState();

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();
    void on_btnBrowseForFile_clicked();
    void on_btnBrowseOutputDir_clicked();

  signals:
    void createLayer( QString, QString );
};

#endif
