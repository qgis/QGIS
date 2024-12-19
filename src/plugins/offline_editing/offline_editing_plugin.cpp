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

#include "qgisinterface.h"
#include "qgsguiutils.h"
#include "qgsproject.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"

#include <QAction>

static const QString sName = QObject::tr( "OfflineEditing" );
static const QString sDescription = QObject::tr( "Allow offline editing and synchronizing with database" );
static const QString sCategory = QObject::tr( "Database" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = QStringLiteral( ":/offline_editing/offline_editing_copy.png" );

QgsOfflineEditingPlugin::QgsOfflineEditingPlugin( QgisInterface *qgisInterface )
  : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType )
  , mQGisIface( qgisInterface )
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
  mActionConvertProject = new QAction( QIcon( ":/offline_editing/offline_editing_copy.png" ), tr( "Convert to Offline Project…" ), this );
  mActionConvertProject->setObjectName( QStringLiteral( "mActionConvertProject" ) );
  // Set the what's this text
  mActionConvertProject->setWhatsThis( tr( "Create offline copies of selected layers and save as offline project" ) );
  // Connect the action to the run
  connect( mActionConvertProject, &QAction::triggered, this, &QgsOfflineEditingPlugin::convertProject );
  // Add the icon to the toolbar
  mQGisIface->addDatabaseToolBarIcon( mActionConvertProject );
  mQGisIface->addPluginToDatabaseMenu( tr( "&Offline Editing" ), mActionConvertProject );
  mActionConvertProject->setEnabled( false );

  mActionSynchronize = new QAction( QIcon( ":/offline_editing/offline_editing_sync.png" ), tr( "Synchronize" ), this );
  mActionSynchronize->setObjectName( QStringLiteral( "mActionSynchronize" ) );
  mActionSynchronize->setWhatsThis( tr( "Synchronize offline project with remote layers" ) );
  connect( mActionSynchronize, &QAction::triggered, this, &QgsOfflineEditingPlugin::synchronize );
  mQGisIface->addDatabaseToolBarIcon( mActionSynchronize );
  mQGisIface->addPluginToDatabaseMenu( tr( "&Offline Editing" ), mActionSynchronize );
  mActionSynchronize->setEnabled( false );

  mOfflineEditing = new QgsOfflineEditing();
  mProgressDialog = new QgsOfflineEditingProgressDialog( mQGisIface->mainWindow(), QgsGuiUtils::ModalDialogFlags );

  connect( mOfflineEditing, &QgsOfflineEditing::progressStarted, this, &QgsOfflineEditingPlugin::showProgress );
  connect( mOfflineEditing, &QgsOfflineEditing::layerProgressUpdated, this, &QgsOfflineEditingPlugin::setLayerProgress );
  connect( mOfflineEditing, &QgsOfflineEditing::progressModeSet, this, &QgsOfflineEditingPlugin::setProgressMode );
  connect( mOfflineEditing, &QgsOfflineEditing::progressUpdated, this, &QgsOfflineEditingPlugin::updateProgress );
  connect( mOfflineEditing, &QgsOfflineEditing::progressStopped, this, &QgsOfflineEditingPlugin::hideProgress );
  connect( mOfflineEditing, &QgsOfflineEditing::warning, mQGisIface->messageBar(), &QgsMessageBar::pushWarning );

  connect( mQGisIface, &QgisInterface::projectRead, this, &QgsOfflineEditingPlugin::updateActions );
  connect( mQGisIface, &QgisInterface::newProjectCreated, this, &QgsOfflineEditingPlugin::updateActions );
  connect( QgsProject::instance(), &QgsProject::writeProject, this, &QgsOfflineEditingPlugin::updateActions );
  connect( QgsProject::instance(), &QgsProject::layerWasAdded, this, &QgsOfflineEditingPlugin::updateActions );
  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( const QString & ) >( &QgsProject::layerWillBeRemoved ), this, &QgsOfflineEditingPlugin::updateActions );
  updateActions();
}

void QgsOfflineEditingPlugin::convertProject()
{
  QgsOfflineEditingPluginGui *myPluginGui = new QgsOfflineEditingPluginGui( mQGisIface->mainWindow(), QgsGuiUtils::ModalDialogFlags );
  myPluginGui->show();

  if ( myPluginGui->exec() == 1 )
  {
    // convert current project for offline editing

    const QStringList selectedLayerIds = myPluginGui->selectedLayerIds();
    if ( selectedLayerIds.isEmpty() )
    {
      return;
    }

    mProgressDialog->setTitle( tr( "Converting to Offline Project" ) );
    if ( mOfflineEditing->convertToOfflineProject( myPluginGui->offlineDataPath(), myPluginGui->offlineDbFile(), selectedLayerIds, myPluginGui->onlySelected(), myPluginGui->dbContainerType(), QString() ) )
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
  mProgressDialog->setTitle( tr( "Synchronizing to Remote Layers" ) );
  mOfflineEditing->synchronize();
  updateActions();
}

void QgsOfflineEditingPlugin::unload()
{
  disconnect( mQGisIface, &QgisInterface::projectRead, this, &QgsOfflineEditingPlugin::updateActions );
  disconnect( mQGisIface, &QgisInterface::newProjectCreated, this, &QgsOfflineEditingPlugin::updateActions );
  disconnect( QgsProject::instance(), &QgsProject::writeProject, this, &QgsOfflineEditingPlugin::updateActions );

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
  const bool hasLayers = QgsProject::instance()->count() > 0;
  const bool isOfflineProject = mOfflineEditing->isOfflineProject();
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
  QString format;
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
QGISEXTERN QgisPlugin *classFactory( QgisInterface *qgisInterfacePointer )
{
  return new QgsOfflineEditingPlugin( qgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN const QString *name()
{
  return &sName;
}

// Return the description
QGISEXTERN const QString *description()
{
  return &sDescription;
}

// Return the category
QGISEXTERN const QString *category()
{
  return &sCategory;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return sPluginType;
}

// Return the version number for the plugin
QGISEXTERN const QString *version()
{
  return &sPluginVersion;
}

QGISEXTERN const QString *icon()
{
  return &sPluginIcon;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin *pluginPointer )
{
  delete pluginPointer;
}
