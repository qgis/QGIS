/***************************************************************************
  coordinatecapture.cpp
//   Capture mouse coordinates in different CRS
  -------------------
         begin                : [PluginDate]
         copyright            : [(C) Your Name and Date]
         email                : [Your Email]

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: plugin.cpp 8053 2008-01-26 13:59:53Z timlinux $ */

//
// QGIS Specific includes
//

#include <qgisinterface.h>
#include <qgisgui.h>
#include "qgsapplication.h"
#include <qgspoint.h>
#include <qgsmapcanvas.h>
#include <qgsmaprenderer.h>
#include <qgis.h>
#include <qgscoordinatereferencesystem.h>
#include <qgscoordinatetransform.h>
#include <qgsgenericprojectionselector.h>

#include "coordinatecapture.h"
#include "coordinatecapturegui.h"

//
// Qt4 Related Includes
//

#include <QAction>
#include <QToolBar>
#include <QDockWidget>
#include <QLayout>
#include <QLineEdit>
#include <QClipboard>
#include <QPushButton>
#include <QToolButton>
#include <QFile>
#include <QLabel>

static const char * const sIdent = "$Id: plugin.cpp 8053 2008-01-26 13:59:53Z timlinux $";
static const QString sName = QObject::tr( "Coordinate Capture" );
static const QString sDescription = QObject::tr( "Capture mouse coordinates in different CRS" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;

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
CoordinateCapture::CoordinateCapture( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{
}

CoordinateCapture::~CoordinateCapture()
{

}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is
 * added to the plugin registry in the QGIS application.
 */
void CoordinateCapture::initGui()
{
  mCrs.createFromSrsId( GEOCRS_ID ); // initialize the CRS object

  connect( mQGisIface->mapCanvas()->mapRenderer(), SIGNAL( destinationSrsChanged() ), this, SLOT( setSourceCrs() ) );
  connect( mQGisIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  setSourceCrs(); //set up the source CRS
  mTransform.setDestCRS( mCrs ); // set the CRS in the transform
  mUserCrsDisplayPrecision = ( mCrs.mapUnits() == QGis::Degrees ) ? 5 : 3; // precision depends on CRS units

  // Create the action for tool
  mQActionPointer = new QAction( QIcon(), tr( "Coordinate Capture" ), this );
  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Click on the map to view coordinates and capture to clipboard." ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  mQGisIface->addPluginToMenu( tr( "&Coordinate Capture" ), mQActionPointer );

  // create our map tool
  mpMapTool = new CoordinateCaptureMapTool( mQGisIface->mapCanvas() );
  connect( mpMapTool, SIGNAL( mouseMoved( QgsPoint ) ), this, SLOT( mouseMoved( QgsPoint ) ) );
  connect( mpMapTool, SIGNAL( mouseClicked( QgsPoint ) ), this, SLOT( mouseClicked( QgsPoint ) ) );


  // create a little widget with x and y display to put into our dock widget
  QWidget * mypWidget = new QWidget();
  QGridLayout *mypLayout = new QGridLayout( mypWidget );
  mypLayout->setColumnMinimumWidth( 0, 36 );
  mypWidget->setLayout( mypLayout );

  mypUserCrsToolButton = new QToolButton( mypWidget );
  mypUserCrsToolButton->setToolTip( tr( "Click to select the CRS to use for coordinate display" ) );
  connect( mypUserCrsToolButton, SIGNAL( clicked() ), this, SLOT( setCRS() ) );

  mypCRSLabel = new QLabel( mypWidget );
  mypCRSLabel->setGeometry( mypUserCrsToolButton->geometry() );

  mpUserCrsEdit = new QLineEdit( mypWidget );
  mpUserCrsEdit->setReadOnly( true );
  mpUserCrsEdit->setToolTip( tr( "Coordinate in your selected CRS" ) );

  mpCanvasEdit = new QLineEdit( mypWidget );
  mpCanvasEdit->setReadOnly( true );
  mpCanvasEdit->setToolTip( tr( "Coordinate in map canvas coordinate reference system" ) );

  QPushButton * mypCopyButton = new QPushButton( mypWidget );
  mypCopyButton->setText( tr( "Copy to clipboard" ) );
  connect( mypCopyButton, SIGNAL( clicked() ), this, SLOT( copy() ) );

  mpTrackMouseButton = new QToolButton( mypWidget );
  mpTrackMouseButton->setCheckable( true );
  mpTrackMouseButton->setToolTip( tr( "Click to enable mouse tracking. Click the canvas to stop" ) );
  mpTrackMouseButton->setChecked( false );

  // Create the action for tool
  mpCaptureButton = new QPushButton( mypWidget );
  mpCaptureButton->setText( tr( "Start capture" ) );
  mpCaptureButton->setToolTip( tr( "Click to enable coordinate capture" ) );
  mpCaptureButton->setIcon( QIcon( ":/coordinate_capture/coordinate_capture.png" ) );
  mpCaptureButton->setWhatsThis( tr( "Click on the map to view coordinates and capture to clipboard." ) );
  connect( mpCaptureButton, SIGNAL( clicked() ), this, SLOT( run() ) );

  // Set the icons
  setCurrentTheme( "" );

  mypLayout->addWidget( mypUserCrsToolButton, 0, 0 );
  mypLayout->addWidget( mpUserCrsEdit, 0, 1 );
  mypLayout->addWidget( mypCRSLabel, 1, 0 );
  mypLayout->addWidget( mpCanvasEdit, 1, 1 );
  mypLayout->addWidget( mpTrackMouseButton, 2, 0 );
  mypLayout->addWidget( mypCopyButton, 2, 1 );
  mypLayout->addWidget( mpCaptureButton, 3, 1 );


  //create the dock widget
  mpDockWidget = new QDockWidget( tr( "Coordinate Capture" ), mQGisIface->mainWindow() );
  mpDockWidget->setObjectName( "CoordinateCapture" );
  mpDockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mQGisIface->addDockWidget( Qt::LeftDockWidgetArea, mpDockWidget );

  // now add our custom widget to the dock - ownership of the widget is passed to the dock
  mpDockWidget->setWidget( mypWidget );

}

//method defined in interface
void CoordinateCapture::help()
{
  //implement me!
}

void CoordinateCapture::setCRS()
{
  QgsGenericProjectionSelector mySelector( mQGisIface->mainWindow() );
  mySelector.setSelectedCrsId( mCrs.srsid() );
  if ( mySelector.exec() )
  {
    mCrs.createFromSrsId( mySelector.selectedCrsId() );
    mTransform.setDestCRS( mCrs );
    mUserCrsDisplayPrecision = ( mCrs.mapUnits() == QGis::Degrees ) ? 5 : 3; //precision depends on CRS units
  }
}

void CoordinateCapture::setSourceCrs()
{
  mTransform.setSourceCrs( mQGisIface->mapCanvas()->mapRenderer()->destinationSrs() );
  mCanvasDisplayPrecision = ( mQGisIface->mapCanvas()->mapRenderer()->destinationSrs().mapUnits() == QGis::Degrees ) ? 5 : 3; // for the map canvas coordinate display
}

void CoordinateCapture::mouseClicked( QgsPoint thePoint )
{
  //clicking on the canvas will update the widgets and then disable
  //tracking so the user can copy the click point coords
  mpTrackMouseButton->setChecked( false );
  update( thePoint );
}
void CoordinateCapture::mouseMoved( QgsPoint thePoint )
{
  //mouse movements will only update the widgets if the
  //tracking button is checked
  if ( mpTrackMouseButton->isChecked() )
  {
    update( thePoint );
  }
}
void CoordinateCapture::update( QgsPoint thePoint )
{
  //this is the coordinate resolved back to lat / lon
  QgsPoint myUserCrsPoint = mTransform.transform( thePoint );
  mpUserCrsEdit->setText( QString::number( myUserCrsPoint.x(), 'f', mUserCrsDisplayPrecision ) + "," +
                          QString::number( myUserCrsPoint.y(), 'f', mUserCrsDisplayPrecision ) );
  // This is the coordinate space of the map canvas
  mpCanvasEdit->setText( QString::number( thePoint.x(), 'f', mCanvasDisplayPrecision ) + "," +
                         QString::number( thePoint.y(), 'f', mCanvasDisplayPrecision ) );
}
void CoordinateCapture::copy()
{
  QClipboard *myClipboard = QApplication::clipboard();
  //if we are on x11 system put text into selection ready for middle button pasting
  if ( myClipboard->supportsSelection() )
  {
    myClipboard->setText( mpUserCrsEdit->text() + "," + mpCanvasEdit->text(), QClipboard::Selection );
    //QString myMessage = tr("Clipboard contents set to: ");
    //statusBar()->showMessage(myMessage + myClipboard->text(QClipboard::Selection));
  }
  else
  {
    //user has an inferior operating system....
    myClipboard->setText( mpUserCrsEdit->text() + "," + mpCanvasEdit->text(), QClipboard::Clipboard );
    //QString myMessage = tr("Clipboard contents set to: ");
    //statusBar()->showMessage(myMessage + myClipboard->text(QClipboard::Clipboard));
  }
}


// Slot called when the menu item is triggered
// If you created more menu items / toolbar buttons in initiGui, you should
// create a separate handler for each action - this single run() method will
// not be enough
void CoordinateCapture::run()
{
  mQGisIface->mapCanvas()->setMapTool( mpMapTool );
  //CoordinateCaptureGui *myPluginGui=new CoordinateCaptureGui(mQGisIface->mainWindow(), QgisGui::ModalDialogFlags);
  //myPluginGui->setAttribute(Qt::WA_DeleteOnClose);

  //myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void CoordinateCapture::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( "&Coordinate Capture", mQActionPointer );
  //mQGisIface->removeToolBarIcon( mQActionPointer );
  mpMapTool->deactivate();
  delete mpMapTool;
  delete mpDockWidget;
  delete mQActionPointer;
}

// Set icons to the current theme
void CoordinateCapture::setCurrentTheme( QString theThemeName )
{
  mQActionPointer->setIcon( QIcon( getIconPath( "coordinate_capture.png" ) ) );
  mpTrackMouseButton->setIcon( QIcon( getIconPath( "tracking.png" ) ) );
  mpCaptureButton->setIcon( QIcon( getIconPath( "coordinate_capture.png" ) ) );
  mypUserCrsToolButton->setIcon( QIcon( getIconPath( "geographic.png" ) ) );
  mypCRSLabel->setPixmap( QPixmap( getIconPath( "transformed.png" ) ) );
}

// Get path to the best available icon file
QString CoordinateCapture::getIconPath( const QString theName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/coordinate_capture/" + theName;
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/coordinate_capture/" + theName;
  QString myQrcPath = ":/coordinate_capture/" + theName;
  if ( QFile::exists( myCurThemePath ) )
  {
    return myCurThemePath;
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    return myDefThemePath;
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    return myQrcPath;
  }
  else
  {
    return "";
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
  return new CoordinateCapture( theQgisInterfacePointer );
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

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
