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
#include "qgsosgearthtilesource.h"

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgslogger.h>
#include <qgsapplication.h>
#include <qgsmapcanvas.h>

#include <QAction>
#include <QToolBar>

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
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ElevationManager>
#include <osgEarthUtil/ObjectPlacer>
#include <osgEarth/TileSource>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/tms/TMSOptions>
#include "Controls"

using namespace osgEarth::Drivers;
using namespace osgEarthUtil::Controls2;


//static const char * const sIdent = "$Id: plugin.cpp 9327 2008-09-14 11:18:44Z jef $";
static const QString sName = QObject::tr( "Globe" );
static const QString sDescription = QObject::tr( "Overlay data on a 3D globe" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;

GlobePlugin::GlobePlugin( QgisInterface* theQgisInterface )
  : QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface ),
    mQActionPointer( NULL ),
    viewer(),
    mQDockWidget( tr( "Globe" ) ),
    mTileSource(0),
    mElevationManager( NULL ),
    mObjectPlacer( NULL )
{
}

GlobePlugin::~GlobePlugin()
{
}

void GlobePlugin::initGui()
{
  // Create the action for tool
  mQActionPointer = new QAction( QIcon( ":/globe/globe_plugin.png" ), tr( "Globe" ), this );
  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Overlay data on a 3D globe" ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mQActionPointer );
  mQGisIface->addPluginToMenu( tr( "&Globe" ), mQActionPointer );
  mQDockWidget.setWidget(&viewer);

  connect(mQGisIface->mapCanvas() , SIGNAL(extentsChanged()),
          this, SLOT( extentsChanged() ) );
  connect(mQGisIface->mapCanvas(), SIGNAL(layersChanged()),
          this, SLOT( layersChanged() ) );
}

struct MyClickHandler : public ControlEventHandler
{
    void onClick( Control* control, int mouseButtonMask )
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

  // read base layers from earth file
  EarthFile earthFile;
  if ( !earthFile.readXML( QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe/globe.earth" ).toStdString() ) )
  {
    return;
  }

  osg::ref_ptr<Map> map = earthFile.getMap();

  // Add base image to the map
  GDALOptions* opt = new GDALOptions();
  opt->url() = QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe/world.tif" ).toStdString();
  osg::ref_ptr<MapLayer> layer = new ImageMapLayer( "World", opt );
  map->addMapLayer( layer );

  // Add QGIS layer to the map
  mTileSource = new QgsOsgEarthTileSource(mQGisIface);
  mTileSource->initialize("", 0);
  mQgisMapLayer = new ImageMapLayer( "QGIS", mTileSource );
  map->addMapLayer( mQgisMapLayer );

  osg::Group* root = new osg::Group();

  // The MapNode will render the Map object in the scene graph.
  mMapNode = new osgEarth::MapNode( map );
  root->addChild( mMapNode );

  // create a surface to house the controls
  ControlCanvas* cs = new ControlCanvas( &viewer );
  root->addChild( cs );

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
  double lat = 47.235;
  double lon = 9.36;
  double gridSize = 0.001;
  for( int i=0; i<10; i++ )
  {
    for( int j=0; j<10; j++ )
    {
      double dx = gridSize * ( rand()/( (double)RAND_MAX + 1.0 ) - 0.5 );
      double dy = gridSize * ( rand()/( (double)RAND_MAX + 1.0 ) - 0.5 );
      placeNode( root, model, lat + i * gridSize + dx, lon + j * gridSize + dy );
    }
  }
#endif

  viewer.setSceneData( root );

  // Set a home viewpoint
  manip->setHomeViewpoint(
    osgEarthUtil::Viewpoint( osg::Vec3d( -90, 0, 0 ), 0.0, -90.0, 4e7 ),
    1.0 );

  // a centered hbox container along the bottom on the screen.
  {
      HBox* bottom = new HBox();
      bottom->setFrame( new RoundedFrame() );
      bottom->getFrame()->setBackColor(0,0,0,0.5);
      bottom->setMargin( 10 );
      bottom->setSpacing( 145 );
      bottom->setVertAlign( Control::ALIGN_BOTTOM );
      bottom->setHorizAlign( Control::ALIGN_CENTER );

      for( int i=0; i<4; ++i )
      {
          LabelControl* label = new LabelControl();
          std::stringstream buf;
          buf << "Label_" << i;
          label->setText( buf.str() );
          label->setMargin( 10 );
          label->setBackColor( 1,1,1,0.4 );
          bottom->addControl( label );

          label->setActiveColor(1,.3,.3,1);
          label->addEventHandler( new MyClickHandler );
      }

      cs->addControl( bottom );
  }

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

void GlobePlugin::placeNode( osg::Group* root, osg::Node* node, double lat, double lon, double alt /*= 0.0*/ )
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
  root->addChild( mt );
}

bool FlyToExtentHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
  if ( ea.getEventType() == ea.KEYDOWN && ea.getKey() == '1' )
  {
    QgsPoint center = mQGisIface->mapCanvas()->extent().center();
    osgEarthUtil::Viewpoint viewpoint( osg::Vec3d(  center.x(), center.y(), 0.0 ), 0.0, -90.0, 1e4 );
    _manip->setViewpoint( viewpoint, 4.0 );
  }
  return false;
}


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
