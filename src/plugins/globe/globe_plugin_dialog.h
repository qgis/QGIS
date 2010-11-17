/*
 * $Id$ 
 */
/***************************************************************************
    globe_plugin_dialog.h - settings dialog for the globe plugin
     --------------------------------------
    Date                 : 11-Nov-2010
    Copyright            : (C) 2010 by Marco Bernasocchi
    Email                : marco at bernawebdesign.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGIS_GLOBE_PLUGIN_DIALOG_H
#define QGIS_GLOBE_PLUGIN_DIALOG_H

#include <ui_globe_plugin_dialog_guibase.h>
#include <QDialog>
#include <QSettings>
#include "qgscontexthelp.h"

class QgsGlobePluginDialog:public QDialog, private Ui::QgsGlobePluginDialogGuiBase
{
  Q_OBJECT 
  
  public:
    QgsGlobePluginDialog( QWidget * parent = 0, Qt::WFlags fl = 0 );
    ~QgsGlobePluginDialog();

  private:
    QString stereoMode;
    QString earthFile;
    QString openFile();
    QSettings settings;
    void setStereoMode();
    void setEarthFile();
    void restartGlobe();
    bool globeRunning();
    void showMessageBox( QString text);

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonSelectEarthFile_clicked();
    void on_comboStereoMode_currentIndexChanged( QString mode );
};

#endif				// QGIS_GLOBE_PLUGIN_DIALOG_H
