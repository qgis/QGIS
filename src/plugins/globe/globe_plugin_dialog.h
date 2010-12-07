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
    QString earthFile;
    QString openFile();
    QSettings settings;
    void updateStereoDialog();
    void restartGlobe();
    bool globeRunning();
    void showMessageBox( QString text);
    //! Set osg/DisplaySettings
    void setStereoConfig();
    //! Init dialog from settings using defaults from osg/DisplaySettings
    void loadStereoConfig();
    //! Save settings
    void saveStereoConfig();
    //! Handle stereoMode
    void setStereoMode();

  private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_comboStereoMode_currentIndexChanged(QString value);
    void on_eyeSeparation_valueChanged(double value);
    void on_screenDistance_valueChanged(double value);
    void on_screenWidth_valueChanged(double value);
    void on_screenHeight_valueChanged(double value);
    void on_splitStereoHorizontalSeparation_valueChanged(int value);
    void on_splitStereoVerticalSeparation_valueChanged(int value);
    void on_splitStereoHorizontalEyeMapping_currentIndexChanged(int value);
    void on_splitStereoVerticalEyeMapping_currentIndexChanged(int value);
    void on_resetStereoDefaults_clicked();
};

#endif				// QGIS_GLOBE_PLUGIN_DIALOG_H
