/***************************************************************************
  heatmap.cpp
  Creates a Heatmap raster for the input point vector
  -------------------
         begin                : January 2012
         copyright            : [(C) Arunmozhi]
         email                : [aruntheguy at gmail dot com]

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// GDAL includes
#include "gdal_priv.h"
#include "cpl_string.h"
#include "cpl_conv.h"

// QGIS Specific includes
#include <qgisinterface.h>
#include <qgisgui.h>

#include "heatmap.h"
#include "heatmapgui.h"

#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

// Qt4 Related Includes
#include <QAction>
#include <QToolBar>
#include <QMessageBox>


static const QString sName = QObject::tr( "Heatmap" );
static const QString sDescription = QObject::tr( "Creates a Heatmap raster for the input point vector" );
static const QString sCategory = QObject::tr( "Raster" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/heatmap/heatmap.png";

/**
 * Constructor for the plugin. The plugin is passed a pointer
 * an interface object that provides access to exposed functions in QGIS.
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
Heatmap::Heatmap( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{
}

Heatmap::~Heatmap()
{

}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is
 * added to the plugin registry in the QGIS application.
 */
void Heatmap::initGui()
{

  // Create the action for tool
  mQActionPointer = new QAction( QIcon( ":/heatmap/heatmap.png" ), tr( "Heatmap" ), this );
  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Creats a heatmap raster for the input point vector." ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mQActionPointer );
  mQGisIface->addPluginToRasterMenu( tr( "&Heatmap" ), mQActionPointer );

}
//method defined in interface
void Heatmap::help()
{
  //implement me!
}

// Slot called when the menu item is triggered
// If you created more menu items / toolbar buttons in initiGui, you should
// create a separate handler for each action - this single run() method will
// not be enough
void Heatmap::run()
{
  HeatmapGui *myPluginGui = new HeatmapGui( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );

  // Connect the createRaster signal to createRaster Slot
  connect( myPluginGui, SIGNAL( createRaster( QgsVectorLayer*, int, float, QString, QString ) ),
           this, SLOT( createRaster( QgsVectorLayer*, int, float, QString, QString ) ) );

  myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void Heatmap::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( "&Heatmap", mQActionPointer );
  mQGisIface->removeToolBarIcon( mQActionPointer );
  delete mQActionPointer;
}

// The worker
void Heatmap::createRaster( QgsVectorLayer* theVectorLayer, int theBuffer, float theDecay, QString theOutputFilename, QString theOutputFormat )
{
  // TODO
  // 1. Get ready the raster writer driver
  //    -> Write out a empty raster file
  // 2. read a point from the vector layer
  // 3. create a square grid for the buffer value and compute the grid
  // 4. Read the corresponding grid from the file
  // 5. Merge the old grid and new grid
  // 6. repeast 2 to 5 untill all points are over
  // 7. Close all the datasets and load the raster to the window
  
  // generic variables
  int xSize, ySize;
  double xResolution, yResolution;

  // Getting the rasterdataset in place
  GDALAllRegister();

  GDALDataset *heatmapDataset;
  GDALDriver *poDriver;
  
  poDriver = GetGDALDriverManager()->GetDriverByName( theOutputFormat );
  if( poDriver == NULL )
  {
    QMessageBox::information( 0, tr("Error in Driver!"), tr("Cannot open the driver for the format specified") );
    return;
  }

  // bounding box info
  QgsRectangle myBBox = theVectorLayer->extent();
  xSize = 500;
  xResolution = myBBox.width()/xSize;
  yResolution = xResolution;
  ySize = myBBox.height()/yResolution;

  double geoTransform[6] = { myBBox.xMinimum(), xResolution, 0, myBBox.yMinimum(), 0, yResolution };

  heatmapDataset = poDriver->Create( theOutputFilename, xSize, ySize, 1, GDT_Float32, NULL );

  heatmapDataset->SetGeoTransform( geoTransform );

  GDALRasterBand *poBand;
  poBand = heatmapDataset->GetRasterBand(1);

  //
  //Write the heatmapDataset->RasterIO function here
  //

  //Finally close the dataset
  GDALClose( (GDALDatasetH) heatmapDataset );

  // Openning the vector features
  QgsVectorDataProvider* myVectorProvider = theVectorLayer->dataProvider();
  if( !myVectorProvider )
  {
    QMessageBox::information( 0, tr( "Error in Point Layer!"), tr("Couldnot identify the vector data provider.") );
    return;
  }

}

//////////////////////////////////////////////////////////////////////////
//
//
//  THE FOLLOWING CODE IS AUTOGENERATED BY THE PLUGIN BUILDER SCRIPT
//    YOU WOULD NORMALLY NOT NEED TO MODIFY THIS, AND YOUR PLUGIN
//      MAY NOT WORK PROPERLY IF YOU MODIFY THIS INCORRECTLY
//
//
//////////////////////////////////////////////////////////////////////////


/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new Heatmap( theQgisInterfacePointer );
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

QGISEXTERN QString icon()
{
  return sPluginIcon;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
