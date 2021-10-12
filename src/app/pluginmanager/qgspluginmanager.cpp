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

#include <cmath>

#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QLibrary>
#include <QStandardItem>
#include <QPushButton>
#include <QRegExp>
#include <QSortFilterProxyModel>
#include <QActionGroup>
#include <QTextStream>
#include <QTimer>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QUrl>

#include "qgsmessagelog.h"

#include "qgis.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsconfig.h"
#include "qgsmessagebar.h"
#include "qgsproviderregistry.h"
#include "qgspluginregistry.h"
#include "qgspluginsortfilterproxymodel.h"
#include "qgspythonrunner.h"
#include "qgspluginmanager.h"
#include "qgisplugin.h"
#include "qgslogger.h"
#include "qgspluginitemdelegate.h"
#include "qgssettings.h"
#ifdef WITH_BINDINGS
#include "qgspythonutils.h"
#endif

// Do we need this?
// #define TESTLIB
#ifdef TESTLIB
// This doesn't work on windows and causes problems with plugins
// on OS X (the code doesn't cause a problem but including dlfcn.h
// renders plugins unloadable)
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACX)
#include <dlfcn.h>
#endif
#endif


QgsPluginManager::QgsPluginManager( QWidget *parent, bool pluginsAreEnabled, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( QStringLiteral( "PluginManager" ), parent, fl )
{
  // initialize pointer
  mPythonUtils = nullptr;

  setupUi( this );
  connect( vwPlugins, &QListView::doubleClicked, this, &QgsPluginManager::vwPlugins_doubleClicked );
  connect( wvDetails, &QgsWebView::linkClicked, this, &QgsPluginManager::wvDetails_linkClicked );
  connect( leFilter, &QgsFilterLineEdit::textChanged, this, &QgsPluginManager::leFilter_textChanged );
  connect( buttonUpgradeAll, &QPushButton::clicked, this, &QgsPluginManager::buttonUpgradeAll_clicked );
  connect( buttonInstall, &QPushButton::clicked, this, &QgsPluginManager::buttonInstall_clicked );
  connect( buttonInstallExperimental, &QPushButton::clicked, this, &QgsPluginManager::buttonInstallExperimental_clicked );
  connect( buttonUninstall, &QPushButton::clicked, this, &QgsPluginManager::buttonUninstall_clicked );
  connect( treeRepositories, &QTreeWidget::itemSelectionChanged, this, &QgsPluginManager::treeRepositories_itemSelectionChanged );
  connect( treeRepositories, &QTreeWidget::doubleClicked, this, &QgsPluginManager::treeRepositories_doubleClicked );
  connect( buttonAddRep, &QPushButton::clicked, this, &QgsPluginManager::buttonAddRep_clicked );
  connect( buttonEditRep, &QPushButton::clicked, this, &QgsPluginManager::buttonEditRep_clicked );
  connect( buttonDeleteRep, &QPushButton::clicked, this, &QgsPluginManager::buttonDeleteRep_clicked );
  connect( buttonRefreshRepos, &QPushButton::clicked, this, &QgsPluginManager::buttonRefreshRepos_clicked );
  connect( ckbExperimental, &QgsCollapsibleGroupBox::toggled, this, &QgsPluginManager::ckbExperimental_toggled );
  connect( ckbDeprecated, &QgsCollapsibleGroupBox::toggled, this, &QgsPluginManager::ckbDeprecated_toggled );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPluginManager::showHelp );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( true );

  // Don't let QgsOptionsDialogBase to narrow the vertical tab list widget
  mOptListWidget->setMaximumWidth( 16777215 );

  // load translated description strings from qgspluginmanager_texts
  initTabDescriptions();

  // set internal variable
  mPluginsAreEnabled = pluginsAreEnabled;

  // Init models
  mModelPlugins = new QStandardItemModel( 0, 1 );
  mModelProxy = new QgsPluginSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModelPlugins );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  mModelProxy->setSortRole( Qt::DisplayRole );
  mModelProxy->setDynamicSortFilter( true );
  mModelProxy->sort( 0, Qt::AscendingOrder );
  vwPlugins->setModel( mModelProxy );
  vwPlugins->setItemDelegate( new QgsPluginItemDelegate( vwPlugins ) );
  vwPlugins->setFocus();

  // Preset widgets
  leFilter->setFocus( Qt::MouseFocusReason );
  leFilter->setShowSearchIcon( true );
  leFilter->setPlaceholderText( tr( "Search…" ) );
  wvDetails->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

  // Connect other signals
  connect( mOptionsListWidget, &QListWidget::currentRowChanged, this, &QgsPluginManager::setCurrentTab );
  connect( vwPlugins->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsPluginManager::currentPluginChanged );
  connect( mModelPlugins, &QStandardItemModel::itemChanged, this, &QgsPluginManager::pluginItemChanged );

  // Restiore UI state for widgets not handled by QgsOptionsDialogBase
  const QgsSettings settings;
  // 1) The second splitter state:
  mPluginsDetailsSplitter->restoreState( settings.value( QStringLiteral( "Windows/PluginManager/secondSplitterState" ) ).toByteArray() );
  // 2) The current mOptionsListWidget index (it will overwrite the "tab" setting of QgsOptionsDialogBase that handles the stackedWidget page
  // instead of the mOptionsListWidget index). Then the signal connected above will update the relevant page as well.
  const int option = settings.value( QStringLiteral( "Windows/PluginManager/option" ), 0 ).toInt();
  mOptionsListWidget->setCurrentRow( option );
  if ( option == 0 )
  {
    // The first option won't fire the currentRowChanged signal, so initialize the first tab explicitly
    setCurrentTab( 0 );
  }

  // Hide widgets only suitable with Python support enabled (they will be uncovered back in setPythonUtils)
  buttonUpgradeAll->hide();
  buttonInstall->hide();
  buttonInstallExperimental->hide();
  buttonUninstall->hide();
  frameSettings->setHidden( true );
  mOptionsListWidget->item( PLUGMAN_TAB_INSTALL_FROM_ZIP )->setHidden( true );

  voteRating->hide();
  voteLabel->hide();
  voteSlider->hide();
  voteSubmit->hide();
#ifndef WITH_QTWEBKIT
  connect( voteSubmit, SIGNAL( clicked() ), this, SLOT( submitVote() ) );
#endif

  // Init the message bar instance
  msgBar = new QgsMessageBar( this );
  msgBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  vlayoutRightColumn->insertWidget( 0, msgBar );
}



QgsPluginManager::~QgsPluginManager()
{
  delete mModelProxy;
  delete mModelPlugins;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/PluginManager/secondSplitterState" ), mPluginsDetailsSplitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/PluginManager/option" ), mOptionsListWidget->currentRow() );
}



void QgsPluginManager::setPythonUtils( QgsPythonUtils *pythonUtils )
{
  mPythonUtils = pythonUtils;

  // get the QgsSettings group from the installer
  QString settingsGroup;
  QgsPythonRunner::eval( QStringLiteral( "pyplugin_installer.instance().exportSettingsGroup()" ), settingsGroup );
  const QgsSettings settings;

  // Now enable Python support:
  // Show and preset widgets only suitable when Python support active
  mOptionsListWidget->item( PLUGMAN_TAB_INSTALL_FROM_ZIP )->setHidden( false );
  buttonUpgradeAll->show();
  buttonInstall->show();
  buttonInstallExperimental->setVisible( settings.value( settingsGroup + "/allowExperimental", false ).toBool() );
  buttonUninstall->show();
  frameSettings->setHidden( false );
  labelNoPython->setHidden( true );
  buttonRefreshRepos->setEnabled( false );
  buttonEditRep->setEnabled( false );
  buttonDeleteRep->setEnabled( false );

  // Add context menu to the plugins list view
  QAction *actionSortByName = new QAction( tr( "Sort by Name" ), vwPlugins );
  QAction *actionSortByDownloads = new QAction( tr( "Sort by Downloads" ), vwPlugins );
  QAction *actionSortByVote = new QAction( tr( "Sort by Vote" ), vwPlugins );
  QAction *actionSortByStatus = new QAction( tr( "Sort by Status" ), vwPlugins );
  QAction *actionSortByDateCreated = new QAction( tr( "Sort by Date Created" ), vwPlugins );
  QAction *actionSortByDateUpdated = new QAction( tr( "Sort by Date Updated" ), vwPlugins );
  actionSortByName->setCheckable( true );
  actionSortByDownloads->setCheckable( true );
  actionSortByVote->setCheckable( true );
  actionSortByStatus->setCheckable( true );
  actionSortByDateCreated->setCheckable( true );
  actionSortByDateUpdated->setCheckable( true );
  QActionGroup *group = new QActionGroup( vwPlugins );
  actionSortByName->setActionGroup( group );
  actionSortByDownloads->setActionGroup( group );
  actionSortByVote->setActionGroup( group );
  actionSortByStatus->setActionGroup( group );
  actionSortByDateCreated->setActionGroup( group );
  actionSortByDateUpdated->setActionGroup( group );
  actionSortByName->setChecked( true );
  vwPlugins->addAction( actionSortByName );
  vwPlugins->addAction( actionSortByDownloads );
  vwPlugins->addAction( actionSortByVote );
  vwPlugins->addAction( actionSortByStatus );
  vwPlugins->addAction( actionSortByDateCreated );
  vwPlugins->addAction( actionSortByDateUpdated );
  vwPlugins->setContextMenuPolicy( Qt::ActionsContextMenu );
  connect( actionSortByName, &QAction::triggered, mModelProxy, &QgsPluginSortFilterProxyModel::sortPluginsByName );
  connect( actionSortByDownloads, &QAction::triggered, mModelProxy, &QgsPluginSortFilterProxyModel::sortPluginsByDownloads );
  connect( actionSortByVote, &QAction::triggered, mModelProxy, &QgsPluginSortFilterProxyModel::sortPluginsByVote );
  connect( actionSortByStatus, &QAction::triggered, mModelProxy, &QgsPluginSortFilterProxyModel::sortPluginsByStatus );
  connect( actionSortByDateCreated, &QAction::triggered, mModelProxy, &QgsPluginSortFilterProxyModel::sortPluginsByDateCreated );
  connect( actionSortByDateUpdated, &QAction::triggered, mModelProxy, &QgsPluginSortFilterProxyModel::sortPluginsByDateUpdated );

  // Initialize the "Install from ZIP" tab widgets
  mZipFileWidget->setDefaultRoot( settings.value( settingsGroup + "/lastZipDirectory", "." ).toString() );
  mZipFileWidget->setFilter( tr( "Plugin packages (*.zip *.ZIP)" ) );
  connect( mZipFileWidget, &QgsFileWidget::fileChanged, this, &QgsPluginManager::mZipFileWidget_fileChanged );
  connect( buttonInstallFromZip, &QPushButton::clicked, this, &QgsPluginManager::buttonInstallFromZip_clicked );

  // Initialize list of allowed checking intervals
  mCheckingOnStartIntervals << 0 << 1 << 3 << 7 << 14 << 30;

  // Initialize the "Settings" tab widgets
  if ( settings.value( settingsGroup + "/checkOnStart", false ).toBool() )
  {
    ckbCheckUpdates->setChecked( true );
  }

  if ( settings.value( settingsGroup + "/allowExperimental", false ).toBool() )
  {
    ckbExperimental->setChecked( true );
  }

  if ( settings.value( settingsGroup + "/allowDeprecated", false ).toBool() )
  {
    ckbDeprecated->setChecked( true );
  }

  const int interval = settings.value( settingsGroup + "/checkOnStartInterval", "" ).toInt();
  const int indx = mCheckingOnStartIntervals.indexOf( interval ); // if none found, just use -1 index.
  comboInterval->setCurrentIndex( indx );
}



void QgsPluginManager::loadPlugin( const QString &id )
{
  const QMap<QString, QString> *plugin = pluginMetadata( id );

  if ( ! plugin )
  {
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString library = plugin->value( QStringLiteral( "library" ) );
  if ( plugin->value( QStringLiteral( "pythonic" ) ) == QLatin1String( "true" ) )
  {
    library = plugin->value( QStringLiteral( "id" ) );
    QgsDebugMsg( "Loading Python plugin: " + library );
    pRegistry->loadPythonPlugin( library );
  }
  else // C++ plugin
  {
    QgsDebugMsg( "Loading C++ plugin: " + library );
    pRegistry->loadCppPlugin( library );
  }

  QgsDebugMsgLevel( "Plugin loaded: " + library, 2 );
  QApplication::restoreOverrideCursor();
}



void QgsPluginManager::unloadPlugin( const QString &id )
{
  const QMap<QString, QString> *plugin = pluginMetadata( id );

  if ( ! plugin )
  {
    return;
  }

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString library = plugin->value( QStringLiteral( "library" ) );

  if ( plugin->value( QStringLiteral( "pythonic" ) ) == QLatin1String( "true" ) )
  {
    library = plugin->value( QStringLiteral( "id" ) );
    QgsDebugMsgLevel( "Unloading Python plugin: " + library, 2 );
    pRegistry->unloadPythonPlugin( library );
  }
  else // C++ plugin
  {
    QgsDebugMsgLevel( "Unloading C++ plugin: " + library, 2 );
    pRegistry->unloadCppPlugin( library );
  }
}



void QgsPluginManager::savePluginState( QString id, bool state )
{
  const QMap<QString, QString> *plugin = pluginMetadata( id );
  if ( ! plugin )
  {
    return;
  }

  QgsSettings settings;
  if ( plugin->value( QStringLiteral( "pythonic" ) ) == QLatin1String( "true" ) )
  {
    // Python plugin
    settings.setValue( "/PythonPlugins/" + id, state );
  }
  else
  {
    // C++ plugin
    // Trim "cpp:" prefix from cpp plugin id
    id = id.mid( 4 );
    settings.setValue( "/Plugins/" + id, state );
  }
}



void QgsPluginManager::getCppPluginsMetadata()
{
  QString sharedLibExtension;
#if defined(Q_OS_WIN) || defined(__CYGWIN__)
  sharedLibExtension = "*.dll";
#else
  sharedLibExtension = QStringLiteral( "*.so*" );
#endif

  // check all libs in the current ans user plugins directories, and get name and descriptions
  // First, the qgis install directory/lib (this info is available from the provider registry so we use it here)
  QgsProviderRegistry *pr = QgsProviderRegistry::instance();
  QStringList myPathList( pr->libraryDirectory().path() );

  const QgsSettings settings;
  const QString myPaths = settings.value( QStringLiteral( "plugins/searchPathsForPlugins" ), "" ).toString();
  if ( !myPaths.isEmpty() )
  {
    myPathList.append( myPaths.split( '|' ) );
  }

  for ( int j = 0; j < myPathList.size(); ++j )
  {
    const QString myPluginDir = myPathList.at( j );
    const QDir pluginDir( myPluginDir, sharedLibExtension, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks );

    if ( pluginDir.count() == 0 )
    {
      QMessageBox::information( this, tr( "No Plugins" ), tr( "No QGIS plugins found in %1" ).arg( myPluginDir ) );
      return;
    }

    for ( uint i = 0; i < pluginDir.count(); i++ )
    {
      const QString lib = QStringLiteral( "%1/%2" ).arg( myPluginDir, pluginDir[i] );

#ifdef TESTLIB
      // This doesn't work on windows and causes problems with plugins
      // on OS X (the code doesn't cause a problem but including dlfcn.h
      // renders plugins unloadable)
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACX)
      // test code to help debug loading problems
      // This doesn't work on windows and causes problems with plugins
      // on OS X (the code doesn't cause a problem but including dlfcn.h
      // renders plugins unloadable)

      //void *handle = dlopen( (const char *) lib, RTLD_LAZY);
      void *handle = dlopen( lib.toLocal8Bit().data(), RTLD_LAZY | RTLD_GLOBAL );
      if ( !handle )
      {
        QgsDebugMsg( QStringLiteral( "Error in dlopen: " ) );
        QgsDebugMsg( dlerror() );
      }
      else
      {
        QgsDebugMsg( "dlopen succeeded for " + lib );
        dlclose( handle );
      }
#endif //#ifndef Q_OS_WIN && Q_OS_MACX
#endif //#ifdef TESTLIB

      QgsDebugMsgLevel( "Examining: " + lib, 2 );
      QLibrary *myLib = new QLibrary( lib );
      const bool loaded = myLib->load();
      if ( !loaded )
      {
        QgsDebugMsgLevel( QStringLiteral( "Failed to load: %1 (%2)" ).arg( myLib->fileName(), myLib->errorString() ), 2 );
        delete myLib;
        continue;
      }

      QgsDebugMsgLevel( "Loaded library: " + myLib->fileName(), 2 );
      //Type is only used in non-provider plugins, so data providers are not picked
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
      icon_t *pIcon = ( icon_t * ) cast_to_fptr( myLib->resolve( "icon" ) );
      experimental_t *pExperimental = ( experimental_t * ) cast_to_fptr( myLib->resolve( "experimental" ) );
      create_date_t *pCreateDate = ( create_date_t * ) cast_to_fptr( myLib->resolve( "create_date" ) );
      update_date_t *pUpdateDate = ( update_date_t * ) cast_to_fptr( myLib->resolve( "update_date" ) );

      // show the values (or lack of) for each function
      if ( pName )
      {
        QgsDebugMsgLevel( "Plugin name: " + *pName(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin name not returned when queried" ), 2 );
      }
      if ( pDesc )
      {
        QgsDebugMsgLevel( "Plugin description: " + *pDesc(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin description not returned when queried" ), 2 );
      }
      if ( pCat )
      {
        QgsDebugMsgLevel( "Plugin category: " + *pCat(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin category not returned when queried" ), 2 );
      }
      if ( pVersion )
      {
        QgsDebugMsgLevel( "Plugin version: " + *pVersion(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin version not returned when queried" ), 2 );
      }
      if ( pIcon )
      {
        QgsDebugMsgLevel( "Plugin icon: " + *pIcon(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin icon not returned when queried" ), 2 );
      }
      if ( pCreateDate )
      {
        QgsDebugMsgLevel( "Plugin create date: " + *pCreateDate(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin create date not returned when queried" ), 2 );
      }
      if ( pUpdateDate )
      {
        QgsDebugMsgLevel( "Plugin update date: " + *pUpdateDate(), 2 );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Plugin update date not returned when queried" ), 2 );
      }

      if ( !pName || !pDesc || !pVersion )
      {
        QgsDebugMsgLevel( "Failed to get name, description, or type for " + myLib->fileName(), 2 );
        delete myLib;
        continue;
      }

      // Add "cpp:" prefix in case of two: Python and C++ plugins with the same name
      const QString baseName = "cpp:" + QFileInfo( lib ).baseName();

      QMap<QString, QString> metadata;
      metadata[QStringLiteral( "id" )] = baseName;
      metadata[QStringLiteral( "name" )] = *pName();
      metadata[QStringLiteral( "description" )] = *pDesc();
      metadata[QStringLiteral( "category" )] = ( pCat ? *pCat() : tr( "Plugins" ) );
      metadata[QStringLiteral( "version_installed" )] = *pVersion();
      metadata[QStringLiteral( "icon" )] = ( pIcon ? *pIcon() : QString() );
      metadata[QStringLiteral( "library" )] = myLib->fileName();
      metadata[QStringLiteral( "pythonic" )] = QStringLiteral( "false" );
      metadata[QStringLiteral( "installed" )] = QStringLiteral( "true" );
      metadata[QStringLiteral( "readonly" )] = QStringLiteral( "true" );
      metadata[QStringLiteral( "status" )] = QStringLiteral( "orphan" );
      metadata[QStringLiteral( "experimental" )] = ( pExperimental ? *pExperimental() : QString() );
      metadata[QStringLiteral( "create_date" )] = ( pCreateDate ? *pCreateDate() : QString() );
      metadata[QStringLiteral( "update_date" )] = ( pUpdateDate ? *pUpdateDate() : QString() );
      mPlugins.insert( baseName, metadata );

      delete myLib;
    }
  }
}



QStandardItem *QgsPluginManager::createSpacerItem( const QString &text, const QString &value )
{
  QStandardItem *mySpacerltem = new QStandardItem( text );
  mySpacerltem->setData( value, PLUGIN_STATUS_ROLE );
  mySpacerltem->setData( "status", SPACER_ROLE );
  mySpacerltem->setEnabled( false );
  mySpacerltem->setEditable( false );
  QFont font = mySpacerltem->font();
  font.setBold( true );
  mySpacerltem->setFont( font );
  mySpacerltem->setTextAlignment( Qt::AlignHCenter );
  return mySpacerltem;
}



void QgsPluginManager::reloadModelData()
{
  mModelPlugins->clear();

  if ( !mCurrentlyDisplayedPlugin.isEmpty() )
  {
    wvDetails->setHtml( QString() );
    buttonInstall->setEnabled( false );
    buttonUninstall->setEnabled( false );
  }

  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( ! it->value( QStringLiteral( "id" ) ).isEmpty() )
    {
      const QString baseName = it->value( QStringLiteral( "id" ) );
      const QString pluginName = it->value( QStringLiteral( "name" ) );
      const QString description = it->value( QStringLiteral( "description" ) );
      const QString author = it->value( QStringLiteral( "author_name" ) );
      const QString createDate = it->value( QStringLiteral( "create_date" ) );
      const QString updateDate = it->value( QStringLiteral( "update_date" ) );
      const QString iconPath = it->value( QStringLiteral( "icon" ) );
      const QString status = it->value( QStringLiteral( "status" ) );
      const QString status_exp = it->value( QStringLiteral( "status_exp" ) );
      const QString error = it->value( QStringLiteral( "error" ) );

      QStandardItem *mypDetailItem = new QStandardItem( pluginName );

      mypDetailItem->setData( baseName, PLUGIN_BASE_NAME_ROLE );
      mypDetailItem->setData( status, PLUGIN_STATUS_ROLE );
      mypDetailItem->setData( status_exp, PLUGIN_STATUSEXP_ROLE );
      mypDetailItem->setData( error, PLUGIN_ERROR_ROLE );
      mypDetailItem->setData( description, PLUGIN_DESCRIPTION_ROLE );
      mypDetailItem->setData( author, PLUGIN_AUTHOR_ROLE );
      mypDetailItem->setData( createDate, PLUGIN_CREATE_DATE );
      mypDetailItem->setData( updateDate, PLUGIN_UPDATE_DATE );
      mypDetailItem->setData( it->value( QStringLiteral( "tags" ) ), PLUGIN_TAGS_ROLE );
      mypDetailItem->setData( it->value( QStringLiteral( "downloads" ) ).rightJustified( 10, '0' ), PLUGIN_DOWNLOADS_ROLE );
      mypDetailItem->setData( it->value( QStringLiteral( "average_vote" ) ), PLUGIN_VOTE_ROLE );
      mypDetailItem->setData( it->value( QStringLiteral( "deprecated" ) ), PLUGIN_ISDEPRECATED_ROLE );

      if ( QFileInfo( iconPath ).isFile() )
      {
        mypDetailItem->setData( QPixmap( iconPath ), Qt::DecorationRole );
      }
      else
      {
        mypDetailItem->setData( QPixmap( QgsApplication::defaultThemePath() + "/propertyicons/plugin.svg" ), Qt::DecorationRole );
      }

      mypDetailItem->setEditable( false );

      // Set checkable if the plugin is installed and not disabled due to incompatibility.
      // Broken plugins are checkable to to allow disabling them
      mypDetailItem->setCheckable( it->value( QStringLiteral( "installed" ) ) == QLatin1String( "true" ) && it->value( QStringLiteral( "error" ) ) != QLatin1String( "incompatible" ) );

      // Set ckeckState depending on the plugin is loaded or not.
      // Initially mark all unchecked, then overwrite state of loaded ones with checked.
      // Only do it with installed plugins, not not initialize checkboxes of not installed plugins at all.
      if ( it->value( QStringLiteral( "installed" ) ) == QLatin1String( "true" ) )
      {
        mypDetailItem->setCheckState( Qt::Unchecked );
      }

      if ( isPluginEnabled( it->value( QStringLiteral( "id" ) ) ) )
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

#ifdef WITH_BINDINGS
  // Add spacers for sort by status
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // TODO: implement better sort method instead of these dummy -Z statuses
    mModelPlugins->appendRow( createSpacerItem( tr( "Only locally available", "category: plugins that are only locally available" ), QStringLiteral( "orphanZ" ) ) );
    if ( hasReinstallablePlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Reinstallable", "category: plugins that are installed and available" ), QStringLiteral( "installedZ" ) ) );
    if ( hasUpgradeablePlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Upgradeable", "category: plugins that are installed and there is a newer version available" ), QStringLiteral( "upgradeableZ" ) ) );
    if ( hasNewerPlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Downgradeable", "category: plugins that are installed and there is an OLDER version available" ), QStringLiteral( "newerZ" ) ) );
    if ( hasAvailablePlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Installable", "category: plugins that are available for installation" ), QStringLiteral( "not installedZ" ) ) );
  }
#endif

  updateWindowTitle();

  buttonUpgradeAll->setEnabled( hasUpgradeablePlugins() );

  // Disable tabs that are empty because of no suitable plugins in the model.
  mOptionsListWidget->item( PLUGMAN_TAB_NOT_INSTALLED )->setHidden( ! hasAvailablePlugins() );
  mOptionsListWidget->item( PLUGMAN_TAB_UPGRADEABLE )->setHidden( ! hasUpgradeablePlugins() );
  mOptionsListWidget->item( PLUGMAN_TAB_NEW )->setHidden( ! hasNewPlugins() );
  mOptionsListWidget->item( PLUGMAN_TAB_INVALID )->setHidden( ! hasInvalidPlugins() );
}



void QgsPluginManager::pluginItemChanged( QStandardItem *item )
{
  const QString id = item->data( PLUGIN_BASE_NAME_ROLE ).toString();

  if ( item->checkState() )
  {
    if ( mPluginsAreEnabled && ! isPluginEnabled( id ) )
    {
      QgsDebugMsg( " Loading plugin: " + id );
      loadPlugin( id );
    }
    else
    {
      // only enable the plugin, as we're in --noplugins mode
      QgsDebugMsg( " Enabling plugin: " + id );
      savePluginState( id, true );
    }
  }
  else if ( ! item->checkState() )
  {
    QgsDebugMsg( " Unloading plugin: " + id );
    unloadPlugin( id );
  }
}



void QgsPluginManager::showPluginDetails( QStandardItem *item )
{
  const QMap<QString, QString> *metadata = pluginMetadata( item->data( PLUGIN_BASE_NAME_ROLE ).toString() );

  if ( !metadata )
  {
    return;
  }

  QString html = "<style>"
                 "  body {"
                 "    background-color:white;"
                 "  }"
                 "  body, table {"
                 "    padding:0px;"
                 "    margin:0px;"
                 "    font-family:Verdana, Sans-serif;"
                 "    font-size:10pt;"
                 "  }"
                 "  a {"
                 "    color:#08c;"
                 "    text-decoration:none;"
                 "  }"
                 "  a:hover,a:focus {"
                 "    color:#005580;"
                 "    text-decoration:underline;"
                 "  }"
                 "  div#votes {"
                 "    width:360px;"
                 "    margin-left:98px;"
                 "    padding-top:3px;"
                 "  }"
                 "  td {"
                 "    vertical-align:top;"
                 "  }"
                 "  td.key {"
                 "    font-weight: bold;"
                 "    white-space:nowrap;"
                 "    padding-right:10px;"
                 "    text-align:right;"
                 "  }"
                 "</style>";

  if ( !metadata->value( QStringLiteral( "plugin_id" ) ).isEmpty() )
  {
#ifdef WITH_QTWEBKIT
    html += QString(
              "<style>"
              "  div#stars_bg {"
              "    background-image: url('qrc:/images/themes/default/stars_empty.svg');"
              "    background-size: 92px 16px;"
              "    width:92px;"
              "    height:16px;"
              "    margin-bottom:16px;"
              "  }"
              "  div#stars {"
              "    background-image: url('qrc:/images/themes/default/stars_full.svg');"
              "    background-size: 92px 16px;"  /*scale to the full width*/
              "    width:%1px;"
              "    height:16px;"
              "  }"
              "</style>" ).arg( metadata->value( QStringLiteral( "average_vote" ) ).toFloat() / 5 * 92 );
    html += QString(
              "<script>"
              "  var plugin_id=%1;"
              "  var vote=0;"
              "  function ready()"
              "  {"
              "    document.getElementById('stars_bg').onmouseover=save_vote;"
              "    document.getElementById('stars_bg').onmouseout=restore_vote;"
              "    document.getElementById('stars_bg').onmousemove=change_vote;"
              "    document.getElementById('stars_bg').onclick=send_vote;"
              "  };"
              "    "
              "  function save_vote(e)"
              "  {"
              "    vote = document.getElementById('stars').style.width"
              "  }"
              "   "
              "  function restore_vote(e)"
              "  {"
              "    document.getElementById('stars').style.width = vote;"
              "  }"
              "   "
              "  function change_vote(e)"
              "  {"
              "    var length = e.x - document.getElementById('stars').getBoundingClientRect().left;"
              "    max = document.getElementById('stars_bg').getBoundingClientRect().right;"
              "    if ( length <= max ) document.getElementById('stars').style.width = length + 'px';"
              "  }"
              "   "
              "  function send_vote(e)"
              "  {"
              "    save_vote();"
              "    result = Number(vote.replace('px',''));"
              "    if (!result) return;"
              "    result = Math.floor(result/92*5)+1;"
              "    document.getElementById('send_vote_trigger').href='rpc2://plugin.vote/'+plugin_id+'/'+result;"
              "    ev=document.createEvent('MouseEvents');"
              "    ev.initEvent('click', false, true);"
              "    document.getElementById('send_vote_trigger').dispatchEvent(ev);"
              "  }"
              "</script>" ).arg( metadata->value( QStringLiteral( "plugin_id" ) ) );
#else
    voteRating->show();
    voteLabel->show();
    voteSlider->show();
    voteSubmit->show();
    QgsDebugMsg( QStringLiteral( "vote slider:%1" ).arg( std::round( metadata->value( "average_vote" ).toFloat() ) ) );
    voteSlider->setValue( std::round( metadata->value( "average_vote" ).toFloat() ) );
    mCurrentPluginId = metadata->value( "plugin_id" ).toInt();
  }
  else
  {
    voteRating->hide();
    voteLabel->hide();
    voteSlider->hide();
    voteSubmit->hide();
    mCurrentPluginId = -1;
#endif
  }

#ifdef WITH_QTWEBKIT
  html += QLatin1String( "<body onload='ready()'>" );
#else
  html += "<body>";
#endif


  // First prepare message box(es)
  if ( ! metadata->value( QStringLiteral( "error" ) ).isEmpty() )
  {
    QString errorMsg;
    if ( metadata->value( QStringLiteral( "error" ) ) == QLatin1String( "incompatible" ) )
    {
      errorMsg = QStringLiteral( "<b>%1</b><br/>%2" ).arg( tr( "This plugin is incompatible with this version of QGIS" ), tr( "Plugin designed for QGIS %1", "compatible QGIS version(s)" ).arg( metadata->value( QStringLiteral( "error_details" ) ) ) );
    }
    else if ( metadata->value( QStringLiteral( "error" ) ) == QLatin1String( "dependent" ) )
    {
      errorMsg = QStringLiteral( "<b>%1:</b><br/>%2" ).arg( tr( "This plugin requires a missing module" ), metadata->value( QStringLiteral( "error_details" ) ) );
    }
    else
    {
      errorMsg = QStringLiteral( "<b>%1</b><br/>%2" ).arg( tr( "This plugin is broken" ), metadata->value( QStringLiteral( "error_details" ) ) );
    }
    html += QString( "<table bgcolor=\"#FFFF88\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#CC0000\">%1</td></tr>"
                     "</table>" ).arg( errorMsg );
  }

  if ( metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "upgradeable" ) || metadata->value( QStringLiteral( "status_exp" ) ) == QLatin1String( "upgradeable" ) )
  {
    html += QString( "<table bgcolor=\"#FFFFAA\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#880000\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "There is a new version available" ) );
  }

  if ( metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "new" ) || metadata->value( QStringLiteral( "status_exp" ) ) == QLatin1String( "new" ) )
  {
    html += QString( "<table bgcolor=\"#CCFFCC\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#008800\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "This is a new plugin" ) );
  }

  if ( metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "newer" ) && metadata->value( QStringLiteral( "status_exp" ) ) == QLatin1String( "newer" ) )
  {
    html += QString( "<table bgcolor=\"#FFFFCC\" cellspacing=\"2\" cellpadding=\"6\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#550000\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "Installed version of this plugin is higher than any version found in repository" ) );
  }

  if ( ! metadata->value( QStringLiteral( "version_available_experimental" ) ).isEmpty() )
  {
    html += QString( "<table bgcolor=\"#EEEEBB\" cellspacing=\"2\" cellpadding=\"2\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#660000\">"
                     "    <img src=\"qrc:/images/themes/default/pluginExperimental.png\" width=\"32\"><b>%1</b>"
                     "  </td></tr>"
                     "</table>" ).arg( tr( "This plugin has an experimental version available" ) );
  }

  if ( metadata->value( QStringLiteral( "deprecated" ) ) == QLatin1String( "true" ) )
  {
    html += QString( "<table bgcolor=\"#EEBBCC\" cellspacing=\"2\" cellpadding=\"2\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#660000\">"
                     "    <img src=\"qrc:/images/themes/default/pluginDeprecated.svg\" width=\"32\"><b>%1</b>"
                     "  </td></tr>"
                     "</table>" ).arg( tr( "This plugin is deprecated" ) );
  }

  if ( metadata->value( QStringLiteral( "readonly" ) ) == QLatin1String( "true" ) )
  {
    html += QString( "<table bgcolor=\"#90EEE9\" cellspacing=\"2\" cellpadding=\"2\" width=\"100%\">"
                     "  <tr><td width=\"100%\" style=\"color:#660000\"><b>%1</b></td></tr>"
                     "</table>" ).arg( tr( "This is a core plugin, so you can't uninstall it" ) );
  }

  // Now the metadata

  html += QLatin1String( "<table cellspacing='4' width='100%'>" );
  html += QLatin1String( "<tr><td colspan='2'>" );

  QString iconPath = metadata->value( QStringLiteral( "icon" ) );

  if ( QFileInfo( iconPath ).isFile() || iconPath.startsWith( QLatin1String( "http" ) ) )
  {
    if ( iconPath.startsWith( QLatin1String( ":/" ) ) )
    {
      iconPath = "qrc" + iconPath;
    }
    else if ( ! iconPath.startsWith( QLatin1String( "http" ) ) )
    {
#if defined(Q_OS_WIN)
      iconPath = "file:///" + iconPath;
#else
      iconPath = "file://" + iconPath;
#endif
    }
    html += QStringLiteral( "<img src=\"%1\" style=\"float:right;max-width:64px;max-height:64px;\">" ).arg( iconPath );
  }

  const QRegularExpression stripHtml = QRegularExpression( QStringLiteral( "&lt;[^\\s].*?&gt;" ) );

  QString name = metadata->value( QStringLiteral( "name" ) );
  name = name.remove( stripHtml );
  html += QStringLiteral( "<h1>%1</h1>" ).arg( name );

  QString description = metadata->value( QStringLiteral( "description" ) );
  description = description.remove( stripHtml );
  html += QStringLiteral( "<h3>%1</h3>" ).arg( description );

  if ( ! metadata->value( QStringLiteral( "about" ) ).isEmpty() )
  {
    QString about = metadata->value( QStringLiteral( "about" ) );
    // The regular expression insures that a new line will be present after the closure of a paragraph tag (i.e. </p>)
    about = about.replace( QRegularExpression( QStringLiteral( "&lt;\\/p&gt;([^\\n])" ) ), QStringLiteral( "&lt;/p&gt;\n\\1" ) ).remove( stripHtml );
    html += about.replace( '\n', QLatin1String( "<br/>" ) );
    html += QLatin1String( "<br/><br/>" );
  }

  QString votes;
#ifndef WITH_QTWEBKIT
  votes += tr( "Average rating %1" ).arg( metadata->value( "average_vote" ).toFloat(), 0, 'f', 1 );
#endif
  if ( ! metadata->value( QStringLiteral( "rating_votes" ) ).isEmpty() )
  {
    if ( !votes.isEmpty() )
      votes += QLatin1String( ", " );
    votes += tr( "%1 rating vote(s)" ).arg( metadata->value( QStringLiteral( "rating_votes" ) ) );
  }
  if ( ! metadata->value( QStringLiteral( "downloads" ) ).isEmpty() )
  {
    if ( !votes.isEmpty() )
      votes += QLatin1String( ", " );
    votes += tr( "%1 downloads" ).arg( metadata->value( QStringLiteral( "downloads" ) ) );
  }

#ifdef WITH_QTWEBKIT
  if ( metadata->value( QStringLiteral( "readonly" ) ) == QLatin1String( "false" ) )
  {
    html += QLatin1String( "<div id='stars_bg'/><div id='stars'/>" );
    html += QLatin1String( "<div id='votes'>" );
    html += votes;
    html += QLatin1String( "</div>" );
    html += QLatin1String( "<div><a id='send_vote_trigger'/></div>" );
  }
#else
  voteRating->setText( votes );
#endif

  html += QLatin1String( "</td></tr>" );
  html += QLatin1String( "<tr><td width='1%'> </td><td width='99%'> </td></tr>" );

  if ( ! metadata->value( QStringLiteral( "category" ) ).isEmpty() )
  {
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td>%2</td></tr>" ).arg( tr( "Category" ), metadata->value( QStringLiteral( "category" ) ) );
  }
  if ( ! metadata->value( QStringLiteral( "tags" ) ).isEmpty() )
  {
    QStringList tags = metadata->value( QStringLiteral( "tags" ) ).toLower().split( ',' );
    for ( auto tag = tags.begin(); tag != tags.end(); ++tag )
    {
      *tag = QStringLiteral( "<a href='rpc2://search.tag/%1/'>%1</a>" ).arg( ( *tag ).trimmed() );
    }
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td>%2</td></tr>" ).arg( tr( "Tags" ), tags.join( QLatin1String( ", " ) ) );
  }

  if ( ! metadata->value( QStringLiteral( "homepage" ) ).isEmpty() || ! metadata->value( QStringLiteral( "tracker" ) ).isEmpty() || ! metadata->value( QStringLiteral( "code_repository" ) ).isEmpty() )
  {
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td>" ).arg( tr( "More info" ) );
    if ( ! metadata->value( QStringLiteral( "homepage" ) ).isEmpty() )
    {
      html += QStringLiteral( "<a href='%1'>%2</a> &nbsp; " ).arg( metadata->value( QStringLiteral( "homepage" ) ), tr( "homepage" ) );
    }
    if ( ! metadata->value( QStringLiteral( "tracker" ) ).isEmpty() )
    {
      html += QStringLiteral( "<a href='%1'>%2</a> &nbsp; " ).arg( metadata->value( QStringLiteral( "tracker" ) ), tr( "bug tracker" ) );
    }
    if ( ! metadata->value( QStringLiteral( "code_repository" ) ).isEmpty() )
    {
      html += QStringLiteral( "<a href='%1'>%2</a>" ).arg( metadata->value( QStringLiteral( "code_repository" ) ), tr( "code repository" ) );
    }
    html += QLatin1String( "</td></tr>" );
  }

  if ( ! metadata->value( QStringLiteral( "author_email" ) ).isEmpty() )
  {
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td><a href='mailto:%2'>%3</a></td></tr>" ).arg( tr( "Author" ), metadata->value( QStringLiteral( "author_email" ) ), metadata->value( QStringLiteral( "author_name" ) ) );
  }
  else if ( ! metadata->value( QStringLiteral( "author_name" ) ).isEmpty() )
  {
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td>%2</td></tr>" ).arg( tr( "Author" ), metadata->value( QStringLiteral( "author_name" ) ) );
  }

  if ( ! metadata->value( QStringLiteral( "version_installed" ) ).isEmpty() )
  {
    QString ver = metadata->value( QStringLiteral( "version_installed" ) );
    if ( ver == QLatin1String( "-1" ) )
    {
      ver = '?';
    }
    QString localDir = metadata->value( QStringLiteral( "library" ) );
    if ( QFileInfo( localDir ).isFile() )
    {
      localDir = QFileInfo( localDir ).canonicalFilePath();
    }
    else
    {
      localDir = QDir( localDir ).canonicalPath();
    }
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td title='%2'><a href='%3'>%4</a></td></tr>"
                          ).arg( tr( "Installed version" ),
                                 QDir::toNativeSeparators( localDir ),
                                 QUrl::fromLocalFile( localDir ).toString(),
                                 ver );
  }

  // if we allow experimental, we show both stable and experimental versions
  if ( ! metadata->value( QStringLiteral( "version_available_stable" ) ).isEmpty() )
  {
    QString downloadUrl = metadata->value( QStringLiteral( "download_url_stable" ) );
    if ( downloadUrl.contains( QStringLiteral( "plugins.qgis.org" ) ) )
    {
      // For the main repo, open the plugin version page instead of the download link. For other repositories the download link is the only known endpoint.
      downloadUrl = downloadUrl.replace( QLatin1String( "download/" ), QString() );
    }

    QString dateUpdatedStr;
    if ( ! metadata->value( QStringLiteral( "update_date" ) ).isEmpty() )
    {
      const QDateTime dateUpdated = QDateTime::fromString( metadata->value( QStringLiteral( "update_date_stable" ) ).trimmed(), Qt::ISODate );
      if ( dateUpdated.isValid() )
        dateUpdatedStr += QStringLiteral( "%1 %2" ).arg( tr( "updated at" ), dateUpdated.toString() );
    }

    html += QStringLiteral( "<tr><td class='key'>%1 </td><td title='%2'><a href='%2'>%3</a> %4</td></tr>"
                          ).arg( tr( "Available version (stable)" ),
                                 downloadUrl,
                                 metadata->value( QStringLiteral( "version_available_stable" ) ),
                                 dateUpdatedStr );
  }

  if ( ! metadata->value( QStringLiteral( "version_available_experimental" ) ).isEmpty() )
  {
    QString downloadUrl = metadata->value( QStringLiteral( "download_url_experimental" ) );
    if ( downloadUrl.contains( QStringLiteral( "plugins.qgis.org" ) ) )
    {
      // For the main repo, open the plugin version page instead of the download link. For other repositories the download link is the only known endpoint.
      downloadUrl = downloadUrl.replace( QLatin1String( "download/" ), QString() );
    }

    QString dateUpdatedStr;
    if ( !metadata->value( QStringLiteral( "update_date_experimental" ) ).isEmpty() )
    {
      const QDateTime dateUpdated = QDateTime::fromString( metadata->value( QStringLiteral( "update_date_experimental" ) ).trimmed(), Qt::ISODate );
      if ( dateUpdated.isValid() )
        dateUpdatedStr += QStringLiteral( "%1 %2" ).arg( tr( "updated at" ), dateUpdated.toString() );
    }

    html += QStringLiteral( "<tr><td class='key'>%1 </td><td title='%2'><a href='%2'>%3</a> %4</td></tr>"
                          ).arg( tr( "Available version (experimental)" ),
                                 downloadUrl,
                                 metadata->value( QStringLiteral( "version_available_experimental" ) ),
                                 dateUpdatedStr );
  }

  if ( ! metadata->value( QStringLiteral( "changelog" ) ).isEmpty() )
  {
    QString changelog = metadata->value( QStringLiteral( "changelog" ) );
    changelog = changelog.trimmed().replace( '\n', QLatin1String( "<br/>" ) );
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td>%2</td></tr>" ).arg( tr( "Changelog" ), changelog );
  }

  if ( ! metadata->value( QStringLiteral( "plugin_dependencies" ) ).isEmpty() )
  {
    QString pluginDependencies = metadata->value( QStringLiteral( "plugin_dependencies" ) );
    pluginDependencies = pluginDependencies.trimmed();
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td>%2</td></tr>" ).arg( tr( "Plugin dependencies" ), pluginDependencies );
  }

  html += QLatin1String( "</table>" );

  html += QLatin1String( "</body>" );

  wvDetails->setHtml( html );

  // Set buttonInstall text (and sometimes focus)
  buttonInstall->setDefault( false );
  if ( metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "upgradeable" ) )
  {
    buttonInstall->setText( tr( "Upgrade Plugin" ) );
    buttonInstall->setDefault( true );
  }
  else if ( metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "newer" ) )
  {
    buttonInstall->setText( tr( "Downgrade Plugin" ) );
  }
  else if ( metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "not installed" ) || metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "new" ) )
  {
    buttonInstall->setText( tr( "Install Plugin" ) );
  }
  else
  {
    // Default (will be grayed out if not available for reinstallation)
    buttonInstall->setText( tr( "Reinstall Plugin" ) );
  }

  // Set buttonInstall text (and sometimes focus)
  buttonInstallExperimental->setDefault( false );
  if ( metadata->value( QStringLiteral( "status_exp" ) ) == QLatin1String( "upgradeable" ) )
  {
    buttonInstallExperimental->setText( tr( "Upgrade Experimental Plugin" ) );
  }
  else if ( metadata->value( QStringLiteral( "status_exp" ) ) == QLatin1String( "newer" ) )
  {
    buttonInstallExperimental->setText( tr( "Downgrade Experimental Plugin" ) );
  }
  else if ( metadata->value( QStringLiteral( "status_exp" ) ) == QLatin1String( "not installed" ) || metadata->value( QStringLiteral( "status" ) ) == QLatin1String( "new" ) )
  {
    buttonInstallExperimental->setText( tr( "Install Experimental Plugin" ) );
  }
  else
  {
    // Default (will be grayed out if not available for reinstallation)
    buttonInstallExperimental->setText( tr( "Reinstall Experimental Plugin" ) );
  }

  // Enable/disable buttons

  const QgsSettings settings;
  QString settingsGroup;
  QgsPythonRunner::eval( QStringLiteral( "pyplugin_installer.instance().exportSettingsGroup()" ), settingsGroup );
  const bool expAllowed = settings.value( settingsGroup + "/allowExperimental", false ).toBool();

  const bool installEnabled = metadata->value( QStringLiteral( "pythonic" ) ).toUpper() == QLatin1String( "TRUE" ) && metadata->value( QStringLiteral( "status" ) ) != QLatin1String( "orphan" ) && metadata->value( QStringLiteral( "status" ) ) != QLatin1String( "none available" );
  const bool installExpEnabled = metadata->value( QStringLiteral( "pythonic" ) ).toUpper() == QLatin1String( "TRUE" ) && metadata->value( QStringLiteral( "status_exp" ) ) != QLatin1String( "orphan" ) && metadata->value( QStringLiteral( "status_exp" ) ) != QLatin1String( "none available" );

  buttonInstall->setEnabled( installEnabled );
  buttonInstall->setVisible( installEnabled || !installExpEnabled );
  buttonInstallExperimental->setEnabled( expAllowed && installExpEnabled );
  buttonInstallExperimental->setVisible( expAllowed && installExpEnabled );

  buttonUninstall->setEnabled( metadata->value( QStringLiteral( "pythonic" ) ).toUpper() == QLatin1String( "TRUE" ) && metadata->value( QStringLiteral( "readonly" ) ) != QLatin1String( "true" ) && ! metadata->value( QStringLiteral( "version_installed" ) ).isEmpty() );

  buttonUninstall->setHidden(
    metadata->value( QStringLiteral( "version_installed" ) ).isEmpty()
  );

  // Store the id of the currently displayed plugin
  mCurrentlyDisplayedPlugin = metadata->value( QStringLiteral( "id" ) );
}



void QgsPluginManager::selectTabItem( int idx )
{
  mOptionsListWidget->setCurrentRow( idx );
}



void QgsPluginManager::clearPythonPluginMetadata()
{
  for ( QMap<QString, QMap<QString, QString> >::iterator it = mPlugins.begin();
        it != mPlugins.end();
      )
  {
    if ( it->value( QStringLiteral( "pythonic" ) ) == QLatin1String( "true" ) )
    {
      it = mPlugins.erase( it );
    }
    else
    {
      ++it;
    }
  }
}



void QgsPluginManager::addPluginMetadata( const QString &key, const QMap<QString, QString> &metadata )
{
  mPlugins.insert( key, metadata );
}



const QMap<QString, QString> *QgsPluginManager::pluginMetadata( const QString &key ) const
{
  const QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.find( key );
  if ( it != mPlugins.end() )
  {
    return &it.value();
  }
  return nullptr;
}

void QgsPluginManager::clearRepositoryList()
{
  treeRepositories->clear();
  buttonRefreshRepos->setEnabled( false );
  buttonEditRep->setEnabled( false );
  buttonDeleteRep->setEnabled( false );
  const auto constActions = treeRepositories->actions();
  for ( QAction *action : constActions )
  {
    treeRepositories->removeAction( action );
  }
}

void QgsPluginManager::addToRepositoryList( const QMap<QString, QString> &repository )
{
  // If it's the second item on the tree, change the button text to plural form and add the filter context menu
  if ( buttonRefreshRepos->isEnabled() && treeRepositories->actions().count() < 1 )
  {
    buttonRefreshRepos->setText( tr( "Reload all Repositories" ) );
    QAction *actionEnableThisRepositoryOnly = new QAction( tr( "Only Show Plugins from Selected Repository" ), treeRepositories );
    treeRepositories->addAction( actionEnableThisRepositoryOnly );
    connect( actionEnableThisRepositoryOnly, &QAction::triggered, this, &QgsPluginManager::setRepositoryFilter );
    treeRepositories->setContextMenuPolicy( Qt::ActionsContextMenu );
    QAction *actionClearFilter = new QAction( tr( "Clear Filter" ), treeRepositories );
    actionClearFilter->setEnabled( repository.value( QStringLiteral( "inspection_filter" ) ) == QLatin1String( "true" ) );
    treeRepositories->addAction( actionClearFilter );
    connect( actionClearFilter, &QAction::triggered, this, &QgsPluginManager::clearRepositoryFilter );
  }

  const QString key = repository.value( QStringLiteral( "name" ) );
  if ( ! key.isEmpty() )
  {
    QTreeWidgetItem *a = new QTreeWidgetItem( treeRepositories );
    a->setText( 1, key );
    a->setText( 2, repository.value( QStringLiteral( "url" ) ) );
    if ( repository.value( QStringLiteral( "enabled" ) ) == QLatin1String( "true" ) && repository.value( QStringLiteral( "valid" ) ) == QLatin1String( "true" ) )
    {
      if ( repository.value( QStringLiteral( "state" ) ) == QLatin1String( "2" ) )
      {
        a->setText( 0, tr( "connected" ) );
        a->setIcon( 0, QIcon( ":/images/themes/default/repositoryConnected.svg" ) );
        a->setToolTip( 0, tr( "The repository is connected" ) );
      }
      else
      {
        a->setText( 0, tr( "unavailable" ) );
        a->setIcon( 0, QIcon( ":/images/themes/default/repositoryUnavailable.svg" ) );
        a->setToolTip( 0, tr( "The repository is enabled, but unavailable" ) );
      }
    }
    else
    {
      a->setText( 0, tr( "disabled" ) );
      a->setIcon( 0, QIcon( ":/images/themes/default/repositoryDisabled.svg" ) );
      if ( repository.value( QStringLiteral( "valid" ) ) == QLatin1String( "true" ) )
      {
        a->setToolTip( 0, tr( "The repository is disabled" ) );
      }
      else
      {
        a->setToolTip( 0, tr( "The repository is blocked due to incompatibility with your QGIS version" ) );
      }

      const QBrush grayBrush = QBrush( QColor( Qt::gray ) );
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
#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // get the QgsSettings group from the installer
    QString settingsGroup;
    QgsPythonRunner::eval( QStringLiteral( "pyplugin_installer.instance().exportSettingsGroup()" ), settingsGroup );
    QgsSettings settings;
    settings.setValue( settingsGroup + "/checkOnStart", QVariant( ckbCheckUpdates->isChecked() ) );
    settings.setValue( settingsGroup + "/checkOnStartInterval", QVariant( mCheckingOnStartIntervals.value( comboInterval->currentIndex() ) ) );
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().onManagerClose()" ) );
  }
#endif
  done( 1 );
}



void QgsPluginManager::setCurrentTab( int idx )
{
  if ( idx == PLUGMAN_TAB_SETTINGS )
  {
    mOptionsStackedWidget->setCurrentIndex( 2 );
  }
  else if ( idx == PLUGMAN_TAB_INSTALL_FROM_ZIP )
  {
    mOptionsStackedWidget->setCurrentIndex( 1 );
  }
  else
  {
    mOptionsStackedWidget->setCurrentIndex( 0 );

    QStringList acceptedStatuses;
    QString tabTitle;
    switch ( idx )
    {
      case PLUGMAN_TAB_ALL:
        // all (statuses ends with Z are for spacers to always sort properly)
        acceptedStatuses << QStringLiteral( "installed" ) << QStringLiteral( "not installed" ) << QStringLiteral( "new" ) << QStringLiteral( "orphan" ) << QStringLiteral( "none available" ) << QStringLiteral( "newer" ) << QStringLiteral( "upgradeable" ) << QStringLiteral( "not installedZ" ) << QStringLiteral( "installedZ" ) << QStringLiteral( "upgradeableZ" ) << QStringLiteral( "orphanZ" ) << QStringLiteral( "newerZZ" ) << QString();
        tabTitle = QStringLiteral( "all_plugins" );
        break;
      case PLUGMAN_TAB_INSTALLED:
        // installed (statuses ends with Z are for spacers to always sort properly)
        acceptedStatuses << QStringLiteral( "installed" ) << QStringLiteral( "orphan" ) << QStringLiteral( "newer" ) << QStringLiteral( "upgradeable" ) << QStringLiteral( "installedZ" ) << QStringLiteral( "upgradeableZ" ) << QStringLiteral( "orphanZ" ) << QStringLiteral( "newerZZ" ) << QString();
        tabTitle = QStringLiteral( "installed_plugins" );
        break;
      case PLUGMAN_TAB_NOT_INSTALLED:
        // not installed (get more)
        acceptedStatuses << QStringLiteral( "not installed" ) << QStringLiteral( "new" );
        tabTitle = QStringLiteral( "not_installed_plugins" );
        break;
      case PLUGMAN_TAB_UPGRADEABLE:
        // upgradeable
        acceptedStatuses << QStringLiteral( "upgradeable" );
        tabTitle = QStringLiteral( "upgradeable_plugins" );
        break;
      case PLUGMAN_TAB_NEW:
        // new
        acceptedStatuses << QStringLiteral( "new" );
        tabTitle = QStringLiteral( "new_plugins" );
        break;
      case PLUGMAN_TAB_INVALID:
        // invalid
        acceptedStatuses << QStringLiteral( "invalid" );
        tabTitle = QStringLiteral( "invalid_plugins" );
        break;
    }
    mModelProxy->setAcceptedStatuses( acceptedStatuses );

    // load tab description HTML to the detail browser
    QString tabInfoHTML;
    const QMap<QString, QString>::const_iterator it = mTabDescriptions.constFind( tabTitle );
    if ( it != mTabDescriptions.constEnd() )
    {
      tabInfoHTML += "<style>"
                     "  body, p {"
                     "      background-color: white;"
                     "      margin: 2px;"
                     "      font-family: Verdana, Sans-serif;"
                     "      font-size: 10pt;"
                     "  }"
                     "</style>";
      // tabInfoHTML += "<style>" + QgsApplication::reportStyleSheet() + "</style>";
      tabInfoHTML += it.value();
    }
    wvDetails->setHtml( tabInfoHTML );

    // disable buttons
    buttonInstall->setEnabled( false );
    buttonUninstall->setEnabled( false );
  }

  updateWindowTitle();
}



void QgsPluginManager::currentPluginChanged( const QModelIndex &index )
{
  if ( index.column() == 0 )
  {
    // If the model has been filtered, the index row in the proxy won't match the index row in the underlying model
    // so we need to jump through this little hoop to get the correct item
    const QModelIndex realIndex = mModelProxy->mapToSource( index );
    QStandardItem *mypItem = mModelPlugins->itemFromIndex( realIndex );
    showPluginDetails( mypItem );
  }
}



void QgsPluginManager::vwPlugins_doubleClicked( const QModelIndex &index )
{
  if ( index.column() == 0 )
  {
    // If the model has been filtered, the index row in the proxy won't match the index row in the underlying model
    // so we need to jump through this little hoop to get the correct item
    const QModelIndex realIndex = mModelProxy->mapToSource( index );
    QStandardItem *mypItem = mModelPlugins->itemFromIndex( realIndex );
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

#ifndef WITH_QTWEBKIT
void QgsPluginManager::submitVote()
{
  if ( mCurrentPluginId < 0 )
    return;

  sendVote( mCurrentPluginId, voteSlider->value() );
}
#endif

void QgsPluginManager::sendVote( int pluginId, int vote )
{
  QString response;
  QgsPythonRunner::eval( QStringLiteral( "pyplugin_installer.instance().sendVote('%1', '%2')" ).arg( pluginId ).arg( vote ), response );
  if ( response == QLatin1String( "True" ) )
  {
    pushMessage( tr( "Vote sent successfully" ), Qgis::MessageLevel::Info );
  }
  else
  {
    pushMessage( tr( "Sending vote to the plugin repository failed." ), Qgis::MessageLevel::Warning );
  }
}

void QgsPluginManager::wvDetails_linkClicked( const QUrl &url )
{
  if ( url.scheme() == QLatin1String( "rpc2" ) )
  {
    if ( url.host() == QLatin1String( "plugin.vote" ) )
    {
      QStringList params = url.path().split( '/' );
      sendVote( params[1].toInt(), params[2].toInt() );
    }
    else if ( url.host() == QLatin1String( "search.tag" ) )
    {
      QStringList params = url.path().split( '/' );
      leFilter->setText( QStringLiteral( "tag:%1" ).arg( params[1] ) );
      mOptionsListWidget->setCurrentRow( PLUGMAN_TAB_ALL );
    }
  }
  else
  {
    QDesktopServices::openUrl( url );
  }
}


void QgsPluginManager::leFilter_textChanged( QString text )
{
  if ( text.startsWith( QLatin1String( "tag:" ), Qt::CaseInsensitive ) )
  {
    text = text.remove( QStringLiteral( "tag:" ) );
    mModelProxy->setFilterRole( PLUGIN_TAGS_ROLE );
    QgsDebugMsg( "PluginManager TAG filter changed to :" + text );
  }
  else
  {
    mModelProxy->setFilterRole( 0 );
    QgsDebugMsg( "PluginManager filter changed to :" + text );
  }

  const QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  const Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  const QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
}



void QgsPluginManager::buttonUpgradeAll_clicked()
{
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().upgradeAllUpgradeable()" ) );
}



void QgsPluginManager::buttonInstall_clicked()
{
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().installPlugin('%1', stable=True)" ).arg( mCurrentlyDisplayedPlugin ) );
}


void QgsPluginManager::buttonInstallExperimental_clicked()
{
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().installPlugin('%1', stable=False)" ).arg( mCurrentlyDisplayedPlugin ) );
}



void QgsPluginManager::buttonUninstall_clicked()
{
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().uninstallPlugin('%1')" ).arg( mCurrentlyDisplayedPlugin ) );
}



void QgsPluginManager::mZipFileWidget_fileChanged( const QString &filePath )
{
  buttonInstallFromZip->setEnabled( QFileInfo( filePath ).isFile() );
}



void QgsPluginManager::buttonInstallFromZip_clicked()
{
  QgsSettings settings;
  const bool showInstallFromZipWarning = settings.value( QStringLiteral( "UI/showInstallFromZipWarning" ), true ).toBool();

  QMessageBox msgbox;
  if ( showInstallFromZipWarning )
  {
    msgbox.setWindowTitle( tr( "Security warning" ) );
    msgbox.setText( tr( "Installing a plugin from an untrusted source can harm your computer. Only continue if you received the plugin from a source you trust. Continue?" ) );
    msgbox.setIcon( QMessageBox::Icon::Warning );
    msgbox.addButton( QMessageBox::Yes );
    msgbox.addButton( QMessageBox::No );
    msgbox.setDefaultButton( QMessageBox::No );
    QCheckBox *cb = new QCheckBox( tr( "Don't show this again." ) );
    msgbox.setCheckBox( cb );
    msgbox.exec();
    settings.setValue( QStringLiteral( "UI/showInstallFromZipWarning" ), !msgbox.checkBox()->isChecked() );
  }

  if ( !showInstallFromZipWarning || msgbox.result() == QMessageBox::Yes )
  {
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().installFromZipFile(r'%1')" ).arg( mZipFileWidget->filePath() ) );
    mZipFileWidget->setFilePath( QString() );
  }
}



void QgsPluginManager::treeRepositories_itemSelectionChanged()
{
  buttonEditRep->setEnabled( ! treeRepositories->selectedItems().isEmpty() );
  buttonDeleteRep->setEnabled( ! treeRepositories->selectedItems().isEmpty() );
}



void QgsPluginManager::treeRepositories_doubleClicked( const QModelIndex & )
{
  buttonEditRep_clicked();
}



void QgsPluginManager::setRepositoryFilter()
{
  QTreeWidgetItem *current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( '\'', QLatin1String( "\\\'" ) ).replace( '\"', QLatin1String( "\\\"" ) );
    QgsDebugMsg( "Disabling all repositories but selected: " + key );
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().setRepositoryInspectionFilter('%1')" ).arg( key ) );
  }
}



void QgsPluginManager::clearRepositoryFilter()
{
  QgsDebugMsg( QStringLiteral( "Enabling all repositories back" ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().setRepositoryInspectionFilter()" ) );
}



void QgsPluginManager::buttonRefreshRepos_clicked()
{
  QgsDebugMsg( QStringLiteral( "Refreshing repositories..." ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().reloadAndExportData()" ) );
}



void QgsPluginManager::buttonAddRep_clicked()
{
  QgsDebugMsg( QStringLiteral( "Adding repository connection..." ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().addRepository()" ) );
}



void QgsPluginManager::buttonEditRep_clicked()
{
  QTreeWidgetItem *current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( '\'', QLatin1String( "\\\'" ) ).replace( '\"', QLatin1String( "\\\"" ) );
    QgsDebugMsg( "Editing repository connection: " + key );
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().editRepository('%1')" ).arg( key ) );
  }
}



void QgsPluginManager::buttonDeleteRep_clicked()
{
  QTreeWidgetItem *current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( '\'', QLatin1String( "\\\'" ) ).replace( '\"', QLatin1String( "\\\"" ) );
    QgsDebugMsg( "Deleting repository connection: " + key );
    QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().deleteRepository('%1')" ).arg( key ) );
  }
}



void QgsPluginManager::ckbExperimental_toggled( bool state )
{
  QString settingsGroup;
  QgsPythonRunner::eval( QStringLiteral( "pyplugin_installer.instance().exportSettingsGroup()" ), settingsGroup );
  QgsSettings settings;
  settings.setValue( settingsGroup + "/allowExperimental", QVariant( state ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.installer_data.plugins.rebuild()" ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().exportPluginsToManager()" ) );
}

void QgsPluginManager::ckbDeprecated_toggled( bool state )
{
  QString settingsGroup;
  QgsPythonRunner::eval( QStringLiteral( "pyplugin_installer.instance().exportSettingsGroup()" ), settingsGroup );
  QgsSettings settings;
  settings.setValue( settingsGroup + "/allowDeprecated", QVariant( state ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.installer_data.plugins.rebuild()" ) );
  QgsPythonRunner::run( QStringLiteral( "pyplugin_installer.instance().exportPluginsToManager()" ) );
}


// PRIVATE METHODS ///////////////////////////////////////////////////////////////////


bool QgsPluginManager::isPluginEnabled( QString key )
{
  const QMap<QString, QString> *plugin = pluginMetadata( key );
  if ( plugin->isEmpty() )
  {
    // No such plugin in the metadata registry
    return false;
  }

  const QgsSettings mySettings;
  if ( plugin->value( QStringLiteral( "pythonic" ) ) != QLatin1String( "true" ) )
  {
    // Trim "cpp:" prefix from cpp plugin id
    key = key.mid( 4 );
    return ( mySettings.value( "/Plugins/" + key, QVariant( false ) ).toBool() );
  }
  else
  {
    return ( plugin->value( QStringLiteral( "installed" ) ) == QLatin1String( "true" ) && mySettings.value( "/PythonPlugins/" + key, QVariant( false ) ).toBool() );
  }
}



bool QgsPluginManager::hasAvailablePlugins()
{
  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( QStringLiteral( "status" ) ) == QLatin1String( "not installed" ) || it->value( QStringLiteral( "status" ) ) == QLatin1String( "new" ) )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasReinstallablePlugins()
{
  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    // plugins marked as "installed" are available for download (otherwise they are marked "orphans")
    if ( it->value( QStringLiteral( "status" ) ) == QLatin1String( "installed" ) )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasUpgradeablePlugins()
{
  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( QStringLiteral( "status" ) ) == QLatin1String( "upgradeable" ) )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasNewPlugins()
{
  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( QStringLiteral( "status" ) ) == QLatin1String( "new" ) )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasNewerPlugins()
{
  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( QStringLiteral( "status" ) ) == QLatin1String( "newer" ) )
    {
      return true;
    }
  }

  return false;
}



bool QgsPluginManager::hasInvalidPlugins()
{
  for ( QMap<QString, QMap<QString, QString> >::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( ! it->value( QStringLiteral( "error" ) ).isEmpty() )
    {
      return true;
    }
  }

  return false;
}



void QgsPluginManager::updateWindowTitle()
{
  QListWidgetItem *curitem = mOptListWidget->currentItem();
  if ( curitem )
  {
    QString title = QStringLiteral( "%1 | %2" ).arg( tr( "Plugins" ), curitem->text() );
    if ( mOptionsListWidget->currentRow() < mOptionsListWidget->count() - 2 && mModelPlugins )
    {
      // if it's not the Settings tab, add the plugin count
      title += QStringLiteral( " (%3)" ).arg( mModelProxy->countWithCurrentStatus() );
    }
    setWindowTitle( title );
  }
  else
  {
    setWindowTitle( mDialogTitle );
  }
}



void QgsPluginManager::showEvent( QShowEvent *e )
{
  if ( mInit )
  {
    updateOptionsListVerticalTabs();
  }
  else
  {
    QTimer::singleShot( 0, this, &QgsPluginManager::warnAboutMissingObjects );
  }

  QgsOptionsDialogBase::showEvent( e );
}



void QgsPluginManager::pushMessage( const QString &text, Qgis::MessageLevel level, int duration )
{
  msgBar->pushMessage( text, level, duration );
}

void QgsPluginManager::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "plugins/plugins.html" ) );
}
