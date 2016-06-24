/***************************************************************************
    offline_editing_plugin.cpp

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "offline_editing_plugin.h"
#include "offline_editing_plugin_gui.h"
#include "offline_editing_progress_dialog.h"

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgsmaplayerregistry.h>
#include <qgsproject.h>
#include <qgsmessagebar.h>
#include <qgsmapcanvas.h>

#include <QAction>

static const QString sName = QObject::tr( "OfflineEditing" );
static const QString sDescription = QObject::tr( "Allow offline editing and synchronizing with database" );
static const QString sCategory = QObject::tr( "Database" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/offline_editing/offline_editing_copy.png";

QgsOfflineEditingPlugin::QgsOfflineEditingPlugin( QgisInterface* theQgisInterface )
    : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
    , mQGisIface( theQgisInterface )
    , mActionConvertProject( nullptr )
    , mActionSynchronize( nullptr )
    , mOfflineEditing( nullptr )
    , mProgressDialog( nullptr )
{
}

QgsOfflineEditingPlugin::~QgsOfflineEditingPlugin()
{
  delete mOfflineEditing;
}

void QgsOfflineEditingPlugin::initGui()
{
  delete mActionConvertProject;

  // Create the action for tool
  mActionConvertProject = new QAction( QIcon( ":/offline_editing/offline_editing_copy.png" ), tr( "Convert to offline project" ), this );
  mActionConvertProject->setObjectName( "mActionConvertProject" );
  // Set the what's this text
  mActionConvertProject->setWhatsThis( tr( "Create offline copies of selected layers and save as offline project" ) );
  // Connect the action to the run
  connect( mActionConvertProject, SIGNAL( triggered() ), this, SLOT( convertProject() ) );
  // Add the icon to the toolbar
  mQGisIface->addDatabaseToolBarIcon( mActionConvertProject );
  mQGisIface->addPluginToDatabaseMenu( tr( "&Offline Editing" ), mActionConvertProject );
  mActionConvertProject->setEnabled( false );

  mActionSynchronize = new QAction( QIcon( ":/offline_editing/offline_editing_sync.png" ), tr( "Synchronize" ), this );
  mActionSynchronize->setObjectName( "mActionSynchronize" );
  mActionSynchronize->setWhatsThis( tr( "Synchronize offline project with remote layers" ) );
  connect( mActionSynchronize, SIGNAL( triggered() ), this, SLOT( synchronize() ) );
  mQGisIface->addDatabaseToolBarIcon( mActionSynchronize );
  mQGisIface->addPluginToDatabaseMenu( tr( "&Offline Editing" ), mActionSynchronize );
  mActionSynchronize->setEnabled( false );

  mOfflineEditing = new QgsOfflineEditing();
  mProgressDialog = new QgsOfflineEditingProgressDialog( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );

  connect( mOfflineEditing, SIGNAL( progressStarted() ), this, SLOT( showProgress() ) );
  connect( mOfflineEditing, SIGNAL( layerProgressUpdated( int, int ) ), this, SLOT( setLayerProgress( int, int ) ) );
  connect( mOfflineEditing, SIGNAL( progressModeSet( QgsOfflineEditing::ProgressMode, int ) ), this, SLOT( setProgressMode( QgsOfflineEditing::ProgressMode, int ) ) );
  connect( mOfflineEditing, SIGNAL( progressUpdated( int ) ), this, SLOT( updateProgress( int ) ) );
  connect( mOfflineEditing, SIGNAL( progressStopped() ), this, SLOT( hideProgress() ) );
  connect( mOfflineEditing, SIGNAL( warning( QString, QString ) ), mQGisIface->messageBar(), SLOT( pushWarning( QString, QString ) ) );

  connect( mQGisIface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( updateActions() ) );
  connect( mQGisIface->mainWindow(), SIGNAL( newProject() ), this, SLOT( updateActions() ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ), this, SLOT( updateActions() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( updateActions() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( updateActions() ) );
  updateActions();
}

void QgsOfflineEditingPlugin::convertProject()
{
  QgsOfflineEditingPluginGui* myPluginGui = new QgsOfflineEditingPluginGui( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );
  myPluginGui->show();

  if ( myPluginGui->exec() == 1 )
  {
    // convert current project for offline editing

    QStringList selectedLayerIds = myPluginGui->selectedLayerIds();
    if ( selectedLayerIds.isEmpty() )
    {
      return;
    }

    mProgressDialog->setTitle( tr( "Converting to offline project" ) );
    if ( mOfflineEditing->convertToOfflineProject( myPluginGui->offlineDataPath(), myPluginGui->offlineDbFile(), selectedLayerIds ) )
    {
      updateActions();
      // Redraw, to make the offline layer visible
      mQGisIface->mapCanvas()->refreshAllLayers();
    }
  }

  delete myPluginGui;
}

void QgsOfflineEditingPlugin::synchronize()
{
  mProgressDialog->setTitle( tr( "Synchronizing to remote layers" ) );
  mOfflineEditing->synchronize();
  updateActions();
}

void QgsOfflineEditingPlugin::unload()
{
  disconnect( mQGisIface->mainWindow(), SIGNAL( projectRead() ), this, SLOT( updateActions() ) );
  disconnect( mQGisIface->mainWindow(), SIGNAL( newProject() ), this, SLOT( updateActions() ) );
  disconnect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ), this, SLOT( updateActions() ) );

  // remove the GUI
  mQGisIface->removePluginDatabaseMenu( tr( "&Offline Editing" ), mActionConvertProject );
  mQGisIface->removeDatabaseToolBarIcon( mActionConvertProject );
  mQGisIface->removePluginDatabaseMenu( tr( "&Offline Editing" ), mActionSynchronize );
  mQGisIface->removeDatabaseToolBarIcon( mActionSynchronize );
  delete mActionConvertProject;
  delete mActionSynchronize;
}

void QgsOfflineEditingPlugin::help()
{
  // TODO: help
}

void QgsOfflineEditingPlugin::updateActions()
{
  bool hasLayers = QgsMapLayerRegistry::instance()->count() > 0;
  bool isOfflineProject = mOfflineEditing->isOfflineProject();
  mActionConvertProject->setEnabled( hasLayers && !isOfflineProject );
  mActionSynchronize->setEnabled( hasLayers && isOfflineProject );
}

void QgsOfflineEditingPlugin::showProgress()
{
  mProgressDialog->show();
}

void QgsOfflineEditingPlugin::setLayerProgress( int layer, int numLayers )
{
  mProgressDialog->setCurrentLayer( layer, numLayers );
}

void QgsOfflineEditingPlugin::setProgressMode( QgsOfflineEditing::ProgressMode mode, int maximum )
{
  QString format = "";
  switch ( mode )
  {
    case QgsOfflineEditing::CopyFeatures:
      format = tr( "%v / %m features copied" );
      break;
    case QgsOfflineEditing::ProcessFeatures:
      format = tr( "%v / %m features processed" );
      break;
    case QgsOfflineEditing::AddFields:
      format = tr( "%v / %m fields added" );
      break;
    case QgsOfflineEditing::AddFeatures:
      format = tr( "%v / %m features added" );
      break;
    case QgsOfflineEditing::RemoveFeatures:
      format = tr( "%v / %m features removed" );
      break;
    case QgsOfflineEditing::UpdateFeatures:
      format = tr( "%v / %m feature updates" );
      break;
    case QgsOfflineEditing::UpdateGeometries:
      format = tr( "%v / %m feature geometry updates" );
      break;

    default:
      break;
  }

  mProgressDialog->setupProgressBar( format, maximum );
}

void QgsOfflineEditingPlugin::updateProgress( int progress )
{
  mProgressDialog->setProgressValue( progress );
}

void QgsOfflineEditingPlugin::hideProgress()
{
  mProgressDialog->hide();
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsOfflineEditingPlugin( theQgisInterfacePointer );
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
