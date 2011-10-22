/***************************************************************************
    globe_plugin_dialog.h - settings dialog for the globe plugin
     --------------------------------------
    Date                 : 11-Nov-2010
    Copyright            : (C) 2010 by Marco Bernasocchi
    Email                : marco at bernawebdesign.ch
 ***************************************************************************
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
#include <QCheckBox>
#include "qgsosgviewer.h"
#include "qgscontexthelp.h"
#include <qgsproject.h>
#include <qgsvectorlayer.h>

class QgsGlobePluginDialog: public QDialog, private Ui::QgsGlobePluginDialogGuiBase
{
    Q_OBJECT

  public:
    QgsGlobePluginDialog( QgsOsgViewer* viewer, QWidget * parent = 0, Qt::WFlags fl = 0 );
    ~QgsGlobePluginDialog();
    void readElevationDatasources();
    QTableWidget *elevationDatasources() { return elevationDatasourcesWidget; }
    void updatePointLayers();
    QgsVectorLayer* modelLayer();
    QString modelPath() { return modelPathLineEdit->text(); }
  private:
    QgsOsgViewer* mViewer;
    QSettings settings;
  private:
    QString openRasterFile();
    void updateStereoDialog();
    bool validateResource( QString type, QString uri, QString& error );
    void saveElevationDatasources();
    void moveRow( QTableWidget* widget, bool up );
    QList<QTableWidgetItem*> takeRow( QTableWidget* widget, int row );
    void setRow( QTableWidget* widget, int row, const QList< QTableWidgetItem* >& rowItems );
    void showMessageBox( QString text );
    QList<QgsVectorLayer*> pointLayers();
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

    //STEREO
    void on_comboStereoMode_currentIndexChanged( QString value );
    void on_eyeSeparation_valueChanged( double value );
    void on_screenDistance_valueChanged( double value );
    void on_screenWidth_valueChanged( double value );
    void on_screenHeight_valueChanged( double value );
    void on_splitStereoHorizontalSeparation_valueChanged( int value );
    void on_splitStereoVerticalSeparation_valueChanged( int value );
    void on_splitStereoHorizontalEyeMapping_currentIndexChanged( int value );
    void on_splitStereoVerticalEyeMapping_currentIndexChanged( int value );
    void on_resetStereoDefaults_clicked();

    //MODEL
    void on_modelBrowse_clicked();

    //ELEVATION
    void on_elevationCombo_currentIndexChanged( QString type );
    void on_elevationBrowse_clicked();
    void on_elevationAdd_clicked();
    void on_elevationRemove_clicked();
    void on_elevationUp_clicked();
    void on_elevationDown_clicked();

  signals:
    void elevationDatasourcesChanged();
};

#endif    // QGIS_GLOBE_PLUGIN_DIALOG_H
