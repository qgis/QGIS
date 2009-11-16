/***************************************************************************
  labeling.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

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
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>

#include "labeling.h"
#include "labelinggui.h"
#include "pallabeling.h"

//
// Qt4 Related Includes
//

#include <QAction>
#include <QMessageBox>
#include <QPainter>
#include <QToolBar>


static const char * const sIdent = "$Id: plugin.cpp 9327 2008-09-14 11:18:44Z jef $";
static const QString sName = QObject::tr( "Labeling" );
static const QString sDescription = QObject::tr( "Smart labeling for vector layers" );
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
Labeling::Labeling( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{
}

Labeling::~Labeling()
{
}

/////////

#include <qgsmaptool.h>
#include <QMouseEvent>
#include <QToolTip>

class LabelingTool : public QgsMapTool
{
  public:
    LabelingTool( PalLabeling* lbl, QgsMapCanvas* canvas ) : QgsMapTool( canvas ), mLBL( lbl ) {}

    virtual void canvasPressEvent( QMouseEvent * e )
    {
      const QList<LabelCandidate>& cand = mLBL->candidates();
      QPointF pt = e->posF();
      for ( int i = 0; i < cand.count(); i++ )
      {
        const LabelCandidate& c = cand[i];
        if ( c.rect.contains( pt ) ) // TODO: handle rotated candidates
        {
          QToolTip::showText( mCanvas->mapToGlobal( e->pos() ), QString::number( c.cost ), mCanvas );
          break;
        }
      }
    }

  protected:
    PalLabeling* mLBL;
};

///////////


/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is
 * added to the plugin registry in the QGIS application.
 */
void Labeling::initGui()
{
  mLBL = new PalLabeling( mQGisIface->mapCanvas()->mapRenderer() );

  // Create the action for tool
  mQActionPointer = new QAction( QIcon( ":/labeling/labeling.png" ), tr( "Labeling" ), this );
  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Replace this with a short description of what the plugin does" ) );
  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mQActionPointer );
  mQGisIface->addPluginToMenu( tr( "&Labeling" ), mQActionPointer );

  /*
  // for testing only
  mActionTool = new QAction( "Ltool", this );
  mQGisIface->addToolBarIcon( mActionTool );
  connect( mActionTool, SIGNAL( triggered() ), this, SLOT( setTool() ) );
  */

  mTool = new LabelingTool( mLBL, mQGisIface->mapCanvas() );

  connect( mQGisIface->mapCanvas(), SIGNAL( renderComplete( QPainter * ) ), this, SLOT( doLabeling( QPainter * ) ) );

  // connect to newly added layers so the labeling hook will be set up
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( layerWasAdded( QgsMapLayer* ) ) );

  // add labeling hooks to all existing layers
  QMap<QString, QgsMapLayer*>& layers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::iterator it = layers.begin(); it != layers.end(); ++it )
  {
    QgsMapLayer* layer = it.value();
    if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
      vlayer->setLabelingEngine( mLBL );
    }
  }
}

void Labeling::doLabeling( QPainter * painter )
{
  mLBL->doLabeling( painter, mQGisIface->mapCanvas()->extent() );
}

// Slot called when the menu item is triggered
// If you created more menu items / toolbar buttons in initiGui, you should
// create a separate handler for each action - this single run() method will
// not be enough
void Labeling::run()
{
  QgsMapLayer* layer = mQGisIface->activeLayer();
  if ( layer == NULL || layer->type() != QgsMapLayer::VectorLayer )
  {
    QMessageBox::warning( mQGisIface->mainWindow(), "Labeling", "Please select a vector layer first." );
    return;
  }
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );

  LabelingGui myPluginGui( mLBL, vlayer, mQGisIface->mainWindow() );

  if ( myPluginGui.exec() )
  {
    // alter labeling - save the changes
    myPluginGui.layerSettings().writeToLayer( vlayer );

    // trigger refresh
    mQGisIface->mapCanvas()->refresh();
  }
}


void Labeling::setTool()
{
  mQGisIface->mapCanvas()->setMapTool( mTool );
}

// Unload the plugin by cleaning up the GUI
void Labeling::unload()
{
  mQGisIface->mapCanvas()->unsetMapTool( mTool );
  delete mTool;

  // remove labeling hook from all layers!
  QMap<QString, QgsMapLayer*>& layers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::iterator it = layers.begin(); it != layers.end(); ++it )
  {
    QgsMapLayer* layer = it.value();
    if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
      vlayer->setLabelingEngine( NULL );
    }
  }

  disconnect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( layerWasAdded( QgsMapLayer* ) ) );
  disconnect( mQGisIface->mapCanvas(), SIGNAL( renderComplete( QPainter * ) ), this, SLOT( doLabeling( QPainter * ) ) );

  // remove the GUI
  mQGisIface->removePluginMenu( "&Labeling", mQActionPointer );
  mQGisIface->removeToolBarIcon( mQActionPointer );
  delete mQActionPointer;

  /*
  // for testing only
  mQGisIface->removeToolBarIcon( mActionTool );
  delete mActionTool;
  */

  delete mLBL;
}

void Labeling::layerWasAdded( QgsMapLayer* layer )
{
  if ( layer->type() != QgsMapLayer::VectorLayer )
    return; // not interested in rasters

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
  // add labeling hook for the newly added layer
  vlayer->setLabelingEngine( mLBL );
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
  return new Labeling( theQgisInterfacePointer );
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
