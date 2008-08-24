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
/*  $Id$ */

// includes
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"
#include "qgsproviderregistry.h"
#include <qgsrasterlayer.h>

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <q3popupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <QFileDialog>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qregexp.h>
#include <qglobal.h>
#include <qinputdialog.h>
#include <QIcon>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PointArray>

//non qt includes

#include <qgsproject.h>
#include <qgsrubberband.h>

extern "C"
{
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "qgsgrassplugin.h"
#include "qgsgrass.h"
#include "qgsgrassprovider.h"

//the gui subclass
#include "qgsgrassutils.h"
#include "qgsgrassattributes.h"
#include "qgsgrassselect.h"
#include "qgsgrassedit.h"
#include "qgsgrasstools.h"
#include "qgsgrassregion.h"
#include "qgsgrassnewmapset.h"
#include "qgslogger.h"

static const QString pluginVersion = QObject::tr( "Version 0.1" );

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp Pointer to the QGIS main window
 * @param theQgisInterFace Pointer to the QGIS interface object
 */
QgsGrassPlugin::QgsGrassPlugin( QgisInterface * theQgisInterFace ):
    qGisInterface( theQgisInterFace )
{
  /** Initialize the plugin and set the required attributes */
  pluginNameQString = tr( "GrassVector" );
  pluginVersionQString = tr( "0.1" );
  pluginDescriptionQString = tr( "GRASS layer" );
}

QgsGrassPlugin::~QgsGrassPlugin()
{
  if ( mTools ) mTools->closeTools();
  QString err = QgsGrass::closeMapset();
}

/* Following functions return name, description, version, and type for the plugin */
QString QgsGrassPlugin::name()
{
  return pluginNameQString;
}

QString QgsGrassPlugin::version()
{
  return pluginVersionQString;
}

QString QgsGrassPlugin::description()
{
  return pluginDescriptionQString;
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
  toolBarPointer = 0;
  mTools = 0;
  mNewMapset = 0;
  mRegion = 0;

  QSettings settings;

  QgsGrass::init();

  mCanvas = qGisInterface->getMapCanvas();
  QWidget* qgis = qGisInterface->getMainWindow();

  // Connect project
  connect( qgis, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  connect( qgis, SIGNAL( newProject() ), this, SLOT( newProject() ) );

  // Create region rubber band
  mRegionBand = new QgsRubberBand( mCanvas, 1 );
  mRegionBand->setZValue( 20 );

  // Create the action for tool
  mOpenMapsetAction = new QAction( getThemeIcon( "grass_open_mapset.png" ), tr( "Open mapset" ), this );
  mNewMapsetAction = new QAction( getThemeIcon( "grass_new_mapset.png" ), tr( "New mapset" ), this );
  mCloseMapsetAction = new QAction( getThemeIcon( "grass_close_mapset.png" ), tr( "Close mapset" ), this );

  mAddVectorAction = new QAction( getThemeIcon( "grass_add_vector.png" ),
                                  tr( "Add GRASS vector layer" ), this );
  mAddRasterAction = new QAction( getThemeIcon( "grass_add_raster.png" ),
                                  tr( "Add GRASS raster layer" ), this );
  mOpenToolsAction = new QAction( getThemeIcon( "grass_tools.png" ),
                                  tr( "Open GRASS tools" ), this );

  mRegionAction = new QAction( getThemeIcon( "grass_region.png" ),
                               tr( "Display Current Grass Region" ), this );
  mRegionAction->setCheckable( true );

  mEditRegionAction = new QAction( getThemeIcon( "grass_region_edit.png" ),
                                   tr( "Edit Current Grass Region" ), this );
  mEditAction = new QAction( getThemeIcon( "grass_edit.png" ),
                             tr( "Edit Grass Vector layer" ), this );
  mNewVectorAction = new QAction( getThemeIcon( "grass_new_vector_layer.png" ), tr( "Create new Grass Vector" ), this );

  mAddVectorAction->setWhatsThis( tr( "Adds a GRASS vector layer to the map canvas" ) );
  mAddRasterAction->setWhatsThis( tr( "Adds a GRASS raster layer to the map canvas" ) );
  mOpenToolsAction->setWhatsThis( tr( "Open GRASS tools" ) );
  mRegionAction->setWhatsThis( tr( "Displays the current GRASS region as a rectangle on the map canvas" ) );
  mEditRegionAction->setWhatsThis( tr( "Edit the current GRASS region" ) );
  mEditAction->setWhatsThis( tr( "Edit the currently selected GRASS vector layer." ) );

  // Connect the action
  connect( mAddVectorAction, SIGNAL( triggered() ), this, SLOT( addVector() ) );
  connect( mAddRasterAction, SIGNAL( triggered() ), this, SLOT( addRaster() ) );
  connect( mOpenToolsAction, SIGNAL( triggered() ), this, SLOT( openTools() ) );
  connect( mEditAction, SIGNAL( triggered() ), this, SLOT( edit() ) );
  connect( mNewVectorAction, SIGNAL( triggered() ), this, SLOT( newVector() ) );
  connect( mRegionAction, SIGNAL( toggled( bool ) ), this, SLOT( switchRegion( bool ) ) );
  connect( mEditRegionAction, SIGNAL( triggered() ), this, SLOT( changeRegion() ) );
  connect( mOpenMapsetAction, SIGNAL( triggered() ), this, SLOT( openMapset() ) );
  connect( mNewMapsetAction, SIGNAL( triggered() ), this, SLOT( newMapset() ) );
  connect( mCloseMapsetAction, SIGNAL( triggered() ), this, SLOT( closeMapset() ) );

  // Add actions to a GRASS plugin menu
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mOpenMapsetAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mNewMapsetAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mCloseMapsetAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mAddVectorAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mAddRasterAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mNewVectorAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mEditAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mOpenToolsAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mRegionAction );
  qGisInterface->addPluginMenu( tr( "&GRASS" ), mEditRegionAction );

  // Add the toolbar to the main window
  toolBarPointer = qGisInterface->addToolBar( tr( "GRASS" ) );
  toolBarPointer->setIconSize( QSize( 24, 24 ) );
  toolBarPointer->setObjectName( "GRASS" );

  // Add to the toolbar
  mOpenMapsetAction->addTo( toolBarPointer );
  mNewMapsetAction->addTo( toolBarPointer );
  mCloseMapsetAction->addTo( toolBarPointer );
  toolBarPointer->addSeparator();
  mAddVectorAction->addTo( toolBarPointer );
  mAddRasterAction->addTo( toolBarPointer );
  mNewVectorAction->addTo( toolBarPointer );
  mEditAction->addTo( toolBarPointer );
  mOpenToolsAction->addTo( toolBarPointer );
  mRegionAction->addTo( toolBarPointer );
  mEditRegionAction->addTo( toolBarPointer );

  // Connect display region
  connect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );

  setEditAction();
  connect( qGisInterface, SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SLOT( setEditAction() ) );

  // Init Region symbology
  mRegionPen.setColor( QColor( settings.readEntry( "/GRASS/region/color", "#ff0000" ) ) );
  mRegionPen.setWidth( settings.readNumEntry( "/GRASS/region/width", 0 ) );
  mRegionBand->setColor( mRegionPen.color() );
  mRegionBand->setWidth( mRegionPen.width() );

  mapsetChanged();
}

void QgsGrassPlugin::mapsetChanged()
{
  if ( !QgsGrass::activeMode() )
  {
    mOpenToolsAction->setEnabled( false );
    mRegionAction->setEnabled( false );
    mEditRegionAction->setEnabled( false );
    mRegionBand->reset();
    mCloseMapsetAction->setEnabled( false );
    mNewVectorAction->setEnabled( false );

    if ( mTools )
    {
      mTools->hide();
      delete mTools;
      mTools = 0;
    }
  }
  else
  {
    mOpenToolsAction->setEnabled( true );
    mRegionAction->setEnabled( true );
    mEditRegionAction->setEnabled( true );
    mCloseMapsetAction->setEnabled( true );
    mNewVectorAction->setEnabled( true );

    QSettings settings;
    bool on = settings.readBoolEntry( "/GRASS/region/on", true );
    mRegionAction->setOn( on );
    switchRegion( on );

    if ( mTools )
    {
      mTools->mapsetChanged();
    }
  }
}

void QgsGrassPlugin::saveMapset()
{
// QgsDebugMsg("entered.");

  // Save working mapset in project file
  QgsProject::instance()->writeEntry( "GRASS", "/WorkingGisdbase",
                                      QgsGrass::getDefaultGisdbase() );

  QgsProject::instance()->writeEntry( "GRASS", "/WorkingLocation",
                                      QgsGrass::getDefaultLocation() );

  QgsProject::instance()->writeEntry( "GRASS", "/WorkingMapset",
                                      QgsGrass::getDefaultMapset() );
}

// Slot called when the "Add GRASS vector layer" menu item is triggered
void QgsGrassPlugin::addVector()
{
// QgsDebugMsg("entered.");
  QString uri;

  QgsGrassSelect *sel = new QgsGrassSelect( QgsGrassSelect::VECTOR );
  if ( sel->exec() )
  {
    uri = sel->gisdbase + "/" + sel->location + "/" + sel->mapset + "/" + sel->map + "/" + sel->layer;
  }
#ifdef QGISDEBUG
// QgsDebugMsg(QString("plugin URI: %1").arg(uri));
#endif
  if ( uri.length() == 0 )
  {
// QgsDebugMsg("Nothing was selected");
    return;
  }
  else
  {
#ifdef QGISDEBUG
// QgsDebugMsg("Add new vector layer");
#endif

    // create vector name: vector layer
    QString name = sel->map;

    QString field;
    QString type;

    QRegExp rx( "(\\d+)_(.+)" );
    if ( rx.search( sel->layer ) != -1 )
    {
      field = rx.cap( 1 );
      type = rx.cap( 2 );
    }

    // Set location
    QgsGrass::setLocation( sel->gisdbase, sel->location );

    /* Open vector */
    QgsGrass::resetError();
    Vect_set_open_level( 2 );
    struct Map_info map;
    int level = Vect_open_old_head( &map, ( char * ) sel->map.ascii(),
                                    ( char * ) sel->mapset.ascii() );

    if ( QgsGrass::getError() != QgsGrass::FATAL )
    {
      if ( level >= 2 )
      {
        // Count layers
        int cnt = 0;
        int ncidx = Vect_cidx_get_num_fields( &map );

        for ( int i = 0; i < ncidx; i++ )
        {
          int field = Vect_cidx_get_field_number( &map, i );

          if ( Vect_cidx_get_type_count( &map, field, GV_POINT | GV_LINE | GV_AREA ) > 0 ||
               ( field > 1 && Vect_cidx_get_type_count( &map, field, GV_BOUNDARY ) ) )
          {
            cnt++;
          }
        }

        if ( cnt > 1 )
        {
          name.append( " " + field );

          // No need to ad type, the type is obvious from the legend
        }
      }

      Vect_close( &map );
    }
    else
    {
      QMessageBox::warning( 0, tr( "Warning" ), "Cannot open GRASS vector:\n " + QgsGrass::getErrorMessage() );
    }

    qGisInterface->addVectorLayer( uri, name, "grass" );
  }
}

// Slot called when the "Add GRASS raster layer" menu item is triggered
void QgsGrassPlugin::addRaster()
{
// QgsDebugMsg("entered.");
  QString uri;

  QgsGrassSelect *sel = new QgsGrassSelect( QgsGrassSelect::RASTER );
  if ( sel->exec() )
  {
    QString element;
    if ( sel->selectedType == QgsGrassSelect::RASTER )
    {
      element = "cellhd";
    }
    else   // GROUP
    {
      element = "group";
    }

    uri = sel->gisdbase + "/" + sel->location + "/" + sel->mapset + "/" + element + "/" + sel->map;
  }

// QgsDebugMsg(QString("plugin URI: %1").arg(uri));

  if ( uri.length() == 0 )
  {
// QgsDebugMsg("Nothing was selected");
    return;
  }
  else
  {

// QgsDebugMsg("Add new raster layer");

    // create raster name
    int pos = uri.findRev( '/' );
    pos = uri.findRev( '/', pos - 1 );
    QString name = uri.right( uri.length() - pos - 1 );
    name.replace( '/', ' ' );

    qGisInterface->addRasterLayer( uri, sel->map );
  }
}

// Open tools
void QgsGrassPlugin::openTools()
{
  if ( !mTools )
  {
    mTools = new QgsGrassTools( qGisInterface, qGisInterface->getMainWindow(), 0, Qt::WType_Dialog );

    connect( mTools, SIGNAL( regionChanged() ), this, SLOT( redrawRegion() ) );
  }

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
  QgsGrassEdit *ed = new QgsGrassEdit( qGisInterface, qGisInterface->activeLayer(), false,
                                       qGisInterface->getMainWindow(), Qt::WType_Dialog );

  if ( ed->isValid() )
  {
    ed->show();
    mCanvas->refresh();
    connect( ed, SIGNAL( finished() ), this, SLOT( setEditAction() ) );
  }
  else
  {
    delete ed;
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

  QgsGrassElementDialog dialog;
  name = dialog.getItem( "vector", tr( "New vector name" ),
                         tr( "New vector name" ), "", "", &ok );

  if ( !ok ) return;

  // Create new map
  QgsGrass::setMapset( QgsGrass::getDefaultGisdbase(),
                       QgsGrass::getDefaultLocation(),
                       QgsGrass::getDefaultMapset() );

  QgsGrass::resetError();
  struct Map_info Map;
  Vect_open_new( &Map, ( char * ) name.ascii(), 0 );

  if ( QgsGrass::getError() == QgsGrass::FATAL )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot create new vector: " )
                          + QgsGrass::getErrorMessage() );
    return;
  }

  Vect_build( &Map, stderr );
  Vect_set_release_support( &Map );
  Vect_close( &Map );

  // Open in GRASS vector provider

  QString uri = QgsGrass::getDefaultGisdbase() + "/"
                + QgsGrass::getDefaultLocation() + "/"
                + QgsGrass::getDefaultMapset() + "/"
                + name + "/0_point";

  QgsVectorLayer* layer = new QgsVectorLayer( uri, name, "grass" );

  if ( !layer )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "New vector created "
                          "but cannot be opened by data provider." ) );
    return;
  }

  QgsGrassEdit *ed = new QgsGrassEdit( qGisInterface, layer, true,
                                       qGisInterface->getMainWindow(), Qt::WType_Dialog );

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
  /*
  if ( !(mProvider->startEdit()) ) {
  QMessageBox::warning( 0, tr("Warning"), tr("Cannot open vector for update.") );
  return;
  }
  */
}

void QgsGrassPlugin::postRender( QPainter *painter )
{
// QgsDebugMsg("entered.");
}

void QgsGrassPlugin::displayRegion()
{
// QgsDebugMsg("entered.");

  mRegionBand->reset();

  // Display region of current mapset if in active mode
  if ( !QgsGrass::activeMode() ) return;

  QString gisdbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();
  QString mapset   = QgsGrass::getDefaultMapset();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "GISDBASE, LOCATION_NAME or MAPSET is not set, "
                          "cannot display current region." ) );
    return;
  }

  QgsGrass::setLocation( gisdbase, location );

  struct Cell_head window;
  char *err = G__get_window( &window, ( char * )"", ( char * )"WIND", ( char * ) mapset.latin1() );

  if ( err )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot read current region: " ) + QString( err ) );
    return;
  }

  std::vector<QgsPoint> points;
  points.resize( 5 );

  points[0].setX( window.west ); points[0].setY( window.south );
  points[1].setX( window.east ); points[1].setY( window.south );
  points[2].setX( window.east ); points[2].setY( window.north );
  points[3].setX( window.west ); points[3].setY( window.north );
  points[4].setX( window.west ); points[4].setY( window.south );

  for ( int i = 0; i < 5; i++ )
  {
    mRegionBand->addPoint( points[i] );
  }
}

void QgsGrassPlugin::switchRegion( bool on )
{
// QgsDebugMsg("entered.");

  QSettings settings;
  settings.writeEntry( "/GRASS/region/on", on );

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

  if ( mRegionAction->isOn() )
  {
    displayRegion();
  }
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
  mRegion = new QgsGrassRegion( this, qGisInterface, qGisInterface->getMainWindow(), Qt::Window );

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
  settings.writeEntry( "/GRASS/region/color", mRegionPen.color().name() );
  settings.writeEntry( "/GRASS/region/width", ( int ) mRegionPen.width() );
}

void QgsGrassPlugin::openMapset()
{
// QgsDebugMsg("entered.");

  QString element;

  QgsGrassSelect *sel = new QgsGrassSelect( QgsGrassSelect::MAPSET );

  if ( !sel->exec() ) return;

  QString err = QgsGrass::openMapset( sel->gisdbase,
                                      sel->location, sel->mapset );

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open the mapset. " ) + err );
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
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot close mapset. " ) + err );
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
                                        this, qGisInterface->getMainWindow() );
  }
  mNewMapset->show();
  mNewMapset->raise();
}

void QgsGrassPlugin::projectRead()
{
// QgsDebugMsg("entered.");

  bool ok;
  QString gisdbase = QgsProject::instance()->readEntry(
                       "GRASS", "/WorkingGisdbase", "", &ok ).trimmed();
  QString location = QgsProject::instance()->readEntry(
                       "GRASS", "/WorkingLocation", "", &ok ).trimmed();
  QString mapset = QgsProject::instance()->readEntry(
                     "GRASS", "/WorkingMapset", "", &ok ).trimmed();

  if ( gisdbase.length() == 0 || location.length() == 0 ||
       mapset.length() == 0 )
  {
    // Mapset not specified
    return;
  }

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
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Cannot close current mapset. " ) + err );
    return;
  }
  mapsetChanged();

  err = QgsGrass::openMapset( gisdbase, location, mapset );

  if ( !err.isNull() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open GRASS mapset. " ) + err );
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
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mAddVectorAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mAddRasterAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mOpenToolsAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mRegionAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mEditRegionAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mEditAction );
  qGisInterface->removePluginMenu( tr( "&GRASS" ), mNewVectorAction );

  delete mOpenMapsetAction;
  delete mNewMapsetAction;
  delete mCloseMapsetAction;
  delete mAddVectorAction;
  delete mAddRasterAction;
  delete mOpenToolsAction;
  delete mRegionAction;
  delete mEditRegionAction;
  delete mEditAction;
  delete mNewVectorAction;

  if ( toolBarPointer )
    delete toolBarPointer;

  // disconnect slots of QgsGrassPlugin so they're not fired also after unload
  disconnect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );
  disconnect( qGisInterface, SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
              this, SLOT( setEditAction() ) );

  QWidget* qgis = qGisInterface->getMainWindow();
  disconnect( qgis, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  disconnect( qgis, SIGNAL( newProject() ), this, SLOT( newProject() ) );
}
// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins dependent on qgsapplication
// and because it needs grass specific path into
// the GRASS plugin resource bundle [TS]
QIcon QgsGrassPlugin::getThemeIcon( const QString theName )
{
  QString myPath = ":/" + QgsApplication::themeName() + "/grass/" + theName;
  QString myDefaultPath = ":/default/grass/" + theName;
  if ( QFile::exists( myPath ) )
  {
    return QIcon( myPath );
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( myDefaultPath );
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
  return QObject::tr( "GRASS" );
}

// Return the description
QGISEXTERN QString description()
{
  return QObject::tr( "GRASS layer" );
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

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
