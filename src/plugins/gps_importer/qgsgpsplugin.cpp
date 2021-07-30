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
#include "qgsbabelformatregistry.h"

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
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGpsPlugin::initGui()
{
  delete mQActionPointer;

  // add an action to the toolbar
  mQActionPointer = new QAction( QIcon(), tr( "&GPS Tools" ), this );
  mQActionPointer->setObjectName( QStringLiteral( "mQActionPointer" ) );
  setCurrentTheme( QString() );

  mQActionPointer->setWhatsThis( tr( "Creates a new GPX layer and displays it on the map canvas" ) );
  connect( mQActionPointer, &QAction::triggered, this, &QgsGpsPlugin::run );

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
    if ( iter.value()->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( iter.value() );
      if ( vLayer->providerType() == QLatin1String( "gpx" ) )
        gpxLayers.push_back( vLayer );
    }
  }

  QgsGpsPluginGui *myPluginGui = new QgsGpsPluginGui( gpxLayers, mQGisInterface->mainWindow(),
      QgsGuiUtils::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //listen for when the layer has been made so we can draw it
  connect( myPluginGui, &QgsGpsPluginGui::drawVectorLayer,
           this, &QgsGpsPlugin::drawVectorLayer );
  connect( myPluginGui, &QgsGpsPluginGui::loadGPXFile,
           this, &QgsGpsPlugin::loadGPXFile );
  connect( myPluginGui, &QgsGpsPluginGui::importGPSFile,
           this, &QgsGpsPlugin::importGPSFile );
  connect( myPluginGui, &QgsGpsPluginGui::downloadFromGPS,
           this, &QgsGpsPlugin::downloadFromGPS );
  connect( myPluginGui, &QgsGpsPluginGui::uploadToGPS,
           this, &QgsGpsPlugin::uploadToGPS );
  connect( this, &QgsGpsPlugin::closeGui, myPluginGui, &QWidget::close );

  myPluginGui->show();
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

void QgsGpsPlugin::importGPSFile( const QString &inputFileName, QgsAbstractBabelFormat *importer,
                                  Qgis::GpsFeatureType type, const QString &outputFileName,
                                  const QString &layerName )
{
  // try to start the gpsbabel process
  QStringList babelArgs = importer->importCommand( mBabelPath, type, inputFileName, outputFileName );

  QgsDebugMsg( QStringLiteral( "Import command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.value( 0 ), babelArgs.mid( 1 ) );
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
  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      drawVectorLayer( outputFileName + "?type=waypoint",
                       layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Route:
      drawVectorLayer( outputFileName + "?type=route",
                       layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Track:
      drawVectorLayer( outputFileName + "?type=track",
                       layerName, QStringLiteral( "gpx" ) );
      break;
  }

  emit closeGui();
}

void QgsGpsPlugin::downloadFromGPS( const QString &device, const QString &port,
                                    Qgis::GpsFeatureType type, const QString &outputFileName,
                                    const QString &layerName )
{
  // what does the user want to download?
  QString features;
  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      features = QStringLiteral( "waypoints" );
      break;
    case Qgis::GpsFeatureType::Route:
      features = QStringLiteral( "routes" );
      break;
    case Qgis::GpsFeatureType::Track:
      features = QStringLiteral( "tracks" );
      break;
  }

  const QgsAbstractBabelFormat *deviceFormat = QgsApplication::gpsBabelFormatRegistry()->deviceFormat( device );
  if ( !deviceFormat )
  {
    QMessageBox::warning( nullptr, tr( "Download from GPS" ),
                          tr( "Could not retrieve device %1" ).arg( device ) );
    return;
  }

  // try to start the gpsbabel process
  QStringList babelArgs = deviceFormat->importCommand( mBabelPath, type, port, outputFileName );
  if ( babelArgs.isEmpty() )
  {
    QMessageBox::warning( nullptr, tr( "Download from GPS" ),
                          tr( "This device does not support downloading of %1." )
                          .arg( features ) );
    return;
  }

  QgsDebugMsg( QStringLiteral( "Download command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.value( 0 ), babelArgs.mid( 1 ) );
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
  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      drawVectorLayer( outputFileName + "?type=waypoint", layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Route:
      drawVectorLayer( outputFileName + "?type=route", layerName, QStringLiteral( "gpx" ) );
      break;
    case Qgis::GpsFeatureType::Track:
      drawVectorLayer( outputFileName + "?type=track", layerName, QStringLiteral( "gpx" ) );
      break;
  }

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
  QString features;
  Qgis::GpsFeatureType type = Qgis::GpsFeatureType::Waypoint;
  if ( source.right( 8 ) == QLatin1String( "waypoint" ) )
  {
    type = Qgis::GpsFeatureType::Waypoint;
    features = QStringLiteral( "waypoints" );
  }
  else if ( source.right( 5 ) == QLatin1String( "route" ) )
  {
    type = Qgis::GpsFeatureType::Route;
    features = QStringLiteral( "routes" );
  }
  else if ( source.right( 5 ) == QLatin1String( "track" ) )
  {
    type = Qgis::GpsFeatureType::Track;
    features = QStringLiteral( "tracks" );
  }
  else
  {
    QgsDebugMsg( source.right( 8 ) );
    assert( false );
  }

  const QgsAbstractBabelFormat *deviceFormat = QgsApplication::gpsBabelFormatRegistry()->deviceFormat( device );
  if ( !deviceFormat )
  {
    QMessageBox::warning( nullptr, tr( "Upload to GPS" ),
                          tr( "Could not retrieve device %1" ).arg( device ) );
    return;
  }

  // try to start the gpsbabel process
  QStringList babelArgs = deviceFormat->exportCommand( mBabelPath, type, source.left( source.lastIndexOf( '?' ) ), port );
  if ( babelArgs.isEmpty() )
  {
    QMessageBox::warning( nullptr, tr( "Upload to GPS" ),
                          tr( "This device does not support uploading of %1." )
                          .arg( features ) );
    return;
  }

  QgsDebugMsg( QStringLiteral( "Upload command: " ) + babelArgs.join( "|" ) );

  QProcess babelProcess;
  babelProcess.start( babelArgs.value( 0 ), babelArgs.mid( 1 ) );
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
}

//! Sets icons to the current theme
void QgsGpsPlugin::setCurrentTheme( const QString &themeName )
{
  Q_UNUSED( themeName )
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/gps_importer/";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/gps_importer/";
  QString myQrcPath = QStringLiteral( ":/" );
  if ( mQActionPointer )
  {
    mQActionPointer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconGps.svg" ) ) );
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
QGISEXTERN const QString *name()
{
  return &name_;
}

// Return the description
QGISEXTERN const QString *description()
{
  return &description_;
}

// Return the category
QGISEXTERN const QString *category()
{
  return &category_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return type_;
}

// Return the version number for the plugin
QGISEXTERN const QString *version()
{
  return &version_;
}

QGISEXTERN const QString *icon()
{
  return &icon_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin *pluginPointer )
{
  delete pluginPointer;
}
