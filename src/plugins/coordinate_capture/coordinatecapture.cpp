/***************************************************************************
    coordinatecapture.cpp
    ---------------------
    begin                : August 2008
    copyright            : (C) 2008 by Tim Sutton
    email                : tim at linfiniti dot com
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

#include "qgisinterface.h"
#include "qgsguiutils.h"
#include "qgsapplication.h"
#include "qgspoint.h"
#include "qgsmapcanvas.h"
#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsdockwidget.h"

#include "coordinatecapture.h"
#include "coordinatecapturegui.h"

//
// Qt Related Includes
//

#include <QAction>
#include <QToolBar>
#include <QLayout>
#include <QLineEdit>
#include <QClipboard>
#include <QPushButton>
#include <QToolButton>
#include <QFile>
#include <QLabel>
#include <QMenu>

static const QString sName = QObject::tr( "Coordinate Capture" );
static const QString sDescription = QObject::tr( "Capture mouse coordinates in different CRS" );
static const QString sCategory = QObject::tr( "Vector" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QString sPluginIcon = QStringLiteral( ":/coordinate_capture/coordinate_capture.png" );
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;

//////////////////////////////////////////////////////////////////////
//
// THE FOLLOWING METHODS ARE MANDATORY FOR ALL PLUGINS
//
//////////////////////////////////////////////////////////////////////


CoordinateCapture::CoordinateCapture( QgisInterface *qgisInterface )
  : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
  , mCanvasDisplayPrecision( 5 )
  , mUserCrsDisplayPrecision( 5 )
  , mQGisIface( qgisInterface )
{
}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is
 * added to the plugin registry in the QGIS application.
 */
void CoordinateCapture::initGui()
{
  mCrs.createFromSrsId( GEOCRS_ID ); // initialize the CRS object

  connect( mQGisIface->mapCanvas(), &QgsMapCanvas::destinationCrsChanged, this, &CoordinateCapture::setSourceCrs );
  connect( mQGisIface, &QgisInterface::currentThemeChanged, this, &CoordinateCapture::setCurrentTheme );

  setSourceCrs(); //set up the source CRS
  mTransform.setDestinationCrs( mCrs ); // set the CRS in the transform
  mUserCrsDisplayPrecision = ( mCrs.mapUnits() == QgsUnitTypes::DistanceDegrees ) ? 5 : 3; // precision depends on CRS units

  //create the dock widget
  mpDockWidget = new QgsDockWidget( tr( "Coordinate Capture" ), mQGisIface->mainWindow() );
  mpDockWidget->setObjectName( QStringLiteral( "CoordinateCapture" ) );
  mpDockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mQGisIface->addDockWidget( Qt::LeftDockWidgetArea, mpDockWidget );

  // Create the action for tool
  mQActionPointer = new QAction( QIcon(), tr( "Coordinate Capture" ), this );
  mQActionPointer->setObjectName( QStringLiteral( "mQActionPointer" ) );
  mQActionPointer->setCheckable( true );
  mQActionPointer->setChecked( mpDockWidget->isVisible() );
  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Click on the map to view coordinates and capture to clipboard." ) );
  // Connect the action to the run
  connect( mQActionPointer, &QAction::triggered, this, &CoordinateCapture::showOrHide );
  mQGisIface->addPluginToVectorMenu( QString(), mQActionPointer );
  mQGisIface->addVectorToolBarIcon( mQActionPointer );

  // create our map tool
  mpMapTool = new CoordinateCaptureMapTool( mQGisIface->mapCanvas() );
  connect( mpMapTool, &CoordinateCaptureMapTool::mouseMoved, this, &CoordinateCapture::mouseMoved );
  connect( mpMapTool, &CoordinateCaptureMapTool::mouseClicked, this, &CoordinateCapture::mouseClicked );

  // create a little widget with x and y display to put into our dock widget
  QWidget *mypWidget = new QWidget();
  QGridLayout *mypLayout = new QGridLayout( mypWidget );
  mypLayout->setColumnMinimumWidth( 0, 36 );
  mypWidget->setLayout( mypLayout );

  mypUserCrsToolButton = new QToolButton( mypWidget );
  mypUserCrsToolButton->setToolTip( tr( "Click to select the CRS to use for coordinate display" ) );
  connect( mypUserCrsToolButton, &QAbstractButton::clicked, this, &CoordinateCapture::setCRS );

  mypCRSLabel = new QLabel( mypWidget );
  mypCRSLabel->setGeometry( mypUserCrsToolButton->geometry() );

  mpUserCrsEdit = new QLineEdit( mypWidget );
  mpUserCrsEdit->setReadOnly( true );
  mpUserCrsEdit->setToolTip( tr( "Coordinate in your selected CRS (lat,lon or east,north)" ) );

  mpCanvasEdit = new QLineEdit( mypWidget );
  mpCanvasEdit->setReadOnly( true );
  mpCanvasEdit->setToolTip( tr( "Coordinate in map canvas coordinate reference system (lat,lon or east,north)" ) );

  QPushButton *mypCopyButton = new QPushButton( mypWidget );
  mypCopyButton->setText( tr( "Copy to Clipboard" ) );
  connect( mypCopyButton, &QAbstractButton::clicked, this, &CoordinateCapture::copy );

  mpTrackMouseButton = new QToolButton( mypWidget );
  mpTrackMouseButton->setCheckable( true );
  mpTrackMouseButton->setToolTip( tr( "Click to enable mouse tracking. Click the canvas to stop" ) );
  mpTrackMouseButton->setChecked( false );

  // Create the action for tool
  mpCaptureButton = new QPushButton( mypWidget );
  mpCaptureButton->setText( tr( "Start Capture" ) );
  mpCaptureButton->setToolTip( tr( "Click to enable coordinate capture" ) );
  mpCaptureButton->setIcon( QIcon( ":/coordinate_capture/coordinate_capture.png" ) );
  mpCaptureButton->setWhatsThis( tr( "Click on the map to view coordinates and capture to clipboard." ) );
  connect( mpCaptureButton, &QAbstractButton::clicked, this, &CoordinateCapture::run );

  // Set the icons
  setCurrentTheme( QString() );

  mypLayout->addWidget( mypUserCrsToolButton, 0, 0 );
  mypLayout->addWidget( mpUserCrsEdit, 0, 1 );
  mypLayout->addWidget( mypCRSLabel, 1, 0 );
  mypLayout->addWidget( mpCanvasEdit, 1, 1 );
  mypLayout->addWidget( mpTrackMouseButton, 2, 0 );
  mypLayout->addWidget( mypCopyButton, 2, 1 );
  mypLayout->addWidget( mpCaptureButton, 3, 1 );

  // now add our custom widget to the dock - ownership of the widget is passed to the dock
  mpDockWidget->setWidget( mypWidget );
  connect( mpDockWidget.data(), &QDockWidget::visibilityChanged, mQActionPointer, &QAction::setChecked );
}

//method defined in interface
void CoordinateCapture::help()
{
  //implement me!
}

void CoordinateCapture::setCRS()
{
  QgsProjectionSelectionDialog mySelector( mQGisIface->mainWindow() );
  mySelector.setCrs( mCrs );
  if ( mySelector.exec() )
  {
    mCrs = mySelector.crs();
    mTransform.setDestinationCrs( mCrs );
    mUserCrsDisplayPrecision = ( mCrs.mapUnits() == QgsUnitTypes::DistanceDegrees ) ? 5 : 3; //precision depends on CRS units
  }
}

void CoordinateCapture::setSourceCrs()
{
  mTransform.setSourceCrs( mQGisIface->mapCanvas()->mapSettings().destinationCrs() );
  mCanvasDisplayPrecision = ( mQGisIface->mapCanvas()->mapSettings().destinationCrs().mapUnits() == QgsUnitTypes::DistanceDegrees ) ? 5 : 3; // for the map canvas coordinate display
}

void CoordinateCapture::mouseClicked( const QgsPointXY &point )
{
  //clicking on the canvas will update the widgets and then disable
  //tracking so the user can copy the click point coords
  mpTrackMouseButton->setChecked( false );
  update( point );
}
void CoordinateCapture::mouseMoved( const QgsPointXY &point )
{
  //mouse movements will only update the widgets if the
  //tracking button is checked
  if ( mpTrackMouseButton->isChecked() )
  {
    update( point );
  }
}
void CoordinateCapture::update( const QgsPointXY &point )
{
  //this is the coordinate resolved back to lat / lon
  QgsPointXY myUserCrsPoint = mTransform.transform( point );
  mpUserCrsEdit->setText( QString::number( myUserCrsPoint.x(), 'f', mUserCrsDisplayPrecision ) + ',' +
                          QString::number( myUserCrsPoint.y(), 'f', mUserCrsDisplayPrecision ) );
  // This is the coordinate space of the map canvas
  mpCanvasEdit->setText( QString::number( point.x(), 'f', mCanvasDisplayPrecision ) + ',' +
                         QString::number( point.y(), 'f', mCanvasDisplayPrecision ) );
}
void CoordinateCapture::copy()
{
  QClipboard *myClipboard = QApplication::clipboard();
  //if we are on x11 system put text into selection ready for middle button pasting
  if ( myClipboard->supportsSelection() )
  {
    myClipboard->setText( mpUserCrsEdit->text() + ',' + mpCanvasEdit->text(), QClipboard::Selection );
  }

  myClipboard->setText( mpUserCrsEdit->text() + ',' + mpCanvasEdit->text(), QClipboard::Clipboard );
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

void CoordinateCapture::showOrHide()
{
  if ( !mpDockWidget )
    run();
  else if ( mQActionPointer->isChecked() )
    mpDockWidget->show();
  else
    mpDockWidget->hide();
}

// Unload the plugin by cleaning up the GUI
void CoordinateCapture::unload()
{
  // remove the GUI
  mQGisIface->vectorMenu()->removeAction( mQActionPointer );
  mQGisIface->removeVectorToolBarIcon( mQActionPointer );
  mpMapTool->deactivate();
  delete mpMapTool;
  mpMapTool = nullptr;
  delete mpDockWidget;
  mpDockWidget = nullptr;
  delete mQActionPointer;
  mQActionPointer = nullptr;
}

// Set icons to the current theme
void CoordinateCapture::setCurrentTheme( const QString &themeName )
{
  Q_UNUSED( themeName )
  if ( mQActionPointer )
    mQActionPointer->setIcon( QIcon( getIconPath( "coordinate_capture.png" ) ) );
  if ( mpDockWidget )
  {
    mpTrackMouseButton->setIcon( QIcon( getIconPath( "tracking.svg" ) ) );
    mpCaptureButton->setIcon( QIcon( getIconPath( "coordinate_capture.png" ) ) );
    mypUserCrsToolButton->setIcon( QIcon( getIconPath( "mIconProjectionEnabled.svg" ) ) );
    mypCRSLabel->setPixmap( QPixmap( getIconPath( QStringLiteral( "transformed.svg" ) ) ) );
  }
}

// Get path to the best available icon file
QString CoordinateCapture::getIconPath( const QString &name )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/coordinate_capture/" + name;
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/coordinate_capture/" + name;
  QString myQrcPath = ":/coordinate_capture/" + name;
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
    return QString();
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
QGISEXTERN QgisPlugin *classFactory( QgisInterface *qgisInterfacePointer )
{
  return new CoordinateCapture( qgisInterfacePointer );
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
QGISEXTERN void unload( QgisPlugin *pluginPointer )
{
  delete pluginPointer;
}
