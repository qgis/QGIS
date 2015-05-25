/***************************************************************************
    qgsgrassplugin.cpp  -  GRASS menu
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassplugin.h"
#include "qgis.h"
#include "qgsgrass.h"

//the gui subclass
#include "qgsgrassedit.h"
#include "qgsgrassnewmapset.h"
#include "qgsgrassregion.h"
#include "qgsgrassselect.h"
#include "qgsgrasstools.h"
#include "qgsgrassutils.h"

// includes
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#endif
#include <grass/version.h>
}

static const QString pluginName = QObject::tr( "GRASS %1" ).arg( GRASS_VERSION_MAJOR );
static const QString pluginDescription = QObject::tr( "GRASS %1 (Geographic Resources Analysis Support System)" ).arg( GRASS_VERSION_MAJOR );
static const QString pluginCategory = QObject::tr( "Plugins" );
static const QString pluginVersion = QObject::tr( "Version 2.0" );
static const QString pluginIcon = ":/images/themes/default/grass/grass_tools.png";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp Pointer to the QGIS main window
 * @param theQgisInterFace Pointer to the QGIS interface object
 */
QgsGrassPlugin::QgsGrassPlugin( QgisInterface * theQgisInterFace )
    : mToolBarPointer( 0 )
    , qGisInterface( theQgisInterFace )
    , mCanvas( 0 )
    , mRegionAction( 0 )
    , mRegion( 0 )
    , mRegionBand( 0 )
    , mTools( 0 )
    , mNewMapset( 0 )
    , mEdit( 0 )
    , mOpenMapsetAction( 0 )
    , mNewMapsetAction( 0 )
    , mCloseMapsetAction( 0 )
    , mOpenToolsAction( 0 )
    , mEditRegionAction( 0 )
    , mEditAction( 0 )
    , mNewVectorAction( 0 )
{
}

QgsGrassPlugin::~QgsGrassPlugin()
{
  QgsDebugMsg( "entered." );
  // When main app is closed, QgsGrassTools (probably because of dock widget) are destroyed before QgsGrassPlugin
  // -> do not call mTools here
  //if ( mTools )
  //  mTools->closeTools();
  if ( mEdit )
    mEdit->closeEdit();
  QString err = QgsGrass::closeMapset();
}

/* Following functions return name, description, version, and type for the plugin */
QString QgsGrassPlugin::name()
{
  return pluginName;
}

QString QgsGrassPlugin::version()
{
  return pluginVersion;
}

QString QgsGrassPlugin::description()
{
  return pluginDescription;
}

QString QgsGrassPlugin::category()
{
  return pluginCategory;
}

void QgsGrassPlugin::help()
{
  //TODO
}

int QgsGrassPlugin::type()
{
  return QgisPlugin::UI;
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGrassPlugin::initGui()
{
  mToolBarPointer = 0;
  mTools = 0;
  mNewMapset = 0;
  mRegion = 0;

  QSettings settings;
  QgsGrass::init();
  mCanvas = qGisInterface->mapCanvas();
  QWidget* qgis = qGisInterface->mainWindow();

  connect( mCanvas, SIGNAL( destinationCrsChanged() ), this, SLOT( setTransform() ) );

  // Connect project
  connect( qgis, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  connect( qgis, SIGNAL( newProject() ), this, SLOT( newProject() ) );

  // Create region rubber band
  mRegionBand = new QgsRubberBand( mCanvas, QGis::Polygon );
  mRegionBand->setZValue( 20 );

  // Create the action for tool (the icons are set later by calling setCurrentTheme)
  mOpenMapsetAction = new QAction( QIcon(), tr( "Open Mapset" ), this );
  mOpenMapsetAction->setObjectName( "mOpenMapsetAction" );
  mNewMapsetAction = new QAction( QIcon(), tr( "New Mapset" ), this );
  mNewMapsetAction->setObjectName( "mNewMapsetAction" );
  mCloseMapsetAction = new QAction( QIcon(), tr( "Close Mapset" ), this );
  mCloseMapsetAction->setObjectName( "mCloseMapsetAction" );

  mOpenToolsAction = new QAction( QIcon(), tr( "Open GRASS Tools" ), this );
  mOpenToolsAction->setObjectName( "mOpenToolsAction" );
  mOpenToolsAction->setWhatsThis( tr( "Open GRASS tools" ) );

  mRegionAction = new QAction( QIcon(), tr( "Display Current Grass Region" ), this );
  mRegionAction->setObjectName( "mRegionAction" );
  mRegionAction->setWhatsThis( tr( "Displays the current GRASS region as a rectangle on the map canvas" ) );
  mRegionAction->setCheckable( true );

  mEditRegionAction = new QAction( QIcon(), tr( "Edit Current Grass Region" ), this );
  mEditRegionAction->setObjectName( "mEditRegionAction" );
  mEditRegionAction->setWhatsThis( tr( "Edit the current GRASS region" ) );
  mEditAction = new QAction( QIcon(), tr( "Edit Grass Vector layer" ), this );
  mEditAction->setObjectName( "mEditAction" );
  mEditAction->setWhatsThis( tr( "Edit the currently selected GRASS vector layer." ) );
  mNewVectorAction = new QAction( QIcon(), tr( "Create New Grass Vector" ), this );
  mNewVectorAction->setObjectName( "mNewVectorAction" );

  // Connect the action
  connect( mOpenToolsAction, SIGNAL( triggered() ), this, SLOT( openTools() ) );
  connect( mEditAction, SIGNAL( triggered() ), this, SLOT( edit() ) );
  connect( mNewVectorAction, SIGNAL( triggered() ), this, SLOT( newVector() ) );
  connect( mRegionAction, SIGNAL( toggled( bool ) ), this, SLOT( switchRegion( bool ) ) );
  connect( mEditRegionAction, SIGNAL( triggered() ), this, SLOT( changeRegion() ) );
  connect( mOpenMapsetAction, SIGNAL( triggered() ), this, SLOT( openMapset() ) );
  connect( mNewMapsetAction, SIGNAL( triggered() ), this, SLOT( newMapset() ) );
  connect( mCloseMapsetAction, SIGNAL( triggered() ), this, SLOT( closeMapset() ) );

  // Add actions to a GRASS plugin menu
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mOpenMapsetAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mNewMapsetAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mCloseMapsetAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mNewVectorAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mEditAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mOpenToolsAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mRegionAction );
  qGisInterface->addPluginToMenu( tr( "&GRASS" ), mEditRegionAction );

  // Add the toolbar to the main window
  mToolBarPointer = qGisInterface->addToolBar( tr( "GRASS" ) );
  mToolBarPointer->setObjectName( "GRASS" );

  // Add to the toolbar
  mToolBarPointer->addAction( mOpenMapsetAction );
  mToolBarPointer->addAction( mNewMapsetAction );
  mToolBarPointer->addAction( mCloseMapsetAction );
  mToolBarPointer->addSeparator();
  mToolBarPointer->addAction( mNewVectorAction );
  mToolBarPointer->addAction( mEditAction );
  mToolBarPointer->addAction( mOpenToolsAction );
  mToolBarPointer->addAction( mRegionAction );
  mToolBarPointer->addAction( mEditRegionAction );

  // Set icons to current theme
  setCurrentTheme( "" );
  // Connect theme change signal
  connect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  // Connect display region
  connect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );

  setEditAction();
  connect( qGisInterface, SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SLOT( setEditAction() ) );

  // Init Region symbology
  mRegionPen.setColor( QColor( settings.value( "/GRASS/region/color", "#ff0000" ).toString() ) );
  mRegionPen.setWidth( settings.value( "/GRASS/region/width", 0 ).toInt() );
  mRegionBand->setColor( mRegionPen.color() );
  mRegionBand->setWidth( mRegionPen.width() );

  mapsetChanged();

  // open tools when plugin is loaded so that main app restores tools dock widget state
  mTools = new QgsGrassTools( qGisInterface, qGisInterface->mainWindow() );
  qGisInterface->addDockWidget( Qt::RightDockWidgetArea, mTools );
}

void QgsGrassPlugin::mapsetChanged()
{
  if ( !QgsGrass::activeMode() )
  {
    mRegionAction->setEnabled( false );
    mEditRegionAction->setEnabled( false );
    mRegionBand->reset();
    mCloseMapsetAction->setEnabled( false );
    mNewVectorAction->setEnabled( false );
  }
  else
  {
    mRegionAction->setEnabled( true );
    mEditRegionAction->setEnabled( true );
    mCloseMapsetAction->setEnabled( true );
    mNewVectorAction->setEnabled( true );

    QSettings settings;
    bool on = settings.value( "/GRASS/region/on", true ).toBool();
    mRegionAction->setChecked( on );
    switchRegion( on );

    if ( mTools )
    {
      mTools->mapsetChanged();
    }

    QString gisdbase = QgsGrass::getDefaultGisdbase();
    QString location = QgsGrass::getDefaultLocation();
    try
    {
      mCrs = QgsGrass::crsDirect( gisdbase, location );
    }
    catch ( QgsGrass::Exception &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( "Cannot read GRASS CRS : " + QString( e.what() ) );
      mCrs = QgsCoordinateReferenceSystem();
    }
    QgsDebugMsg( "mCrs: " + mCrs.toWkt() );
    setTransform();
    redrawRegion();
  }
}

void QgsGrassPlugin::saveMapset()
{
// QgsDebugMsg("entered.");

  // Save working mapset in project file
  QgsProject::instance()->writeEntry( "GRASS", "/WorkingGisdbase",
                                      QgsProject::instance()->writePath( QgsGrass::getDefaultGisdbase() ) );

  QgsProject::instance()->writeEntry( "GRASS", "/WorkingLocation",
                                      QgsGrass::getDefaultLocation() );

  QgsProject::instance()->writeEntry( "GRASS", "/WorkingMapset",
                                      QgsGrass::getDefaultMapset() );
}

// Open tools
void QgsGrassPlugin::openTools()
{
#if 0
  if ( !mTools )
  {
    mTools = new QgsGrassTools( qGisInterface, qGisInterface->mainWindow(), 0, Qt::Dialog );

    connect( mTools, SIGNAL( regionChanged() ), this, SLOT( redrawRegion() ) );
  }
#endif
  mTools->show();
}


// Start vector editing
void QgsGrassPlugin::edit()
{
  if ( QgsGrassEdit::isRunning() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "GRASS Edit is already running." ) );
    return;
  }

  mEditAction->setEnabled( false );
  mEdit = new QgsGrassEdit( qGisInterface, qGisInterface->activeLayer(), false,
                            qGisInterface->mainWindow(), Qt::Dialog );

  if ( mEdit->isValid() )
  {
    mEdit->show();
    mCanvas->refresh();
    connect( mEdit, SIGNAL( finished() ), this, SLOT( setEditAction() ) );
    connect( mEdit, SIGNAL( finished() ), this, SLOT( cleanUp() ) );
    connect( mEdit, SIGNAL( destroyed() ), this, SLOT( editClosed() ) );
    connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( closeEdit( QString ) ) );
  }
  else
  {
    delete mEdit;
    mEdit = NULL;
    mEditAction->setEnabled( true );
  }
}

void QgsGrassPlugin::setEditAction()
{
// QgsDebugMsg("entered.");

  QgsMapLayer *layer = ( QgsMapLayer * ) qGisInterface->activeLayer();

  if ( QgsGrassEdit::isEditable( layer ) )
  {
    mEditAction->setEnabled( true );
  }
  else
  {
    mEditAction->setEnabled( false );
  }
}

void QgsGrassPlugin::closeEdit( QString layerId )
{
  if ( mEdit->layer()->id() == layerId )
  {
    mEdit->closeEdit();
  }
}

void QgsGrassPlugin::editClosed()
{
  if ( mEdit == sender() )
    mEdit = 0;
}

void QgsGrassPlugin::cleanUp()
{
  disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( closeEdit( QString ) ) );
}

void QgsGrassPlugin::newVector()
{
// QgsDebugMsg("entered.");


  if ( QgsGrassEdit::isRunning() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "GRASS Edit is already running." ) );
    return;
  }

  bool ok;
  QString name;

  QgsGrassElementDialog dialog( qGisInterface->mainWindow() );
  name = dialog.getItem( "vector", tr( "New vector name" ),
                         tr( "New vector name" ), "", "", &ok );

  if ( !ok )
    return;

  // Create new map
  QgsGrass::setMapset( QgsGrass::getDefaultGisdbase(),
                       QgsGrass::getDefaultLocation(),
                       QgsGrass::getDefaultMapset() );

  try
  {
    struct Map_info Map;
    Vect_open_new( &Map, name.toUtf8().data(), 0 );

#if defined(GRASS_VERSION_MAJOR) && defined(GRASS_VERSION_MINOR) && \
  ( ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR >= 4 ) || GRASS_VERSION_MAJOR > 6 )
    Vect_build( &Map );
#else
    Vect_build( &Map, stderr );
#endif
    Vect_set_release_support( &Map );
    Vect_close( &Map );
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Cannot create new vector: %1" ).arg( e.what() ) );
    return;
  }



  // Open in GRASS vector provider

  QString uri = QgsGrass::getDefaultGisdbase() + "/"
                + QgsGrass::getDefaultLocation() + "/"
                + QgsGrass::getDefaultMapset() + "/"
                + name + "/0_point";

  QgsVectorLayer* layer = new QgsVectorLayer( uri, name, "grass" );

  if ( !layer )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "New vector created but cannot be opened by data provider." ) );
    return;
  }

  QgsGrassEdit *ed = new QgsGrassEdit( qGisInterface, layer, true,
                                       qGisInterface->mainWindow(), Qt::Dialog );

  if ( ed->isValid() )
  {
    ed->show();
    mCanvas->refresh();
  }
  else
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot start editing." ) );
    delete ed;
  }
#if  0
  if ( !( mProvider->startEdit() ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open vector for update." ) );
    return;
  }
#endif
}

void QgsGrassPlugin::postRender( QPainter *painter )
{
  Q_UNUSED( painter );
  // We have to redraw rectangle, because canvas->mapRenderer()->destinationCrs is set after GRASS plugin constructor! This way it is redrawn also if canvas CRS has changed.
  displayRegion();
// QgsDebugMsg("entered.");
}

void QgsGrassPlugin::displayRegion()
{
// QgsDebugMsg("entered.");

  mRegionBand->reset();
  if ( !mRegionAction->isChecked() )
    return;

  // Display region of current mapset if in active mode
  if ( !QgsGrass::activeMode() )
    return;

  QString gisdbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();
  QString mapset   = QgsGrass::getDefaultMapset();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region." ) );
    return;
  }

  QgsGrass::setLocation( gisdbase, location );

  struct Cell_head window;
  char *err = G__get_window( &window, ( char * ) "", ( char * ) "WIND", mapset.toLatin1().data() );

  if ( err )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot read current region: %1" ).arg( err ) );
    return;
  }

  QgsRectangle rect( QgsPoint( window.west, window.north ), QgsPoint( window.east, window.south ) );

  QgsGrassRegionEdit::drawRegion( mCanvas, mRegionBand, rect, &mCoordinateTransform );
}

void QgsGrassPlugin::switchRegion( bool on )
{
// QgsDebugMsg("entered.");

  QSettings settings;
  settings.setValue( "/GRASS/region/on", on );

  if ( on )
  {
    displayRegion();
  }
  else
  {
    mRegionBand->reset();
  }
}

void QgsGrassPlugin::redrawRegion()
{
// QgsDebugMsg("entered.");

  displayRegion();
}

void QgsGrassPlugin::changeRegion( void )
{
// QgsDebugMsg("entered.");

  if ( mRegion )   // running
  {
    mRegion->show();
    return;
  }

  // Warning: don't use Qt::WType_Dialog, it would ignore restorePosition
  mRegion = new QgsGrassRegion( this, qGisInterface, qGisInterface->mainWindow() );

  connect( mRegion, SIGNAL( destroyed( QObject * ) ), this, SLOT( regionClosed() ) );

  mRegion->show();
}

void QgsGrassPlugin::regionClosed()
{
  mRegion = 0;
}

QPen & QgsGrassPlugin::regionPen()
{
  return mRegionPen;
}

void QgsGrassPlugin::setRegionPen( QPen & pen )
{
  mRegionPen = pen;

  mRegionBand->setColor( mRegionPen.color() );
  mRegionBand->setWidth( mRegionPen.width() );

  QSettings settings;
  settings.setValue( "/GRASS/region/color", mRegionPen.color().name() );
  settings.setValue( "/GRASS/region/width", ( int ) mRegionPen.width() );
}

void QgsGrassPlugin::openMapset()
{
// QgsDebugMsg("entered.");

  QString element;

  QgsGrassSelect *sel = new QgsGrassSelect( qGisInterface->mainWindow(), QgsGrassSelect::MAPSET );

  if ( !sel->exec() )
    return;

  QString err = QgsGrass::openMapset( sel->gisdbase,
                                      sel->location, sel->mapset );

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open the mapset. %1" ).arg( err ) );
    return;
  }

  saveMapset();
  mapsetChanged();
}

void QgsGrassPlugin::closeMapset()
{
// QgsDebugMsg("entered.");

  QString err = QgsGrass::closeMapset();

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot close mapset. %1" ).arg( err ) );
    return;
  }

  saveMapset();
  mapsetChanged();
}

void QgsGrassPlugin::newMapset()
{
  if ( !QgsGrassNewMapset::isRunning() )
  {
    mNewMapset = new QgsGrassNewMapset( qGisInterface,
                                        this, qGisInterface->mainWindow() );
  }
  mNewMapset->show();
  mNewMapset->raise();
}

void QgsGrassPlugin::projectRead()
{
  QgsDebugMsg( "entered." );

  bool ok;
  QString gisdbase = QgsProject::instance()->readPath(
                       QgsProject::instance()->readEntry(
                         "GRASS", "/WorkingGisdbase", "", &ok ).trimmed()
                     );
  QString location = QgsProject::instance()->readEntry(
                       "GRASS", "/WorkingLocation", "", &ok ).trimmed();
  QString mapset = QgsProject::instance()->readEntry(
                     "GRASS", "/WorkingMapset", "", &ok ).trimmed();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() )
  {
    return;
  }

  QgsDebugMsg( "Working mapset specified" );

  QString currentPath = QgsGrass::getDefaultGisdbase() + "/"
                        + QgsGrass::getDefaultLocation() + "/"
                        + QgsGrass::getDefaultMapset();

  QString newPath = gisdbase + "/" + location + "/" + mapset;

  if ( QFileInfo( currentPath ).canonicalPath() ==
       QFileInfo( newPath ).canonicalPath() )
  {
    // The same mapset is already open
    return;
  }

  QString err = QgsGrass::closeMapset();
  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot close current mapset. %1" ).arg( err ) );
    return;
  }
  mapsetChanged();

  err = QgsGrass::openMapset( gisdbase, location, mapset );

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open GRASS mapset. %1" ).arg( err ) );
    return;
  }

  mapsetChanged();
}

void QgsGrassPlugin::newProject()
{
// QgsDebugMsg("entered.");
}

// Unload the plugin by cleaning up the GUI
void QgsGrassPlugin::unload()
{
  // Close mapset
  QString err = QgsGrass::closeMapset();

  // remove the GUI
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mOpenMapsetAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mNewMapsetAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mCloseMapsetAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mOpenToolsAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mRegionAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mEditRegionAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mEditAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mNewVectorAction );

  delete mOpenMapsetAction;
  delete mNewMapsetAction;
  delete mCloseMapsetAction;
  delete mOpenToolsAction;
  delete mRegionAction;
  delete mEditRegionAction;
  delete mEditAction;
  delete mNewVectorAction;

  delete mToolBarPointer;
  mToolBarPointer = 0;

  // disconnect slots of QgsGrassPlugin so they're not fired also after unload
  disconnect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );
  disconnect( qGisInterface, SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
              this, SLOT( setEditAction() ) );

  QWidget* qgis = qGisInterface->mainWindow();
  disconnect( qgis, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  disconnect( qgis, SIGNAL( newProject() ), this, SLOT( newProject() ) );

  delete mTools;
  mTools = 0;
}

// Set icons to the current theme
void QgsGrassPlugin::setCurrentTheme( QString theThemeName )
{
  Q_UNUSED( theThemeName );
  if ( mToolBarPointer )
  {
    mOpenMapsetAction->setIcon( getThemeIcon( "grass_open_mapset.png" ) );
    mNewMapsetAction->setIcon( getThemeIcon( "grass_new_mapset.png" ) );
    mCloseMapsetAction->setIcon( getThemeIcon( "grass_close_mapset.png" ) );

    mOpenToolsAction->setIcon( getThemeIcon( "grass_tools.png" ) );

    mRegionAction->setIcon( getThemeIcon( "grass_region.png" ) );

    mEditRegionAction->setIcon( getThemeIcon( "grass_region_edit.png" ) );
    mEditAction->setIcon( getThemeIcon( "grass_edit.png" ) );
    mNewVectorAction->setIcon( getThemeIcon( "grass_new_vector_layer.png" ) );
  }
}

// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins dependent on qgsapplication
// and because it needs grass specific path into
// the GRASS plugin resource bundle [TS]
QIcon QgsGrassPlugin::getThemeIcon( const QString &theName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/grass/" + theName;
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/grass/" + theName;
  QString myQrcPath = ":/default/grass/" + theName;
  if ( QFile::exists( myCurThemePath ) )
  {
    return QIcon( myCurThemePath );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    return QIcon( myDefThemePath );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    return QIcon( myQrcPath );
  }
  else
  {
    return QIcon();
  }
}

void QgsGrassPlugin::setTransform()
{
  if ( mCrs.isValid() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    QgsDebugMsg( "srcCrs: " + mCrs.toWkt() );
    QgsDebugMsg( "destCrs " + mCanvas->mapSettings().destinationCrs().toWkt() );
    mCoordinateTransform.setSourceCrs( mCrs );
    mCoordinateTransform.setDestCRS( mCanvas->mapSettings().destinationCrs() );
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
  return new QgsGrassPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return pluginName;
}

// Return the description
QGISEXTERN QString description()
{
  return pluginDescription;
}

// Return the category
QGISEXTERN QString category()
{
  return pluginCategory;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return pluginVersion;
}

QGISEXTERN QString icon()
{
  return pluginIcon;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
