/***************************************************************************
    globe_plugin.cpp

    Globe Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Include this first to avoid _POSIX_C_SOURCE redefined warnings
// see http://bytes.com/topic/python/answers/30009-warning-_posix_c_source-redefined
#include "globe_plugin.h"
#include "qgsglobeplugindialog.h"
#include "qgsglobefeatureidentify.h"
#include "qgsglobefrustumhighlight.h"
#include "qgsglobetilesource.h"
#include "qgsglobevectorlayerproperties.h"
#include "qgsglobewidget.h"
#include "featuresource/qgsglobefeatureoptions.h"

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgscrscache.h>
#include <qgslogger.h>
#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsproject.h>
#include <qgspoint.h>
#include <qgsdistancearea.h>
#include <symbology-ng/qgsrendererv2.h>
#include <symbology-ng/qgssymbolv2.h>
#include <qgspallabeling.h>

#include <QAction>
#include <QDir>
#include <QDockWidget>
#include <QStringList>

#include <osg/Light>
#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgEarthQt/ViewerWidget>
#include <osgEarth/ElevationQuery>
#include <osgEarth/Notify>
#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarth/Registry>
#include <osgEarth/TileSource>
#include <osgEarth/Version>
#include <osgEarthDrivers/engine_mp/MPTerrainEngineOptions>
#include <osgEarthUtil/Controls>
#include <osgEarthUtil/EarthManipulator>
#if OSGEARTH_VERSION_LESS_THAN( 2, 6, 0 )
#include <osgEarthUtil/SkyNode>
#else
#include <osgEarthUtil/Sky>
#endif
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/wms/WMSOptions>

#if OSGEARTH_VERSION_GREATER_OR_EQUAL( 2, 2, 0 )
#include <osgEarthDrivers/cache_filesystem/FileSystemCache>
#endif
#if OSGEARTH_VERSION_GREATER_OR_EQUAL( 2, 5, 0 )
#include <osgEarthUtil/VerticalScale>
#endif
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
#include <osgEarthUtil/FeatureQueryTool>
#include <osgEarthFeatures/FeatureDisplayLayout>

#define MOVE_OFFSET 0.05

static const QString sName = QObject::tr( "Globe" );
static const QString sDescription = QObject::tr( "Overlay data on a 3D globe" );
static const QString sCategory = QObject::tr( "Plugins" );
static const QString sPluginVersion = QObject::tr( "Version 1.0" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sIcon = ":/globe/icon.svg";
static const QString sExperimental = QString( "false" );


class NavigationControlHandler : public osgEarth::Util::Controls::ControlEventHandler
{
  public:
    virtual void onMouseDown() { }
    virtual void onClick( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*aa*/ ) {}
};

class ZoomControlHandler : public NavigationControlHandler
{
  public:
    ZoomControlHandler( osgEarth::Util::EarthManipulator* manip, double dx, double dy )
        : _manip( manip ), _dx( dx ), _dy( dy ) { }
    virtual void onMouseDown() override
    {
      _manip->zoom( _dx, _dy );
    }
  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;
    double _dx;
    double _dy;
};

class HomeControlHandler : public NavigationControlHandler
{
  public:
    HomeControlHandler( osgEarth::Util::EarthManipulator* manip ) : _manip( manip ) { }
    virtual void onClick( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) override
    {
      _manip->home( ea, aa );
    }
  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;
};

class SyncExtentControlHandler : public NavigationControlHandler
{
  public:
    SyncExtentControlHandler( GlobePlugin* globe ) : mGlobe( globe ) { }
    virtual void onClick( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*aa*/ ) override
    {
      mGlobe->syncExtent();
    }
  private:
    GlobePlugin* mGlobe;
};

class PanControlHandler : public NavigationControlHandler
{
  public:
    PanControlHandler( osgEarth::Util::EarthManipulator* manip, double dx, double dy ) : _manip( manip ), _dx( dx ), _dy( dy ) { }
    virtual void onMouseDown() override
    {
      _manip->pan( _dx, _dy );
    }
  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;
    double _dx;
    double _dy;
};

class RotateControlHandler : public NavigationControlHandler
{
  public:
    RotateControlHandler( osgEarth::Util::EarthManipulator* manip, double dx, double dy ) : _manip( manip ), _dx( dx ), _dy( dy ) { }
    virtual void onMouseDown() override
    {
      if ( 0 == _dx && 0 == _dy )
        _manip->setRotation( osg::Quat() );
      else
        _manip->rotate( _dx, _dy );
    }
  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;
    double _dx;
    double _dy;
};

// An event handler that will print out the coordinates at the clicked point
class QueryCoordinatesHandler : public osgGA::GUIEventHandler
{
  public:
    QueryCoordinatesHandler( GlobePlugin* globe ) :  mGlobe( globe ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
      if ( ea.getEventType() == osgGA::GUIEventAdapter::MOVE )
      {
        osgViewer::View* view = static_cast<osgViewer::View*>( aa.asView() );
        osgUtil::LineSegmentIntersector::Intersections hits;
        if ( view->computeIntersections( ea.getX(), ea.getY(), hits ) )
        {
          osgEarth::GeoPoint isectPoint;
          isectPoint.fromWorld( mGlobe->mapNode()->getMapSRS()->getGeodeticSRS(), hits.begin()->getWorldIntersectPoint() );
          mGlobe->showCurrentCoordinates( isectPoint );
        }
      }
      return false;
    }

  private:
    GlobePlugin* mGlobe;
};

class KeyboardControlHandler : public osgGA::GUIEventHandler
{
  public:
    KeyboardControlHandler( osgEarth::Util::EarthManipulator* manip ) : _manip( manip ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) override;

  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;
};

class NavigationControl : public osgEarth::Util::Controls::ImageControl
{
  public:
    NavigationControl( osg::Image* image = 0 ) : ImageControl( image ),  mMousePressed( false ) {}

  protected:
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osgEarth::Util::Controls::ControlContext& cx ) override;

  private:
    bool mMousePressed;
};


GlobePlugin::GlobePlugin( QgisInterface* theQgisInterface )
    : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
    , mQGisIface( theQgisInterface )
    , mViewerWidget( 0 )
    , mDockWidget( 0 )
    , mSettingsDialog( 0 )
    , mSelectedLat( 0. )
    , mSelectedLon( 0. )
    , mSelectedElevation( 0. )
    , mLayerPropertiesFactory( 0 )
{
#ifdef Q_OS_MACX
  // update path to osg plugins on Mac OS X
  if ( !getenv( "OSG_LIBRARY_PATH" ) )
  {
    // OSG_PLUGINS_PATH value set by CMake option
    QString ogsPlugins( OSG_PLUGINS_PATH );
    QString bundlePlugins = QgsApplication::pluginPath() + "/../osgPlugins";
    if ( QFile::exists( bundlePlugins ) )
    {
      // add internal osg plugin path if bundled osg
      ogsPlugins = bundlePlugins;
    }
    if ( QFile::exists( ogsPlugins ) )
    {
      osgDB::Registry::instance()->setLibraryFilePathList( QDir::cleanPath( ogsPlugins ).toStdString() );
    }
  }
#endif
}

GlobePlugin::~GlobePlugin() {}

void GlobePlugin::initGui()
{

  mSettingsDialog = new QgsGlobePluginDialog( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );
  connect( mSettingsDialog, SIGNAL( settingsApplied() ), this, SLOT( applySettings() ) );

  mActionToggleGlobe = new QAction( QIcon( ":/globe/globe.png" ), tr( "Launch Globe" ), this );
  mActionToggleGlobe->setCheckable( true );
  mQGisIface->addToolBarIcon( mActionToggleGlobe );
  mQGisIface->addPluginToMenu( tr( "&Globe" ), mActionToggleGlobe );

  mLayerPropertiesFactory = new QgsGlobeLayerPropertiesFactory( this );
  mQGisIface->registerMapLayerConfigWidgetFactory( mLayerPropertiesFactory );

  connect( mActionToggleGlobe, SIGNAL( triggered( bool ) ), this, SLOT( setGlobeEnabled( bool ) ) );
  connect( mLayerPropertiesFactory, SIGNAL( layerSettingsChanged( QgsMapLayer* ) ), this, SLOT( layerChanged( QgsMapLayer* ) ) );
  connect( this, SIGNAL( xyCoordinates( const QgsPoint & ) ), mQGisIface->mapCanvas(), SIGNAL( xyCoordinates( const QgsPoint & ) ) );
  connect( mQGisIface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
}

void GlobePlugin::run()
{
  if ( mViewerWidget != 0 )
  {
    return;
  }
#ifdef GLOBE_SHOW_TILE_STATS
  QgsGlobeTileStatistics* tileStats = new QgsGlobeTileStatistics();
  connect( tileStats, SIGNAL( changed( int, int ) ), this, SLOT( updateTileStats( int, int ) ) );
#endif
  QSettings settings;

//    osgEarth::setNotifyLevel( osg::DEBUG_INFO );

  mOsgViewer = new osgViewer::Viewer();
  mOsgViewer->setThreadingModel( osgViewer::Viewer::SingleThreaded );
  mOsgViewer->setRunFrameScheme( osgViewer::Viewer::ON_DEMAND );
  // Set camera manipulator with default home position
  osgEarth::Util::EarthManipulator* manip = new osgEarth::Util::EarthManipulator();
  mOsgViewer->setCameraManipulator( manip );
  osgEarth::Util::Viewpoint viewpoint;
  viewpoint.focalPoint() = osgEarth::GeoPoint( osgEarth::SpatialReference::get( "wgs84" ), 0., 0., 0. );
  viewpoint.heading() = 0.;
  viewpoint.pitch() = -90.;
  viewpoint.range() = 2e7;

  manip->setHomeViewpoint( viewpoint, 1. );
  manip->home( 0 );

  setupProxy();

  // Tile stats label
#ifdef GLOBE_SHOW_TILE_STATS
  mStatsLabel = new osgEarth::Util::Controls::LabelControl( "", 10 );
  mStatsLabel->setPosition( 0, 0 );
  osgEarth::Util::Controls::ControlCanvas::get( mOsgViewer )->addControl( mStatsLabel.get() );
#endif

  mDockWidget = new QgsGlobeWidget( mQGisIface, mQGisIface->mainWindow() );
  connect( mDockWidget, SIGNAL( destroyed( QObject* ) ), this, SLOT( reset() ) );
  connect( mDockWidget, SIGNAL( layersChanged() ), this, SLOT( updateLayers() ) );
  connect( mDockWidget, SIGNAL( showSettings() ), this, SLOT( showSettings() ) );
  connect( mDockWidget, SIGNAL( refresh() ), this, SLOT( rebuildQGISLayer() ) );
  connect( mDockWidget, SIGNAL( syncExtent() ), this, SLOT( syncExtent() ) );
  mQGisIface->addDockWidget( Qt::RightDockWidgetArea, mDockWidget );

  if ( getenv( "GLOBE_MAPXML" ) )
  {
    char* mapxml = getenv( "GLOBE_MAPXML" );
    QgsDebugMsg( mapxml );
    osg::Node* node = osgDB::readNodeFile( mapxml );
    if ( !node )
    {
      QgsDebugMsg( "Failed to load earth file " );
      return;
    }
    mMapNode = osgEarth::MapNode::findMapNode( node );
    mRootNode = new osg::Group();
    mRootNode->addChild( node );
  }
  else
  {
    QString cacheDirectory = settings.value( "cache/directory", QgsApplication::qgisSettingsDirPath() + "cache" ).toString();
    osgEarth::Drivers::FileSystemCacheOptions cacheOptions;
    cacheOptions.rootPath() = cacheDirectory.toStdString();

    osgEarth::MapOptions mapOptions;
    mapOptions.cache() = cacheOptions;
    osgEarth::Map *map = new osgEarth::Map( /*mapOptions*/ );

    // The MapNode will render the Map object in the scene graph.
    osgEarth::MapNodeOptions mapNodeOptions;
    mMapNode = new osgEarth::MapNode( map, mapNodeOptions );

    mRootNode = new osg::Group();
    mRootNode->addChild( mMapNode );

    osgEarth::Registry::instance()->unRefImageDataAfterApply() = false;

    // Add draped layer
    osgEarth::TileSourceOptions opts;
    opts.L2CacheSize() = 0;
    opts.tileSize() = 128;
    mTileSource = new QgsGlobeTileSource( mQGisIface->mapCanvas(), opts );

    osgEarth::ImageLayerOptions options( "QGIS" );
    options.driver()->L2CacheSize() = 0;
    options.cachePolicy() = osgEarth::CachePolicy::USAGE_NO_CACHE;
    mQgisMapLayer = new osgEarth::ImageLayer( options, mTileSource );
    map->addImageLayer( mQgisMapLayer );


    // Create the frustum highlight callback
    mFrustumHighlightCallback = new QgsGlobeFrustumHighlightCallback(
      mOsgViewer, mMapNode->getTerrain(), mQGisIface->mapCanvas(), QColor( 0, 0, 0, 50 ) );
  }

  mRootNode->addChild( osgEarth::Util::Controls::ControlCanvas::get( mOsgViewer ) );

  mOsgViewer->setSceneData( mRootNode );

  mOsgViewer->addEventHandler( new QueryCoordinatesHandler( this ) );
  mOsgViewer->addEventHandler( new KeyboardControlHandler( manip ) );
  mOsgViewer->addEventHandler( new osgViewer::StatsHandler() );
  mOsgViewer->addEventHandler( new osgViewer::WindowSizeHandler() );
  mOsgViewer->addEventHandler( new osgGA::StateSetManipulator( mOsgViewer->getCamera()->getOrCreateStateSet() ) );
  mOsgViewer->getCamera()->addCullCallback( new osgEarth::Util::AutoClipPlaneCullCallback( mMapNode ) );

  // osgEarth benefits from pre-compilation of GL objects in the pager. In newer versions of
  // OSG, this activates OSG's IncrementalCompileOpeartion in order to avoid frame breaks.
  mOsgViewer->getDatabasePager()->setDoPreCompile( true );

  mViewerWidget = new osgEarth::QtGui::ViewerWidget( mOsgViewer );
  if ( settings.value( "/Plugin-Globe/anti-aliasing", true ).toBool() &&
       settings.value( "/Plugin-Globe/anti-aliasing-level", "" ).toInt() > 0 )
  {
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers( true );
    glf.setSamples( settings.value( "/Plugin-Globe/anti-aliasing-level", "" ).toInt() );
    mViewerWidget->setFormat( glf );
  }

  mDockWidget->setWidget( mViewerWidget );
  mViewerWidget->setParent( mDockWidget );

  mFeatureQueryToolIdentifyCb = new QgsGlobeFeatureIdentifyCallback( mQGisIface->mapCanvas() );
  mFeatureQueryTool = new osgEarth::Util::FeatureQueryTool();
  mFeatureQueryTool->addChild( mMapNode );
  mFeatureQueryTool->setDefaultCallback( mFeatureQueryToolIdentifyCb.get() );

  setupControls();
  // FIXME: Workaround for OpenGL errors, in some manner related to the SkyNode,
  // which appear when launching the globe a second time:
  // Delay applySettings one event loop iteration, i.e. one update call of the GL canvas
  QTimer* timer = new QTimer();
  QTimer* timer2 = new QTimer();
  connect( timer, SIGNAL( timeout() ), timer, SLOT( deleteLater() ) );
  connect( timer2, SIGNAL( timeout() ), timer2, SLOT( deleteLater() ) );
  connect( timer, SIGNAL( timeout() ), this, SLOT( applySettings() ) );
  connect( timer2, SIGNAL( timeout() ), this, SLOT( updateLayers() ) );
  timer->start( 0 );
  timer2->start( 100 );
}

void GlobePlugin::showSettings()
{
  mSettingsDialog->exec();
}

void GlobePlugin::projectRead()
{
  mSettingsDialog->readProjectSettings();
  applyProjectSettings();
}

void GlobePlugin::applySettings()
{
  if ( !mOsgViewer )
  {
    return;
  }

  osgEarth::Util::EarthManipulator* manip = dynamic_cast<osgEarth::Util::EarthManipulator*>( mOsgViewer->getCameraManipulator() );
  osgEarth::Util::EarthManipulator::Settings* settings = manip->getSettings();
  settings->setScrollSensitivity( mSettingsDialog->getScrollSensitivity() );
  if ( !mSettingsDialog->getInvertScrollWheel() )
  {
    settings->bindScroll( osgEarth::Util::EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_UP );
    settings->bindScroll( osgEarth::Util::EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_DOWN );
  }
  else
  {
    settings->bindScroll( osgEarth::Util::EarthManipulator::ACTION_ZOOM_OUT, osgGA::GUIEventAdapter::SCROLL_UP );
    settings->bindScroll( osgEarth::Util::EarthManipulator::ACTION_ZOOM_IN, osgGA::GUIEventAdapter::SCROLL_DOWN );
  }

  // Advanced settings
  enableFrustumHighlight( mSettingsDialog->getFrustumHighlighting() );
  enableFeatureIdentification( mSettingsDialog->getFeatureIdenification() );

  applyProjectSettings();
}

void GlobePlugin::applyProjectSettings()
{
  if ( mOsgViewer && !getenv( "GLOBE_MAPXML" ) )
  {
    // Imagery settings
    QList<QgsGlobePluginDialog::LayerDataSource> imageryDataSources = mSettingsDialog->getImageryDataSources();
    if ( imageryDataSources != mImagerySources )
    {
      mImagerySources = imageryDataSources;
      QgsDebugMsg( "imageryLayersChanged: Globe Running, executing" );
      osg::ref_ptr<osgEarth::Map> map = mMapNode->getMap();

      if ( map->getNumImageLayers() > 1 )
      {
        mOsgViewer->getDatabasePager()->clear();
      }

      // Remove image layers
      osgEarth::ImageLayerVector list;
      map->getImageLayers( list );
      for ( osgEarth::ImageLayerVector::iterator i = list.begin(); i != list.end(); ++i )
      {
        if ( *i != mQgisMapLayer )
          map->removeImageLayer( *i );
      }

      // Add image layers
      foreach ( const QgsGlobePluginDialog::LayerDataSource& datasource, mImagerySources )
      {
        osgEarth::ImageLayer* layer = 0;
        if ( "Raster" == datasource.type )
        {
          osgEarth::Drivers::GDALOptions options;
          options.url() = datasource.uri.toStdString();
          layer = new osgEarth::ImageLayer( datasource.uri.toStdString(), options );
        }
        else if ( "TMS" == datasource.type )
        {
          osgEarth::Drivers::TMSOptions options;
          options.url() = datasource.uri.toStdString();
          layer = new osgEarth::ImageLayer( datasource.uri.toStdString(), options );
        }
        else if ( "WMS" == datasource.type )
        {
          osgEarth::Drivers::WMSOptions options;
          options.url() = datasource.uri.toStdString();
          layer = new osgEarth::ImageLayer( datasource.uri.toStdString(), options );
        }
        map->insertImageLayer( layer, 0 );
      }
    }

    // Elevation settings
    QList<QgsGlobePluginDialog::LayerDataSource> elevationDataSources = mSettingsDialog->getElevationDataSources();
    if ( elevationDataSources != mElevationSources )
    {
      mElevationSources = elevationDataSources;
      QgsDebugMsg( "elevationLayersChanged: Globe Running, executing" );
      osg::ref_ptr<osgEarth::Map> map = mMapNode->getMap();

      if ( map->getNumElevationLayers() > 1 )
      {
        mOsgViewer->getDatabasePager()->clear();
      }

      // Remove elevation layers
      osgEarth::ElevationLayerVector list;
      map->getElevationLayers( list );
      for ( osgEarth::ElevationLayerVector::iterator i = list.begin(); i != list.end(); ++i )
      {
        map->removeElevationLayer( *i );
      }

      // Add elevation layers
      foreach ( const QgsGlobePluginDialog::LayerDataSource& datasource, mElevationSources )
      {
        osgEarth::ElevationLayer* layer = 0;
        if ( "Raster" == datasource.type )
        {
          osgEarth::Drivers::GDALOptions options;
          options.interpolation() = osgEarth::Drivers::INTERP_NEAREST;
          options.url() = datasource.uri.toStdString();
          layer = new osgEarth::ElevationLayer( datasource.uri.toStdString(), options );
        }
        else if ( "TMS" == datasource.type )
        {
          osgEarth::Drivers::TMSOptions options;
          options.url() = datasource.uri.toStdString();
          layer = new osgEarth::ElevationLayer( datasource.uri.toStdString(), options );
        }
        map->addElevationLayer( layer );
      }
    }
#if OSGEARTH_VERSION_GREATER_OR_EQUAL( 2, 5, 0 )
    double verticalScaleValue = mSettingsDialog->getVerticalScale();
    if ( !mVerticalScale.get() || mVerticalScale->getScale() != verticalScaleValue )
    {
      mMapNode->getTerrainEngine()->removeEffect( mVerticalScale );
      mVerticalScale = new osgEarth::Util::VerticalScale();
      mVerticalScale->setScale( verticalScaleValue );
      mMapNode->getTerrainEngine()->addEffect( mVerticalScale );
    }
#endif

    // Sky settings
    if ( mSettingsDialog->getSkyEnabled() )
    {
      // Create if not yet done
      if ( !mSkyNode.get() )
      {
        mSkyNode = osgEarth::Util::SkyNode::create( mMapNode );
        mSkyNode->attach( mOsgViewer );
        mRootNode->addChild( mSkyNode );
        // Insert sky between root and map
        mSkyNode->addChild( mMapNode );
        mRootNode->removeChild( mMapNode );
      }

      mSkyNode->setLighting( mSettingsDialog->getSkyAutoAmbience() ? osg::StateAttribute::ON : osg::StateAttribute::OFF );
      double ambient = mSettingsDialog->getSkyMinAmbient();
      mSkyNode->getSunLight()->setAmbient( osg::Vec4( ambient, ambient, ambient, 1 ) );

      QDateTime dateTime = mSettingsDialog->getSkyDateTime();
      mSkyNode->setDateTime( osgEarth::DateTime(
                               dateTime.date().year(),
                               dateTime.date().month(),
                               dateTime.date().day(),
                               dateTime.time().hour() + dateTime.time().minute() / 60.0 ) );
    }
    else if ( mSkyNode != 0 )
    {
      mRootNode->addChild( mMapNode );
      mSkyNode->removeChild( mMapNode );
      mRootNode->removeChild( mSkyNode );
      mSkyNode = 0;
    }
  }
}

QgsRectangle GlobePlugin::getQGISLayerExtent() const
{
  QList<QgsRectangle> extents = mLayerExtents.values();
  QgsRectangle fullExtent = extents.isEmpty() ? QgsRectangle() : extents.front();
  for ( int i = 1, n = extents.size(); i < n; ++i )
  {
    if ( !extents[i].isNull() )
      fullExtent.combineExtentWith( extents[i] );
  }
  return fullExtent;
}

void GlobePlugin::showCurrentCoordinates( const osgEarth::GeoPoint& geoPoint )
{
  osg::Vec3d pos = geoPoint.vec3d();
  emit xyCoordinates( QgsCoordinateTransformCache::instance()->transform( GEO_EPSG_CRS_AUTHID, mQGisIface->mapCanvas()->mapSettings().destinationCrs().authid() )->transform( QgsPoint( pos.x(), pos.y() ) ) );
}

void GlobePlugin::setSelectedCoordinates( const osg::Vec3d &coords )
{
  mSelectedLon = coords.x();
  mSelectedLat = coords.y();
  mSelectedElevation = coords.z();
  emit newCoordinatesSelected( QgsPoint( mSelectedLon, mSelectedLat ) );
}

osg::Vec3d GlobePlugin::getSelectedCoordinates()
{
  return osg::Vec3d( mSelectedLon, mSelectedLat, mSelectedElevation );
}

void GlobePlugin::syncExtent()
{
  const QgsMapSettings& mapSettings = mQGisIface->mapCanvas()->mapSettings();
  QgsRectangle extent = mQGisIface->mapCanvas()->extent();

  long epsgGlobe = 4326;
  QgsCoordinateReferenceSystem globeCrs;
  globeCrs.createFromOgcWmsCrs( QString( "EPSG:%1" ).arg( epsgGlobe ) );

  // transform extent to WGS84
  if ( mapSettings.destinationCrs().authid().compare( QString( "EPSG:%1" ).arg( epsgGlobe ), Qt::CaseInsensitive ) != 0 )
  {
    QgsCoordinateReferenceSystem srcCRS( mapSettings.destinationCrs() );
    extent = QgsCoordinateTransform( srcCRS, globeCrs ).transformBoundingBox( extent );
  }

  QgsDistanceArea dist;
  dist.setSourceCrs( globeCrs );
  dist.setEllipsoidalMode( true );
  dist.setEllipsoid( "WGS84" );

  QgsPoint ll = QgsPoint( extent.xMinimum(), extent.yMinimum() );
  QgsPoint ul = QgsPoint( extent.xMinimum(), extent.yMaximum() );
  double height = dist.measureLine( ll, ul );
//  double height = dist.computeDistanceBearing( ll, ul );

  double camViewAngle = 30;
  double camDistance = height / tan( camViewAngle * osg::PI / 180 ); //c = b*cotan(B(rad))
#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
  osgEarth::Util::Viewpoint viewpoint( osg::Vec3d( extent.center().x(), extent.center().y(), 0.0 ), 0.0, -90.0, camDistance );
#else
  osgEarth::Util::Viewpoint viewpoint;
  viewpoint.focalPoint() = osgEarth::GeoPoint( osgEarth::SpatialReference::get( "wgs84" ), extent.center().x(), extent.center().y(), 0.0 );
  viewpoint.heading() = 0.0;
  viewpoint.pitch() = -90.0;
  viewpoint.range() = camDistance;
#endif

  OE_NOTICE << "map extent: " << height << " camera distance: " << camDistance << std::endl;

  osgEarth::Util::EarthManipulator* manip = dynamic_cast<osgEarth::Util::EarthManipulator*>( mOsgViewer->getCameraManipulator() );
  manip->setRotation( osg::Quat() );
  manip->setViewpoint( viewpoint, 4.0 );
}

void GlobePlugin::addControl( osgEarth::Util::Controls::Control* control, int x, int y, int w, int h, osgEarth::Util::Controls::ControlEventHandler* handler )
{
  control->setPosition( x, y );
  control->setHeight( h );
  control->setWidth( w );
  control->addEventHandler( handler );
  osgEarth::Util::Controls::ControlCanvas::get( mOsgViewer )->addControl( control );
}

void GlobePlugin::addImageControl( const std::string& imgPath, int x, int y, osgEarth::Util::Controls::ControlEventHandler *handler )
{
  osg::Image* image = osgDB::readImageFile( imgPath );
  osgEarth::Util::Controls::ImageControl* control = new NavigationControl( image );
  control->setPosition( x, y );
  control->setWidth( image->s() );
  control->setHeight( image->t() );
  if ( handler )
    control->addEventHandler( handler );
  osgEarth::Util::Controls::ControlCanvas::get( mOsgViewer )->addControl( control );
}

void GlobePlugin::setupControls()
{
  std::string imgDir = QDir::cleanPath( QgsApplication::pkgDataPath() + "/globe/gui" ).toStdString();
  if ( QgsApplication::isRunningFromBuildDir() )
  {
    imgDir = QDir::cleanPath( QgsApplication::buildSourcePath() + "/src/plugins/globe/images/gui" ).toStdString();
  }
  osgEarth::Util::EarthManipulator* manip = dynamic_cast<osgEarth::Util::EarthManipulator*>( mOsgViewer->getCameraManipulator() );

  // Rotate and tiltcontrols
  int imgLeft = 16;
  int imgTop = 20;
  addImageControl( imgDir + "/YawPitchWheel.png", 16, 20 );
  addControl( new NavigationControl, imgLeft, imgTop + 18, 20, 22, new RotateControlHandler( manip, -MOVE_OFFSET, 0 ) );
  addControl( new NavigationControl, imgLeft + 36, imgTop + 18, 20, 22, new RotateControlHandler( manip, MOVE_OFFSET, 0 ) );
  addControl( new NavigationControl, imgLeft + 20, imgTop + 18, 16, 22, new RotateControlHandler( manip, 0, 0 ) );
  addControl( new NavigationControl, imgLeft + 20, imgTop, 24, 19, new RotateControlHandler( manip, 0, -MOVE_OFFSET ) );
  addControl( new NavigationControl, imgLeft + 16, imgTop + 36, 24, 19, new RotateControlHandler( manip, 0, MOVE_OFFSET ) );

  // Move controls
  imgTop = 80;
  addImageControl( imgDir + "/MoveWheel.png", imgLeft, imgTop );
  addControl( new NavigationControl, imgLeft, imgTop + 18, 20, 22, new PanControlHandler( manip, MOVE_OFFSET, 0 ) );
  addControl( new NavigationControl, imgLeft + 36, imgTop + 18, 20, 22, new PanControlHandler( manip, -MOVE_OFFSET, 0 ) );
  addControl( new NavigationControl, imgLeft + 20, imgTop, 24, 19, new PanControlHandler( manip, 0, -MOVE_OFFSET ) );
  addControl( new NavigationControl, imgLeft + 16, imgTop + 36, 24, 19, new PanControlHandler( manip, 0, MOVE_OFFSET ) );
  addControl( new NavigationControl, imgLeft + 20, imgTop + 18, 16, 22, new HomeControlHandler( manip ) );

  // Zoom controls
  imgLeft = 28;
  imgTop = imgTop + 62;
  addImageControl( imgDir + "/button-background.png", imgLeft, imgTop );
  addImageControl( imgDir + "/zoom-in.png", imgLeft + 3, imgTop + 2, new ZoomControlHandler( manip, 0, -MOVE_OFFSET ) );
  addImageControl( imgDir + "/zoom-out.png", imgLeft + 3, imgTop + 29, new ZoomControlHandler( manip, 0, MOVE_OFFSET ) );
}

void GlobePlugin::setupProxy()
{
  QSettings settings;
  settings.beginGroup( "proxy" );
  if ( settings.value( "/proxyEnabled" ).toBool() )
  {
    osgEarth::ProxySettings proxySettings( settings.value( "/proxyHost" ).toString().toStdString(),
                                           settings.value( "/proxyPort" ).toInt() );
    if ( !settings.value( "/proxyUser" ).toString().isEmpty() )
    {
      QString auth = settings.value( "/proxyUser" ).toString() + ":" + settings.value( "/proxyPassword" ).toString();
      qputenv( "OSGEARTH_CURL_PROXYAUTH", auth.toLocal8Bit() );
    }
    //TODO: settings.value("/proxyType")
    //TODO: URL exlusions
    osgEarth::HTTPClient::setProxySettings( proxySettings );
  }
  settings.endGroup();
}

void GlobePlugin::refreshQGISMapLayer( const QgsRectangle& dirtyRect )
{
  if ( mTileSource )
  {
    mOsgViewer->getDatabasePager()->clear();
    mTileSource->refresh( dirtyRect );
    mOsgViewer->requestRedraw();
  }
}

void GlobePlugin::updateTileStats( int queued, int tot )
{
  if ( mStatsLabel )
    mStatsLabel->setText( QString( "Queued tiles: %1\nTot tiles: %2" ).arg( queued ).arg( tot ).toStdString() );
}

void GlobePlugin::addModelLayer( QgsVectorLayer* vLayer, QgsGlobeVectorLayerConfig* layerConfig )
{
  QgsGlobeFeatureOptions  featureOpt;
  featureOpt.setLayer( vLayer );
  osgEarth::Style style;

  if ( !vLayer->rendererV2()->symbols().isEmpty() )
  {
    Q_FOREACH ( QgsSymbolV2* sym, vLayer->rendererV2()->symbols() )
    {
      if ( sym->type() == QgsSymbolV2::Line )
      {
        osgEarth::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::LineSymbol>();
        QColor color = sym->color();
        ls->stroke()->color() = osg::Vec4f( color.redF(), color.greenF(), color.blueF(), color.alphaF() * ( 100.f - vLayer->layerTransparency() ) / 100.f );
        ls->stroke()->width() = 1.0f;
      }
      else if ( sym->type() == QgsSymbolV2::Fill )
      {
        // TODO access border color, etc.
        osgEarth::PolygonSymbol* poly = style.getOrCreateSymbol<osgEarth::PolygonSymbol>();
        QColor color = sym->color();
        poly->fill()->color() = osg::Vec4f( color.redF(), color.greenF(), color.blueF(), color.alphaF() * ( 100.f - vLayer->layerTransparency() ) / 100.f );
        style.addSymbol( poly );
      }
    }
  }
  else
  {
    osgEarth::PolygonSymbol* poly = style.getOrCreateSymbol<osgEarth::PolygonSymbol>();
    poly->fill()->color() = osg::Vec4f( 1.f, 0, 0, 1.f - vLayer->layerTransparency() / 255.f );
    style.addSymbol( poly );
    osgEarth::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::LineSymbol>();
    ls->stroke()->color() = osg::Vec4f( 1.f, 0, 0, 1.f - vLayer->layerTransparency() / 255.f );
    ls->stroke()->width() = 1.0f;
  }

  osgEarth::AltitudeSymbol* altitudeSymbol = style.getOrCreateSymbol<osgEarth::AltitudeSymbol>();
  altitudeSymbol->clamping() = layerConfig->altitudeClamping;
  altitudeSymbol->technique() = layerConfig->altitudeTechnique;
  altitudeSymbol->binding() = layerConfig->altitudeBinding;
  altitudeSymbol->verticalOffset() = layerConfig->verticalOffset;
  altitudeSymbol->verticalScale() = layerConfig->verticalScale;
  altitudeSymbol->clampingResolution() = layerConfig->clampingResolution;
  style.addSymbol( altitudeSymbol );

  if ( layerConfig->extrusionEnabled )
  {
    osgEarth::ExtrusionSymbol* extrusionSymbol = style.getOrCreateSymbol<osgEarth::ExtrusionSymbol>();
    bool extrusionHeightOk = false;
    float extrusionHeight = layerConfig->extrusionHeight.toFloat( &extrusionHeightOk );
    if ( extrusionHeightOk )
    {
      extrusionSymbol->height() = extrusionHeight;
    }
    else
    {
      extrusionSymbol->heightExpression() = layerConfig->extrusionHeight.toStdString();
    }

    extrusionSymbol->flatten() = layerConfig->extrusionFlatten;
    extrusionSymbol->wallGradientPercentage() = layerConfig->extrusionWallGradient;
    style.addSymbol( extrusionSymbol );
  }

  if ( layerConfig->labelingEnabled )
  {
    osgEarth::TextSymbol* textSymbol = style.getOrCreateSymbol<osgEarth::TextSymbol>();
    textSymbol->declutter() = layerConfig->labelingDeclutter;
    QgsPalLayerSettings lyr;
    lyr.readFromLayer( vLayer );
    QString labelingExpr = lyr.getLabelExpression()->expression();
    textSymbol->content() = QString( "[%1]" ).arg( labelingExpr ).toStdString();
    textSymbol->font() = lyr.textFont.family().toStdString();
    textSymbol->size() = lyr.textFont.pointSize();
    textSymbol->alignment() = osgEarth::TextSymbol::ALIGN_CENTER_TOP;
    osgEarth::Stroke stroke;
    stroke.color() = osgEarth::Symbology::Color( lyr.bufferColor.redF(), lyr.bufferColor.greenF(), lyr.bufferColor.blueF(), lyr.bufferColor.alphaF() );
    textSymbol->halo() = stroke;
    textSymbol->haloOffset() = lyr.bufferSize;
  }

  osgEarth::RenderSymbol* renderSymbol = style.getOrCreateSymbol<osgEarth::RenderSymbol>();
  renderSymbol->lighting() = layerConfig->lightingEnabled;
  renderSymbol->backfaceCulling() = false;
  style.addSymbol( renderSymbol );

  osgEarth::Drivers::FeatureGeomModelOptions geomOpt;
  geomOpt.featureOptions() = featureOpt;
  geomOpt.styles() = new osgEarth::StyleSheet();
  geomOpt.styles()->addStyle( style );

  geomOpt.featureIndexing() = osgEarth::Features::FeatureSourceIndexOptions();

#if 0
  FeatureDisplayLayout layout;
  layout.tileSizeFactor() = 45.0;
  layout.addLevel( FeatureLevel( 0.0f, 200000.0f ) );
  geomOpt.layout() = layout;
#endif

  osgEarth::ModelLayerOptions modelOptions( vLayer->id().toStdString(), geomOpt );

  osgEarth::ModelLayer* nLayer = new osgEarth::ModelLayer( modelOptions );

  mMapNode->getMap()->addModelLayer( nLayer );
}

void GlobePlugin::updateLayers()
{
  if ( mOsgViewer )
  {
    // Get previous full extent
    QgsRectangle dirtyExtent = getQGISLayerExtent();
    mLayerExtents.clear();

    QStringList drapedLayers;
    QStringList selectedLayers = mDockWidget->getSelectedLayers();

    // Disconnect any previous repaintRequested signals
    foreach ( const QString& layerId, mTileSource->layerSet() )
    {
      QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
      if ( mapLayer )
        disconnect( mapLayer, SIGNAL( repaintRequested() ), this, SLOT( layerChanged() ) );
      if ( dynamic_cast<QgsVectorLayer*>( mapLayer ) )
        disconnect( static_cast<QgsVectorLayer*>( mapLayer ), SIGNAL( layerTransparencyChanged( int ) ), this, SLOT( layerChanged() ) );
    }
    osgEarth::ModelLayerVector modelLayers;
    mMapNode->getMap()->getModelLayers( modelLayers );
    foreach ( const osg::ref_ptr<osgEarth::ModelLayer>& modelLayer, modelLayers )
    {
      QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( QString::fromStdString( modelLayer->getName() ) );
      if ( mapLayer )
        disconnect( mapLayer, SIGNAL( repaintRequested() ), this, SLOT( layerChanged() ) );
      if ( dynamic_cast<QgsVectorLayer*>( mapLayer ) )
        disconnect( static_cast<QgsVectorLayer*>( mapLayer ), SIGNAL( layerTransparencyChanged( int ) ), this, SLOT( layerChanged() ) );
      if ( !selectedLayers.contains( QString::fromStdString( modelLayer->getName() ) ) )
        mMapNode->getMap()->removeModelLayer( modelLayer );
    }

    Q_FOREACH ( const QString& layerId, selectedLayers )
    {
      QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
      connect( mapLayer, SIGNAL( repaintRequested() ), this, SLOT( layerChanged() ) );

      QgsGlobeVectorLayerConfig* layerConfig = 0;
      if ( dynamic_cast<QgsVectorLayer*>( mapLayer ) )
      {
        layerConfig = QgsGlobeVectorLayerConfig::getConfig( static_cast<QgsVectorLayer*>( mapLayer ) );
        connect( static_cast<QgsVectorLayer*>( mapLayer ), SIGNAL( layerTransparencyChanged( int ) ), this, SLOT( layerChanged() ) );
      }

      if ( layerConfig && ( layerConfig->renderingMode == QgsGlobeVectorLayerConfig::RenderingModeModelSimple || layerConfig->renderingMode == QgsGlobeVectorLayerConfig::RenderingModeModelAdvanced ) )
      {
        if ( !mMapNode->getMap()->getModelLayerByName( mapLayer->id().toStdString() ) )
          addModelLayer( static_cast<QgsVectorLayer*>( mapLayer ), layerConfig );
      }
      else
      {
        drapedLayers.append( mapLayer->id() );
        QgsRectangle extent = QgsCoordinateTransformCache::instance()->transform( mapLayer->crs().authid(), GEO_EPSG_CRS_AUTHID )->transform( mapLayer->extent() );
        mLayerExtents.insert( mapLayer->id(), extent );
      }
    }

    mTileSource->setLayerSet( drapedLayers );
    QgsRectangle newExtent = getQGISLayerExtent();
    if ( dirtyExtent.isNull() )
      dirtyExtent = newExtent;
    else if ( !newExtent.isNull() )
      dirtyExtent.combineExtentWith( newExtent );
    refreshQGISMapLayer( dirtyExtent );
  }
}

void GlobePlugin::layerChanged( QgsMapLayer* mapLayer )
{
  if ( !mapLayer )
  {
    mapLayer = qobject_cast<QgsMapLayer*>( QObject::sender() );
  }
  if ( mapLayer->isEditable() )
  {
    return;
  }
  if ( mMapNode )
  {
    QgsGlobeVectorLayerConfig* layerConfig = 0;
    if ( dynamic_cast<QgsVectorLayer*>( mapLayer ) )
    {
      layerConfig = QgsGlobeVectorLayerConfig::getConfig( static_cast<QgsVectorLayer*>( mapLayer ) );
    }

    if ( layerConfig && ( layerConfig->renderingMode == QgsGlobeVectorLayerConfig::RenderingModeModelSimple || layerConfig->renderingMode == QgsGlobeVectorLayerConfig::RenderingModeModelAdvanced ) )
    {
      // If was previously a draped layer, refresh the draped layer
      if ( mTileSource->layerSet().contains( mapLayer->id() ) )
      {
        QStringList layerSet = mTileSource->layerSet();
        layerSet.removeAll( mapLayer->id() );
        mTileSource->setLayerSet( layerSet );
        QgsRectangle dirtyExtent = mLayerExtents[mapLayer->id()];
        mLayerExtents.remove( mapLayer->id() );
        refreshQGISMapLayer( dirtyExtent );
      }
      mMapNode->getMap()->removeModelLayer( mMapNode->getMap()->getModelLayerByName( mapLayer->id().toStdString() ) );
      addModelLayer( static_cast<QgsVectorLayer*>( mapLayer ), layerConfig );
    }
    else
    {
      // Re-insert into layer set if necessary
      if ( !mTileSource->layerSet().contains( mapLayer->id() ) )
      {
        QStringList layerSet;
        foreach ( const QString& layer, mDockWidget->getSelectedLayers() )
        {
          if ( ! mMapNode->getMap()->getModelLayerByName( layer.toStdString() ) )
          {
            layerSet.append( layer );
          }
        }
        mTileSource->setLayerSet( layerSet );
        QgsRectangle extent = QgsCoordinateTransformCache::instance()->transform( mapLayer->crs().authid(), GEO_EPSG_CRS_AUTHID )->transform( mapLayer->extent() );
        mLayerExtents.insert( mapLayer->id(), extent );
      }
      // Remove any model layer of that layer, in case one existed
      mMapNode->getMap()->removeModelLayer( mMapNode->getMap()->getModelLayerByName( mapLayer->id().toStdString() ) );
      QgsRectangle layerExtent = QgsCoordinateTransformCache::instance()->transform( mapLayer->crs().authid(), GEO_EPSG_CRS_AUTHID )->transform( mapLayer->extent() );
      QgsRectangle dirtyExtent = layerExtent;
      if ( mLayerExtents.contains( mapLayer->id() ) )
      {
        if ( dirtyExtent.isNull() )
          dirtyExtent = mLayerExtents[mapLayer->id()];
        else if ( !mLayerExtents[mapLayer->id()].isNull() )
          dirtyExtent.combineExtentWith( mLayerExtents[mapLayer->id()] );
      }
      mLayerExtents[mapLayer->id()] = layerExtent;
      refreshQGISMapLayer( dirtyExtent );
    }
  }
}

void GlobePlugin::rebuildQGISLayer()
{
  if ( mMapNode )
  {
    mMapNode->getMap()->removeImageLayer( mQgisMapLayer );
    mLayerExtents.clear();

    osgEarth::TileSourceOptions opts;
    opts.L2CacheSize() = 0;
    opts.tileSize() = 128;
    mTileSource = new QgsGlobeTileSource( mQGisIface->mapCanvas(), opts );

    osgEarth::ImageLayerOptions options( "QGIS" );
    options.driver()->L2CacheSize() = 0;
    options.cachePolicy() = osgEarth::CachePolicy::USAGE_NO_CACHE;
    mQgisMapLayer = new osgEarth::ImageLayer( options, mTileSource );
    mMapNode->getMap()->addImageLayer( mQgisMapLayer );
    updateLayers();
  }
}

void GlobePlugin::setGlobeEnabled( bool enabled )
{
  if ( enabled )
  {
    run();
  }
  else if ( mDockWidget )
  {
    mDockWidget->close(); // triggers reset
  }
}

void GlobePlugin::reset()
{
  mStatsLabel = 0;
  mActionToggleGlobe->blockSignals( true );
  mActionToggleGlobe->setChecked( false );
  mActionToggleGlobe->blockSignals( false );
  mMapNode->getMap()->removeImageLayer( mQgisMapLayer ); // abort any rendering
  mOsgViewer = 0;
  mMapNode = 0;
  mRootNode = 0;
  mSkyNode = 0;
  mBaseLayer = 0;
  mBaseLayerUrl.clear();
  mQgisMapLayer = 0;
  mTileSource = 0;
  mVerticalScale = 0;
  mFrustumHighlightCallback = 0;
  mFeatureQueryToolIdentifyCb = 0;
#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
  mFeatureQueryToolHighlightCb = 0;
#endif
  mFeatureQueryTool = 0;
  mViewerWidget = 0;
  mDockWidget = 0;
  mImagerySources.clear();
  mElevationSources.clear();
  mLayerExtents.clear();
#ifdef GLOBE_SHOW_TILE_STATS
  disconnect( QgsGlobeTileStatistics::instance(), SIGNAL( changed( int, int ) ), this, SLOT( updateTileStats( int, int ) ) );
  delete QgsGlobeTileStatistics::instance();
#endif
}

void GlobePlugin::unload()
{
  if ( mDockWidget )
  {
    disconnect( mDockWidget, SIGNAL( destroyed( QObject* ) ), this, SLOT( reset() ) );
    delete mDockWidget;
    reset();
  }
  mQGisIface->removePluginMenu( tr( "&Globe" ), mActionToggleGlobe );
  mQGisIface->removeToolBarIcon( mActionToggleGlobe );
  mQGisIface->unregisterMapLayerConfigWidgetFactory( mLayerPropertiesFactory );
  delete mLayerPropertiesFactory;
  mLayerPropertiesFactory = 0;
  delete mSettingsDialog;
  mSettingsDialog = 0;

  disconnect( this, SIGNAL( xyCoordinates( const QgsPoint & ) ),
              mQGisIface->mapCanvas(), SIGNAL( xyCoordinates( const QgsPoint & ) ) );
}

void GlobePlugin::enableFrustumHighlight( bool status )
{
  if ( status )
    mMapNode->getTerrainEngine()->addUpdateCallback( mFrustumHighlightCallback );
  else
    mMapNode->getTerrainEngine()->removeUpdateCallback( mFrustumHighlightCallback );
}

void GlobePlugin::enableFeatureIdentification( bool status )
{
  if ( status )
    mOsgViewer->addEventHandler( mFeatureQueryTool );
  else
    mOsgViewer->removeEventHandler( mFeatureQueryTool );
}

bool NavigationControl::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osgEarth::Util::Controls::ControlContext& cx )
{
  if ( ea.getEventType() == osgGA::GUIEventAdapter::PUSH )
  {
    mMousePressed = true;
  }
  else if ( ea.getEventType() == osgGA::GUIEventAdapter::FRAME && mMousePressed )
  {
    float canvasY = cx._vp->height() - ( ea.getY() - cx._view->getCamera()->getViewport()->y() );
    float canvasX = ea.getX() - cx._view->getCamera()->getViewport()->x();

    if ( intersects( canvasX, canvasY ) )
    {
      for ( osgEarth::Util::Controls::ControlEventHandlerList::const_iterator i = _eventHandlers.begin(); i != _eventHandlers.end(); ++i )
      {
        NavigationControlHandler* handler = dynamic_cast<NavigationControlHandler*>( i->get() );
        if ( handler )
        {
          handler->onMouseDown();
        }
      }
    }
    else
    {
      mMousePressed = false;
    }
  }
  else if ( ea.getEventType() == osgGA::GUIEventAdapter::RELEASE )
  {
    for ( osgEarth::Util::Controls::ControlEventHandlerList::const_iterator i = _eventHandlers.begin(); i != _eventHandlers.end(); ++i )
    {
      NavigationControlHandler* handler = dynamic_cast<NavigationControlHandler*>( i->get() );
      if ( handler )
      {
        handler->onClick( ea, aa );
      }
    }
    mMousePressed = false;
  }
  return Control::handle( ea, aa, cx );
}

bool KeyboardControlHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
  if ( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN )
  {
    //move map
    if ( ea.getKey() == '4' )
      _manip->pan( -MOVE_OFFSET, 0 );
    else if ( ea.getKey() == '6' )
      _manip->pan( MOVE_OFFSET, 0 );
    else if ( ea.getKey() == '2' )
      _manip->pan( 0, MOVE_OFFSET );
    else if ( ea.getKey() == '8' )
      _manip->pan( 0, -MOVE_OFFSET );
    //rotate
    else if ( ea.getKey() == '/' )
      _manip->rotate( MOVE_OFFSET, 0 );
    else if ( ea.getKey() == '*' )
      _manip->rotate( -MOVE_OFFSET, 0 );
    //tilt
    else if ( ea.getKey() == '9' )
      _manip->rotate( 0, MOVE_OFFSET );
    else if ( ea.getKey() == '3' )
      _manip->rotate( 0, -MOVE_OFFSET );
    //zoom
    else if ( ea.getKey() == '-' )
      _manip->zoom( 0, MOVE_OFFSET );
    else if ( ea.getKey() == '+' )
      _manip->zoom( 0, -MOVE_OFFSET );
    //reset
    else if ( ea.getKey() == '5' )
      _manip->home( ea, aa );
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

// Return the category
QGISEXTERN QString category()
{
  return sCategory;
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

// Return the icon
QGISEXTERN QString icon()
{
  return sIcon;
}

// Return the experimental status for the plugin
QGISEXTERN QString experimental()
{
  return sExperimental;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
