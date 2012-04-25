/***************************************************************************
  roadgraphplugin.cpp - implemention of plugin
  --------------------------------------
  Date                 : 2010-10-10
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/


// QGIS Specific includes
#include <qgsapplication.h>
#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsmaptoolemitpoint.h>
#include <qgsmaprenderer.h>

#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>

#include <qgslinevectorlayerdirector.h>
#include <qgsgraphbuilder.h>
#include <qgsgraph.h>
#include <qgsdistancearcproperter.h>

// Road grap plugin includes
#include "roadgraphplugin.h"
#include "shortestpathwidget.h"
#include "settingsdlg.h"
#include "speedproperter.h"
#include "units.h"

#include "linevectorlayersettings.h"

//
// Qt4 Related Includes
//

#include <QAction>
#include <QLabel>
#include <QLocale>
#include <QToolBar>
#include <QPushButton>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QDebug>

// standard includes

static const QString sName = QObject::tr( "Road graph plugin" );
static const QString sDescription = QObject::tr( "It solves the shortest path problem." );
static const QString sCategory = QObject::tr( "Vector" );
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
 * @param theQgisInterface - Pointer to the QGIS interface object
 */
RoadGraphPlugin::RoadGraphPlugin( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{

  mQShortestPathDock = NULL;
  mSettings = new RgLineVectorLayerSettings();
  mTimeUnitName = "h";
  mDistanceUnitName = "km";
  mTopologyToleranceFactor = 0.0;
}

RoadGraphPlugin::~RoadGraphPlugin()
{

}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is
 * added to the plugin registry in the QGIS application.
 */
void RoadGraphPlugin::initGui()
{
  // create shorttest path dock
  mQShortestPathDock = new RgShortestPathWidget( mQGisIface->mainWindow() , this );
  mQGisIface->addDockWidget( Qt::LeftDockWidgetArea, mQShortestPathDock );

  // Create the action for tool
  mQSettingsAction  = new QAction( QIcon( ":/roadgraph/road.png" ), tr( "Settings" ), this );

  // Set the what's this text
  mQSettingsAction->setWhatsThis( tr( "Road graph plugin settings" ) );

  setGuiElementsToDefault();

  // Connect the action to slots
  connect( mQSettingsAction, SIGNAL( triggered() ), this, SLOT( property() ) );

  mQGisIface->addPluginToVectorMenu( tr( "Road graph" ), mQSettingsAction );

  connect( mQGisIface, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  connect( mQGisIface, SIGNAL( newProjectCreated() ), this, SLOT( newProject() ) );
  connect( mQGisIface, SIGNAL( projectRead() ), mQShortestPathDock, SLOT( clear() ) );
  connect( mQGisIface, SIGNAL( newProjectCreated() ), mQShortestPathDock, SLOT( clear() ) );

  // load settings
  projectRead();
} // RoadGraphPlugin::initGui()

// Unload the plugin by cleaning up the GUI
void RoadGraphPlugin::unload()
{
  // remove the GUI
  mQGisIface->removePluginVectorMenu( tr( "Road graph" ), mQSettingsAction );

  // disconnect
  disconnect( mQGisIface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  disconnect( mQGisIface->mainWindow(), SIGNAL( newProject() ), this, SLOT( newProject() ) );

  delete mQSettingsAction;
  delete mQShortestPathDock;
} // RoadGraphPlugin::unload()

void RoadGraphPlugin::setGuiElementsToDefault()
{

} // RoadGraphPlugin::setGuiElementsToDefault()

//method defined in interface
void RoadGraphPlugin::help()
{
  //implement me!
} // RoadGraphPlugin::help()

void RoadGraphPlugin::onShowDirection()
{
  mQGisIface->mapCanvas()->refresh();
} // RoadGraphPlugin::onShowDirection()


void RoadGraphPlugin::newProject()
{
  setGuiElementsToDefault();
}

void RoadGraphPlugin::property()
{
  RgSettingsDlg dlg( mSettings, mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );

  dlg.setTimeUnitName( mTimeUnitName );
  dlg.setDistanceUnitName( mDistanceUnitName );
  dlg.setTopologyTolerance( mTopologyToleranceFactor );

  if ( !dlg.exec() )
    return;

  mTimeUnitName = dlg.timeUnitName();
  mDistanceUnitName = dlg.distanceUnitName();
  mTopologyToleranceFactor = dlg.topologyTolerance();

  mSettings->write( QgsProject::instance() );
  QgsProject::instance()->writeEntry( "roadgraphplugin", "/pluginTimeUnit", mTimeUnitName );
  QgsProject::instance()->writeEntry( "roadgraphplugin", "/pluginDistanceUnit", mDistanceUnitName );
  QgsProject::instance()->writeEntry( "roadgraphplugin", "/topologyToleranceFactor", mTopologyToleranceFactor );
  setGuiElementsToDefault();
} //RoadGraphPlugin::property()

void RoadGraphPlugin::projectRead()
{
  mSettings->read( QgsProject::instance() );
  mTimeUnitName = QgsProject::instance()->readEntry( "roadgraphplugin", "/pluginTimeUnit", "h" );
  mDistanceUnitName = QgsProject::instance()->readEntry( "roadgraphplugin", "/pluginDistanceUnit", "km" );
  mTopologyToleranceFactor =
    QgsProject::instance()->readDoubleEntry( "roadgraphplugin", "/topologyToleranceFactor", 0.0 );
  setGuiElementsToDefault();
}// RoadGraphplguin::projectRead()

QgisInterface* RoadGraphPlugin::iface()
{
  return mQGisIface;
}

const QgsGraphDirector* RoadGraphPlugin::director() const
{
  QString layerId;
  QgsVectorLayer *layer = NULL;
  QMap< QString, QgsMapLayer* > mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap< QString, QgsMapLayer* >::const_iterator it;
  for ( it = mapLayers.begin(); it != mapLayers.end(); ++it )
  {
    if ( it.value()->name() != mSettings->mLayer )
      continue;
    layer = dynamic_cast< QgsVectorLayer* >( it.value() );
    break;
  }
  if ( layer == NULL )
    return NULL;
  if ( layer->geometryType() == QGis::Line )
  {
    QgsVectorDataProvider *provider = dynamic_cast< QgsVectorDataProvider* >( layer->dataProvider() );
    if ( provider == NULL )
      return NULL;
    SpeedUnit speedUnit = SpeedUnit::byName( mSettings->mSpeedUnitName );

    QgsLineVectorLayerDirector * director =
      new QgsLineVectorLayerDirector( layer,
                                      provider->fieldNameIndex( mSettings->mDirection ),
                                      mSettings->mFirstPointToLastPointDirectionVal,
                                      mSettings->mLastPointToFirstPointDirectionVal,
                                      mSettings->mBothDirectionVal,
                                      mSettings->mDefaultDirection
                                    );
    director->addProperter( new QgsDistanceArcProperter() );
    director->addProperter( new RgSpeedProperter( provider->fieldNameIndex( mSettings->mSpeed ),
                            mSettings->mDefaultSpeed, speedUnit.multipler() ) );
    return director;
  }
  return NULL;
}

QString RoadGraphPlugin::timeUnitName()
{
  return mTimeUnitName;
}

QString RoadGraphPlugin::distanceUnitName()
{
  return mDistanceUnitName;
}

double RoadGraphPlugin::topologyToleranceFactor()
{
  return mTopologyToleranceFactor;
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
  return new RoadGraphPlugin( theQgisInterfacePointer );

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

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
