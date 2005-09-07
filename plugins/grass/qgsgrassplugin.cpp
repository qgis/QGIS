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
#include "../../src/qgisapp.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsrasterlayer.h"
#include "../../src/qgisiface.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeatureattribute.h"

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qregexp.h>

//non qt includes
#include <iostream>

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "qgsgrassplugin.h"
#include "../../providers/grass/qgsgrass.h"
#include "../../providers/grass/qgsgrassprovider.h"

//the gui subclass
#include "qgsgrassattributes.h"
#include "qgsgrassselect.h"
#include "qgsgrassedit.h"
#include "qgsgrasstools.h"
#include "qgsgrassregion.h"

// xpm for creating the toolbar icon
#include "add_vector.xpm"
#include "add_raster.xpm"
#include "grass_tools.xpm"
#include "grass_edit.xpm"
#include "grass_region.xpm"
#include "grass_region_edit.xpm"
static const char *pluginVersion = "0.1";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp Pointer to the QGIS main window
 * @param theQgisInterFace Pointer to the QGIS interface object
 */
QgsGrassPlugin::QgsGrassPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
  qgisMainWindowPointer(theQGisApp), qGisInterface(theQgisInterFace)
{
  /** Initialize the plugin and set the required attributes */
  pluginNameQString = "GrassVector";
  pluginVersionQString = "0.1";
  pluginDescriptionQString = "GRASS layer";
}

QgsGrassPlugin::~QgsGrassPlugin()
{

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

  QSettings settings;

  // Require GISBASE to be set. This should point to the location of
  // the GRASS installation. The GRASS libraries use it to know
  // where to look for things.

  // Look first to see if GISBASE env var is already set.
  // This is set when QGIS is run from within GRASS
  // or when set explicitly by the user.
  // This value should always take precedence.
  QString gisBase = getenv("GISBASE");
#ifdef QGISDEBUG
  qDebug( "%s:%d GRASS gisBase from GISBASE env var is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  if ( !isValidGrassBaseDir(gisBase) ) {
    // Look for gisbase in QSettings
    gisBase = settings.readEntry("/qgis/grass/gisbase", "");
#ifdef QGISDEBUG
    qDebug( "%s:%d GRASS gisBase from QSettings is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  }

  if ( !isValidGrassBaseDir(gisBase) ) {
    // Use the location specified --with-grass during configure
    gisBase = GRASS_BASE;
#ifdef QGISDEBUG
    qDebug( "%s:%d GRASS gisBase from configure is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  }

  while ( !isValidGrassBaseDir(gisBase) ) {
    // Keep asking user for GISBASE until we get a valid one
    //QMessageBox::warning( 0, "Warning", "QGIS can't find your GRASS installation,\nGRASS data "
    //    "cannot be used.\nPlease select your GISBASE.\nGISBASE is full path to the\n"
    //    "directory where GRASS is installed." );
    // XXX Need to subclass this and add explantory message above to left side
    gisBase = QFileDialog::getExistingDirectory(
        gisBase, qgisMainWindowPointer,
        "get GISBASE" ,
        "Choose GISBASE ...", TRUE );
    if (gisBase == QString::null)
    {
      // User pressed cancel. No GRASS for you!
      return;
    }
  }

#ifdef QGISDEBUG
  qDebug( "%s:%d Valid GRASS gisBase is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  QString gisBaseEnv = "GISBASE=" + gisBase;
  /* _Correct_ putenv() implementation is not making copy! */ 
  char *gisBaseEnvChar = new char[gisBaseEnv.length()+1];
  strcpy ( gisBaseEnvChar, const_cast<char *>(gisBaseEnv.ascii()) ); 
  putenv( gisBaseEnvChar );
  settings.writeEntry("/qgis/grass/gisbase", gisBase);

  mCanvas = qGisInterface->getMapCanvas();

  // Create the action for tool
  QAction *addVectorAction = new QAction("Add GRASS vector layer", QIconSet(icon_add_vector), 
      "Add GRASS vector layer",0, this, "addVector");
  addVectorAction->setWhatsThis("Adds a GRASS vector layer to the map canvas");
  QAction *addRasterAction = new QAction("Add GRASS raster layer", QIconSet(icon_add_raster), 
      "Add GRASS raster layer",0, this, "addRaster");
  addRasterAction->setWhatsThis("Adds a GRASS raster layer to the map canvas");

  QAction *openToolsAction = new QAction("Open GRASS tools", QIconSet(icon_grass_tools), 
      "Open GRASS tools",0, this, "openTools");
  addRasterAction->setWhatsThis("Open GRASS tools");


  mRegionAction = new QAction("Display Current Grass Region", QIconSet(icon_grass_region), 
      "Display Current Grass Region",0, this, "region", true);
  mRegionAction->setWhatsThis("Displays the current GRASS region as a rectangle on the map canvas");
  QAction *editRegionAction = new QAction("Edit Current Grass Region", QIconSet(icon_grass_region_edit), 
      "Edit Current Grass Region",0, this, "editRegion");
  editRegionAction->setWhatsThis("Edit the current GRASS region");
  QAction *editAction = new QAction("Edit Grass Vector layer", QIconSet(icon_grass_edit), 
      "Edit Grass Vector layer",0, this, "edit");
  editAction->setWhatsThis("Edit the currently selected GRASS vector layer.");
  if ( !QgsGrass::activeMode() )  {
    openToolsAction->setEnabled(false);
    mRegionAction->setEnabled(false);
    editRegionAction->setEnabled(false);
  } else {
    openToolsAction->setEnabled(true);
    mRegionAction->setEnabled(true);
    editRegionAction->setEnabled(true);
    bool on = settings.readBoolEntry ("/qgis/grass/region/on", true );
    mRegionAction->setOn(on);
  }

  // Connect the action 
  connect(addVectorAction, SIGNAL(activated()), this, SLOT(addVector()));
  connect(addRasterAction, SIGNAL(activated()), this, SLOT(addRaster()));
  connect(openToolsAction, SIGNAL(activated()), this, SLOT(openTools()));
  connect(editAction, SIGNAL(activated()), this, SLOT(edit()));
  connect(mRegionAction, SIGNAL(toggled(bool)), this, SLOT(switchRegion(bool)));
  connect(editRegionAction, SIGNAL(activated()), this, SLOT(changeRegion()));

  // Create GRASS plugin menu entry
  QPopupMenu *pluginMenu = qGisInterface->getPluginMenu("&GRASS");

  // Add actions to the menu
  addVectorAction->addTo(pluginMenu);
  addRasterAction->addTo(pluginMenu);
  openToolsAction->addTo(pluginMenu);
  mRegionAction->addTo(pluginMenu);
  editRegionAction->addTo(pluginMenu);
  editAction->addTo(pluginMenu);

  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "GRASS");
  toolBarPointer->setLabel(tr("GRASS"));

  // Add to the toolbar
  addVectorAction->addTo(toolBarPointer);
  addRasterAction->addTo(toolBarPointer);
  openToolsAction->addTo(toolBarPointer);
  mRegionAction->addTo(toolBarPointer);
  editRegionAction->addTo(toolBarPointer);
  editAction->addTo(toolBarPointer);

  // Connect display region
  connect( mCanvas, SIGNAL(renderComplete(QPainter *)), this, SLOT(postRender(QPainter *)));

  // Init Region symbology
  mRegionPen.setColor( QColor ( settings.readEntry ("/qgis/grass/region/color", "#ff0000" ) ) );
  mRegionPen.setWidth( settings.readNumEntry ("/qgis/grass/region/width", 0 ) );

  //openTools(); // debug only
}

/*
 * Check if given directory contains a GRASS installation
 */
bool QgsGrassPlugin::isValidGrassBaseDir(QString const gisBase)
{
  if ( gisBase.isEmpty() )
  {
    return FALSE;
  }

  QFileInfo gbi ( gisBase + "/etc/element_list" );
  if ( gbi.exists() ) {
    return TRUE;
  } else {
    return FALSE;
  }
}

// Slot called when the "Add GRASS vector layer" menu item is activated
void QgsGrassPlugin::addVector()
{
  QString uri;

  QgsGrassSelect *sel = new QgsGrassSelect(QgsGrassSelect::VECTOR );
  if ( sel->exec() ) {
    uri = sel->gisdbase + "/" + sel->location + "/" + sel->mapset + "/" + sel->map + "/" + sel->layer;
  }
#ifdef QGISDEBUG
  std::cerr << "plugin URI: " << uri << std::endl;
#endif
  if ( uri.length() == 0 ) {
    std::cerr << "Nothing was selected" << std::endl;
    return;
  } else {
#ifdef QGISDEBUG
    std::cout << "Add new vector layer" << std::endl;
#endif

    // create vector name: vector layer
    QString name = sel->map;

    QString field; 
    QString type; 

    QRegExp rx ( "(\\d+)_(.+)" );
    if ( rx.search ( sel->layer ) != -1 ) 
    {
      field = rx.cap(1);
      type = rx.cap(2);
    }

    // Set location
    QgsGrass::setLocation ( sel->gisdbase, sel->location );

    /* Open vector */
    QgsGrass::resetError();
    Vect_set_open_level (2);
    struct Map_info map;
    int level = Vect_open_old_head (&map, (char *) sel->map.ascii(),
        (char *) sel->mapset.ascii());

    if ( QgsGrass::getError() != QgsGrass::FATAL )
    {
      if ( level >= 2 ) 
      {
        // Count layers
        int cnt = 0; 
        int ncidx = Vect_cidx_get_num_fields ( &map );

        for ( int i = 0; i < ncidx; i++ ) 
        {
          int field = Vect_cidx_get_field_number ( &map, i);

          if ( Vect_cidx_get_type_count( &map, field, GV_POINT|GV_LINE|GV_AREA) > 0 ||
              (field > 1 && Vect_cidx_get_type_count( &map, field, GV_BOUNDARY) ) )
          {
            cnt++;
          }
        }

        if( cnt > 1 ) 
        {
          name.append ( " " + field );

          // No need to ad type, the type is obvious from the legend 
        }
      }

      Vect_close ( &map );
    } else {
      std::cerr << "Cannot open GRASS vector: " << QgsGrass::getErrorMessage() << std::endl;
    }

    qGisInterface->addVectorLayer( uri, name, "grass");
  }
}

// Slot called when the "Add GRASS raster layer" menu item is activated
void QgsGrassPlugin::addRaster()
{
  QString uri;

  std::cerr << "QgsGrassPlugin::addRaster" << std::endl;

  QgsGrassSelect *sel = new QgsGrassSelect(QgsGrassSelect::RASTER );
  if ( sel->exec() ) {
    QString element;
    if ( sel->selectedType == QgsGrassSelect::RASTER ) {
      element = "cellhd";
    } else { // GROUP
      element = "group";
    }

    uri = sel->gisdbase + "/" + sel->location + "/" + sel->mapset + "/" + element + "/" + sel->map;
  }
#ifdef QGISDEBUG
  std::cerr << "plugin URI: " << uri << std::endl;
#endif
  if ( uri.length() == 0 ) {
    std::cerr << "Nothing was selected" << std::endl;
    return;
  } else {
#ifdef QGISDEBUG
    std::cout << "Add new raster layer" << std::endl;
#endif
    // create raster name
    int pos = uri.findRev('/');
    pos = uri.findRev('/', pos-1);
    QString name = uri.right( uri.length() - pos - 1 );
    name.replace('/', ' ');

    qGisInterface->addRasterLayer( uri );
  }
}

// Open tools
void QgsGrassPlugin::openTools()
{
  if ( !mTools ) 
    mTools = new QgsGrassTools ( qgisMainWindowPointer, qGisInterface, qgisMainWindowPointer, 0, Qt::WType_Dialog );

  mTools->show();
}


// Start vector editing
void QgsGrassPlugin::edit()
{
  if ( QgsGrassEdit::isRunning() ) {
    QMessageBox::warning( 0, "Warning", "GRASS Edit is already running." );
    return;
  }

  QgsGrassEdit *ed = new QgsGrassEdit( qgisMainWindowPointer, qGisInterface, qgisMainWindowPointer, 0, Qt::WType_Dialog );

  if ( ed->isValid() ) {
    ed->show();
    mCanvas->refresh();
  } else {
    delete ed;
  }
}

void QgsGrassPlugin::postRender(QPainter *painter)
{
#ifdef QGISDEBUG
  std::cout << "QgsGrassPlugin::postRender()" << std::endl;
#endif

  if ( QgsGrass::activeMode() && mRegionAction->isEnabled() && mRegionAction->isOn() ) {
    displayRegion(painter);
  }
}

void QgsGrassPlugin::displayRegion(QPainter *painter)
{
#ifdef QGISDEBUG
  std::cout << "QgsGrassPlugin::displayRegion()" << std::endl;
#endif


  // Display region of current mapset if in active mode
  if ( !QgsGrass::activeMode() ) return;

  QString gisdbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();
  QString mapset   = QgsGrass::getDefaultMapset();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() ) {
    QMessageBox::warning( 0, "Warning", "GISDBASE, LOCATION_NAME or MAPSET is not set, "
        "cannot display current region." );
    return;
  }

  QgsGrass::setLocation ( gisdbase, location );

  struct Cell_head window;
  char *err = G__get_window ( &window, "", "WIND", (char *) mapset.latin1() );

  if ( err ) {
    QMessageBox::warning( 0, "Warning", "Cannot read current region: " + QString(err) );
    return;
  }

  std::vector<QgsPoint> points;
  points.resize(5);

  points[0].setX(window.west); points[0].setY(window.south);
  points[1].setX(window.east); points[1].setY(window.south);
  points[2].setX(window.east); points[2].setY(window.north);
  points[3].setX(window.west); points[3].setY(window.north);
  points[4].setX(window.west); points[4].setY(window.south);

  QgsMapToPixel *transform = mCanvas->getCoordinateTransform();
  QPointArray pointArray(5);

  for ( int i = 0; i < 5; i++ ) {
    transform->transform( &(points[i]) );
    pointArray.setPoint( i, 
        static_cast<int>(points[i].x()), 
        static_cast<int>(points[i].y()) );
  }

  painter->setPen ( mRegionPen );
  painter->drawPolyline ( pointArray );
}

void QgsGrassPlugin::switchRegion(bool on)
{
#ifdef QGISDEBUG
  std::cout << "QgsGrassPlugin::switchRegion()" << std::endl;
#endif

  QSettings settings;
  settings.writeEntry ("/qgis/grass/region/on", on );

  QPixmap *pixmap = mCanvas->canvasPixmap();
  QPainter p;
  p.begin(pixmap);

  if ( on ) {
    displayRegion(&p);
  } else {
    // This is not perfect, but user can see reaction and it is fast
    QPen pen = mRegionPen;
    mRegionPen.setColor( QColor(255,255,255) ); // TODO: background color
    displayRegion(&p);
    mRegionPen = pen;
  }

  p.end();
  mCanvas->repaint(false);
}

void QgsGrassPlugin::changeRegion(void)
{
#ifdef QGISDEBUG
  std::cout << "QgsGrassPlugin::changeRegion()" << std::endl;
#endif

  if ( QgsGrassRegion::isRunning() ) {
    QMessageBox::warning( 0, "Warning", "The Region tool is already running." );
    return;
  }

  QgsGrassRegion *reg = new QgsGrassRegion(this, qgisMainWindowPointer, qGisInterface, 
      qgisMainWindowPointer, 0, 
      Qt::WType_Dialog );

  reg->show();
}

QPen & QgsGrassPlugin::regionPen()
{
  return mRegionPen;
}

void QgsGrassPlugin::setRegionPen(QPen & pen)
{
  mRegionPen = pen;

  QSettings settings;
  settings.writeEntry ("/qgis/grass/region/color", mRegionPen.color().name() );
  settings.writeEntry ("/qgis/grass/region/width", (int) mRegionPen.width() );
}

// Unload the plugin by cleaning up the GUI
void QgsGrassPlugin::unload()
{
  // remove the GUI
  for (int i = 0; i < menuId.size(); ++i)
    qGisInterface->removePluginMenuItem("&GRASS", menuId[i]);

  if ( toolBarPointer )
    delete toolBarPointer;
}
/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsGrassPlugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
extern "C" QString name()
{
  return QString("GRASS");
}

// Return the description
extern "C" QString description()
{
  return QString("GRASS layer");
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
  return QgisPlugin::UI;
}

// Return the version number for the plugin
extern "C" QString version()
{
  return pluginVersion;
}

// Delete ourself
extern "C" void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
