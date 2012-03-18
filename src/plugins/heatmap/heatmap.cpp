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
#include "qgsdistancearea.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"

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
static const QString sPluginVersion = QObject::tr( "Version 0.2" );
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
  HeatmapGui d( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );

  if ( d.exec() == QDialog::Accepted )
  {
    // everything runs here

    // Get the required data from the dialog
    QgsRectangle myBBox = d.bbox();
    int columns = d.columns();
    int rows = d.rows();
    float cellsize = d.cellSizeX(); // or d.cellSizeY();  both have the same value
    float myDecay = d.decayRatio();

    // Getting the rasterdataset in place
    GDALAllRegister();

    GDALDataset *emptyDataset;
    GDALDriver *myDriver;

    myDriver = GetGDALDriverManager()->GetDriverByName( d.outputFormat().toUtf8() );
    if ( myDriver == NULL )
    {
      QMessageBox::information( 0, tr( "GDAL driver error" ), tr( "Cannot open the driver for the specified format" ) );
      return;
    }

    double geoTransform[6] = { myBBox.xMinimum(), cellsize, 0, myBBox.yMinimum(), 0, cellsize };
    emptyDataset = myDriver->Create( d.outputFilename().toUtf8(), columns, rows, 1, GDT_Float32, NULL );
    emptyDataset->SetGeoTransform( geoTransform );

    GDALRasterBand *poBand;
    poBand = emptyDataset->GetRasterBand( 1 );
    poBand->SetNoDataValue( NO_DATA );

    float* line = ( float * ) CPLMalloc( sizeof( float ) * columns );
    for ( int i = 0; i < columns ; i++ )
      line[i] = NO_DATA;
    // Write the empty raster
    for ( int i = 0; i < rows ; i++ )
    {
      poBand->RasterIO( GF_Write, 0, 0, columns, 1, line, columns, 1, GDT_Float32, 0, 0 );
    }

    CPLFree( line );
    //close the dataset
    GDALClose(( GDALDatasetH ) emptyDataset );

    // open the raster in GA_Update mode
    GDALDataset *heatmapDS;
    heatmapDS = ( GDALDataset * ) GDALOpen( d.outputFilename().toUtf8(), GA_Update );
    if ( !heatmapDS )
    {
      QMessageBox::information( 0, tr( "Raster update error" ), tr( "Could not open the created raster for updating. The heatmap was not generated." ) );
      return;
    }
    poBand = heatmapDS->GetRasterBand( 1 );
    // Start working on the input vector
    QgsVectorLayer* inputLayer = d.inputVectorLayer();
    QgsVectorDataProvider* myVectorProvider = inputLayer->dataProvider();
    if ( !myVectorProvider )
    {
      QMessageBox::information( 0, tr( "Point layer error" ), tr( "Could not identify the vector data provider." ) );
      return;
    }

    QgsAttributeList myAttrList;
    int rField = 0;
    int wField = 0;
    if ( d.variableRadius() )
    {
      rField = d.radiusField();
      myAttrList.append( rField );
      QgsDebugMsg( tr( "Radius Field index received: %1" ).arg( rField ) );
    }
    if ( d.weighted() )
    {
      wField = d.weightField();
      myAttrList.append( wField );
    }
    // This might have attributes or mightnot have attibutes at all
    // based on the variableRadius() and weighted()
    myVectorProvider->select( myAttrList );
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
      if (( myPoint.x() < myBBox.xMinimum() ) || ( myPoint.y() < myBBox.yMinimum() ) )
      {
        continue;
      }
      float radius;
      if ( d.variableRadius() )
      {
        QgsAttributeMap myAttrMap = myFeature.attributeMap();
        radius = myAttrMap.value( rField ).toFloat();
      }
      else
      {
        radius = d.radius();
      }
      //convert the radius to map units if it is in meters
      if ( d.radiusUnit() == HeatmapGui::Meters )
      {
        radius = mapUnitsOf( radius, inputLayer->crs() );
      }
      // convert radius in map units to pixel count
      int myBuffer = radius / cellsize;
      if ( radius - ( cellsize * myBuffer ) > 0.5 )
      {
        ++myBuffer;
      }
      int blockSize = 2 * myBuffer + 1; //Block SIDE would be more appropriate
      // calculate the pixel position
      unsigned int xPosition, yPosition;
      xPosition = (( myPoint.x() - myBBox.xMinimum() ) / cellsize ) - myBuffer;
      yPosition = (( myPoint.y() - myBBox.yMinimum() ) / cellsize ) - myBuffer;

      // get the data
      float *dataBuffer = ( float * ) CPLMalloc( sizeof( float ) * blockSize * blockSize );
      poBand->RasterIO( GF_Read, xPosition, yPosition, blockSize, blockSize,
                        dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 );

      float weight = 1.0;
      if ( d.weighted() )
      {
        QgsAttributeMap myAttrMap = myFeature.attributeMap();
        weight = myAttrMap.value( wField ).toFloat();
      }

      for ( int xp = 0; xp <= myBuffer; xp++ )
      {
        for ( int yp = 0; yp <= myBuffer; yp++ )
        {
          float distance = sqrt( pow( xp, 2.0 ) + pow( yp, 2.0 ) );
          float pixelValue = weight * ( 1 - (( 1 - myDecay ) * distance / myBuffer ) );

          // clearing anamolies along the axes
          if ( xp == 0 && yp == 0 )
          {
            pixelValue /= 4;
          }
          else if ( xp == 0 || yp == 0 )
          {
            pixelValue /= 2;
          }

          if ( distance <= myBuffer )
          {
            int pos[4];
            pos[0] = ( myBuffer + xp ) * blockSize + ( myBuffer + yp );
            pos[1] = ( myBuffer + xp ) * blockSize + ( myBuffer - yp );
            pos[2] = ( myBuffer - xp ) * blockSize + ( myBuffer + yp );
            pos[3] = ( myBuffer - xp ) * blockSize + ( myBuffer - yp );
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

      poBand->RasterIO( GF_Write, xPosition, yPosition, blockSize, blockSize,
                        dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 );
      CPLFree( dataBuffer );
    }
    //Finally close the dataset
    GDALClose(( GDALDatasetH ) heatmapDS );

    // Open the file in QGIS window
    mQGisIface->addRasterLayer( d.outputFilename(), QFileInfo( d.outputFilename() ).baseName() );
  }
}

/*
 *
 * Local functions
 *
 */
float Heatmap::mapUnitsOf( float meters, QgsCoordinateReferenceSystem layerCrs )
{
  // Worker to transform metres input to mapunits
  QgsDistanceArea da;
  da.setSourceCrs( layerCrs.srsid() );
  da.setEllipsoid( layerCrs.ellipsoidAcronym() );
  if ( da.geographic() )
  {
    da.setProjectionsEnabled( true );
  }
  return meters / da.measureLine( QgsPoint( 0.0, 0.0 ), QgsPoint( 0.0, 1.0 ) );
}

// Unload the plugin by cleaning up the GUI
void Heatmap::unload()
{
  // remove the GUI
  mQGisIface->removePluginRasterMenu( tr( "&Heatmap" ), mQActionPointer );
  mQGisIface->removeRasterToolBarIcon( mQActionPointer );
  delete mQActionPointer;
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
