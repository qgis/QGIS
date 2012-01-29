/***************************************************************************
  qgsgpsplugin.cpp - GPS related tools
   -------------------
  Date                 : Jan 21, 2004
  Copyright            : (C) 2004 by Tim Sutton
  Email                : tim@linfiniti.com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// includes

#include "qgisinterface.h"
#include "qgisgui.h"
#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"
#include "qgsvectordataprovider.h"
#include "qgsgpsplugin.h"
#include "qgslogger.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QFile>
#include <QToolBar>
#include <QProcess>
#include <QProgressDialog>
#include <QSettings>
#include <QStringList>

//non qt includes
#include <cassert>
#include <fstream>

//the gui subclass
#include "qgsgpsplugingui.h"


static const QString name_ = QObject::tr( "GPS Tools" );
static const QString description_ = QObject::tr( "Tools for loading and importing GPS data" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
static const QString icon_ = ":/gps_importer.png";


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsGPSPlugin::QgsGPSPlugin( QgisInterface * theQgisInterFace ):
    QgisPlugin( name_, description_, version_, type_ ),
    mQGisInterface( theQgisInterFace )
{
  setupBabel();
}

QgsGPSPlugin::~QgsGPSPlugin()
{
  // delete all our babel formats
  BabelMap::iterator iter;
  for ( iter = mImporters.begin(); iter != mImporters.end(); ++iter )
    delete iter->second;
  std::map<QString, QgsGPSDevice*>::iterator iter2;
  for ( iter2 = mDevices.begin(); iter2 != mDevices.end(); ++iter2 )
    delete iter2->second;
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGPSPlugin::initGui()
{
  // add an action to the toolbar
  mQActionPointer = new QAction( QIcon(), tr( "&GPS Tools" ), this );
  mCreateGPXAction = new QAction( QIcon(), tr( "&Create new GPX layer" ), this );
  setCurrentTheme( "" );

  mQActionPointer->setWhatsThis( tr( "Creates a new GPX layer and displays it on the map canvas" ) );
  mCreateGPXAction->setWhatsThis( tr( "Creates a new GPX layer and displays it on the map canvas" ) );
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  connect( mCreateGPXAction, SIGNAL( triggered() ), this, SLOT( createGPX() ) );

  mQGisInterface->layerToolBar()->addAction( mQActionPointer );
  mQGisInterface->addPluginToMenu( tr( "&Gps" ), mQActionPointer );
  mQGisInterface->addPluginToMenu( tr( "&Gps" ), mCreateGPXAction );

  // this is called when the icon theme is changed
  connect( mQGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );
}

//method defined in interface
void QgsGPSPlugin::help()
{
  //implement me!
}

// Slot called when the menu item is activated
void QgsGPSPlugin::run()
{
  // find all GPX layers
  std::vector<QgsVectorLayer*> gpxLayers;
  QMap<QString, QgsMapLayer*>::const_iterator iter;
  QgsMapLayerRegistry* registry = QgsMapLayerRegistry::instance();
  for ( iter =  registry->mapLayers().begin();
        iter != registry->mapLayers().end(); ++iter )
  {
    if ( iter.value()->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer *>( iter.value() );
      if ( vLayer->providerType() == "gpx" )
        gpxLayers.push_back( vLayer );
    }
  }

  QgsGPSPluginGui *myPluginGui =
    new QgsGPSPluginGui( mImporters, mDevices, gpxLayers, mQGisInterface->mainWindow(),
                         QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //listen for when the layer has been made so we can draw it
  connect( myPluginGui, SIGNAL( drawVectorLayer( QString, QString, QString ) ),
           this, SLOT( drawVectorLayer( QString, QString, QString ) ) );
  connect( myPluginGui, SIGNAL( loadGPXFile( QString, bool, bool, bool ) ),
           this, SLOT( loadGPXFile( QString, bool, bool, bool ) ) );
  connect( myPluginGui, SIGNAL( importGPSFile( QString, QgsBabelFormat*, bool,
                                bool, bool, QString, QString ) ),
           this, SLOT( importGPSFile( QString, QgsBabelFormat*, bool, bool,
                                      bool, QString, QString ) ) );
  connect( myPluginGui, SIGNAL( convertGPSFile( QString, int,
                                QString, QString ) ),
           this, SLOT( convertGPSFile( QString, int,
                                       QString, QString ) ) );
  connect( myPluginGui, SIGNAL( downloadFromGPS( QString, QString, bool, bool,
                                bool, QString, QString ) ),
           this, SLOT( downloadFromGPS( QString, QString, bool, bool, bool,
                                        QString, QString ) ) );
  connect( myPluginGui, SIGNAL( uploadToGPS( QgsVectorLayer*, QString, QString ) ),
           this, SLOT( uploadToGPS( QgsVectorLayer*, QString, QString ) ) );
  connect( this, SIGNAL( closeGui() ), myPluginGui, SLOT( close() ) );

  myPluginGui->show();
}

void QgsGPSPlugin::createGPX()
{
  QSettings settings;
  QString dir = settings.value( "/Plugin-GPS/gpxdirectory", "." ).toString();
  QString fileName =
    QFileDialog::getSaveFileName( mQGisInterface->mainWindow(),
                                  tr( "Save new GPX file as..." ),
                                  dir,
                                  tr( "GPS eXchange file" ) + " (*.gpx)" );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.toLower().endsWith( ".gpx" ) )
    {
      fileName += ".gpx";
    }
    QFileInfo fileInfo( fileName );
    std::ofstream ofs( fileName.toUtf8() );
    if ( !ofs )
    {
      QMessageBox::warning( NULL, tr( "Could not create file" ),
                            tr( "Unable to create a GPX file with the given name. "
                                "Try again with another name or in another "
                                "directory." ) );
      return;
    }
    settings.setValue( "/Plugin-GPS/gpxdirectory", fileInfo.absolutePath() );

    ofs << "<gpx></gpx>" << std::endl;

    emit drawVectorLayer( fileName + "?type=track",
                          fileInfo.baseName() + ", tracks", "gpx" );
    emit drawVectorLayer( fileName + "?type=route",
                          fileInfo.baseName() + ", routes", "gpx" );
    emit drawVectorLayer( fileName + "?type=waypoint",
                          fileInfo.baseName() + ", waypoints", "gpx" );
  }
}

void QgsGPSPlugin::drawVectorLayer( QString thePathNameQString,
                                    QString theBaseNameQString,
                                    QString theProviderQString )
{
  mQGisInterface->addVectorLayer( thePathNameQString, theBaseNameQString,
                                  theProviderQString );
}

// Unload the plugin by cleaning up the GUI
void QgsGPSPlugin::unload()
{
  // remove the GUI
  mQGisInterface->layerToolBar()->addAction( mQActionPointer );
  mQGisInterface->removePluginMenu( tr( "&Gps" ), mQActionPointer );
  mQGisInterface->removePluginMenu( tr( "&Gps" ), mCreateGPXAction );
  mQGisInterface->removeToolBarIcon( mQActionPointer );
  delete mQActionPointer;
}

void QgsGPSPlugin::loadGPXFile( QString fileName, bool loadWaypoints, bool loadRoutes,
                                bool loadTracks )
{
  //check if input file is readable
  QFileInfo fileInfo( fileName );
  if ( !fileInfo.isReadable() )
  {
    QMessageBox::warning( NULL, tr( "GPX Loader" ),
                          tr( "Unable to read the selected file.\n"
                              "Please reselect a valid file." ) );
    return;
  }

  // add the requested layers
  if ( loadTracks )
    emit drawVectorLayer( fileName + "?type=track",
                          fileInfo.baseName() + ", tracks", "gpx" );
  if ( loadRoutes )
    emit drawVectorLayer( fileName + "?type=route",
                          fileInfo.baseName() + ", routes", "gpx" );
  if ( loadWaypoints )
    emit drawVectorLayer( fileName + "?type=waypoint",
                          fileInfo.baseName() + ", waypoints", "gpx" );

  emit closeGui();
}

void QgsGPSPlugin::importGPSFile( QString inputFileName, QgsBabelFormat* importer,
                                  bool importWaypoints, bool importRoutes,
                                  bool importTracks, QString outputFileName,
                                  QString layerName )
{
  // what features does the user want to import?
  QString typeArg;
  if ( importWaypoints )
    typeArg = "-w";
  else if ( importRoutes )
    typeArg = "-r";
  else if ( importTracks )
    typeArg = "-t";

  // try to start the gpsbabel process
  QStringList babelArgs =
    importer->importCommand( mBabelPath, typeArg,
                             inputFileName, outputFileName );

  QgsDebugMsg( QString( "Import command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( " " ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( NULL, tr( "Could not start process" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Importing data..." ), tr( "Cancel" ), 0, 0 );
  progressDialog.setWindowModality( Qt::WindowModal );
  for ( int i = 0; babelProcess.state() == QProcess::Running; ++i )
  {
    progressDialog.setValue( i / 64 );
    if ( progressDialog.wasCanceled() )
      return;
  }

  babelProcess.waitForFinished();

  // did we get any data?
  if ( babelProcess.exitCode() != 0 )
  {
    QString babelError( babelProcess.readAllStandardError() );
    QString errorMsg( tr( "Could not import data from %1!\n\n" )
                      .arg( inputFileName ) );
    errorMsg += babelError;
    QMessageBox::warning( NULL, tr( "Error importing data" ), errorMsg );
    return;
  }

  // add the layer
  if ( importTracks )
    emit drawVectorLayer( outputFileName + "?type=track",
                          layerName, "gpx" );
  if ( importRoutes )
    emit drawVectorLayer( outputFileName + "?type=route",
                          layerName, "gpx" );
  if ( importWaypoints )
    emit drawVectorLayer( outputFileName + "?type=waypoint",
                          layerName, "gpx" );

  emit closeGui();
}

void QgsGPSPlugin::convertGPSFile( QString inputFileName,
                                   int convertType,
                                   QString outputFileName,
                                   QString layerName )
{
  // what features does the user want to import?
  QStringList convertStrings;

  switch ( convertType )
  {
    case 0: convertStrings << "-x" << "transform,wpt=rte,del"; break;
    case 1: convertStrings << "-x" << "transform,rte=wpt,del"; break;
    case 2: convertStrings << "-x" << "transform,trk=wpt,del"; break;
    case 3: convertStrings << "-x" << "transform,wpt=trk,del"; break;
    default:
      QgsDebugMsg( "Illegal conversion index!" );
      return;
  }

  // try to start the gpsbabel process
  QStringList babelArgs;
  babelArgs << mBabelPath << "-i" << "gpx" << "-f" << QString( "\"%1\"" ).arg( inputFileName )
  << convertStrings << "-o" << "gpx" << "-F" << QString( "\"%1\"" ).arg( outputFileName );
  QgsDebugMsg( QString( "Conversion command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( " " ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( NULL, tr( "Could not start process" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Importing data..." ), tr( "Cancel" ), 0, 0 );
  progressDialog.setWindowModality( Qt::WindowModal );
  for ( int i = 0; babelProcess.state() == QProcess::Running; ++i )
  {
    progressDialog.setValue( i / 64 );
    if ( progressDialog.wasCanceled() )
      return;
  }

  // did we get any data?
  if ( babelProcess.exitStatus() != 0 )
  {
    QString babelError( babelProcess.readAllStandardError() );
    QString errorMsg( tr( "Could not convert data from %1!\n\n" )
                      .arg( inputFileName ) );
    errorMsg += babelError;
    QMessageBox::warning( NULL, tr( "Error converting data" ), errorMsg );
    return;
  }

  // add the layer
  switch ( convertType )
  {
    case 0:
    case 3:
      emit drawVectorLayer( outputFileName + "?type=waypoint",
                            layerName, "gpx" );
      break;
    case 1:
      emit drawVectorLayer( outputFileName + "?type=route",
                            layerName, "gpx" );
      break;
    case 2:
      emit drawVectorLayer( outputFileName + "?type=track",
                            layerName, "gpx" );
      break;
    default:
      QgsDebugMsg( "Illegal conversion index!" );
      return;
  }

  emit closeGui();
}

void QgsGPSPlugin::downloadFromGPS( QString device, QString port,
                                    bool downloadWaypoints, bool downloadRoutes,
                                    bool downloadTracks, QString outputFileName,
                                    QString layerName )
{
  // what does the user want to download?
  QString typeArg, features;
  if ( downloadWaypoints )
  {
    typeArg = "-w";
    features = "waypoints";
  }
  else if ( downloadRoutes )
  {
    typeArg = "-r";
    features = "routes";
  }
  else if ( downloadTracks )
  {
    typeArg = "-t";
    features = "tracks";
  }

  // try to start the gpsbabel process
  QStringList babelArgs =
    mDevices[device]->importCommand( mBabelPath, typeArg,
                                     port, outputFileName );
  if ( babelArgs.isEmpty() )
  {
    QMessageBox::warning( NULL, tr( "Not supported" ),
                          tr( "This device does not support downloading of %1." )
                          .arg( features ) );
    return;
  }

  QgsDebugMsg( QString( "Download command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( " " ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( NULL, tr( "Could not start process" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Downloading data..." ), tr( "Cancel" ), 0, 0 );
  progressDialog.setWindowModality( Qt::WindowModal );
  for ( int i = 0; babelProcess.state() == QProcess::Running; ++i )
  {
    progressDialog.setValue( i / 64 );
    if ( progressDialog.wasCanceled() )
      return;
  }

  // did we get any data?
  if ( babelProcess.exitStatus() != 0 )
  {
    QString babelError( babelProcess.readAllStandardError() );
    QString errorMsg( tr( "Could not download data from GPS!\n\n" ) );
    errorMsg += babelError;
    QMessageBox::warning( NULL, tr( "Error downloading data" ), errorMsg );
    return;
  }

  // add the layer
  if ( downloadWaypoints )
    emit drawVectorLayer( outputFileName + "?type=waypoint",
                          layerName, "gpx" );
  if ( downloadRoutes )
    emit drawVectorLayer( outputFileName + "?type=route",
                          layerName, "gpx" );
  if ( downloadTracks )
    emit drawVectorLayer( outputFileName + "?type=track",
                          layerName, "gpx" );

  // everything was OK, remember the device and port for next time
  QSettings settings;
  settings.setValue( "/Plugin-GPS/lastdldevice", device );
  settings.setValue( "/Plugin-GPS/lastdlport", port );

  emit closeGui();
}

void QgsGPSPlugin::uploadToGPS( QgsVectorLayer* gpxLayer, QString device,
                                QString port )
{
  const QString& source( gpxLayer->dataProvider()->dataSourceUri() );

  // what kind of data does the user want to upload?
  QString typeArg, features;
  if ( source.right( 8 ) == "waypoint" )
  {
    typeArg = "-w";
    features = "waypoints";
  }
  else if ( source.right( 5 ) == "route" )
  {
    typeArg = "-r";
    features = "routes";
  }
  else if ( source.right( 5 ) == "track" )
  {
    typeArg = "-t";
    features = "tracks";
  }
  else
  {
    QgsDebugMsg( source.right( 8 ) );
    assert( false );
  }

  // try to start the gpsbabel process
  QStringList babelArgs =
    mDevices[device]->exportCommand( mBabelPath, typeArg,
                                     source.left( source.lastIndexOf( '?' ) ), port );
  if ( babelArgs.isEmpty() )
  {
    QMessageBox::warning( NULL, tr( "Not supported" ),
                          tr( "This device does not support uploading of %1." )
                          .arg( features ) );
    return;
  }

  QgsDebugMsg( QString( "Upload command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( " " ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( NULL, tr( "Could not start process" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Uploading data..." ), tr( "Cancel" ), 0, 0 );
  progressDialog.setWindowModality( Qt::WindowModal );
  for ( int i = 0; babelProcess.state() == QProcess::Running; ++i )
  {
    progressDialog.setValue( i / 64 );
    if ( progressDialog.wasCanceled() )
      return;
  }

  // did we get an error?
  if ( babelProcess.exitStatus() != 0 )
  {
    QString babelError( babelProcess.readAllStandardError() );
    QString errorMsg( tr( "Error while uploading data to GPS!\n\n" ) );
    errorMsg += babelError;
    QMessageBox::warning( NULL, tr( "Error uploading data" ), errorMsg );
    return;
  }

  // everything was OK, remember this device for next time
  QSettings settings;
  settings.setValue( "/Plugin-GPS/lastuldevice", device );
  settings.setValue( "/Plugin-GPS/lastulport", port );

  emit closeGui();
}

void QgsGPSPlugin::setupBabel()
{
  // where is gpsbabel?
  QSettings settings;
  mBabelPath = settings.value( "/Plugin-GPS/gpsbabelpath", "" ).toString();
  if ( mBabelPath.isEmpty() )
    mBabelPath = "gpsbabel";
  // the importable formats
  mImporters["Shapefile"] =
    new QgsSimpleBabelFormat( "shape", true, true, true );
  mImporters["Geocaching.com .loc"] =
    new QgsSimpleBabelFormat( "geo", true, false, false );
  mImporters["Magellan Mapsend"] =
    new QgsSimpleBabelFormat( "mapsend", true, true, true );
  mImporters["Garmin PCX5"] =
    new QgsSimpleBabelFormat( "pcx", true, false, true );
  mImporters["Garmin Mapsource"] =
    new QgsSimpleBabelFormat( "mapsource", true, true, true );
  mImporters["GPSUtil"] =
    new QgsSimpleBabelFormat( "gpsutil", true, false, false );
  mImporters["PocketStreets 2002/2003 Pushpin"] =
    new QgsSimpleBabelFormat( "psp", true, false, false );
  mImporters["CoPilot Flight Planner"] =
    new QgsSimpleBabelFormat( "copilot", true, false, false );
  mImporters["Magellan Navigator Companion"] =
    new QgsSimpleBabelFormat( "magnav", true, false, false );
  mImporters["Holux"] =
    new QgsSimpleBabelFormat( "holux", true, false, false );
  mImporters["Topo by National Geographic"] =
    new QgsSimpleBabelFormat( "tpg", true, false, false );
  mImporters["TopoMapPro"] =
    new QgsSimpleBabelFormat( "tmpro", true, false, false );
  mImporters["GeocachingDB"] =
    new QgsSimpleBabelFormat( "gcdb", true, false, false );
  mImporters["Tiger"] =
    new QgsSimpleBabelFormat( "tiger", true, false, false );
  mImporters["EasyGPS Binary Format"] =
    new QgsSimpleBabelFormat( "easygps", true, false, false );
  mImporters["Delorme Routes"] =
    new QgsSimpleBabelFormat( "saroute", false, false, true );
  mImporters["Navicache"] =
    new QgsSimpleBabelFormat( "navicache", true, false, false );
  mImporters["PSITrex"] =
    new QgsSimpleBabelFormat( "psitrex", true, true, true );
  mImporters["Delorme GPS Log"] =
    new QgsSimpleBabelFormat( "gpl", false, false, true );
  mImporters["OziExplorer"] =
    new QgsSimpleBabelFormat( "ozi", true, false, false );
  mImporters["NMEA Sentences"] =
    new QgsSimpleBabelFormat( "nmea", true, false, true );
  mImporters["Delorme Street Atlas 2004 Plus"] =
    new QgsSimpleBabelFormat( "saplus", true, false, false );
  mImporters["Microsoft Streets and Trips"] =
    new QgsSimpleBabelFormat( "s_and_t", true, false, false );
  mImporters["NIMA/GNIS Geographic Names"] =
    new QgsSimpleBabelFormat( "nima", true, false, false );
  mImporters["Maptech"] =
    new QgsSimpleBabelFormat( "mxf", true, false, false );
  mImporters["Mapopolis.com Mapconverter Application"] =
    new QgsSimpleBabelFormat( "mapconverter", true, false, false );
  mImporters["GPSman"] =
    new QgsSimpleBabelFormat( "gpsman", true, false, false );
  mImporters["GPSDrive"] =
    new QgsSimpleBabelFormat( "gpsdrive", true, false, false );
  mImporters["Fugawi"] =
    new QgsSimpleBabelFormat( "fugawi", true, false, false );
  mImporters["DNA"] =
    new QgsSimpleBabelFormat( "dna", true, false, false );

  // and the GPS devices
  mDevices["Garmin serial"] =
    new QgsGPSDevice( "%babel -w -i garmin -o gpx %in %out",
                      "%babel -w -i gpx -o garmin %in %out",
                      "%babel -r -i garmin -o gpx %in %out",
                      "%babel -r -i gpx -o garmin %in %out",
                      "%babel -t -i garmin -o gpx %in %out",
                      "%babel -t -i gpx -o garmin %in %out" );
  QStringList deviceNames = settings.value( "/Plugin-GPS/devicelist" ).
                            toStringList();

  QStringList::const_iterator iter;

  for ( iter = deviceNames.begin(); iter != deviceNames.end(); ++iter )
  {
    QString wptDownload = settings.
                          value( QString( "/Plugin-GPS/devices/%1/wptdownload" ).
                                 arg( *iter ), "" ).toString();
    QString wptUpload = settings.
                        value( QString( "/Plugin-GPS/devices/%1/wptupload" ).arg( *iter ), "" ).
                        toString();
    QString rteDownload = settings.
                          value( QString( "/Plugin-GPS/devices/%1/rtedownload" ).arg( *iter ), "" ).
                          toString();
    QString rteUpload = settings.
                        value( QString( "/Plugin-GPS/devices/%1/rteupload" ).arg( *iter ), "" ).
                        toString();
    QString trkDownload = settings.
                          value( QString( "/Plugin-GPS/devices/%1/trkdownload" ).arg( *iter ), "" ).
                          toString();
    QString trkUpload = settings.
                        value( QString( "/Plugin-GPS/devices/%1/trkupload" ).arg( *iter ), "" ).
                        toString();
    mDevices[*iter] = new QgsGPSDevice( wptDownload, wptUpload,
                                        rteDownload, rteUpload,
                                        trkDownload, trkUpload );
  }
}

//! Set icons to the current theme
void QgsGPSPlugin::setCurrentTheme( QString theThemeName )
{
  Q_UNUSED( theThemeName );
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/gps_importer.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/gps_importer.png";
  QString myQrcPath = ":/gps_importer.png";
  if ( QFile::exists( myCurThemePath ) )
  {
    mQActionPointer->setIcon( QIcon( myCurThemePath ) );
    mCreateGPXAction->setIcon( QIcon( myCurThemePath ) );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    mQActionPointer->setIcon( QIcon( myDefThemePath ) );
    mCreateGPXAction->setIcon( QIcon( myDefThemePath ) );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    mQActionPointer->setIcon( QIcon( myQrcPath ) );
    mCreateGPXAction->setIcon( QIcon( myQrcPath ) );
  }
  else
  {
    mQActionPointer->setIcon( QIcon() );
    mCreateGPXAction->setIcon( QIcon() );
  }
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsGPSPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return name_;
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return type_;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
