/***************************************************************************
  plugin.cpp 
  Plugin to draw scale bar on map
Functions:

-------------------
begin                : Jun 1, 2004
copyright            : (C) 2004 by Peter Brewer
email                : sbr00pwb@users.sourceforge.net

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

#include <qgisapp.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include "plugin.h"


#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qpen.h> 
#include <qgspoint.h>
#include <qpointarray.h>
#include <qgscoordinatetransform.h>
#include <qstring.h> 
#include <qfontmetrics.h> 
#include <qfont.h> 
#include <qpaintdevicemetrics.h> 
#include <qspinbox.h> 


//non qt includes
#include <iostream>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
// 
static const char * const ident_ = "$Id$";

static const char * const name_ = "ScaleBar";
static const char * const description_ = "Plugin to draw scale bar on map";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
Plugin::Plugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin(name_,description_,version_,type_)
{
  mPreferredSize = 100;
  mPlacement = "Top Left";
  mStyle = "Tick Down";
}

Plugin::~Plugin()
{

}

/*
 * Initialize the GUI interface for the plugin 
 */
void Plugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

  pluginMenu->insertItem(QIconSet(icon),"&ScaleBar", this, SLOT(run()));

  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Scale Bar", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("Scale Bar", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  //render the scale bar each time the map is rendered
  connect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete(QPainter *)), this, SLOT(renderScaleBar(QPainter *)));
  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Scale Bar");
  toolBarPointer->setLabel("Scale Bar");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);

}


//method defined in interface
void Plugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void Plugin::run()
{
  PluginGui *myPluginGui=new PluginGui(qgisMainWindowPointer,"Scale Bar",true,0);
  myPluginGui->setPreferredSize(mPreferredSize);
  myPluginGui->setPlacement(mPlacement);
  myPluginGui->setEnabled(mEnabled);
  myPluginGui->setStyle(mStyle);
  connect(myPluginGui, SIGNAL(refreshCanvas()), this, SLOT(refreshCanvas()));
  connect(myPluginGui, SIGNAL(changePreferredSize(int)), this, SLOT(setPreferredSize(int)));
  connect(myPluginGui, SIGNAL(changePlacement(QString)), this, SLOT(setPlacement(QString)));
  connect(myPluginGui, SIGNAL(changeEnabled(bool)), this, SLOT(setEnabled(bool)));
  connect(myPluginGui, SIGNAL(changeStyle(QString)), this, SLOT(setStyle(QString)));
  myPluginGui->show();
  
  //set the map units in the spin box
  int myUnits=qGisInterface->getMapCanvas()->mapUnits();
  switch (myUnits)
    {
      case 0: myPluginGui->spnSize->setSuffix(tr(" metres")); break;
      case 1: myPluginGui->spnSize->setSuffix(tr(" feet")); break;
      case 2: myPluginGui->spnSize->setSuffix(tr(" degrees")); break;
      default: std::cout << "Error: not picked up map units - actual value = " << myUnits << std::endl; 
    }; 
}


void Plugin::refreshCanvas()
{
 qGisInterface->getMapCanvas()->refresh();
}



// Actual drawing of Scale Bar
void Plugin::renderScaleBar(QPainter * theQPainter)
{
//Large if statement which determines whether to render the scale bar
if (mEnabled)  
{ 
    // Hard coded sizes
    int myOriginX=20;
    int myOriginY=20;
    int myMajorTickSize=8;
    int myTextOffsetX=3;
    int myTextOffsetY=3;
    int myActualSize=mPreferredSize;
    int myMargin=20;

    //Get canvas dimensions
    QPaintDeviceMetrics myMetrics( theQPainter->device() );
      int myCanvasHeight = myMetrics.height();
      int myCanvasWidth = myMetrics.width();
    
    //Get map units per pixel   
    double myMuppDouble=qGisInterface->getMapCanvas()->mupp();
             
    //Calculate size of scale bar for preferred number of map units
    int myScaleBarWidth = (int) mPreferredSize / myMuppDouble;

    //If scale bar is very small reset to 1/4 of the canvas wide
    if (myScaleBarWidth < 30)
    {
      myScaleBarWidth = (int) myMuppDouble * (myCanvasWidth/4);
      myActualSize = (int) myScaleBarWidth * myMuppDouble;
    };
    
    //if scale bar is more than half the canvas wide keep halving until not
    while (myScaleBarWidth > myCanvasWidth/2)
    {
      myScaleBarWidth = (int) myScaleBarWidth /2;
      myActualSize = (int) myScaleBarWidth * myMuppDouble;
    };    
    
    //Get type of map units and set scale bar text
    int myMapUnits=qGisInterface->getMapCanvas()->mapUnits();
    QString myScaleBarUnitLabel;
    switch (myMapUnits)
    {
      case 0: myScaleBarUnitLabel=tr(" metres"); break;
      case 1: myScaleBarUnitLabel=tr(" feet"); break;
      case 2: myScaleBarUnitLabel=tr(" degrees"); break;
      default: std::cout << "Error: not picked up map units - actual value = " << myMapUnits << std::endl; 
    };  

    //Set font and calculate width of unit label
    QFont myFont( "helvetica", 10 );
    theQPainter->setFont(myFont);
    QFontMetrics fm( myFont );
    int myFontWidth = fm.width( myScaleBarUnitLabel );
    int myFontHeight = fm.height();
      
    //Set the maximum label
    QString myScaleBarMaxLabel=QString::number(myActualSize);
     
    //Calculate total width of scale bar and label
    int myTotalScaleBarWidth = myScaleBarWidth + myFontWidth;

    //determine the origin of scale bar depending on placement selected
    if (mPlacement==tr("Top Left"))
    { 
    	myOriginX=myMargin;
	myOriginY=myMargin;
    }
    else if (mPlacement==tr("Bottom Left"))
    {
        myOriginX=myMargin;
	myOriginY=myCanvasHeight - myMargin; 
    }
    else if (mPlacement==tr("Top Right"))
    {
        myOriginX=myCanvasWidth - myTotalScaleBarWidth -myMargin;
	myOriginY=myMargin;
    }
    else
    //defaulting to bottom right
    {
        myOriginX=myCanvasWidth - myTotalScaleBarWidth -myMargin;
	myOriginY=myCanvasHeight - myMargin;
    }
  
    //Set pen to draw with
    //Perhaps enable colour selection in future?
    QPen pen( black, 2 );             
    theQPainter->setPen( pen ); 
          
    //Create array of vertices for scale bar depending on style    
    if (mStyle==tr("Tick Down"))
    {    
         QPointArray myTickDownArray(4);
         myTickDownArray.putPoints(0,4,
    	   myOriginX                    , (myOriginY + myMajorTickSize) ,  
    	   myOriginX                    ,  myOriginY                    ,
    	   (myScaleBarWidth + myOriginX),  myOriginY                    ,
    	   (myScaleBarWidth + myOriginX), (myOriginY + myMajorTickSize)
    	 ); 
	 theQPainter->drawPolyline(myTickDownArray);    
    }
    else if (mStyle==tr("Tick Up"))
    {
         QPointArray myTickUpArray(4);
	 myTickUpArray.putPoints(0,4,
    	   myOriginX                    ,  myOriginY                    ,  
    	   myOriginX                    ,  myOriginY + myMajorTickSize  ,
    	   (myScaleBarWidth + myOriginX),  myOriginY + myMajorTickSize  ,
    	   (myScaleBarWidth + myOriginX),  myOriginY
    	 ); 
	 theQPainter->drawPolyline(myTickUpArray);
    }	    
    else if (mStyle==tr("Bar"))
    {
         QPointArray myBarArray(2);
	 myBarArray.putPoints(0,2,
    	   myOriginX                    ,  (myOriginY + (myMajorTickSize/2)),  
    	   (myScaleBarWidth + myOriginX),  (myOriginY + (myMajorTickSize/2))
    	 ); 
	 theQPainter->drawPolyline(myBarArray);
    }	     
    else if (mStyle==tr("Box"))
    {
         QPointArray myBoxArray(5);
	 myBoxArray.putPoints(0,5,
    	   myOriginX                    ,  myOriginY,  
    	   (myScaleBarWidth + myOriginX),  myOriginY,
	   (myScaleBarWidth + myOriginX), (myOriginY+myMajorTickSize),
	   myOriginX                    , (myOriginY+myMajorTickSize),
	   myOriginX                    ,  myOriginY
    	 ); 
	 theQPainter->drawPolyline(myBoxArray);
    }	    
    
    //Do actual drawing of scale bar
   
    
    //Do drawing of scale bar text
    //Draw minimum label
    myFontWidth = fm.width( "0" );
    myFontHeight = fm.height();
    theQPainter->drawText(
    (myOriginX-(myFontWidth/2)),(myOriginY-(myFontHeight/4)),
    "0"
    );    
    //Draw maximum label      
    myFontWidth = fm.width( myScaleBarMaxLabel );
    myFontHeight = fm.height();  
    theQPainter->drawText(
    (myOriginX+myScaleBarWidth-(myFontWidth/2)),(myOriginY-(myFontHeight/4)),
    myScaleBarMaxLabel
    );
    //Draw unit label
    myFontWidth = fm.width( myScaleBarUnitLabel );
    myFontHeight = fm.height();
    theQPainter->drawText(
    (myOriginX+myScaleBarWidth+myTextOffsetX),(myOriginY+myMajorTickSize),
    myScaleBarUnitLabel
    );
}
}

//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void Plugin::drawRasterLayer(QString theQString)
{
  qGisInterface->addRasterLayer(theQString);
}
//!draw a vector layer in the qui - intended to respond to signal sent by diolog when it as finished creating a layer
////needs to be given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void Plugin::drawVectorLayer(QString thePathNameQString, QString theBaseNameQString, QString theProviderQString)
{
 qGisInterface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString);
}

// Unload the plugin by cleaning up the GUI
void Plugin::unload()
{
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  delete toolBarPointer;
}







  //! set placement of scale bar
  void Plugin::setPlacement(QString theQString)
  {
    mPlacement = theQString;
    refreshCanvas();
  }

  //! set preferred size of scale bar
  void Plugin::setPreferredSize(int thePreferredSize)
  {
    mPreferredSize = thePreferredSize;
    refreshCanvas();
  }
  //! set scale bar enable
  void Plugin::setEnabled(bool theBool)
  {
    mEnabled = theBool;
    refreshCanvas();
  }
  //! set scale bar enable
  void Plugin::setStyle(QString theStyleQString)
  {
    mStyle = theStyleQString;
    refreshCanvas();
  }  
  
/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new Plugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
extern "C" QString name()
{
    return name_;
}

// Return the description
extern "C" QString description()
{
    return description_;
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return type_;
}

// Return the version number for the plugin
extern "C" QString version()
{
  return version_;
}

// Delete ourself
extern "C" void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
