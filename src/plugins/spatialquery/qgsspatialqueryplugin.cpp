/***************************************************************************
                          qgsspatialqueryplugin.cpp
    A plugin that makes spatial queries on vector layers
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: qgsspatialqueryplugin.cpp 15302 2011-03-01 08:00:54Z timlinux $ */


//
// Required qgis includes
//
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"

//
// Required plugin includes
//
#include "qgsspatialqueryplugin.h"
#include "qgsspatialquerydialog.h"

//
// Required Qt includes
//
#include <QAction>
#include <QFile>
#include <QMessageBox>


#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const QString name_ = QObject::tr( "Spatial Query Plugin" );
static const QString description_ = QObject::tr( "A plugin that makes spatial queries on vector layers" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
static const QString icon_ = ":/icons/spatialquery.png";

/**
* Constructor for the plugin. The plugin is passed a pointer to the main app
* and an interface object that provides access to exposed functions in QGIS.
* @param qgis Pointer to the QGIS main window
* @parma mIface Pointer to the QGIS interface object
*/
QgsSpatialQueryPlugin::QgsSpatialQueryPlugin( QgisInterface* iface )
    : QgisPlugin( name_, description_, version_, type_ ),
    mIface( iface )
{
  mDialog = NULL;
}

QgsSpatialQueryPlugin::~QgsSpatialQueryPlugin()
{
  mIface = NULL;
  delete mSpatialQueryAction;
  delete mDialog;
}
/*
* Initialize the GUI interface for the plugin
*/
void QgsSpatialQueryPlugin::initGui()
{
  // Create the action for tool
  mSpatialQueryAction = new QAction( QIcon(), tr( "&Spatial Query" ), this );

  // Connect the action to the spatialQuery slot
  connect( mSpatialQueryAction, SIGNAL( activated() ), this, SLOT( run() ) );

  setCurrentTheme( "" );
  // this is called when the icon theme is changed
  connect( mIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  // Add the icon to the toolbar and to the plugin menu
  mIface->addToolBarIcon( mSpatialQueryAction );
  mIface->addPluginToMenu( tr( "&Spatial Query" ), mSpatialQueryAction );

}

//Unload the plugin by cleaning up the GUI
void QgsSpatialQueryPlugin::unload()
{
  // remove the GUI
  mIface->removeToolBarIcon( mSpatialQueryAction );
  mIface->removePluginMenu( tr( "&Spatial Query" ), mSpatialQueryAction );

  delete mSpatialQueryAction;

}

void QgsSpatialQueryPlugin::run()
{
  if ( !mDialog )
  {
    QString msg;
    if ( ! QgsSpatialQueryDialog::hasPossibleQuery( msg ) )
    {
      QMessageBox::warning( mIface->mainWindow(), tr( "Not possible execute the query" ), msg, QMessageBox::Ok );
      return;
    }
    mDialog = new QgsSpatialQueryDialog( mIface->mainWindow(), mIface );
    mDialog->setModal( false );
    mDialog->show();
  }
  else
  {
    if ( !mDialog->isVisible() )
    {
      delete mDialog;
      mDialog = NULL;
      run();
    }
    else
    {
      mDialog->activateWindow();
    }
  }

}

//! Set icons to the current theme
void QgsSpatialQueryPlugin::setCurrentTheme( QString )
{
  mSpatialQueryAction->setIcon( getThemeIcon( "/spatialquery.png" ) );
}

QIcon QgsSpatialQueryPlugin::getThemeIcon( const QString &theName )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + "/plugins" + theName ) )
  {
    return QIcon( QgsApplication::activeThemePath() + "/plugins" + theName );
  }
  else if ( QFile::exists( QgsApplication::defaultThemePath() + "/plugins" + theName ) )
  {
    return QIcon( QgsApplication::defaultThemePath() + "/plugins" + theName );
  }
  else
  {
    return QIcon( ":/icons" + theName );
  }
}

void QgsSpatialQueryPlugin::MsgDEBUG( QString sMSg )
{
  QMessageBox::warning( 0, tr( "DEBUG" ), sMSg, QMessageBox::Ok );
}


/**
* Required extern functions needed  for every plugin
* These functions can be called prior to creating an instance
* of the plugin class
*/
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin* classFactory( QgisInterface* iface )
{
  return new QgsSpatialQueryPlugin( iface );
}
// Return the name of the plugin

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

// Return the version
QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin* theSpatialQueryPluginPointer )
{
  delete theSpatialQueryPluginPointer;
}
