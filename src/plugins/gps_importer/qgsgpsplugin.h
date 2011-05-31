/***************************************************************************
                          qgsgpsplugin.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGPSPLUGIN_H
#define QGSGPSPLUGIN_H
#include "qgsbabelformat.h"
#include "qgsgpsdevice.h"
#include "qgisplugin.h"

class QgisInterface;
class QgsVectorLayer;
class QAction;

/** A plugin with various GPS tools.
*/
class QgsGPSPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    /** Constructor for a plugin. The QgisInterface pointer
        is passed by QGIS when it attempts to instantiate the plugin.
        @param qI Pointer to the QgisInterface object.
    */
    QgsGPSPlugin( QgisInterface * );

    //! Destructor
    virtual ~QgsGPSPlugin();

  public slots:
    //! init the gui
    virtual void initGui();
    //! Show the dialog box
    void run();
    //! Create a new GPX layer
    void createGPX();
    //! Add a vector layer given vectorLayerPath, baseName, providerKey
    void drawVectorLayer( QString, QString, QString );
    //! unload the plugin
    void unload();
    //! show the help document
    void help();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );

    //! load a GPX file
    void loadGPXFile( QString fileName, bool loadWaypoints, bool loadRoutes,
                      bool loadTracks );
    void importGPSFile( QString inputFileName, QgsBabelFormat* importer,
                        bool importWaypoints, bool importRoutes,
                        bool importTracks, QString outputFileName,
                        QString layerName );
    void convertGPSFile( QString inputFileName,
                         int convertType,
                         QString outputFileName,
                         QString layerName );
    void downloadFromGPS( QString device, QString port,
                          bool downloadWaypoints, bool downloadRoutes,
                          bool downloadTracks, QString outputFileName,
                          QString layerName );
    void uploadToGPS( QgsVectorLayer* gpxLayer, QString device,
                      QString port );

  signals:

    void closeGui();

  private:

    //! Initializes all variables needed to run GPSBabel.
    void setupBabel();

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisInterface;
    //! Pointer to the QAction object used in the menu and toolbar
    QAction *mQActionPointer;
    //! Pointer to the QAction used for creating a new GPX layer
    QAction *mCreateGPXAction;
    //! The path to the GPSBabel program
    QString mBabelPath;
    //! Importers for external GPS data file formats
    std::map<QString, QgsBabelFormat*> mImporters;
    //! Upload/downloaders for GPS devices
    std::map<QString, QgsGPSDevice*> mDevices;
};

#endif
