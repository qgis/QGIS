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

// includes

#include "qgisinterface.h"
#include "qgisgui.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"

#include "plugin.h"

#include <QPainter>
#include <QMenu>
#include <QDate>
#include <QTextDocument>
#include <QMatrix>
#include <QFile>

//non qt includes
#include <cmath>

//the gui subclass
#include "plugingui.h"
#include "qgslogger.h"


static const QString name_ = QObject::tr( "CopyrightLabel" );
static const QString description_ = QObject::tr( "Draws copyright information" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QString icon_ = ":/copyright_label.png";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param _qI Pointer to the QGIS interface object
 */
QgsCopyrightLabelPlugin::QgsCopyrightLabelPlugin( QgisInterface * theQgisInterFace ):
    QgisPlugin( name_, description_, version_, type_ ),
    qGisInterface( theQgisInterFace )
{
  mPlacementLabels << tr( "Bottom Left" ) << tr( "Top Left" )
  << tr( "Top Right" ) << tr( "Bottom Right" );
}

QgsCopyrightLabelPlugin::~QgsCopyrightLabelPlugin()
{}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsCopyrightLabelPlugin::initGui()
{
  // Create the action for tool
  myQActionPointer = new QAction( QIcon(), tr( "&Copyright Label" ), this );
  setCurrentTheme( "" );
  myQActionPointer->setWhatsThis( tr( "Creates a copyright label that is displayed on the map canvas." ) );
  // Connect the action to the run
  connect( myQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // This calls the renderer everytime the cnavas has drawn itself
  connect( qGisInterface->mapCanvas(), SIGNAL( renderComplete( QPainter * ) ), this, SLOT( renderLabel( QPainter * ) ) );
  //this resets this plugin up if a project is loaded
  connect( qGisInterface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );

  // this is called when the icon theme is changed
  connect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon( myQActionPointer );
  qGisInterface->addPluginToMenu( tr( "&Decorations" ), myQActionPointer );
  //initialise default values in the gui
  projectRead();
}

void QgsCopyrightLabelPlugin::projectRead()
{
#ifdef QGISDEBUG
// QgsDebugMsg("+++++++++ Copyright plugin - project read slot called....");
#endif    //default text to start with - try to fetch it from qgsproject

  QDate now;
  QString defString;

  now = QDate::currentDate();
  defString = "&copy; QGIS " + now.toString( "yyyy" );

  // there is no font setting in the UI, so just use the Qt/QGIS default font (what mQFont gets when created)
  //  mQFont.setFamily( QgsProject::instance()->readEntry( "CopyrightLabel", "/FontName", "Sans Serif" ) );
  //  mQFont.setPointSize( QgsProject::instance()->readNumEntry( "CopyrightLabel", "/FontSize", 9 ) );
  mLabelQString = QgsProject::instance()->readEntry( "CopyrightLabel", "/Label", defString );
  mPlacementIndex = QgsProject::instance()->readNumEntry( "CopyrightLabel", "/Placement", 3 );
  mEnable = QgsProject::instance()->readBoolEntry( "CopyrightLabel", "/Enabled", true );
  mLabelQColor.setNamedColor( QgsProject::instance()->readEntry( "CopyrightLabel", "/Color", "#000000" ) ); // default color is black
}
//method defined in interface
void QgsCopyrightLabelPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsCopyrightLabelPlugin::run()
{
  QgsCopyrightLabelPluginGui *myPluginGui = new QgsCopyrightLabelPluginGui( qGisInterface->mainWindow(), QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //listen for when the layer has been made so we can draw it
  //connect(myPluginGui, SIGNAL(drawRasterLayer(QString)), this, SLOT(drawRasterLayer(QString)));
  //connect(myPluginGui, SIGNAL(drawVectorLayer(QString,QString,QString)), this, SLOT(drawVectorLayer(QString,QString,QString)));
  //refresh the canvas when the user presses ok
  connect( myPluginGui, SIGNAL( changeFont( QFont ) ), this, SLOT( setFont( QFont ) ) );
  connect( myPluginGui, SIGNAL( changeLabel( QString ) ), this, SLOT( setLabel( QString ) ) );
  connect( myPluginGui, SIGNAL( changeColor( QColor ) ), this, SLOT( setColor( QColor ) ) );
  connect( myPluginGui, SIGNAL( changePlacement( int ) ), this, SLOT( setPlacement( int ) ) );
  connect( myPluginGui, SIGNAL( enableCopyrightLabel( bool ) ), this, SLOT( setEnable( bool ) ) );
  myPluginGui->setText( mLabelQString );
  myPluginGui->setPlacementLabels( mPlacementLabels );
  myPluginGui->setPlacement( mPlacementIndex );
  myPluginGui->setColor( mLabelQColor );
  myPluginGui->setEnabled( mEnable );
  myPluginGui->show();
}
//! Refresh the map display using the mapcanvas exported via the plugin interface
void QgsCopyrightLabelPlugin::refreshCanvas()
{
  qGisInterface->mapCanvas()->refresh();
}

void QgsCopyrightLabelPlugin::renderLabel( QPainter * theQPainter )
{
  //Large IF statement to enable/disable copyright label
  if ( mEnable )
  {
    // need width/height of paint device
    int myHeight = theQPainter->device()->height();
    int myWidth = theQPainter->device()->width();

    QTextDocument text;
    text.setDefaultFont( mQFont );
    // To set the text color in a QTextDocument we use a CSS style
    QString style = "<style type=\"text/css\"> p {color: " +
                    mLabelQColor.name() + "}</style>";
    text.setHtml( style + "<p>" + mLabelQString + "</p>" );
    QSizeF size = text.size();

    float myXOffset( 0 ), myYOffset( 0 );
    //Determine placement of label from form combo box
    switch ( mPlacementIndex )
    {
      case 0: // Bottom Left
        //Define bottom left hand corner start point
        myYOffset = myHeight - ( size.height() + 5 );
        myXOffset = 5;
        break;
      case 1: // Top left
        //Define top left hand corner start point
        myYOffset = 0;;
        myXOffset = 5;
        break;
      case 2: // Top Right
        //Define top right hand corner start point
        myYOffset = 0;
        myXOffset = myWidth - ( size.width() + 5 );
        break;
      case 3: // Bottom Right
        //Define bottom right hand corner start point
        myYOffset = myHeight - ( size.height() + 5 );
        myXOffset = myWidth - ( size.width() + 5 );
        break;
      default:
        QgsDebugMsg( QString( "Unknown placement index of %1" ).arg( mPlacementIndex ) );
    }

    //Paint label to canvas
    QMatrix worldMatrix = theQPainter->worldMatrix();
    theQPainter->translate( myXOffset, myYOffset );
    text.drawContents( theQPainter );
    // Put things back how they were
    theQPainter->setWorldMatrix( worldMatrix );
  }
}
// Unload the plugin by cleaning up the GUI
void QgsCopyrightLabelPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenu( tr( "&Decorations" ), myQActionPointer );
  qGisInterface->removeToolBarIcon( myQActionPointer );
  // remove the copyright from the canvas
  disconnect( qGisInterface->mapCanvas(), SIGNAL( renderComplete( QPainter * ) ),
              this, SLOT( renderLabel( QPainter * ) ) );
  refreshCanvas();

  delete myQActionPointer;
}


//! change the copyright font
void QgsCopyrightLabelPlugin::setFont( QFont theQFont )
{
  mQFont = theQFont;
  //save state to the project file.....
  QgsProject::instance()->writeEntry( "CopyrightLabel", "/FontName", theQFont.family() );
  //save state to the project file.....
  QgsProject::instance()->writeEntry( "CopyrightLabel", "/FontSize", theQFont.pointSize() );
  refreshCanvas();
}
//! change the copyright text
void QgsCopyrightLabelPlugin::setLabel( QString theLabelQString )
{
  mLabelQString = theLabelQString;
  QgsProject::instance()->writeEntry( "CopyrightLabel", "/Label", mLabelQString );
  refreshCanvas();
}
//! change the copyright text color
void QgsCopyrightLabelPlugin::setColor( QColor theQColor )
{
  mLabelQColor = theQColor;
  QgsProject::instance()->writeEntry( "CopyrightLabel", "/Color", mLabelQColor.name() );
  refreshCanvas();
}

//! set placement of copyright label
void QgsCopyrightLabelPlugin::setPlacement( int placementIndex )
{
  mPlacementIndex = placementIndex;
  QgsProject::instance()->writeEntry( "CopyrightLabel", "/Placement", mPlacementIndex );
  refreshCanvas();
}

//! set whether copyright label is enabled
void QgsCopyrightLabelPlugin::setEnable( bool theBool )
{
  mEnable = theBool;
  QgsProject::instance()->writeEntry( "CopyrightLabel", "/Enabled", mEnable );
  refreshCanvas();
}

//! Set icons to the current theme
void QgsCopyrightLabelPlugin::setCurrentTheme( QString theThemeName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/copyright_label.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/copyright_label.png";
  QString myQrcPath = ":/copyright_label.png";
  if ( QFile::exists( myCurThemePath ) )
  {
    myQActionPointer->setIcon( QIcon( myCurThemePath ) );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    myQActionPointer->setIcon( QIcon( myDefThemePath ) );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    myQActionPointer->setIcon( QIcon( myQrcPath ) );
  }
  else
  {
    myQActionPointer->setIcon( QIcon() );
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
  return new QgsCopyrightLabelPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return name_;
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return type_;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
