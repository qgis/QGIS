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
#include "qgsguiutils.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"
#include "qgsvectordataprovider.h"
#include "qgsgpsplugin.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QFile>
#include <QMenu>
#include <QToolBar>
#include <QProcess>
#include <QProgressDialog>
#include <QStringList>

//non qt includes
#include <cassert>
#include <fstream>

//the gui subclass
#include "qgsgpsplugingui.h"


static const QString name_ = QObject::tr( "GPS Tools" );
static const QString description_ = QObject::tr( "Tools for loading and importing GPS data" );
static const QString category_ = QObject::tr( "Vector" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PluginType type_ = QgisPlugin::UI;
static const QString icon_ = QStringLiteral( ":/gps_importer.svg" );

QgsGpsPlugin::QgsGpsPlugin( QgisInterface *qgisInterFace )
  : QgisPlugin( name_, description_, category_, version_, type_ )
  , mQGisInterface( qgisInterFace )
{
  setupBabel();
}

QgsGpsPlugin::~QgsGpsPlugin()
{
  // delete all our babel formats
  BabelMap::iterator iter;
  for ( iter = mImporters.begin(); iter != mImporters.end(); ++iter )
    delete iter->second;
  std::map<QString, QgsGpsDevice *>::iterator iter2;
  for ( iter2 = mDevices.begin(); iter2 != mDevices.end(); ++iter2 )
    delete iter2->second;
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGpsPlugin::initGui()
{
  delete mQActionPointer;
  delete mCreateGPXAction;

  // add an action to the toolbar
  mQActionPointer = new QAction( QIcon(), tr( "&GPS Tools" ), this );
  mQActionPointer->setObjectName( QStringLiteral( "mQActionPointer" ) );
  mCreateGPXAction = new QAction( QIcon(), tr( "&Create new GPX layer" ), this );
  mCreateGPXAction->setObjectName( QStringLiteral( "mCreateGPXAction" ) );
  setCurrentTheme( QString() );

  mQActionPointer->setWhatsThis( tr( "Creates a new GPX layer and displays it on the map canvas" ) );
  mCreateGPXAction->setWhatsThis( tr( "Creates a new GPX layer and displays it on the map canvas" ) );
  connect( mQActionPointer, &QAction::triggered, this, &QgsGpsPlugin::run );
  connect( mCreateGPXAction, &QAction::triggered, this, &QgsGpsPlugin::createGPX );

  mQGisInterface->layerToolBar()->insertAction( nullptr, mCreateGPXAction );
  mQGisInterface->newLayerMenu()->addAction( mCreateGPXAction );
  mQGisInterface->addPluginToVectorMenu( QString(), mQActionPointer );
  mQGisInterface->addVectorToolBarIcon( mQActionPointer );

  // this is called when the icon theme is changed
  connect( mQGisInterface, &QgisInterface::currentThemeChanged, this, &QgsGpsPlugin::setCurrentTheme );
}

//method defined in interface
void QgsGpsPlugin::help()
{
  //implement me!
}

// Slot called when the menu item is activated
void QgsGpsPlugin::run()
{
  // find all GPX layers
  std::vector<QgsVectorLayer *> gpxLayers;
  QMap<QString, QgsMapLayer *>::const_iterator iter;
  QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  for ( iter = layers.constBegin();
        iter != layers.constEnd(); ++iter )
  {
    if ( iter.value()->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( iter.value() );
      if ( vLayer->providerType() == QLatin1String( "gpx" ) )
        gpxLayers.push_back( vLayer );
    }
  }

  QgsGpsPluginGui *myPluginGui =
    new QgsGpsPluginGui( mImporters, mDevices, gpxLayers, mQGisInterface->mainWindow(),
                         QgsGuiUtils::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //listen for when the layer has been made so we can draw it
  connect( myPluginGui, &QgsGpsPluginGui::drawVectorLayer,
           this, &QgsGpsPlugin::drawVectorLayer );
  connect( myPluginGui, &QgsGpsPluginGui::loadGPXFile,
           this, &QgsGpsPlugin::loadGPXFile );
  connect( myPluginGui, &QgsGpsPluginGui::importGPSFile,
           this, &QgsGpsPlugin::importGPSFile );
  connect( myPluginGui, &QgsGpsPluginGui::convertGPSFile,
           this, &QgsGpsPlugin::convertGPSFile );
  connect( myPluginGui, &QgsGpsPluginGui::downloadFromGPS,
           this, &QgsGpsPlugin::downloadFromGPS );
  connect( myPluginGui, &QgsGpsPluginGui::uploadToGPS,
           this, &QgsGpsPlugin::uploadToGPS );
  connect( this, &QgsGpsPlugin::closeGui, myPluginGui, &QWidget::close );

  myPluginGui->show();
}

void QgsGpsPlugin::createGPX()
{
  QgsSettings settings;
  QString dir = settings.value( QStringLiteral( "Plugin-GPS/gpxdirectory" ), QDir::homePath() ).toString();
  QString fileName =
    QFileDialog::getSaveFileName( mQGisInterface->mainWindow(),
                                  tr( "Save New GPX File As" ),
                                  dir,
                                  tr( "GPS eXchange file" ) + " (*.gpx)" );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( QLatin1String( ".gpx" ), Qt::CaseInsensitive ) )
    {
      fileName += QLatin1String( ".gpx" );
    }
    QFileInfo fileInfo( fileName );
    std::ofstream ofs( fileName.toUtf8() );
    if ( !ofs )
    {
      QMessageBox::warning( nullptr, tr( "Save New GPX File" ),
                            tr( "Unable to create a GPX file with the given name. "
                                "Try again with another name or in another "
                                "directory." ) );
      return;
    }
    settings.setValue( QStringLiteral( "Plugin-GPS/gpxdirectory" ), fileInfo.absolutePath() );

    ofs << "<gpx></gpx>" << std::endl;

    drawVectorLayer( fileName + "?type=track",
                     fileInfo.baseName() + ", tracks", QStringLiteral( "gpx" ) );
    drawVectorLayer( fileName + "?type=route",
                     fileInfo.baseName() + ", routes", QStringLiteral( "gpx" ) );
    drawVectorLayer( fileName + "?type=waypoint",
                     fileInfo.baseName() + ", waypoints", QStringLiteral( "gpx" ) );
  }
}

void QgsGpsPlugin::drawVectorLayer( const QString &pathNameQString,
                                    const QString &baseNameQString,
                                    const QString &providerQString )
{
  mQGisInterface->addVectorLayer( pathNameQString, baseNameQString,
                                  providerQString );
}

// Unload the plugin by cleaning up the GUI
void QgsGpsPlugin::unload()
{
  // remove the GUI
  mQGisInterface->layerToolBar()->removeAction( mCreateGPXAction );
  mQGisInterface->newLayerMenu()->removeAction( mCreateGPXAction );
  mQGisInterface->vectorMenu()->removeAction( mQActionPointer );
  mQGisInterface->removeVectorToolBarIcon( mQActionPointer );
  delete mQActionPointer;
  mQActionPointer = nullptr;
}

void QgsGpsPlugin::loadGPXFile( const QString &fileName, bool loadWaypoints, bool loadRoutes,
                                bool loadTracks )
{
  //check if input file is readable
  QFileInfo fileInfo( fileName );
  if ( !fileInfo.isReadable() )
  {
    QMessageBox::warning( nullptr, tr( "GPX Loader" ),
                          tr( "Unable to read the selected file.\n"
                              "Please reselect a valid file." ) );
    return;
  }

  // add the requested layers
  if ( loadTracks )
    drawVectorLayer( fileName + "?type=track",
                     fileInfo.baseName() + ", tracks", QStringLiteral( "gpx" ) );
  if ( loadRoutes )
    drawVectorLayer( fileName + "?type=route",
                     fileInfo.baseName() + ", routes", QStringLiteral( "gpx" ) );
  if ( loadWaypoints )
    drawVectorLayer( fileName + "?type=waypoint",
                     fileInfo.baseName() + ", waypoints", QStringLiteral( "gpx" ) );

  emit closeGui();
}

void QgsGpsPlugin::importGPSFile( const QString &inputFileName, QgsBabelFormat *importer,
                                  bool importWaypoints, bool importRoutes,
                                  bool importTracks, const QString &outputFileName,
                                  const QString &layerName )
{
  // what features does the user want to import?
  QString typeArg;
  if ( importWaypoints )
    typeArg = QStringLiteral( "-w" );
  else if ( importRoutes )
    typeArg = QStringLiteral( "-r" );
  else if ( importTracks )
    typeArg = QStringLiteral( "-t" );

  // try to start the gpsbabel process
  QStringList babelArgs =
    importer->importCommand( mBabelPath, typeArg,
                             inputFileName, outputFileName );

  QgsDebugMsg( QStringLiteral( "Import command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( QStringLiteral( " " ) ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( nullptr, tr( "Import GPS File" ),
                          tr( "Could not start GPSBabel." ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Importing data…" ), tr( "Cancel" ), 0, 0 );
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
    QMessageBox::warning( nullptr, tr( "Import GPS File" ), errorMsg );
    return;
  }

  // add the layer
  if ( importTracks )
    drawVectorLayer( outputFileName + "?type=track",
                     layerName, QStringLiteral( "gpx" ) );
  if ( importRoutes )
    drawVectorLayer( outputFileName + "?type=route",
                     layerName, QStringLiteral( "gpx" ) );
  if ( importWaypoints )
    drawVectorLayer( outputFileName + "?type=waypoint",
                     layerName, QStringLiteral( "gpx" ) );

  emit closeGui();
}

void QgsGpsPlugin::convertGPSFile( const QString &inputFileName,
                                   int convertType,
                                   const QString &outputFileName,
                                   const QString &layerName )
{
  // what features does the user want to import?
  QStringList convertStrings;

  switch ( convertType )
  {
    case 0:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,wpt=rte,del" );
      break;
    case 1:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,rte=wpt,del" );
      break;
    case 2:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,trk=wpt,del" );
      break;
    case 3:
      convertStrings << QStringLiteral( "-x" ) << QStringLiteral( "transform,wpt=trk,del" );
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Illegal conversion index!" ) );
      return;
  }

  // try to start the gpsbabel process
  QStringList babelArgs;
  babelArgs << mBabelPath << QStringLiteral( "-i" ) << QStringLiteral( "gpx" ) << QStringLiteral( "-f" ) << QStringLiteral( "\"%1\"" ).arg( inputFileName )
            << convertStrings << QStringLiteral( "-o" ) << QStringLiteral( "gpx" ) << QStringLiteral( "-F" ) << QStringLiteral( "\"%1\"" ).arg( outputFileName );
  QgsDebugMsg( QStringLiteral( "Conversion command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( QStringLiteral( " " ) ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( nullptr, tr( "Convert GPS File" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Importing data…" ), tr( "Cancel" ), 0, 0 );
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
    QMessageBox::warning( nullptr, tr( "Convert GPS File" ), errorMsg );
    return;
  }

  // add the layer
  switch ( convertType )
  {
    case 0:
    case 3:
      drawVectorLayer( outputFileName + "?type=waypoint",
                       layerName, QStringLiteral( "gpx" ) );
      break;
    case 1:
      drawVectorLayer( outputFileName + "?type=route",
                       layerName, QStringLiteral( "gpx" ) );
      break;
    case 2:
      drawVectorLayer( outputFileName + "?type=track",
                       layerName, QStringLiteral( "gpx" ) );
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Illegal conversion index!" ) );
      return;
  }

  emit closeGui();
}

void QgsGpsPlugin::downloadFromGPS( const QString &device, const QString &port,
                                    bool downloadWaypoints, bool downloadRoutes,
                                    bool downloadTracks, const QString &outputFileName,
                                    const QString &layerName )
{
  // what does the user want to download?
  QString typeArg, features;
  if ( downloadWaypoints )
  {
    typeArg = QStringLiteral( "-w" );
    features = QStringLiteral( "waypoints" );
  }
  else if ( downloadRoutes )
  {
    typeArg = QStringLiteral( "-r" );
    features = QStringLiteral( "routes" );
  }
  else if ( downloadTracks )
  {
    typeArg = QStringLiteral( "-t" );
    features = QStringLiteral( "tracks" );
  }

  // try to start the gpsbabel process
  QStringList babelArgs =
    mDevices[device]->importCommand( mBabelPath, typeArg,
                                     port, outputFileName );
  if ( babelArgs.isEmpty() )
  {
    QMessageBox::warning( nullptr, tr( "Download from GPS" ),
                          tr( "This device does not support downloading of %1." )
                          .arg( features ) );
    return;
  }

  QgsDebugMsg( QStringLiteral( "Download command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( QStringLiteral( " " ) ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( nullptr, tr( "Download from GPS" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Downloading data…" ), tr( "Cancel" ), 0, 0 );
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
    QMessageBox::warning( nullptr, tr( "Download from GPS" ), errorMsg );
    return;
  }

  // add the layer
  if ( downloadWaypoints )
    drawVectorLayer( outputFileName + "?type=waypoint",
                     layerName, QStringLiteral( "gpx" ) );
  if ( downloadRoutes )
    drawVectorLayer( outputFileName + "?type=route",
                     layerName, QStringLiteral( "gpx" ) );
  if ( downloadTracks )
    drawVectorLayer( outputFileName + "?type=track",
                     layerName, QStringLiteral( "gpx" ) );

  // everything was OK, remember the device and port for next time
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Plugin-GPS/lastdldevice" ), device );
  settings.setValue( QStringLiteral( "Plugin-GPS/lastdlport" ), port );

  emit closeGui();
}

void QgsGpsPlugin::uploadToGPS( QgsVectorLayer *gpxLayer, const QString &device,
                                const QString &port )
{
  const QString &source( gpxLayer->dataProvider()->dataSourceUri() );

  // what kind of data does the user want to upload?
  QString typeArg, features;
  if ( source.right( 8 ) == QLatin1String( "waypoint" ) )
  {
    typeArg = QStringLiteral( "-w" );
    features = QStringLiteral( "waypoints" );
  }
  else if ( source.right( 5 ) == QLatin1String( "route" ) )
  {
    typeArg = QStringLiteral( "-r" );
    features = QStringLiteral( "routes" );
  }
  else if ( source.right( 5 ) == QLatin1String( "track" ) )
  {
    typeArg = QStringLiteral( "-t" );
    features = QStringLiteral( "tracks" );
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
    QMessageBox::warning( nullptr, tr( "Upload to GPS" ),
                          tr( "This device does not support uploading of %1." )
                          .arg( features ) );
    return;
  }

  QgsDebugMsg( QStringLiteral( "Upload command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.join( QStringLiteral( " " ) ) );
  if ( !babelProcess.waitForStarted() )
  {
    QMessageBox::warning( nullptr, tr( "Upload to GPS" ),
                          tr( "Could not start GPSBabel!" ) );
    return;
  }

  // wait for gpsbabel to finish (or the user to cancel)
  QProgressDialog progressDialog( tr( "Uploading data…" ), tr( "Cancel" ), 0, 0 );
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
    QMessageBox::warning( nullptr, tr( "Upload to GPS" ), errorMsg );
    return;
  }

  // everything was OK, remember this device for next time
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Plugin-GPS/lastuldevice" ), device );
  settings.setValue( QStringLiteral( "Plugin-GPS/lastulport" ), port );

  emit closeGui();
}

void QgsGpsPlugin::setupBabel()
{
  // where is gpsbabel?
  QgsSettings settings;
  mBabelPath = settings.value( QStringLiteral( "Plugin-GPS/gpsbabelpath" ), QString() ).toString();
  if ( mBabelPath.isEmpty() )
    mBabelPath = QStringLiteral( "gpsbabel" );
  // the importable formats
  mImporters[QStringLiteral( "Shapefile" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "shape" ), true, true, true );
  mImporters[QStringLiteral( "Geocaching.com .loc" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "geo" ), true, false, false );
  mImporters[QStringLiteral( "Magellan Mapsend" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "mapsend" ), true, true, true );
  mImporters[QStringLiteral( "Garmin PCX5" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "pcx" ), true, false, true );
  mImporters[QStringLiteral( "Garmin Mapsource" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "mapsource" ), true, true, true );
  mImporters[QStringLiteral( "GPSUtil" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "gpsutil" ), true, false, false );
  mImporters[QStringLiteral( "PocketStreets 2002/2003 Pushpin" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "psp" ), true, false, false );
  mImporters[QStringLiteral( "CoPilot Flight Planner" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "copilot" ), true, false, false );
  mImporters[QStringLiteral( "Magellan Navigator Companion" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "magnav" ), true, false, false );
  mImporters[QStringLiteral( "Holux" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "holux" ), true, false, false );
  mImporters[QStringLiteral( "Topo by National Geographic" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "tpg" ), true, false, false );
  mImporters[QStringLiteral( "TopoMapPro" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "tmpro" ), true, false, false );
  mImporters[QStringLiteral( "GeocachingDB" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "gcdb" ), true, false, false );
  mImporters[QStringLiteral( "Tiger" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "tiger" ), true, false, false );
  mImporters[QStringLiteral( "EasyGPS Binary Format" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "easygps" ), true, false, false );
  mImporters[QStringLiteral( "Delorme Routes" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "saroute" ), false, false, true );
  mImporters[QStringLiteral( "Navicache" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "navicache" ), true, false, false );
  mImporters[QStringLiteral( "PSITrex" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "psitrex" ), true, true, true );
  mImporters[QStringLiteral( "Delorme GPS Log" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "gpl" ), false, false, true );
  mImporters[QStringLiteral( "OziExplorer" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "ozi" ), true, false, false );
  mImporters[QStringLiteral( "NMEA Sentences" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "nmea" ), true, false, true );
  mImporters[QStringLiteral( "Delorme Street Atlas 2004 Plus" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "saplus" ), true, false, false );
  mImporters[QStringLiteral( "Microsoft Streets and Trips" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "s_and_t" ), true, false, false );
  mImporters[QStringLiteral( "NIMA/GNIS Geographic Names" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "nima" ), true, false, false );
  mImporters[QStringLiteral( "Maptech" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "mxf" ), true, false, false );
  mImporters[QStringLiteral( "Mapopolis.com Mapconverter Application" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "mapconverter" ), true, false, false );
  mImporters[QStringLiteral( "GPSman" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "gpsman" ), true, false, false );
  mImporters[QStringLiteral( "GPSDrive" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "gpsdrive" ), true, false, false );
  mImporters[QStringLiteral( "Fugawi" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "fugawi" ), true, false, false );
  mImporters[QStringLiteral( "DNA" )] =
    new QgsSimpleBabelFormat( QStringLiteral( "dna" ), true, false, false );

  // and the GPS devices
  mDevices[QStringLiteral( "Garmin serial" )] =
    new QgsGpsDevice( QStringLiteral( "%babel -w -i garmin -o gpx %in %out" ),
                      QStringLiteral( "%babel -w -i gpx -o garmin %in %out" ),
                      QStringLiteral( "%babel -r -i garmin -o gpx %in %out" ),
                      QStringLiteral( "%babel -r -i gpx -o garmin %in %out" ),
                      QStringLiteral( "%babel -t -i garmin -o gpx %in %out" ),
                      QStringLiteral( "%babel -t -i gpx -o garmin %in %out" ) );
  QStringList deviceNames = settings.value( QStringLiteral( "Plugin-GPS/devicelist" ) ).
                            toStringList();

  QStringList::const_iterator iter;

  for ( iter = deviceNames.constBegin(); iter != deviceNames.constEnd(); ++iter )
  {
    QString wptDownload = settings.
                          value( QStringLiteral( "/Plugin-GPS/devices/%1/wptdownload" ).
                                 arg( *iter ), "" ).toString();
    QString wptUpload = settings.
                        value( QStringLiteral( "/Plugin-GPS/devices/%1/wptupload" ).arg( *iter ), "" ).
                        toString();
    QString rteDownload = settings.
                          value( QStringLiteral( "/Plugin-GPS/devices/%1/rtedownload" ).arg( *iter ), "" ).
                          toString();
    QString rteUpload = settings.
                        value( QStringLiteral( "/Plugin-GPS/devices/%1/rteupload" ).arg( *iter ), "" ).
                        toString();
    QString trkDownload = settings.
                          value( QStringLiteral( "/Plugin-GPS/devices/%1/trkdownload" ).arg( *iter ), "" ).
                          toString();
    QString trkUpload = settings.
                        value( QStringLiteral( "/Plugin-GPS/devices/%1/trkupload" ).arg( *iter ), "" ).
                        toString();
    mDevices[*iter] = new QgsGpsDevice( wptDownload, wptUpload,
                                        rteDownload, rteUpload,
                                        trkDownload, trkUpload );
  }
}

//! Sets icons to the current theme
void QgsGpsPlugin::setCurrentTheme( const QString &themeName )
{
  Q_UNUSED( themeName );
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/gps_importer/";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/gps_importer/";
  QString myQrcPath = QStringLiteral( ":/" );
  if ( mQActionPointer )
  {
    if ( QFile::exists( myCurThemePath ) )
    {
      mQActionPointer->setIcon( QIcon( myCurThemePath + "import_gpx.svg" ) );
      mCreateGPXAction->setIcon( QIcon( myCurThemePath + "create_gpx.svg" ) );
    }
    else if ( QFile::exists( myDefThemePath ) )
    {
      mQActionPointer->setIcon( QIcon( myDefThemePath + "import_gpx.svg" ) );
      mCreateGPXAction->setIcon( QIcon( myDefThemePath + "create_gpx.svg" ) );
    }
    else if ( QFile::exists( myQrcPath ) )
    {
      mQActionPointer->setIcon( QIcon( myQrcPath + "import_gpx.svg" ) );
      mCreateGPXAction->setIcon( QIcon( myQrcPath + "create_gpx.svg" ) );
    }
    else
    {
      mQActionPointer->setIcon( QIcon() );
      mCreateGPXAction->setIcon( QIcon() );
    }
  }
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin *classFactory( QgisInterface *qgisInterfacePointer )
{
  return new QgsGpsPlugin( qgisInterfacePointer );
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

// Return the category
QGISEXTERN QString category()
{
  return category_;
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
QGISEXTERN void unload( QgisPlugin *pluginPointer )
{
  delete pluginPointer;
}
