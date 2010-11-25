/***************************************************************************
    globe_plugin.cpp

    Globe Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "globe_plugin.h"
#include "globe_plugin_gui.h"
#include "globe_plugin_dialog.h"
#include "qgsosgearthtilesource.h"

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgslogger.h>
#include <qgsapplication.h>
#include <qgsmapcanvas.h>

#include <QAction>
#include <QToolBar>
#include <QMessageBox>

#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>

#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>

#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgEarth/Notify>
#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarth/TileSource>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/tms/TMSOptions>

using namespace osgEarth::Drivers;
using namespace osgEarthUtil::Controls2;


//static const char * const sIdent = "$Id: plugin.cpp 9327 2008-09-14 11:18:44Z jef $";
static const QString sName = QObject::tr( "Globe" );
static const QString sDescription = QObject::tr( "Overlay data on a 3D globe" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;


//constructor
GlobePlugin::GlobePlugin( QgisInterface* theQgisInterface )
  : QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface ),
    mQActionPointer( NULL ),
    mQActionSettingsPointer( NULL ),
    viewer(),
    mQDockWidget( tr( "Globe" ) ),
    mSettingsDialog( theQgisInterface->mainWindow(), QgisGui::ModalDialogFlags ),
    mTileSource(0),
    mElevationManager( NULL ),
    mObjectPlacer( NULL )
{
}

//destructor
GlobePlugin::~GlobePlugin()
{
}

void GlobePlugin::initGui()
{
  // Create the action for tool
  mQActionPointer = new QAction( QIcon( ":/globe/globe.png" ), tr( "Launch Globe" ), this );
  mQActionSettingsPointer = new QAction( QIcon( ":/globe/globe.png" ), tr( "Globe Settings" ), this );
  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Overlay data on a 3D globe" ) );
  mQActionSettingsPointer->setWhatsThis( tr( "Settings for 3D globe" ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Connect to the setting slot
  connect( mQActionSettingsPointer, SIGNAL( triggered() ), this, SLOT( settings() ) );
  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mQActionPointer );
  //Add menu
  mQGisIface->addPluginToMenu( tr( "&Globe" ), mQActionPointer );
  mQGisIface->addPluginToMenu( tr( "&Globe" ), mQActionSettingsPointer );
  mQDockWidget.setWidget(&viewer);

  connect(mQGisIface->mapCanvas() , SIGNAL(extentsChanged()),
          this, SLOT( extentsChanged() ) );
  connect(mQGisIface->mapCanvas(), SIGNAL(layersChanged()),
          this, SLOT( layersChanged() ) );
}

struct MyClickHandler : public ControlEventHandler
{
    void onClick( Control* control, int mouseButtonMask)
    {
        OE_NOTICE << "Thank you for clicking on " << typeid(control).name()
                  << std::endl;
    }
};

void GlobePlugin::run()
{
#ifdef QGISDEBUG
  if ( !getenv( "OSGNOTIFYLEVEL" ) ) osgEarth::setNotifyLevel(osg::DEBUG_INFO);
#endif

  mQGisIface->addDockWidget(Qt::RightDockWidgetArea, &mQDockWidget );

  viewer.show();

#ifdef GLOBE_OSG_STANDALONE_VIEWER
  osgViewer::Viewer viewer;
#endif

  setupProxy();

  // install the programmable manipulator.
  osgEarthUtil::EarthManipulator* manip = new osgEarthUtil::EarthManipulator();
  viewer.setCameraManipulator( manip );

  setupMap();

  viewer.setSceneData( mRootNode );

  // Set a home viewpoint
  manip->setHomeViewpoint(
    osgEarthUtil::Viewpoint( osg::Vec3d( -90, 0, 0 ), 0.0, -90.0, 4e7 ),
    1.0 );

  // create a surface to house the controls
  mControlCanvas = new ControlCanvas( &viewer );
  mRootNode->addChild( mControlCanvas );
  setupControls();

  // add our fly-to handler
  viewer.addEventHandler(new FlyToExtentHandler( manip, mQGisIface ));

  // add some stock OSG handlers:
  viewer.addEventHandler(new osgViewer::StatsHandler());
  viewer.addEventHandler(new osgViewer::WindowSizeHandler());
  viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

#ifdef GLOBE_OSG_STANDALONE_VIEWER
  viewer.run();
#endif
}

void GlobePlugin::settings()
{
  if (mSettingsDialog.exec())
  {
    //viewer stereo settings set by mSettingsDialog and stored in QSettings
  }
}

void GlobePlugin::setupMap()
{
  // read base layers from earth file
  QString earthFileName = QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe/globe.earth" );
	EarthFile earthFile;
  QFile earthFileTemplate( earthFileName );
  if (!earthFileTemplate.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    return;
  }

  QTextStream in(&earthFileTemplate);
  QString earthxml = in.readAll();
  QSettings settings;
  QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
  earthxml.replace( "/home/pi/devel/gis/qgis/.qgis/cache", cacheDirectory );
  earthxml.replace( "/usr/share/osgearth/data", QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe" ) );

  //prefill cache
  if ( !QFile::exists( cacheDirectory + "/worldwind_srtm" ) )
  {
    copyFolder( QgsApplication::pkgDataPath() + "/globe/data/worldwind_srtm", cacheDirectory + "/globe/worldwind_srtm" );
  }

  std::istringstream istream( earthxml.toStdString() );
  if ( !earthFile.readXML( istream, earthFileName.toStdString() ) )
  {
    return;
  }

  osg::ref_ptr<Map> map = earthFile.getMap();

  // Add QGIS layer to the map
  mTileSource = new QgsOsgEarthTileSource(mQGisIface);
  mTileSource->initialize("");
  mQgisMapLayer = new ImageMapLayer( "QGIS", mTileSource );
  map->addMapLayer( mQgisMapLayer );
  mQgisMapLayer->setCache( 0 ); //disable caching

  mRootNode = new osg::Group();

  // The MapNode will render the Map object in the scene graph.
  mMapNode = new osgEarth::MapNode( map );
  mRootNode->addChild( mMapNode );

  // model placement utils
  mElevationManager = new osgEarthUtil::ElevationManager( mMapNode->getMap() );
  mElevationManager->setTechnique( osgEarthUtil::ElevationManager::TECHNIQUE_GEOMETRIC );
  mElevationManager->setMaxTilesToCache( 50 );

  mObjectPlacer = new osgEarthUtil::ObjectPlacer( mMapNode );

#if 0
  // model placement test

  // create simple tree model from primitives
  osg::TessellationHints* hints = new osg::TessellationHints();
  hints->setDetailRatio(0.1);

  osg::Cylinder* cylinder = new osg::Cylinder( osg::Vec3(0 ,0, 5), 0.5, 10 );
  osg::ShapeDrawable* cylinderDrawable = new osg::ShapeDrawable( cylinder, hints );
  cylinderDrawable->setColor( osg::Vec4( 0.5, 0.25, 0.125, 1.0 ) );
  osg::Geode* cylinderGeode = new osg::Geode();
  cylinderGeode->addDrawable( cylinderDrawable );

  osg::Cone* cone = new osg::Cone( osg::Vec3(0 ,0, 10), 4, 10 );
  osg::ShapeDrawable* coneDrawable = new osg::ShapeDrawable( cone, hints );
  coneDrawable->setColor( osg::Vec4( 0.0, 0.5, 0.0, 1.0 ) );
  osg::Geode* coneGeode = new osg::Geode();
  coneGeode->addDrawable( coneDrawable );

  osg::Group* model = new osg::Group();
  model->addChild( cylinderGeode );
  model->addChild( coneGeode );

  // place models on jittered grid
  srand( 23 );
  double lat = 47.1786;
  double lon = 10.111;
  double gridSize = 0.001;
  for( int i=0; i<10; i++ )
  {
    for( int j=0; j<10; j++ )
    {
      double dx = gridSize * ( rand()/( (double)RAND_MAX + 1.0 ) - 0.5 );
      double dy = gridSize * ( rand()/( (double)RAND_MAX + 1.0 ) - 0.5 );
      placeNode( model, lat + i * gridSize + dx, lon + j * gridSize + dy );
    }
  }
#endif
}

void GlobePlugin::setupControls()
{
 
  std::string imgDir = QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe/gui" ).toStdString();
  
//MOVE CONTROLS
  //Horizontal container
  HBox* moveHControls = new HBox();
  moveHControls->setFrame( new RoundedFrame() );
  //moveHControls->getFrame()->setBackColor(0.5,0.5,0.5,0.1);
  moveHControls->setMargin( 10 );
  moveHControls->setSpacing( 10 );
  moveHControls->setVertAlign( Control::ALIGN_CENTER );
  moveHControls->setHorizAlign( Control::ALIGN_CENTER );
  moveHControls->setPosition( 5, 35 );
  
  //Move Left
  osg::Image* moveLeftImg = osgDB::readImageFile( imgDir + "/move-left.png" );
  ImageControl* moveLeft = new NavigationControl( moveLeftImg );
  moveLeft->addEventHandler( new MyClickHandler );
  
  //Move Right
  osg::Image* moveRightImg = osgDB::readImageFile( imgDir + "/move-right.png" );
  ImageControl* moveRight = new ImageControl( moveRightImg );
  moveRight->addEventHandler( new MyClickHandler );
  
  //Move Reset
  osg::Image* moveResetImg = osgDB::readImageFile( imgDir + "/move-reset.png" );
  ImageControl* moveReset = new ImageControl( moveResetImg );
  moveReset->addEventHandler( new MyClickHandler );
  
  
  //Vertical container
  VBox* moveVControls = new VBox();
  moveVControls->setFrame( new RoundedFrame() );
  //moveVControls->getFrame()->setBackColor(0.5,0.5,0.5,0.1);
  moveVControls->setMargin( 10 );
  moveVControls->setSpacing( 30 );
  moveVControls->setVertAlign( Control::ALIGN_CENTER );
  moveVControls->setHorizAlign( Control::ALIGN_CENTER );
  moveVControls->setPosition( 40, 5 );
  
  //Move Up
  osg::Image* moveUpImg = osgDB::readImageFile( imgDir + "/move-up.png" );
  ImageControl* moveUp = new ImageControl( moveUpImg );
  moveUp->addEventHandler( new MyClickHandler );
  
  //Move Down
  osg::Image* moveDownImg = osgDB::readImageFile( imgDir + "/move-down.png" );
  ImageControl* moveDown = new ImageControl( moveDownImg );
  moveDown->addEventHandler( new MyClickHandler );
  
  //add controls to moveControls group
  moveHControls->addControl( moveLeft );
  moveHControls->addControl( moveReset );
  moveHControls->addControl( moveRight );
  moveVControls->addControl( moveUp );
  moveVControls->addControl( moveDown );
  
//END MOVE CONTROLS
  
//ROTATE CONTROLS
  //Horizontal container
  HBox* rotateControls = new HBox();
  rotateControls->setFrame( new RoundedFrame() );
  //rotateControls->getFrame()->setBackColor(0.5,0.5,0.5,0.1);
  rotateControls->setMargin( 10 );
  rotateControls->setSpacing( 10 );
  rotateControls->setVertAlign( Control::ALIGN_CENTER );
  rotateControls->setHorizAlign( Control::ALIGN_CENTER );
  rotateControls->setPosition( 5, 120 );
  
  //Rotate CCW
  osg::Image* rotateCCWImg = osgDB::readImageFile( imgDir + "/rotate-ccw.png" );
  ImageControl* rotateCCW = new ImageControl( rotateCCWImg );
  rotateCCW->addEventHandler( new MyClickHandler );
  
  //Rotate CW
  osg::Image* rotateCWImg = osgDB::readImageFile( imgDir + "/rotate-cw.png" );
  ImageControl* rotateCW = new ImageControl( rotateCWImg );
  rotateCW->addEventHandler( new MyClickHandler );
  
  //Rotate Reset
  osg::Image* rotateResetImg = osgDB::readImageFile( imgDir + "/rotate-reset.png" );
  ImageControl* rotateReset = new ImageControl( rotateResetImg );
  rotateReset->addEventHandler( new MyClickHandler );
  
  //add controls to moveControls group
  rotateControls->addControl( rotateCCW );
  rotateControls->addControl( rotateReset );
  rotateControls->addControl( rotateCW );

//END ROTATE CONTROLS

//TILT CONTROLS
  //Vertical container
  VBox* tiltControls = new VBox();
  tiltControls->setFrame( new RoundedFrame() );
  //tiltControls->getFrame()->setBackColor(0.5,0.5,0.5,0.1);
  tiltControls->setMargin( 10 );
  tiltControls->setSpacing( 30 );
  tiltControls->setVertAlign( Control::ALIGN_CENTER );
  tiltControls->setHorizAlign( Control::ALIGN_CENTER );
  tiltControls->setPosition( 40, 90 );
  
  //tilt Up
  osg::Image* tiltUpImg = osgDB::readImageFile( imgDir + "/tilt-up.png" );
  ImageControl* tiltUp = new ImageControl( tiltUpImg );
  tiltUp->addEventHandler( new MyClickHandler );
  
  //tilt Down
  osg::Image* tiltDownImg = osgDB::readImageFile( imgDir + "/tilt-down.png" );
  ImageControl* tiltDown = new ImageControl( tiltDownImg );
  tiltDown->addEventHandler( new MyClickHandler );
  
  //add controls to tiltControls group
  tiltControls->addControl( tiltUp );
  tiltControls->addControl( tiltDown );
  
//END TILT CONTROLS
  
//ZOOM CONTROLS
  //Vertical container
  VBox* zoomControls = new VBox();
  zoomControls->setFrame( new RoundedFrame() );
  //zoomControls->getFrame()->setBackColor(0.5,0.5,0.5,0.1);
  zoomControls->setMargin( 10 );
  zoomControls->setSpacing( 5 );
  zoomControls->setVertAlign( Control::ALIGN_CENTER );
  zoomControls->setHorizAlign( Control::ALIGN_CENTER );
  zoomControls->setPosition( 40, 180 );
  
  //Zoom In
  osg::Image* zoomInImg = osgDB::readImageFile( imgDir + "/zoom-in.png" );
  ImageControl* zoomIn = new ImageControl( zoomInImg );
  zoomIn->addEventHandler( new MyClickHandler );
  
  //Zoom Out
  osg::Image* zoomOutImg = osgDB::readImageFile( imgDir + "/zoom-out.png" );
  ImageControl* zoomOut = new ImageControl( zoomOutImg );
  zoomOut->addEventHandler( new MyClickHandler );
  
  //Zoom Reset
  osg::Image* zoomResetImg = osgDB::readImageFile( imgDir + "/zoom-reset.png" );
  ImageControl* zoomReset = new ImageControl( zoomResetImg );
  zoomReset->addEventHandler( new MyClickHandler );
  
  //add controls to zoomControls group
  zoomControls->addControl( zoomIn );
  zoomControls->addControl( zoomReset );
  zoomControls->addControl( zoomOut );
  
//END ZOOM CONTROLS
  
//add controls groups to canavas
  mControlCanvas->addControl( moveVControls );
  mControlCanvas->addControl( moveHControls );
  mControlCanvas->addControl( rotateControls );
  mControlCanvas->addControl( tiltControls );
  mControlCanvas->addControl( zoomControls );
  
}

void GlobePlugin::setupProxy()
{
  QSettings settings;
  settings.beginGroup( "proxy" );
  if (settings.value("/proxyEnabled").toBool())
  {
    ProxySettings proxySettings(settings.value("/proxyHost").toString().toStdString(),
      settings.value("/proxyPort").toInt());
    if (!settings.value("/proxyUser").toString().isEmpty())
    {
      QString auth = settings.value("/proxyUser").toString() + ":" + settings.value("/proxyPassword").toString();
      setenv("OSGEARTH_CURL_PROXYAUTH", auth.toStdString().c_str(), 0);
    }
    //TODO: settings.value("/proxyType")
    //TODO: URL exlusions
    HTTPClient::setProxySettings(proxySettings);
  }
  settings.endGroup();
}

void GlobePlugin::extentsChanged()
{
    QgsDebugMsg("extentsChanged: " + mQGisIface->mapCanvas()->extent().toString());
}

typedef std::list< osg::ref_ptr<VersionedTile> > TileList;

void GlobePlugin::layersChanged()
{
  QgsDebugMsg("layersChanged");
  if (mTileSource) {
    /*
    //viewer.getDatabasePager()->clear();
    //mMapNode->getTerrain()->incrementRevision();
    TileList tiles;
    mMapNode->getTerrain()->getVersionedTiles( tiles );
    for( TileList::iterator i = tiles.begin(); i != tiles.end(); i++ ) {
      //i->get()->markTileForRegeneration();
      i->get()->updateImagery( mQgisMapLayer->getId(), mMapNode->getMap(), mMapNode->getEngine() );
    }
    */
  }
  if (mTileSource && mMapNode->getMap()->getImageMapLayers().size() > 1)
  {
    QgsDebugMsg("removeMapLayer");
    QgsDebugMsg(QString("getImageMapLayers().size = %1").arg(mMapNode->getMap()->getImageMapLayers().size() ));
    mMapNode->getMap()->removeMapLayer( mQgisMapLayer );
    QgsDebugMsg(QString("getImageMapLayers().size = %1").arg(mMapNode->getMap()->getImageMapLayers().size() ));
    QgsDebugMsg("addMapLayer");
    mTileSource = new QgsOsgEarthTileSource(mQGisIface);
    mTileSource->initialize("", 0);
    mQgisMapLayer = new ImageMapLayer( "QGIS", mTileSource );
    mMapNode->getMap()->addMapLayer( mQgisMapLayer );
    QgsDebugMsg(QString("getImageMapLayers().size = %1").arg(mMapNode->getMap()->getImageMapLayers().size() ));
  }
}

void GlobePlugin::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( "&Globe", mQActionPointer );
  mQGisIface->removeToolBarIcon( mQActionPointer );
  delete mQActionPointer;
}

void GlobePlugin::help()
{
}

void GlobePlugin::placeNode( osg::Node* node, double lat, double lon, double alt /*= 0.0*/ )
{
  // get elevation
  double elevation = 0.0;
  double resolution = 0.0;
  mElevationManager->getElevation( lon, lat, 0, NULL, elevation, resolution );

  // place model
  osg::Matrix mat;
  mObjectPlacer->createPlacerMatrix( lat, lon, elevation + alt, mat );

  osg::MatrixTransform* mt = new osg::MatrixTransform( mat );
  mt->addChild( node );
  mRootNode->addChild( mt );
}

void GlobePlugin::copyFolder(QString sourceFolder, QString destFolder)
{
  QDir sourceDir(sourceFolder);
  if(!sourceDir.exists())
    return;
  QDir destDir(destFolder);
  if(!destDir.exists())
  {
    destDir.mkpath(destFolder);
  }
  QStringList files = sourceDir.entryList(QDir::Files);
  for(int i = 0; i< files.count(); i++)
  {
    QString srcName = sourceFolder + "/" + files[i];
    QString destName = destFolder + "/" + files[i];
    QFile::copy(srcName, destName);
  }
  files.clear();
  files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
  for(int i = 0; i< files.count(); i++)
  {
    QString srcName = sourceFolder + "/" + files[i];
    QString destName = destFolder + "/" + files[i];
    copyFolder(srcName, destName);
  }
}

bool FlyToExtentHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{ 
  float deg = 3.14159 / 180;
  
  //this should be the way to implement this I think, but it segfaults...
  //TODO: put this in the correct place, here is ok for now to test
  //_manipSettings.bindKey(osgEarthUtil::EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::KEY_Page_Up);
  //_manip->applySettings( _manipSettings );
  
  
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '1' )
  
  {
    QgsPoint center = mQGisIface->mapCanvas()->extent().center();
    osgEarthUtil::Viewpoint viewpoint( osg::Vec3d(  center.x(), center.y(), 0.0 ), 0.0, -90.0, 1e4 );
    _manip->setViewpoint( viewpoint, 4.0 );
  }
  
  
  /*if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '4' )
  {
    _manip->pan(-1,0);
  }
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '6' )
  {
    _manip->pan(1,0);
  }  
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '8' )
  {
    _manip->pan(0,1);
  }
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '2' )
  {
    _manip->pan(0,-1);
  }  
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '+' )
  {
    _manip->rotate(0,1*deg);
  }
  */
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '-' )
  {
    _manip->rotate(0,-1*deg);
  }  
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '/' )
  {
    _manip->rotate(1*deg,0);
  }
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '*' )
  {
    _manip->rotate(-1*deg,0);
  } 
  
  return false;
}

// ----------

bool
NavigationControl::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, ControlContext& cx )
{
    if ( ea.getEventType() == osgGA::GUIEventAdapter::PUSH )
    {
        OE_NOTICE << "Thank you for pushing " << std::endl;
        aa.requestContinuousUpdate(true);
    }
        OE_NOTICE << "getEventType " << ea.getEventType() << std::endl;
    return Control::handle( ea, aa, cx );
}

// ----------

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new GlobePlugin( theQgisInterfacePointer );
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return sName;
}

// Return the description
QGISEXTERN QString description()
{
  return sDescription;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return sPluginType;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return sPluginVersion;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
