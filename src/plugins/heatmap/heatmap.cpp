/***************************************************************************
                            heatmap.cpp
        Generate a heatmap raster for a input point vector
        --------------------------------------------------
         begin                : [29 Dec 2011]
         copyright            : [(C) Arunmozhi and 2012]
         email                : [aruntheguy at gmail dot com]

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//
// QGIS Specific includes
//

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgis.h>

#include "qgsvectorlayer.h"
#include "qgsinterpolator.h"
#include "qgsgridfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometry.h"

#include "heatmap.h"
#include "heatmapgui.h"

//
// Qt4 Related Includes
//

#include <QAction>
#include <QToolBar>
#include <QMessageBox>
#include <QFileInfo>

// C++ Std Libraries
#include <cmath>

static const QString sName = QObject::tr( "Heatmap" );
static const QString sDescription = QObject::tr( "Generate a heatmap raster for a input point vector." );
static const QString sCategory = QObject::tr( "Raster" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/heatmap/heatmap.png";

//////////////////////////////////////////////////////////////////////
//
// THE FOLLOWING METHODS ARE MANDATORY FOR ALL PLUGINS
//
//////////////////////////////////////////////////////////////////////

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
  mQActionPointer->setWhatsThis( tr( "Generate a Heatmap raster for a Point vector layer" ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mQActionPointer );
  mQGisIface->addPluginToMenu( tr( "&Heatmap" ), mQActionPointer );

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
  HeatmapGui *myHeatmapPluginGui = new HeatmapGui( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );
  myHeatmapPluginGui->setAttribute( Qt::WA_DeleteOnClose );

  // Listen for the signal from gui to create the raster
  connect( myHeatmapPluginGui, SIGNAL( createRasterOutput( QgsVectorLayer*, QString ) ), this, SLOT( createRasterOutput( QgsVectorLayer*, QString ) ) );

  myHeatmapPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void Heatmap::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( "&Heatmap", mQActionPointer );
  mQGisIface->removeToolBarIcon( mQActionPointer );
  delete mQActionPointer;
}

// Create Raster and load it into view
void Heatmap::createRasterOutput( QgsVectorLayer* theLayer, QString theFileName )
{
    int myColumns, myRows;
    double myXCellSize, myYCellSize;

    // Check if it is a point layer
    if( theLayer->geometryType() != 0 )
    {
        QMessageBox msgBox;
        msgBox.setText( "The Vector layer selected is not a Point layer" );
        msgBox.exec();
        return;
    }

    QgsVectorDataProvider* myProvider = theLayer->dataProvider();
    if( !myProvider )
    {
        return;
    }

    // Bounding Box
    QgsRectangle myBBox = theLayer->extent();

    // Rows & Columns
    myColumns = 500;
    //Cell Sizes are Equal
    myXCellSize = myBBox.width()/myColumns;
    myYCellSize = myXCellSize;
    //Calculate rows based on cellsize
    myRows = myBBox.height()/myYCellSize;

    // Actual code to do the plotting on raster

    QgsAttributeList dummyList;

    myProvider->select(dummyList);

    QgsFeature myFeature;
    // A 2D container
    QVector< QVector<double> > myRasterGrid( myRows, QVector<double>(myColumns, 0) );
    while( myProvider->nextFeature( myFeature ) )
    {
        QgsGeometry* myPointGeometry;
        myPointGeometry = myFeature.geometry();
        // convert the geometry to point
        QgsPoint myPoint;
        myPoint = myPointGeometry->asPoint();
        if( ( myPoint.x() == 0 ) && ( myPoint.y() == 0 ) )
        {
            continue;
        }
        // calculate the pixel position ( xColumn, yRow ) of the point
        int xPosition, yPosition;
        xPosition = ( myPoint.x() - myBBox.xMinimum() )/ myXCellSize;
        yPosition = ( myBBox.yMaximum() - myPoint.y() )/ myYCellSize;
        // Accomodating the min and max tradeoff min accuracy 
        if( xPosition != 0 )
        {
            xPosition -= 1;
        }
        if( yPosition != 0 )
        {
            yPosition -= 1;
        }

        // TODO Get buffersize (in Pixels) and Decay from the GUI
        int theBufferSize = 10;
        double theDecay = 0.8;

        // store the value at x,y of a 2D data structure (QVector may be!)
        myRasterGrid[yPosition][xPosition] += 1.0;

        createDecayBuffer( myRasterGrid, xPosition, yPosition, theBufferSize, theDecay );
    }

    // Write the grid file
    QFile outputFile( theFileName );

    if( !outputFile.open( QFile::WriteOnly ) )
    {
        return;
    }

    QTextStream outStream( &outputFile );
    outStream.setRealNumberPrecision(8);
    // Header section
    outStream << "NCOLS " << myColumns << endl;
    outStream << "NROWS " << myRows << endl;
    outStream << "XLLCORNER " << myBBox.xMinimum() << endl;
    outStream << "YLLCORNER " <<  myBBox.yMinimum() << endl;
    if ( myXCellSize == myYCellSize ) //standard way
    {
         outStream << "CELLSIZE " << myXCellSize << endl;
    }
    else //this is supported by GDAL but probably not by other products
    {
        outStream << "DX " << myXCellSize << endl;
        outStream << "DY " << myYCellSize << endl;
    }
    outStream << "NODATA_VALUE -9999" << endl;

    for( int i = 0; i < myRows; i+=1 )
    {
        for( int j = 0; j < myColumns; j+=1 )
        {
            if(myRasterGrid[i][j] == 0)
            {
                outStream<<"-9999 ";
            }
            else
            {
                outStream<<myRasterGrid[i][j]<<" ";
            }
        }
        outStream<<endl;
    }

    // Open the file in QGIS window
    mQGisIface->addRasterLayer( theFileName, QFileInfo( theFileName ).baseName() );
}

// Fuction which computes buffers
void Heatmap::createDecayBuffer( QVector< QVector<double> >& theGrid, int xPos, int yPos, int theBuffer, double theDecay )
{
    double distance,pixelValue;
    for( int x = 0; x <= theBuffer; x += 1 )
    {
        for( int y = 0; y <= theBuffer; y += 1 )
        {
            // The radial distance should be the buffer value
            distance = sqrt( pow(x,2) + pow(y,2) );
            if( distance <= theBuffer )
            {
                pixelValue = 1 - ( (1-theDecay) * distance / theBuffer );
                if( ((xPos + x) < theGrid[0].size()) && ((yPos + y) < theGrid.size()) )
                {
                    theGrid[ yPos + y ][xPos + x] += pixelValue;
                }
                if( ((xPos + x) < theGrid[0].size()) && ((yPos - y) > 0 ) )
                {
                    theGrid[ yPos - y ][ xPos + x ] += pixelValue;
                }
                if( ((xPos - x) > 0) && ((yPos + y) < theGrid.size()) )
                {
                    theGrid[ yPos + y ][xPos - x] += pixelValue;
                }
                if( ((xPos - x) > 0) && ((yPos - y) > 0 ) )
                {
                    theGrid[ yPos - y ][ xPos - x ] += pixelValue;
                }
            }
        }
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
