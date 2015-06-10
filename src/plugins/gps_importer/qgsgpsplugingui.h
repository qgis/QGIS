/***************************************************************************
                          qgsgpsplugingui.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSPLUGINGUI_H
#define QGSGPSPLUGINGUI_H

#include "qgsvectorlayer.h"
#include "ui_qgsgpspluginguibase.h"
#include "qgsbabelformat.h"
#include "qgsgpsdevice.h"
#include "qgscontexthelp.h"

#include <vector>

#include <QString>


/**
@author Tim Sutton
*/
class QgsGPSPluginGui : public QDialog, private Ui::QgsGPSPluginGuiBase
{
    Q_OBJECT

  public:
    QgsGPSPluginGui( const BabelMap& importers,
                     std::map<QString, QgsGPSDevice*>& devices,
                     std::vector<QgsVectorLayer*> gpxMapLayers,
                     QWidget* parent, Qt::WindowFlags );
    ~QgsGPSPluginGui();

  public slots:

    void openDeviceEditor();
    void devicesUpdated();
    void enableRelevantControls();

    void on_pbnGPXSelectFile_clicked();

    void on_pbnIMPInput_clicked();
    void on_pbnIMPOutput_clicked();

    void on_pbnCONVInput_clicked();
    void on_pbnCONVOutput_clicked();

    void on_pbnDLOutput_clicked();

  private:
    void populateDeviceComboBox();
    void populateULLayerComboBox();
    void populateIMPBabelFormats();
    void populatePortComboBoxes();
    void populateCONVDialog();

    void saveState();
    void restoreState();

#if 0
    void populateLoadDialog();
    void populateDLDialog();
    void populateULDialog();
    void populateIMPDialog();
#endif

  private slots:

    void on_pbnRefresh_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  signals:
    void drawRasterLayer( QString );
    void drawVectorLayer( QString, QString, QString );
    void loadGPXFile( QString fileName, bool showWaypoints, bool showRoutes,
                      bool showTracks );
    void importGPSFile( QString inputFileName, QgsBabelFormat* importer,
                        bool importWaypoints, bool importRoutes,
                        bool importTracks, QString outputFileName,
                        QString layerName );
    void convertGPSFile( QString inputFileName,
                         int convertType,
                         QString outputFileName,
                         QString layerName );
    void downloadFromGPS( QString device, QString port, bool downloadWaypoints,
                          bool downloadRoutes, bool downloadTracks,
                          QString outputFileName, QString layerName );
    void uploadToGPS( QgsVectorLayer* gpxLayer, QString device, QString port );

  private:

    std::vector<QgsVectorLayer*> mGPXLayers;
    const BabelMap& mImporters;
    std::map<QString, QgsGPSDevice*>& mDevices;
    QString mBabelFilter;
    QString mImpFormat;
    QAbstractButton *pbnOK;
};

#endif
