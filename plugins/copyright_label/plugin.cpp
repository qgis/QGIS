/***************************************************************************
  plugin.cpp
  Import tool for various worldmap analysis output files
Functions:

-------------------
begin                : Jan 21, 2004
copyright            : (C) 2004 by Tim Sutton
email                : tim@linfiniti.com

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
#include <qpixmap.h>
#include <qpainter.h>
#include <qfont.h>
#include <qrect.h>
#include <qbrush.h>
#include <qbutton.h>
#include <qcheckbox.h>


//non qt includes
#include <iostream>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
//
static const char * const ident_ = "$Id$";

static const char * const name_ = "CopyrightLabel";
static const char * const description_ = "This plugin overlays copyright information on the rendered map.";
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

  pluginMenu->insertItem(QIconSet(icon),"&CopyrightLabel", this, SLOT(run()));

  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Decorations", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("Copyright Label", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // This calls the north arrow renderer everytime the cnavas has drawn itself
  connect(qGisInterface->getMapCanvas(), SIGNAL(renderComplete(QPainter *)), this, SLOT(renderLabel(QPainter *)));

  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Decorations");
  //default text to start with
  toolBarPointer->setLabel("Copyright Label");
  mLabelQString = QString(" QGIS 2004");
  mQFont = QFont("time", 12, QFont::Bold);
  mLabelQColor = QColor(Qt::black);

  //default placement to start with
  mPlacement=tr("Bottom Right");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);
  refreshCanvas();

}
//method defined in interface
void Plugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void Plugin::run()
{
  PluginGui *myPluginGui=new PluginGui(qgisMainWindowPointer,"Copyright Label",true,0);
  //listen for when the layer has been made so we can draw it
  //connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  //connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  //refresh the canvas when the user presses ok
  connect(myPluginGui, SIGNAL(refreshCanvas()), this, SLOT(refreshCanvas()));
  connect(myPluginGui, SIGNAL(changeFont(QFont )), this, SLOT(setFont(QFont )));
  connect(myPluginGui, SIGNAL(changeLabel(QString )), this, SLOT(setLabel(QString )));
  connect(myPluginGui, SIGNAL(changeColor(QColor)), this, SLOT(setColor(QColor)));
  connect(myPluginGui, SIGNAL(changePlacement(QString)), this, SLOT(setPlacement(QString)));
  connect(myPluginGui, SIGNAL(enableCopyrightLabel(bool)), this, SLOT(setEnable(bool)));
  myPluginGui->setText(mLabelQString);
  myPluginGui->setPlacement(mPlacement);
  myPluginGui->show();
}
//! Refresh the map display using the mapcanvas exported via the plugin interface
void Plugin::refreshCanvas()
{
 qGisInterface->getMapCanvas()->refresh();
}

void Plugin::renderLabel(QPainter * theQPainter)
{
  //Large IF statement to enable/disable copyright label
  if (mEnable)
  {
    //@todo softcode this!myQSimpleText.height()
    int myRotationInt = 90;

    //hard coded cludge for getting a colorgroup.  Needs to be replaced
    QButton * myQButton =new QButton();
    QColorGroup myQColorGroup = myQButton->colorGroup();

    QSimpleRichText myQSimpleText(mLabelQString, mQFont);
    myQSimpleText.setWidth( theQPainter, (theQPainter->viewport().width()-10) );

    //Get canvas dimensions
    int myYOffset = theQPainter->viewport().height();
    int myXOffset = theQPainter->viewport().width();


    //Determine placement of label from form combo box
    if (mPlacement==tr("Bottom Left"))
    {
      //Define bottom left hand corner start point
      myYOffset = myYOffset - (myQSimpleText.height()+5);
      myXOffset = 5;
    }
    else if (mPlacement==tr("Top Left"))
    {
      //Define top left hand corner start point
      myYOffset = 5;
      myXOffset = 5;
    }
    else if (mPlacement==tr("Top Right"))
    {
      //Define top right hand corner start point
      myYOffset = 5;
      myXOffset = myXOffset - (myQSimpleText.widthUsed()+5);
    }
    else // defaulting to bottom right
    {
      //Define bottom right hand corner start point
      myYOffset = myYOffset - (myQSimpleText.height()+5);
      myXOffset = myXOffset - (myQSimpleText.widthUsed()+5);
    }

    //Paint label to canvas
    QRect myRect(myXOffset,myYOffset,myQSimpleText.widthUsed(),myQSimpleText.height());
    myQSimpleText.draw (theQPainter, myXOffset, myYOffset, myRect, myQColorGroup);

    //myQPainter.setFont(mQFont);
    //myQPainter.setPen(mLabelQColor);
    //myQPainter.drawText(10, myQPixmap->height()-10, mLabelQString);

    //myQPainter.rotate(myRotationInt);
  }
}
// Unload the plugin by cleaning up the GUI
void Plugin::unload()
{
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  delete toolBarPointer;
}


  //! change the copyright font
  void Plugin::setFont(QFont theQFont)
  {
    mQFont = theQFont;
    refreshCanvas();
  }
  //! change the copyright text
  void Plugin::setLabel(QString theLabelQString)
  {
    mLabelQString = theLabelQString;
    refreshCanvas();
  }
  //! change the copyright text colour
  void Plugin::setColor(QColor theQColor)
  {
    mLabelQColor = theQColor;
    refreshCanvas();
  }

  //! set placement of copyright label
  void Plugin::setPlacement(QString theQString)
  {
    mPlacement = theQString;
    refreshCanvas();
  }

  //! set whether copyright label is enabled
  void Plugin::setEnable(bool theBool)
  {
    mEnable = theBool;
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
