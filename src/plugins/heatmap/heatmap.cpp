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
#include <QFileInfo>
#include <QProgressDialog>

#define NO_DATA -9999


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
  mQActionPointer->setWhatsThis( tr( "Creates a heatmap raster for the input point vector." ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  mQGisIface->addRasterToolBarIcon( mQActionPointer );
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
  mQGisIface->removePluginRasterMenu( tr( "&Heatmap" ), mQActionPointer );
  mQGisIface->removeRasterToolBarIcon( mQActionPointer );
  delete mQActionPointer;
}

// The worker
void Heatmap::createRaster( QgsVectorLayer* theVectorLayer, int theBuffer, float theDecay, QString theOutputFilename, QString theOutputFormat )
{
  // generic variables
  int xSize, ySize;
  double xResolution, yResolution;
  double rasterX, rasterY;

  // Getting the rasterdataset in place
  GDALAllRegister();

  GDALDataset *emptyDataset;
  GDALDriver *myDriver;

  myDriver = GetGDALDriverManager()->GetDriverByName( theOutputFormat.toUtf8() );
  if ( myDriver == NULL )
  {
    QMessageBox::information( 0, tr( "GDAL driver error" ), tr( "Cannot open the driver for the specified format" ) );
    return;
  }

  // bounding box info
  QgsRectangle myBBox = theVectorLayer->extent();
  // fixing  a base width of 500 px/cells
  xSize = 500;
  xResolution = myBBox.width() / xSize;
  yResolution = xResolution;
  ySize = myBBox.height() / yResolution;
  // add extra extend to cover the corner points' heat region
  xSize = xSize + ( theBuffer * 2 ) + 10 ;
  ySize = ySize + ( theBuffer * 2 ) + 10 ;
  // Define the new lat,lon for the buffered raster area
  rasterX = myBBox.xMinimum() - ( theBuffer + 5 ) * xResolution;
  rasterY = myBBox.yMinimum() - ( theBuffer + 5 ) * yResolution;

  double geoTransform[6] = { rasterX, xResolution, 0, rasterY, 0, yResolution };

  emptyDataset = myDriver->Create( theOutputFilename.toUtf8(), xSize, ySize, 1, GDT_Float32, NULL );

  emptyDataset->SetGeoTransform( geoTransform );

  GDALRasterBand *poBand;
  poBand = emptyDataset->GetRasterBand( 1 );
  poBand->SetNoDataValue( NO_DATA );

  float* line = ( float * ) CPLMalloc( sizeof( float ) * xSize );
  for ( int i = 0; i < xSize; i++ )
    line[i] = NO_DATA;
  // Write the empty raster
  for ( int i = 0; i < ySize ; i++ )
  {
    poBand->RasterIO( GF_Write, 0, 0, xSize, 1, line, xSize, 1, GDT_Float32, 0, 0 );
  }

  CPLFree( line );
  //close the dataset
  GDALClose(( GDALDatasetH ) emptyDataset );

  // open the raster in GA_Update mode
  GDALDataset *heatmapDS;
  heatmapDS = ( GDALDataset * ) GDALOpen( theOutputFilename.toUtf8(), GA_Update );
  if ( !heatmapDS )
  {
    QMessageBox::information( 0, tr( "Raster update error" ), tr( "Could not open the created raster for updating. The heatmap was not generated." ) );
    return;
  }
  poBand = heatmapDS->GetRasterBand( 1 );
  // Get the data buffer ready
  int blockSize = 2 * theBuffer + 1; // block SIDE would have been more appropriate
  // Open the vector features
  QgsVectorDataProvider* myVectorProvider = theVectorLayer->dataProvider();
  if ( !myVectorProvider )
  {
    QMessageBox::information( 0, tr( "Point layer error" ), tr( "Could not identify the vector data provider." ) );
    return;
  }
  QgsAttributeList dummyList;
  myVectorProvider->select( dummyList );

  int totalFeatures = myVectorProvider->featureCount();
  int counter = 0;

  QProgressDialog p( "Creating Heatmap ... ", "Abort", 0, totalFeatures );
  p.setWindowModality( Qt::WindowModal );

  QgsFeature myFeature;

  while ( myVectorProvider->nextFeature( myFeature ) )
  {
    counter++;
    p.setValue( counter );
    if ( p.wasCanceled() )
    {
      QMessageBox::information( 0, tr( "Heatmap generation aborted" ), tr( "QGIS will now load the partially-computed raster." ) );
      break;
    }

    QgsGeometry* myPointGeometry;
    myPointGeometry = myFeature.geometry();
    // convert the geometry to point
    QgsPoint myPoint;
    myPoint = myPointGeometry->asPoint();
    // avoiding any empty points or out of extent points
    if (( myPoint.x() < rasterX ) || ( myPoint.y() < rasterY ) )
    {
      continue;
    }
    // calculate the pixel position
    unsigned int xPosition, yPosition;
    xPosition = (( myPoint.x() - rasterX ) / xResolution ) - theBuffer;
    yPosition = (( myPoint.y() - rasterY ) / yResolution ) - theBuffer;

    // get the data
    float *dataBuffer = ( float * ) CPLMalloc( sizeof( float ) * blockSize * blockSize );
    poBand->RasterIO( GF_Read, xPosition, yPosition, blockSize, blockSize, dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 );

    for ( int xp = 0; xp <= theBuffer; xp++ )
    {
      for ( int yp = 0; yp <= theBuffer; yp++ )
      {
        float distance = sqrt( pow( xp, 2.0 ) + pow( yp, 2.0 ) );
        float pixelValue = 1 - (( 1 - theDecay ) * distance / theBuffer );

        // clearing anamolies along the axes
        if ( xp == 0 && yp == 0 )
        {
          pixelValue /= 4;
        }
        else if ( xp == 0 || yp == 0 )
        {
          pixelValue /= 2;
        }

        if ( distance <= theBuffer )
        {
          int pos[4];
          pos[0] = ( theBuffer + xp ) * blockSize + ( theBuffer + yp );
          pos[1] = ( theBuffer + xp ) * blockSize + ( theBuffer - yp );
          pos[2] = ( theBuffer - xp ) * blockSize + ( theBuffer + yp );
          pos[3] = ( theBuffer - xp ) * blockSize + ( theBuffer - yp );
          for ( int p = 0; p < 4; p++ )
          {
            if ( dataBuffer[ pos[p] ] == NO_DATA )
            {
              dataBuffer[ pos[p] ] = 0;
            }
            dataBuffer[ pos[p] ] += pixelValue;
          }
        }
      }
    }

    poBand->RasterIO( GF_Write, xPosition, yPosition, blockSize, blockSize, dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 );
    CPLFree( dataBuffer );
  }

  //Finally close the dataset
  GDALClose(( GDALDatasetH ) heatmapDS );

  // Open the file in QGIS window
  mQGisIface->addRasterLayer( theOutputFilename, QFileInfo( theOutputFilename ).baseName() );
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
