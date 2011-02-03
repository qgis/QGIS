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

// Road grap plugin includes
#include "roadgraphplugin.h"
#include "shortestpathwidget.h"
#include "settingsdlg.h"

#include "linevectorlayerdirector.h"
#include "linevectorlayersettings.h"
#include "simplegraphbuilder.h"
//
// Qt4 Related Includes
//

#include <QAction>
#include <QLabel>
#include <QLocale>
#include <QToolBar>
#include <QPainter>
#include <QPushButton>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QDebug>

// standard includes

static const char * const sIdent = "$Id: roadgraphplugin.cpp 9327 2009-04-20 10:09:44Z YEKST $";
static const QString sName = QObject::tr( "Road graph plugin" );
static const QString sDescription = QObject::tr( "It solves the shortest path problem." );
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
    QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{

  mQShortestPathDock = NULL;
  mSettings = new RgLineVectorLayerSettings();
  mTimeUnitName = "h";
  mDistanceUnitName = "km";
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
  mQSettingsAction  = new QAction( QIcon( ":/roadgraph/road.png" ), tr( "Road graph settings" ), this );
  mQShowDirectionAction  = new QAction( QIcon( ":/roadgraph/showdirect.png" ), tr( "Show road's direction" ), this );
  mInfoAction = new QAction( QIcon( ":/roadgraph/about.png" ), tr( "About" ), this );

  // Set the what's this text
  mQSettingsAction->setWhatsThis( tr( "Road graph plugin settings" ) );
  mQShowDirectionAction->setWhatsThis( tr( "Roads direction viewer" ) );
  mInfoAction->setWhatsThis( tr( "About Road graph plugin" ) );

  mQShowDirectionAction->setCheckable( true );

  setGuiElementsToDefault();

  // Connect the action to slots
  connect( mQSettingsAction, SIGNAL( triggered() ), this, SLOT( property() ) );
  connect( mQShowDirectionAction, SIGNAL( triggered() ), this, SLOT( onShowDirection() ) );
  connect( mInfoAction, SIGNAL( triggered() ), SLOT( about() ) );

  // Add the icons to the toolbar
  mQGisIface->addToolBarIcon( mQShowDirectionAction );

  mQGisIface->addPluginToMenu( tr( "Road graph" ), mQSettingsAction );
  mQGisIface->addPluginToMenu( tr( "Road graph" ), mQShowDirectionAction );
  mQGisIface->addPluginToMenu( tr( "Road graph" ), mInfoAction );

  connect( mQGisIface->mapCanvas(), SIGNAL( renderComplete( QPainter* ) ), this, SLOT( render( QPainter* ) ) );
  connect( mQGisIface, SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  connect( mQGisIface , SIGNAL( newProjectCreated() ), this, SLOT( newProject() ) );
  // load settings
  projectRead();
} // RoadGraphPlugin::initGui()

// Unload the plugin by cleaning up the GUI
void RoadGraphPlugin::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( tr( "Road graph" ), mQSettingsAction );
  mQGisIface->removePluginMenu( tr( "Road graph" ), mQShowDirectionAction );

  mQGisIface->removeToolBarIcon( mQShowDirectionAction );

  // disconnect
  disconnect( mQGisIface->mapCanvas(), SIGNAL( renderComplete( QPainter* ) ), this, SLOT( render( QPainter* ) ) );
  disconnect( mQGisIface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( projectRead() ) );
  disconnect( mQGisIface->mainWindow(), SIGNAL( newProject() ), this, SLOT( newProject() ) );

  delete mQSettingsAction;
  delete mQShowDirectionAction;
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

  if ( !dlg.exec() )
    return;

  mTimeUnitName = dlg.timeUnitName();
  mDistanceUnitName = dlg.distanceUnitName();

  mSettings->write( QgsProject::instance() );
  QgsProject::instance()->writeEntry( "roadgraphplugin", "/pluginTimeUnit", mTimeUnitName );
  QgsProject::instance()->writeEntry( "roadgraphplugin", "/pluginDistanceUnit", mDistanceUnitName );

  setGuiElementsToDefault();
} //RoadGraphPlugin::property()

void RoadGraphPlugin::about()
{
  QDialog dlg( mQGisIface->mainWindow() );
  dlg.setWindowFlags( dlg.windowFlags() | Qt::MSWindowsFixedSizeDialogHint );
  dlg.setWindowFlags( dlg.windowFlags() &~ Qt::WindowContextHelpButtonHint );
  dlg.setWindowTitle( tr( "About RoadGraph" ) );
  QVBoxLayout *lines = new QVBoxLayout( &dlg );
  QLabel *title = new QLabel( "<b>RoadGraph plugin</b>" );
  title->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  QLabel *version = new QLabel( sPluginVersion );
  version->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  lines->addWidget( title );
  lines->addWidget( version );
  lines->addWidget( new QLabel( tr( "Find shortest path on road's graph" ) ) );
  lines->addWidget( new QLabel( tr( "<b>Developers:</b>" ) ) );
  lines->addWidget( new QLabel( "    Sergey Yakushev" ) );
  lines->addWidget( new QLabel( tr( "<b>Homepage:</b>" ) ) );

  QSettings settings;
  QString localeFullName, localeShortName;
  bool overrideLocale = settings.value( "locale/overrideFlag", QVariant( false ) ).toBool();
  if ( !overrideLocale )
  {
    localeFullName = QLocale().system().name();
  }
  else
  {
    localeFullName = settings.value( "locale/userLocale", QVariant( "" ) ).toString();
  }

  localeShortName = localeFullName.left( 2 );
  QLabel *link = new QLabel();
  if ( localeShortName == "ru" || localeShortName == "uk" )
  {
    link->setText( "<a href=\"http://gis-lab.info/qa/road-graph.html\">http://gis-lab.info/qa/road-graph.html</a>" );
  }
  else
  {
    link->setText( "<a href=\"http://gis-lab.info/qa/road-graph-eng.html\">http://gis-lab.info/qa/road-graph-eng.html</a>" );
  }

  link->setOpenExternalLinks( true );
  lines->addWidget( link );

  QPushButton *btnClose = new QPushButton( tr( "Close" ) );
  lines->addWidget( btnClose );
  QObject::connect( btnClose, SIGNAL( clicked() ), &dlg, SLOT( close() ) );

  dlg.exec();
} //RoadGraphPlugin::about()

void RoadGraphPlugin::projectRead()
{
  mSettings->read( QgsProject::instance() );
  mTimeUnitName = QgsProject::instance()->readEntry( "roadgraphplugin", "/pluginTimeUnit", "h" );
  mDistanceUnitName = QgsProject::instance()->readEntry( "roadgraphplugin", "/pluginDistanceUnit", "km" );
  setGuiElementsToDefault();
}// RoadGraphplguin::projectRead()

QgisInterface* RoadGraphPlugin::iface()
{
  return mQGisIface;
}

const RgGraphDirector* RoadGraphPlugin::director() const
{
  QString layerId;
  QgsVectorLayer *layer = NULL;
  QMap< QString, QgsMapLayer* > mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap< QString, QgsMapLayer* >::const_iterator it;
  for ( it = mapLayers.begin(); it != mapLayers.end(); ++it )
  {
    if ( it.value()->name() != mSettings->mLayer )
      continue;
    layerId = it.key();
    layer = dynamic_cast< QgsVectorLayer* >( it.value() );
    break;
  }
  if ( layer == NULL )
    return NULL;

  QgsVectorDataProvider *provider = dynamic_cast< QgsVectorDataProvider* >( layer->dataProvider() );
  if ( provider == NULL )
    return NULL;

  RgLineVectorLayerDirector * director =
    new RgLineVectorLayerDirector( layerId,
                                   provider->fieldNameIndex( mSettings->mDirection ),
                                   mSettings->mFirstPointToLastPointDirectionVal,
                                   mSettings->mLastPointToFirstPointDirectionVal,
                                   mSettings->mBothDirectionVal,
                                   mSettings->mDefaultDirection,
                                   mSettings->mSpeedUnitName,
                                   provider->fieldNameIndex( mSettings->mSpeed ),
                                   mSettings->mDefaultSpeed );

  return director;
}
void RoadGraphPlugin::render( QPainter *painter )
{
  if ( !mQShowDirectionAction->isChecked() )
    return;

  const RgGraphDirector *graphDirector = director();

  if ( graphDirector == NULL )
    return;

  RgSimpleGraphBuilder builder( mQGisIface->mapCanvas()->mapRenderer()->destinationSrs() );
  QVector< QgsPoint > null;
  graphDirector->makeGraph( &builder , null, null );
  AdjacencyMatrix m = builder.adjacencyMatrix();

  AdjacencyMatrix::iterator it1;
  AdjacencyMatrixString::iterator it2;
  for ( it1 = m.begin(); it1 != m.end(); ++it1 )
  {
    for ( it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 )
    {
      QgsPoint p1 =  mQGisIface->mapCanvas()->getCoordinateTransform()->transform( it1->first );
      QgsPoint p2 =  mQGisIface->mapCanvas()->getCoordinateTransform()->transform( it2->first );
      double  x1 = p1.x(),
                   y1 = p1.y(),
                        x2 = p2.x(),
                             y2 = p2.y();

      double length = sqrt( pow( x2 - x1, 2.0 ) + pow( y2 - y1, 2.0 ) );
      double Cos = ( x2 - x1 ) / length;
      double Sin = ( y2 - y1 ) / length;
      double centerX = ( x1 + x2 ) / 2;
      double centerY = ( y1 + y2 ) / 2;
      double r = mArrowSize;

      QPointF pt1( centerX - Sin*r, centerY + Cos*r );
      QPointF pt2( centerX + Sin*r, centerY - Cos*r );

      QVector<QPointF> tmp;
      tmp.resize( 3 );
      tmp[0] = QPointF( centerX + Cos * r * 2, centerY + Sin * r * 2 );
      tmp[1] = pt1;
      tmp[2] = pt2;
      painter->drawPolygon( tmp );
    }
  }
  delete graphDirector;
}// RoadGraphPlugin::render()
QString RoadGraphPlugin::timeUnitName()
{
  return mTimeUnitName;
}

QString RoadGraphPlugin::distanceUnitName()
{
  return mDistanceUnitName;
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
