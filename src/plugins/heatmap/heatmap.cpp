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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
  HeatmapGui d( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags, &mSessionSettings );

  if ( d.exec() == QDialog::Accepted )
  {
    // everything runs here

    // Get the required data from the dialog
    QgsRectangle myBBox = d.bbox();
    int columns = d.columns();
    int rows = d.rows();
    double cellsize = d.cellSizeX(); // or d.cellSizeY();  both have the same value
    mDecay = d.decayRatio();
    int kernelShape = d.kernelShape();

    // Start working on the input vector
    QgsVectorLayer* inputLayer = d.inputVectorLayer();

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
    // Set the projection on the raster destination to match the input layer
    emptyDataset->SetProjection( inputLayer->crs().toWkt().toLocal8Bit().data() );

    GDALRasterBand *poBand;
    poBand = emptyDataset->GetRasterBand( 1 );
    poBand->SetNoDataValue( NO_DATA );

    float* line = ( float * ) CPLMalloc( sizeof( float ) * columns );
    for ( int i = 0; i < columns ; i++ )
    {
      line[i] = NO_DATA;
    }
    // Write the empty raster
    for ( int i = 0; i < rows ; i++ )
    {
      poBand->RasterIO( GF_Write, 0, i, columns, 1, line, columns, 1, GDT_Float32, 0, 0 );
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

    QgsAttributeList myAttrList;
    int rField = 0;
    int wField = 0;

    // Handle different radius options
    double radius;
    double radiusToMapUnits = 1;
    int myBuffer = 0;
    if ( d.variableRadius() )
    {
      rField = d.radiusField();
      myAttrList.append( rField );
      QgsDebugMsg( QString( "Radius Field index received: %1" ).arg( rField ) );

      // If not using map units, then calculate a conversion factor to convert the radii to map units
      if ( d.radiusUnit() == HeatmapGui::Meters )
      {
        radiusToMapUnits = mapUnitsOf( 1, inputLayer->crs() );
      }
    }
    else
    {
      radius = d.radius(); // radius returned by d.radius() is already in map units
      myBuffer = bufferSize( radius, cellsize );
    }

    if ( d.weighted() )
    {
      wField = d.weightField();
      myAttrList.append( wField );
    }

    // This might have attributes or mightnot have attibutes at all
    // based on the variableRadius() and weighted()
    QgsFeatureIterator fit = inputLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( myAttrList ) );
    int totalFeatures = inputLayer->featureCount();
    int counter = 0;

    QProgressDialog p( tr( "Creating heatmap" ), tr( "Abort" ), 0, totalFeatures, mQGisIface->mainWindow() );
    p.setWindowModality( Qt::ApplicationModal );
    p.show();

    QgsFeature myFeature;

    while ( fit.nextFeature( myFeature ) )
    {
      counter++;
      p.setValue( counter );
      QApplication::processEvents();
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
      if (( myPoint.x() < myBBox.xMinimum() ) || ( myPoint.y() < myBBox.yMinimum() )
          || ( myPoint.x() > myBBox.xMaximum() ) || ( myPoint.y() > myBBox.yMaximum() ) )
      {
        continue;
      }

      // If radius is variable then fetch it and calculate new pixel buffer size
      if ( d.variableRadius() )
      {
        radius = myFeature.attribute( rField ).toDouble() * radiusToMapUnits;
        myBuffer = bufferSize( radius, cellsize );
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

      double weight = 1.0;
      if ( d.weighted() )
      {
        weight = myFeature.attribute( wField ).toDouble();
      }

      for ( int xp = 0; xp <= myBuffer; xp++ )
      {
        for ( int yp = 0; yp <= myBuffer; yp++ )
        {
          double distance = sqrt( pow( xp, 2.0 ) + pow( yp, 2.0 ) );

          // is pixel outside search bandwidth of feature?
          if ( distance > myBuffer )
          {
            continue;
          }

          double pixelValue = weight * calculateKernelValue( distance, myBuffer, kernelShape );

          // clearing anamolies along the axes
          if ( xp == 0 && yp == 0 )
          {
            pixelValue /= 4;
          }
          else if ( xp == 0 || yp == 0 )
          {
            pixelValue /= 2;
          }

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

      poBand->RasterIO( GF_Write, xPosition, yPosition, blockSize, blockSize,
                        dataBuffer, blockSize, blockSize, GDT_Float32, 0, 0 );
      CPLFree( dataBuffer );
    }
    // Finally close the dataset
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

double Heatmap::mapUnitsOf( double meters, QgsCoordinateReferenceSystem layerCrs )
{
  // Worker to transform metres input to mapunits
  QgsDistanceArea da;
  da.setSourceCrs( layerCrs.srsid() );
  da.setEllipsoid( layerCrs.ellipsoidAcronym() );
  if ( da.geographic() )
  {
    da.setEllipsoidalMode( true );
  }
  return meters / da.measureLine( QgsPoint( 0.0, 0.0 ), QgsPoint( 0.0, 1.0 ) );
}

int Heatmap::bufferSize( double radius, double cellsize )
{
  // Calculate the buffer size in pixels

  int buffer = radius / cellsize;
  if ( radius - ( cellsize * buffer ) > 0.5 )
  {
    ++buffer;
  }
  return buffer;
}

double Heatmap::calculateKernelValue( double distance, int bandwidth, int kernelShape )
{
  switch ( kernelShape )
  {
    case Heatmap::Triangular:
      return triangularKernel( distance , bandwidth );

    case Heatmap::Uniform:
      return uniformKernel( distance, bandwidth );

    case Heatmap::Quartic:
      return quarticKernel( distance, bandwidth );

    case Heatmap::Triweight:
      return triweightKernel( distance, bandwidth );

    case Heatmap::Epanechnikov:
      return epanechnikovKernel( distance, bandwidth );
  }
  return 0;

}

/* The kernel functions below are taken from "Kernel Smoothing" by Wand and Jones (1995), p. 175
 *
 * Each kernel is multiplied by a normalizing constant "k", which normalizes the kernel area
 * to 1 for a given bandwidth size.
 *
 * k is calculated by polar double integration of the kernel function
 * between a radius of 0 to the specified bandwidth and equating the area to 1. */

double Heatmap::uniformKernel( double distance, int bandwidth )
{
  Q_UNUSED( distance );
  // Normalizing constant
  double k = 2. / ( M_PI * ( double )bandwidth );

  // Derived from Wand and Jones (1995), p. 175
  return k * ( 0.5 / ( double )bandwidth );
}

double Heatmap::quarticKernel( double distance, int bandwidth )
{
  // Normalizing constant
  double k = 16. / ( 5. * M_PI * pow(( double )bandwidth, 2 ) );

  // Derived from Wand and Jones (1995), p. 175
  return k * ( 15. / 16. ) * pow( 1. - pow( distance / ( double )bandwidth, 2 ), 2 );
}

double Heatmap::triweightKernel( double distance, int bandwidth )
{
  // Normalizing constant
  double k = 128. / ( 35. * M_PI * pow(( double )bandwidth, 2 ) );

  // Derived from Wand and Jones (1995), p. 175
  return k * ( 35. / 32. ) * pow( 1. - pow( distance / ( double )bandwidth, 2 ), 3 );
}

double Heatmap::epanechnikovKernel( double distance, int bandwidth )
{
  // Normalizing constant
  double k = 8. / ( 3. * M_PI * pow(( double )bandwidth, 2 ) );

  // Derived from Wand and Jones (1995), p. 175
  return k * ( 3. / 4. ) * ( 1. - pow( distance / ( double )bandwidth, 2 ) );
}

double Heatmap::triangularKernel( double distance, int bandwidth )
{
  // Normalizing constant. In this case it's calculated a little different
  // due to the inclusion of the non-standard "decay" parameter

  if ( mDecay >= 0 )
  {
    double k = 3. / (( 1. + 2. * mDecay ) * M_PI * pow(( double )bandwidth, 2 ) );

    // Derived from Wand and Jones (1995), p. 175 (with addition of decay parameter)
    return k * ( 1. - ( 1. - mDecay ) * ( distance / ( double )bandwidth ) );
  }
  else
  {
    // Non-standard or mathematically valid negative decay ("coolmap")
    return ( 1. - ( 1. - mDecay ) * ( distance / ( double )bandwidth ) );
  }
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
