/***************************************************************************
                          qgspluginmanager.cpp  -  description
                             -------------------
    begin                : Someday 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QLibrary>
#include <QSettings>
#include <QStandardItem>
#include <QPushButton>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QActionGroup>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsconfig.h"
#include "qgsproviderregistry.h"
#include "qgspluginregistry.h"
#include "qgspythonrunner.h"
#include "qgspluginmanager.h"
#include "qgisplugin.h"
#include "qgslogger.h"

// Do we need this?
// #define TESTLIB
#ifdef TESTLIB
// This doesn't work on WIN32 and causes problems with plugins
// on OS X (the code doesn't cause a problem but including dlfcn.h
// renders plugins unloadable)
#if !defined(WIN32) && !defined(Q_OS_MACX)
#include <dlfcn.h>
#endif
#endif


// QSettings group constans
const QString reposGroup = "/Qgis/plugin-repos";
const QString settingsGroup = "/Qgis/plugin-installer";
const QString seenPluginGroup = "/Qgis/plugin-seen";


QgsPluginManager::QgsPluginManager( QWidget * parent, Qt::WFlags fl )
    : QgsOptionsDialogBase( "PluginManager", parent, fl )
{
  setupUi( this );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( true );

  // init models
  mModelPlugins = new QStandardItemModel( 0, 1 );
  mModelProxy = new QgsPluginSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModelPlugins );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  vwPlugins->setModel( mModelProxy );
  vwPlugins->setFocus();

  // context menu
  QAction* actionSortByName = new QAction( tr( "sort by name" ), vwPlugins );
  QAction* actionSortByDownloads = new QAction( tr( "sort by downloads" ), vwPlugins );
  QAction* actionSortByVote = new QAction( tr( "sort by vote" ), vwPlugins );
  QAction* actionSortByStatus = new QAction( tr( "sort by status" ), vwPlugins );
  QAction* actionSortByRepository = new QAction( tr( "sort by repository" ), vwPlugins );
  actionSortByName->setCheckable( true );
  actionSortByDownloads->setCheckable( true );
  actionSortByVote->setCheckable( true );
  actionSortByStatus->setCheckable( true );
  actionSortByRepository->setCheckable( true );
  QActionGroup * group = new QActionGroup( vwPlugins );
  actionSortByName->setActionGroup( group );
  actionSortByDownloads->setActionGroup( group );
  actionSortByVote->setActionGroup( group );
  actionSortByStatus->setActionGroup( group );
  actionSortByRepository->setActionGroup( group );
  actionSortByName->setChecked( true );
  vwPlugins->addAction( actionSortByName );
  vwPlugins->addAction( actionSortByDownloads );
  vwPlugins->addAction( actionSortByVote );
  vwPlugins->addAction( actionSortByStatus );
  vwPlugins->addAction( actionSortByRepository );
  vwPlugins->setContextMenuPolicy( Qt::ActionsContextMenu );
  connect( actionSortByName, SIGNAL( triggered( ) ), mModelProxy, SLOT( sortPluginsByName( ) ) );
  connect( actionSortByDownloads, SIGNAL( triggered( ) ), mModelProxy, SLOT( sortPluginsByDownloads( ) ) );
  connect( actionSortByVote, SIGNAL( triggered( ) ), mModelProxy, SLOT( sortPluginsByVote( ) ) );
  connect( actionSortByStatus, SIGNAL( triggered( ) ), mModelProxy, SLOT( sortPluginsByStatus( ) ) );
  connect( actionSortByRepository, SIGNAL( triggered( ) ), mModelProxy, SLOT( sortPluginsByRepository( ) ) );
  mModelProxy->setSortRole( Qt::DisplayRole );

  // Connect other signals
  connect( mOptionsListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( setCurrentTab( int ) ) );
  connect( vwPlugins->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ), this, SLOT( currentPluginChanged( const QModelIndex & ) ) );
  connect( mModelPlugins, SIGNAL( itemChanged( QStandardItem * ) ), this, SLOT( pluginItemChanged( QStandardItem * ) ) );

  QString wellcomeMsg = tr( "To enable or disable plugin, click its checkbox or doubleclick its name..." );
  tbDetails->setHtml( wellcomeMsg );
  leFilter->setFocus( Qt::MouseFocusReason );
  rbFilterNames->setChecked( true );
  buttonRefreshRepos->setEnabled( false );
  buttonEditRep->setEnabled( false );
  buttonDeleteRep->setEnabled( false );

  // Don't restore the last used tab from QSettings
  mOptionsListWidget->setCurrentRow( 0 );

  // Force setting the status filter (if the active tab was 0, the previous line doesn't take any action)
  setCurrentTab( 0 );

  // Initialize list of allowed checking intervals
  checkingOnStartIntervals << 0 << 1 << 3 << 7 << 14 << 30 ;

  // Initialize the "Setting" tab widgets
  QSettings settings;
  if ( settings.value( settingsGroup + "/checkOnStart", false ).toBool() )
  {
    ckbCheckUpdates->setChecked( true );
  }

  if ( settings.value( settingsGroup + "/allowExperimental", false ).toBool() )
  {
    ckbExperimental->setChecked( true );
  }

  int interval = settings.value( settingsGroup + "/checkOnStartInterval", "" ).toInt( );
  int indx = checkingOnStartIntervals.indexOf( interval ); // if none found, just use -1 index.
  comboInterval->setCurrentIndex( indx );
}



QgsPluginManager::~QgsPluginManager()
{
  delete mModelProxy;
  delete mModelPlugins;
}



void QgsPluginManager::setPythonUtils( QgsPythonUtils* pythonUtils )
{
  mPythonUtils = pythonUtils;
}



void QgsPluginManager::loadPlugin( QString id )
{
  QMap<QString, QString>* plugin = pluginMetadata( id );

  if ( ! plugin )
  {
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString library = plugin->value( "library" );
  if ( plugin->value( "pythonic" ) == "true" )
  {
    library = plugin->value( "id" );
    QgsDebugMsg( "Loading Python plugin: " + library );
    pRegistry->loadPythonPlugin( library );
  }
  else // C++ plugin
  {
    QgsDebugMsg( "Loading C++ plugin: " + library );
    pRegistry->loadCppPlugin( library );
  }

  QgsDebugMsg( "Plugin loaded: " + library );
  QApplication::restoreOverrideCursor();
}



void QgsPluginManager::unloadPlugin( QString id )
{
  QMap<QString, QString>* plugin = pluginMetadata( id );

  if ( ! plugin )
  {
    return;
  }

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString library = plugin->value( "library" );

  if ( plugin->value( "pythonic" ) == "true" )
  {
    library = plugin->value( "id" );
    QgsDebugMsg( "Unloading Python plugin: " + library );
    pRegistry->unloadPythonPlugin( library );
  }
  else // C++ plugin
  {
    QgsDebugMsg( "Unloading C++ plugin: " + library );
    pRegistry->unloadCppPlugin( library );
  }
}



void QgsPluginManager::getCppPluginDescriptions()
{
  QString sharedLibExtension;
#if defined(WIN32) || defined(__CYGWIN__)
  sharedLibExtension = "*.dll";
#else
  sharedLibExtension = "*.so*";
#endif

  // check all libs in the current ans user plugins directories, and get name and descriptions
  // First, the qgis install directory/lib (this info is available from the provider registry so we use it here)
  QgsProviderRegistry *pr = QgsProviderRegistry::instance();
  QStringList myPathList( pr->libraryDirectory().path() );

  QSettings settings;
  QString myPaths = settings.value( "plugins/searchPathsForPlugins", "" ).toString();
  if ( !myPaths.isEmpty() )
  {
    myPathList.append( myPaths.split( "|" ) );
  }

  for ( int j = 0; j < myPathList.size(); ++j )
  {
    QString myPluginDir = myPathList.at( j );
    QDir pluginDir( myPluginDir, sharedLibExtension, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks );

    if ( pluginDir.count() == 0 )
    {
      QMessageBox::information( this, tr( "No Plugins" ), tr( "No QGIS plugins found in %1" ).arg( myPluginDir ) );
      return;
    }

    for ( uint i = 0; i < pluginDir.count(); i++ )
    {
      QString lib = QString( "%1/%2" ).arg( myPluginDir ).arg( pluginDir[i] );

#ifdef TESTLIB
      // This doesn't work on WIN32 and causes problems with plugins
      // on OS X (the code doesn't cause a problem but including dlfcn.h
      // renders plugins unloadable)
#if !defined(WIN32) && !defined(Q_OS_MACX)
      // test code to help debug loading problems
      // This doesn't work on WIN32 and causes problems with plugins
      // on OS X (the code doesn't cause a problem but including dlfcn.h
      // renders plugins unloadable)

      //void *handle = dlopen( (const char *) lib, RTLD_LAZY);
      void *handle = dlopen( lib.toLocal8Bit().data(), RTLD_LAZY | RTLD_GLOBAL );
      if ( !handle )
      {
        QgsDebugMsg( "Error in dlopen: " );
        QgsDebugMsg( dlerror() );
      }
      else
      {
        QgsDebugMsg( "dlopen suceeded for " + lib );
        dlclose( handle );
      }
#endif //#ifndef WIN32 && Q_OS_MACX
#endif //#ifdef TESTLIB

      QgsDebugMsg( "Examining: " + lib );
      QLibrary *myLib = new QLibrary( lib );
      bool loaded = myLib->load();
      if ( !loaded )
      {
        QgsDebugMsg( QString( "Failed to load: %1 (%2)" ).arg( myLib->fileName() ).arg( myLib->errorString() ) );
        delete myLib;
        continue;
      }

      QgsDebugMsg( "Loaded library: " + myLib->fileName() );

      // Don't bother with libraries that are providers
      //if(!myLib->resolve( "isProvider" ) )

      //MH: Replaced to allow for plugins that are linked to providers
      //type is only used in non-provider plugins
      if ( !myLib->resolve( "type" ) )
      {
        delete myLib;
        continue;
      }

      // resolve the metadata from plugin
      name_t *pName = ( name_t * ) cast_to_fptr( myLib->resolve( "name" ) );
      description_t *pDesc = ( description_t * ) cast_to_fptr( myLib->resolve( "description" ) );
      category_t *pCat = ( category_t * ) cast_to_fptr( myLib->resolve( "category" ) );
      version_t *pVersion = ( version_t * ) cast_to_fptr( myLib->resolve( "version" ) );
      icon_t* pIcon = ( icon_t * ) cast_to_fptr( myLib->resolve( "icon" ) );

      // show the values (or lack of) for each function
      if ( pName )
      {
        QgsDebugMsg( "Plugin name: " + pName() );
      }
      else
      {
        QgsDebugMsg( "Plugin name not returned when queried" );
      }
      if ( pDesc )
      {
        QgsDebugMsg( "Plugin description: " + pDesc() );
      }
      else
      {
        QgsDebugMsg( "Plugin description not returned when queried" );
      }
      if ( pCat )
      {
        QgsDebugMsg( "Plugin category: " + pCat() );
      }
      else
      {
        QgsDebugMsg( "Plugin category not returned when queried" );
      }
      if ( pVersion )
      {
        QgsDebugMsg( "Plugin version: " + pVersion() );
      }
      else
      {
        QgsDebugMsg( "Plugin version not returned when queried" );
      }
      if ( pIcon )
      {
        QgsDebugMsg( "Plugin icon: " + pIcon() );
      }

      if ( !pName || !pDesc || !pVersion )
      {
        QgsDebugMsg( "Failed to get name, description, or type for " + myLib->fileName() );
        delete myLib;
        continue;
      }

      // Add "cpp:" prefix in case of two: Python and C++ plugins with the same name
      QString baseName = "cpp:" + QFileInfo( lib ).baseName();

      QMap<QString, QString> metadata;
      metadata["id"] = baseName;
      metadata["name"] = pName();
      metadata["description"] = pDesc();
      metadata["category"] = ( pCat ? pCat() : tr( "Plugins" ) );
      metadata["version_installed"] = pVersion();
      metadata["icon"] = ( pIcon ? pIcon() : QString() );
      metadata["library"] = myLib->fileName();
      metadata["pythonic"] = "false";
      metadata["installed"] = "true";
      metadata["readonly"] = "true";
      mPlugins.insert( baseName, metadata );

      delete myLib;
    }
  }
}



void QgsPluginManager::reloadModelData()
{
  mModelPlugins->clear();

  for ( QMap<QString, QMap<QString, QString> >::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {

    if ( ! it->value( "id" ).isEmpty() )
    {
      QString baseName = it->value( "id" );
      QString pluginName = it->value( "name" );
      QString description = it->value( "description" );
      QString author = it->value( "author_name" );
      QString iconPath = it->value( "icon" );
      QString status = it->value( "status" );
      QString error = it->value( "error" );

      QStandardItem * mypDetailItem = new QStandardItem( pluginName.left( 32 ) );

      mypDetailItem->setData( baseName, PLUGIN_BASE_NAME_ROLE );
      mypDetailItem->setData( status, PLUGIN_STATUS_ROLE );
      mypDetailItem->setData( error, PLUGIN_ERROR_ROLE );
      mypDetailItem->setData( description, PLUGIN_DESCRIPTION_ROLE );
      mypDetailItem->setData( author, PLUGIN_AUTHOR_ROLE );
      mypDetailItem->setData( it->value( "tags" ), PLUGIN_TAGS_ROLE );
      mypDetailItem->setData( it->value( "downloads" ).rightJustified( 10, '0' ), PLUGIN_DOWNLOADS_ROLE );
      mypDetailItem->setData( it->value( "zip_repository" ), PLUGIN_REPOSITORY_ROLE );
      mypDetailItem->setData( it->value( "average_vote" ), PLUGIN_VOTE_ROLE );

      if ( QFileInfo( iconPath ).isFile() )
      {
        mypDetailItem->setIcon( QPixmap( iconPath ) );
      }
      else
      {
        mypDetailItem->setIcon( QPixmap( QgsApplication::defaultThemePath() + "/plugin.png" ) );
      }

      mypDetailItem->setEditable( false );

      // set item display style
      if ( ! it->value( "error" ).isEmpty() )
      {
        QBrush brush = mypDetailItem->foreground();
        brush.setColor( Qt::red );
        mypDetailItem->setForeground( brush );
      }
      if ( ! it->value( "error" ).isEmpty() || it->value( "status" ) == "upgradeable" || it->value( "status" ) == "new" )
      {
        QFont font = mypDetailItem->font();
        font.setBold( true );
        mypDetailItem->setFont( font );
      }

      // set checkable if the plugin is loadable.
      mypDetailItem->setCheckable( it->value( "installed" ) == "true" && it->value( "error" ).isEmpty() );

      // Set ckeckState depending on the plugin is loaded or not.
      // Initially mark all unchecked, then overwrite state of loaded ones with checked.
      // Only do it with installed plugins, not not initialize checkboxes of not installed plugins at all.
      if ( it->value( "installed" ) == "true" )
      {
        mypDetailItem->setCheckState( Qt::Unchecked );
      }

      if ( isPluginLoaded( it->value( "id" ) ) )
      {
        mypDetailItem->setCheckState( Qt::Checked );
      }

      // Add items to model
      mModelPlugins->appendRow( mypDetailItem );

      // Repaint the details view if the currently displayed data are changed.
      if ( baseName == mCurrentlyDisplayedPlugin )
      {
        showPluginDetails( mypDetailItem );
      }
    }
  }

  updateTabTitle();

  buttonUpgradeAll->setEnabled( hasUpgradeablePlugins() );

  // Disable tabs that are empty because of no suitable plugins in the model.
  mOptionsListWidget->item( 2 )->setHidden( ! hasUpgradeablePlugins() );
  mOptionsListWidget->item( 3 )->setHidden( ! hasNewPlugins() );
  mOptionsListWidget->item( 4 )->setHidden( ! hasInvalidPlugins() );
}



void QgsPluginManager::pluginItemChanged( QStandardItem * item )
{
  QString id = item->data( PLUGIN_BASE_NAME_ROLE ).toString();
  if ( item->checkState() && ! isPluginLoaded( id ) )
  {
    QgsDebugMsg( " Loading plugin: " + id );
    loadPlugin( id );
  }
  else if ( ! item->checkState() && isPluginLoaded( id ) )
  {
    QgsDebugMsg( " Unloading plugin: " + id );
    unloadPlugin( id );
  }
}



void QgsPluginManager::showPluginDetails( QStandardItem * item )
{
  QMap<QString, QString> * metadata = pluginMetadata( item->data( PLUGIN_BASE_NAME_ROLE ).toString() );

  if ( ! metadata ) return;

  QString html = "";

  //   // A future mockup for install/uninstall html controls
  //   "<table bgcolor=\"#CCCCCC\" cellspacing=\"5\" cellpadding=\"10\" width=\"100%\" height=\"100px\">"
  //   "<tr><td colspan=\"3\"><b>This plugin is installed</b></td></tr>"
  //   "<tr>"
  //   "  <td></td>"
  //   "  <td bgcolor=\"#AAFFAA\" align=\"right\"><a href=\"foo\" style=\"text-decoration:none;\" ><b>REINSTALL</b></a></td>"
  //   "  <td bgcolor=\"#FFFFAA\" align=\"right\"><a href=\"foo\" style=\"text-decoration:none;\" ><b>UNINSTALL</b></a></td>"
  //   "</tr>"
  //   "</table><br/>";
  //   // -----------------------------------------
  //   // Print all tags for debug purposes
  //   QList<QString> keys = metadata->keys();
  //   for ( int i=0; i < keys.size(); ++i )
  //   {
  //     html += QString( "%1: %2 <br/>" ).arg( keys.at( i ) ).arg( metadata->value( keys.at( i ) ) );
  //   }

  // First prepare message box(es)
  if ( ! metadata->value( "error" ).isEmpty() )
  {
    QString errorMsg;
    if ( metadata->value( "error" ) == "incompatible" )
    {
      errorMsg = QString( "<b>%1</b><br/><%2> %3" ).arg( tr( "This plugin is incompatible with this version of QGIS" ) ).arg( tr( "Compatible versions:" ) ).arg( metadata->value( "error_details" ) );
    }
    else if ( metadata->value( "error" ) == "dependent" )
    {
      errorMsg = QString( "<b>%1</b>:<br/>%2" ).arg( tr( "This plugin requires a missing module" ) ).arg( metadata->value( "error_details" ) );
    }
    else
    {
      errorMsg = QString( "<b>%1</b><br/>%2" ).arg( tr( "This plugin is broken" ) ).arg( metadata->value( "error_details" ) );
    }
    html += QString( "<table bgcolor=\"#EEEE00\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#CC0000\">%1</td></tr>"
                     "</table>" ).arg( errorMsg );
  }
  if ( metadata->value( "status" ) == "upgradeable" )
  {
    html += QString( "<table bgcolor=\"#FFFFAA\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#880000\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "There is a new version available" ) );
  };
  if ( metadata->value( "status" ) == "new" )
  {
    html += QString( "<table bgcolor=\"#CCFFCC\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#008800\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "This is a new plugin" ) );
  };
  if ( metadata->value( "status" ) == "newer" )
  {
    html += QString( "<table bgcolor=\"#FFFFCC\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#550000\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "Installed version of this plugin is higher than any version found in repository" ) );
  };
  if ( metadata->value( "experimental" ) == "true" )
  {
    html += QString( "<table bgcolor=\"#EEEEBB\" cellspacing=\"2\" cellpadding=\"2\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#660000\">"
                     "    <img src=\":/images/themes/default/pluginExperimental.png\" width=\"32\"><b>%1</b>"
                     "  </td></tr>"
                     "</table>" ).arg( tr( "This plugin is experimental" ) );
  };
  // if ( metadata->value( "status" ) == t.b.d. )
  // {
  //   html += QString( "<table bgcolor=\"#CCCCFF\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
  //           "  <tr><td width=\"100%\" style=\"color:#000088\"><b>%1</b></td></tr>"
  //           "</table>" ).arg( tr( "Installing..." ) );
  // };

  // Now the metadata
  html += "<table cellspacing=\"10\" width=\"100%\"><tr><td>";
  html += QString( "<h1>%1</h1>" ).arg( metadata->value( "name" ) );

  if ( QFileInfo( metadata->value( "icon" ) ).isFile() )
  {
    html += QString( "<img src=\"%1\" style=\"float:right;\">" ).arg( metadata->value( "icon" ) );
  }

  html += QString( "<h3>%2</h3>" ).arg( metadata->value( "description" ) );

  if ( ! metadata->value( "category" ).isEmpty() )
  {
    html += QString( "%1: %2 <br/>" ).arg( tr( "Category" ) ).arg( metadata->value( "category" ) );
  }
  if ( ! metadata->value( "tags" ).isEmpty() )
  {
    html += QString( "%1: %2 <br/>" ).arg( tr( "Tags" ) ).arg( metadata->value( "tags" ) );
  }
  if ( ! metadata->value( "author_email" ).isEmpty() )
  {
    html += QString( "%1: <a href='mailto:%2'>%3</a><br/>" ).arg( tr( "Author" ) ).arg( metadata->value( "author_email" ) ).arg( metadata->value( "author_name" ) );
  }
  else if ( ! metadata->value( "author_name" ).isEmpty() )
  {
    html += QString( "%1: %2<br/>" ).arg( tr( "Author" ) ).arg( metadata->value( "author_name" ) );
  }
  if ( ! metadata->value( "homepage" ).isEmpty() || ! metadata->value( "tracker" ).isEmpty() || ! metadata->value( "code_repository" ).isEmpty() )
  {
    html += QString( "%1: " ).arg( tr( "More info" ) );
    if ( ! metadata->value( "homepage" ).isEmpty() )
    {
      html += QString( "<a href='%1'>%2</a> &nbsp; " ).arg( metadata->value( "homepage" ) ).arg( tr( "homepage" ) );
    }
    if ( ! metadata->value( "tracker" ).isEmpty() )
    {
      html += QString( "<a href='%1'>%2</a> &nbsp; " ).arg( metadata->value( "tracker" ) ).arg( tr( "tracker" ) );
    }
    if ( ! metadata->value( "code_repository" ).isEmpty() )
    {
      html += QString( "<a href='%1'>%2</a>" ).arg( metadata->value( "code_repository" ) ).arg( tr( "code_ repository" ) );
    }
    html += "<br/>";
  }

  if ( ! metadata->value( "average_vote" ).isEmpty() )
  {
    // draw stars
    int stars = qRound( metadata->value( "average_vote" ).toFloat() );
    for ( int i = 0; i < stars; i++ )
    {
      html += "<img src=\":/images/themes/default/mIconNew.png\">";
    }
    html += tr( "<br/>%1 rating vote(s)<br/>" ).arg( metadata->value( "rating_votes" ) );
  }
  if ( ! metadata->value( "downloads" ).isEmpty() )
  {
    html += tr( "%1 downloads<br/>" ).arg( metadata->value( "downloads" ) );
  }

  html += "<br/>" ;

  if ( ! metadata->value( "version_installed" ).isEmpty() )
  {
    QString ver = metadata->value( "version_installed" );
    if ( ver == "-1" ) ver = "?";
    html += tr( "Installed version: %1 (in %2)<br/>" ).arg( ver ).arg( metadata->value( "library" ) );
  }
  if ( ! metadata->value( "version_available" ).isEmpty() )
  {
    html += tr( "Available version: %1 (in %2)<br/>" ).arg( metadata->value( "version_available" ) ).arg( metadata->value( "zip_repository" ) );
  }

  if ( ! metadata->value( "changelog" ).isEmpty() )
  {
    html += "<br/>" ;
    QString changelog = tr( "changelog:<br/>%1 <br/>" ).arg( metadata->value( "changelog" ) );
    html += changelog.replace( "\n", "<br/>" );
  }

  html += "</td></tr></table>";

  tbDetails->setHtml( html );

  // Set buttonInstall text
  if ( metadata->value( "status" ) == "upgradeable" )
  {
    buttonInstall->setText( tr( "Upgrade plugin" ) );
  }
  else if ( metadata->value( "status" ) == "newer" )
  {
    buttonInstall->setText( tr( "Downgrade plugin" ) );
  }
  else if ( metadata->value( "status" ) == "installed" )
  {
    buttonInstall->setText( tr( "Reinstall plugin" ) );
  }
  else if ( metadata->value( "status" ) == "not installed" || metadata->value( "status" ) == "new" )
  {
    buttonInstall->setText( tr( "Install plugin" ) );
  }
  else
  {
    buttonInstall->setText( tr( "Install/upgrade plugin" ) );
  }

  // Enable/disable buttons
  buttonInstall->setEnabled( metadata->value( "pythonic" ).toUpper() == "TRUE" && metadata->value( "status" ) != "orphan" );
  buttonUninstall->setEnabled( metadata->value( "pythonic" ).toUpper() == "TRUE" && metadata->value( "status" ) != "readonly" && metadata->value( "status" ) != "not installed" && metadata->value( "status" ) != "new" );

  // Store the id of the currently displayed plugin
  mCurrentlyDisplayedPlugin = metadata->value( "id" );
}



void QgsPluginManager::selectTabItem( int idx )
{
  mOptionsListWidget->setCurrentRow( idx );
}



void QgsPluginManager::clearPythonPluginMetadata( )
{
  for ( QMap<QString, QMap<QString, QString> >::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    if ( it->value( "pythonic" ) == "true" )
    {
      mPlugins.remove( it.key() );
    }
  }
}



void QgsPluginManager::addPluginMetadata( QString key,  QMap<QString, QString> metadata )
{
  mPlugins.insert( key, metadata );
}



QMap<QString, QString>* QgsPluginManager::pluginMetadata( QString key )
{
  QMap<QString, QMap<QString, QString> >::iterator it = mPlugins.find( key );
  if ( it != mPlugins.end() )
  {
    return &it.value();
  }
  return NULL;
}



//! Clear the repository listWidget
void QgsPluginManager::clearRepositoryList()
{
  treeRepositories->clear();
  buttonRefreshRepos->setEnabled( false );
  buttonEditRep->setEnabled( false );
  buttonDeleteRep->setEnabled( false );
}



//! Add repository to the repository listWidget
void QgsPluginManager::addToRepositoryList( QMap<QString, QString> repository )
{
  QString key = repository.value( "name" );
  if ( ! key.isEmpty() )
  {
    QTreeWidgetItem * a = new QTreeWidgetItem( treeRepositories );
    a->setText( 1, key );
    a->setText( 2, repository.value( "url" ) );
    if ( repository.value( "enabled" ) == "true" && repository.value( "valid" ) == "true" )
    {
      if ( repository.value( "state" ) == "2" )
      {
        a->setText( 0, tr( "connected" ) );
        a->setIcon( 0, QIcon( ":/images/themes/default/repositoryConnected.png" ) );
        a->setToolTip( 0, tr( "The repository is connected" ) );
      }
      else
      {
        a->setText( 0, tr( "unavailable" ) );
        a->setIcon( 0, QIcon( ":/images/themes/default/repositoryUnavailable.png" ) );
        a->setToolTip( 0, tr( "The repository is enabled, but unavailable" ) );
      }
    }
    else
    {
      a->setText( 0, tr( "disabled" ) );
      a->setIcon( 0, QIcon( ":/images/themes/default/repositoryDisabled.png" ) );
      if ( repository.value( "valid" ) == "true" )
      {
        a->setToolTip( 0, tr( "The repository is disabled" ) );
      }
      else
      {
        a->setToolTip( 0, tr( "The repository is blocked due to incompatibility with your QGIS version" ) );
      }

      QBrush grayBrush = QBrush( QColor( Qt::gray ) );
      a->setForeground( 0, grayBrush );
      a->setForeground( 1, grayBrush );
      a->setForeground( 2, grayBrush );
    }
  }
  treeRepositories->resizeColumnToContents( 0 );
  treeRepositories->resizeColumnToContents( 1 );
  treeRepositories->resizeColumnToContents( 2 );
  treeRepositories->sortItems( 1, Qt::AscendingOrder );
  buttonRefreshRepos->setEnabled( true );
}



// SLOTS ///////////////////////////////////////////////////////////////////



// "Close" button clicked
void QgsPluginManager::reject()
{
  QSettings settings;
  settings.setValue( settingsGroup + "/checkOnStart", QVariant( ckbCheckUpdates->isChecked() ) );
  settings.setValue( settingsGroup + "/checkOnStartInterval", QVariant( checkingOnStartIntervals.value( comboInterval->currentIndex() ) ) );
  QgsPythonRunner::run( "pyplugin_installer.instance().onManagerClose()" );
  done( 1 );
}



void QgsPluginManager::setCurrentTab( int idx )
{
  if ( idx == ( mOptionsListWidget->count() - 1 ) )
  {
    QgsDebugMsg( "Switching current tab to Settings" );
    mOptionsStackedWidget->setCurrentIndex( 1 );
  }
  else
  {
    QgsDebugMsg( "Switching current tab to Plugins" );
    mOptionsStackedWidget->setCurrentIndex( 0 );

    QStringList acceptedStatuses;
    switch ( idx )
    {
      case 0:
        // installed
        acceptedStatuses << "installed" << "orphan" << "newer" << "upgradeable" << "";
        break;
      case 1:
        // not installed (get more)
        acceptedStatuses << "not installed" << "new" ;
        break;
      case 2:
        // upgradeable
        acceptedStatuses << "upgradeable" ;
        break;
      case 3:
        // new
        acceptedStatuses << "new" ;
        break;
      case 4:
        // invalid
        acceptedStatuses << "invalid" ;
        break;
    }
    mModelProxy->setAcceptedStatuses( acceptedStatuses );

    updateTabTitle();
  }
}



void QgsPluginManager::currentPluginChanged( const QModelIndex & theIndex )
{
  if ( theIndex.column() == 0 )
  {
    // If the model has been filtered, the index row in the proxy won't match the index row in the underlying model
    // so we need to jump through this little hoop to get the correct item
    QModelIndex realIndex = mModelProxy->mapToSource( theIndex );
    QStandardItem* mypItem = mModelPlugins->itemFromIndex( realIndex );
    showPluginDetails( mypItem );
  }
}



void QgsPluginManager::on_vwPlugins_clicked( const QModelIndex &theIndex )
{
  if ( theIndex.column() == 0 )
  {
    // If the model has been filtered, the index row in the proxy won't match the index row in the underlying model
    // so we need to jump through this little hoop to get the correct item
    QModelIndex realIndex = mModelProxy->mapToSource( theIndex );
    QStandardItem* mypItem = mModelPlugins->itemFromIndex( realIndex );
    if ( !mypItem->isEnabled() )
    {
      //The item is inactive (uncompatible or broken plugin), so it can't be selected. Display it's data anyway.
      vwPlugins->clearSelection();
      showPluginDetails( mypItem );
    }
  }
}



void QgsPluginManager::on_vwPlugins_doubleClicked( const QModelIndex & theIndex )
{
  if ( theIndex.column() == 0 )
  {
    // If the model has been filtered, the index row in the proxy won't match the index row in the underlying model
    // so we need to jump through this little hoop to get the correct item
    QModelIndex realIndex = mModelProxy->mapToSource( theIndex );
    QStandardItem* mypItem = mModelPlugins->itemFromIndex( realIndex );
    if ( mypItem->isCheckable() )
    {
      if ( mypItem->checkState() == Qt::Checked )
      {
        mypItem->setCheckState( Qt::Unchecked );
      }
      else
      {
        mypItem->setCheckState( Qt::Checked );
      }
    }
  }
}



void QgsPluginManager::on_leFilter_textChanged( QString theText )
{
  QgsDebugMsg( "PluginManager filter changed to :" + theText );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( theText, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
}



void QgsPluginManager::on_rbFilterNames_toggled( bool checked )
{
  if ( checked )
  {
    mModelProxy->setFilterRole( Qt::DisplayRole );
  }
}



void QgsPluginManager::on_rbFilterDescriptions_toggled( bool checked )
{
  if ( checked )
  {
    mModelProxy->setFilterRole( PLUGIN_DESCRIPTION_ROLE );
  }
}



void QgsPluginManager::on_rbFilterTags_toggled( bool checked )
{
  if ( checked )
  {
    mModelProxy->setFilterRole( PLUGIN_TAGS_ROLE );
  }
}



void QgsPluginManager::on_rbFilterAuthors_toggled( bool checked )
{
  if ( checked )
  {
    mModelProxy->setFilterRole( PLUGIN_AUTHOR_ROLE );
  }
}



void QgsPluginManager::on_buttonUpgradeAll_clicked( )
{
  QgsPythonRunner::run( "pyplugin_installer.instance().upgradeAllUpgradeable()" );
}



void QgsPluginManager::on_buttonInstall_clicked( )
{
  QgsPythonRunner::run( QString( "pyplugin_installer.instance().installPlugin('%1')" ).arg( mCurrentlyDisplayedPlugin ) );
}



void QgsPluginManager::on_buttonUninstall_clicked( )
{
  QgsPythonRunner::run( QString( "pyplugin_installer.instance().uninstallPlugin('%1')" ).arg( mCurrentlyDisplayedPlugin ) );
}



void QgsPluginManager::on_treeRepositories_itemSelectionChanged( )
{
  buttonEditRep->setEnabled( ! treeRepositories -> selectedItems().isEmpty() );
  buttonDeleteRep->setEnabled( ! treeRepositories -> selectedItems().isEmpty() );
}



void QgsPluginManager::on_treeRepositories_doubleClicked( QModelIndex )
{
  on_buttonEditRep_clicked();
}



void QgsPluginManager::on_buttonRefreshRepos_clicked( )
{
  QgsDebugMsg( "Refreshing repositories..." );
  QgsPythonRunner::run( "pyplugin_installer.instance().reloadAndExportData()" );
}



void QgsPluginManager::on_buttonAddRep_clicked( )
{
  QgsDebugMsg( "Adding repository connection..." );
  QgsPythonRunner::run( "pyplugin_installer.instance().addRepository()" );
}



void QgsPluginManager::on_buttonEditRep_clicked( )
{
  QTreeWidgetItem * current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( "\'", "\\\'" ).replace( "\"", "\\\"" );
    QgsDebugMsg( "Editing repository connection: " + key );
    QgsPythonRunner::run( QString( "pyplugin_installer.instance().editRepository('%1')" ).arg( key ) );
  }
}



void QgsPluginManager::on_buttonDeleteRep_clicked( )
{
  QTreeWidgetItem * current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( "\'", "\\\'" ).replace( "\"", "\\\"" );
    QgsDebugMsg( "Deleting repository connection: " + key );
    QgsPythonRunner::run( QString( "pyplugin_installer.instance().deleteRepository('%1')" ).arg( key ) );
  }
}



void QgsPluginManager::on_ckbExperimental_toggled( bool state )
{
  QSettings settings;
  settings.setValue( settingsGroup + "/allowExperimental", QVariant( state ) );
  QgsPythonRunner::run( "pyplugin_installer.installer_data.plugins.rebuild()" );
  QgsPythonRunner::run( "pyplugin_installer.instance().exportPluginsToManager()" );
}



// PRIVATE METHODS ///////////////////////////////////////////////////////////////////



bool QgsPluginManager::isPluginLoaded( QString key )
{
  QMap<QString, QString>* plugin = pluginMetadata( key );
  if ( plugin->isEmpty() )
  {
    return false;
  }

  QString library = key;
  if ( plugin->value( "pythonic" ) != "true" )
  {
    // trim "cpp:" prefix from cpp plugin id
    key = key.mid( 4 );
    library = plugin->value( "library" );
  }

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  if ( pRegistry->isLoaded( key ) )
  {
    // TODO: this check shouldn't be necessary, plugin base names must be unique
    if ( pRegistry->library( key ) == library )
    {
      return true;
    }
  }
  return false;
}



bool QgsPluginManager::hasUpgradeablePlugins( )
{
  for ( QMap<QString,  QMap<QString, QString> >::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    if ( it->value( "status" ) == "upgradeable" )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasNewPlugins( )
{
  for ( QMap<QString,  QMap<QString, QString> >::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    if ( it->value( "status" ) == "new" )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasInvalidPlugins( )
{
  for ( QMap<QString,  QMap<QString, QString> >::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    if ( ! it->value( "error" ).isEmpty() )
    {
      return true;
    }
  }

  return false;
}



void QgsPluginManager::updateTabTitle()
{
  lbStatusFilter->setText( QString( " %1 > %2 (%3)" ).arg( tr( "Plugins" ) )
                           .arg( mOptionsListWidget->currentItem()->text() )
                           .arg( mModelProxy->countWithCurrentStatus() ) );
}

