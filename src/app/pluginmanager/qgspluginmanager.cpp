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

#include "qgspluginmanager.h"

#include <cmath>

#include "qgis.h"
#include "qgisplugin.h"
#include "qgsapplication.h"
#include "qgshelp.h"
#include "qgslogger.h"
#include "qgsmessagebar.h"
#include "qgsmessagelog.h"
#include "qgspluginitemdelegate.h"
#include "qgspluginregistry.h"
#include "qgspluginsortfilterproxymodel.h"
#include "qgsproviderregistry.h"
#include "qgspythonrunner.h"
#include "qgssettings.h"
#include "qgsvariantutils.h"

#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLibrary>
#include <QLineEdit>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QRegExp>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QTextStream>
#include <QTimer>
#include <QUrl>

#include "moc_qgspluginmanager.cpp"

#ifdef WITH_BINDINGS
#include "qgspythonutils.h"
#endif

// Do we need this?
// #define TESTLIB
#ifdef TESTLIB
// This doesn't work on windows and causes problems with plugins
// on OS X (the code doesn't cause a problem but including dlfcn.h
// renders plugins unloadable)
#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
#include <dlfcn.h>
#endif
#endif

const QgsSettingsEntryBool *QgsPluginManager::settingsAutomaticallyCheckForPluginUpdates = new QgsSettingsEntryBool( u"automatically-check-for-updates"_s, sTreePluginManager, true, u"Automatically check for plugin updates on startup"_s );
const QgsSettingsEntryBool *QgsPluginManager::settingsAllowExperimental = new QgsSettingsEntryBool( u"allow-experimental"_s, sTreePluginManager, false, u"Allow experimental plugins."_s );
const QgsSettingsEntryBool *QgsPluginManager::settingsAllowDeprecated = new QgsSettingsEntryBool( u"allow-deprecated"_s, sTreePluginManager, false, u"Allow deprecated plugins."_s );
const QgsSettingsEntryVariant *QgsPluginManager::settingsCheckOnStartLastDate = new QgsSettingsEntryVariant( u"check-on-start-last-date"_s, sTreePluginManager, QgsVariantUtils::createNullVariant( QMetaType::Type::QDate ), u"Date last time the check was performed."_s );
const QgsSettingsEntryStringList *QgsPluginManager::settingsSeenPlugins = new QgsSettingsEntryStringList( u"seen-plugins"_s, sTreePluginManager, {}, u"Date last time the check was performed."_s );
const QgsSettingsEntryString *QgsPluginManager::settingsLastZipDirectory = new QgsSettingsEntryString( u"last-zip-directory"_s, sTreePluginManager, QString(), u"Last ZIP directory."_s );
const QgsSettingsEntryBool *QgsPluginManager::settingsShowInstallFromZipWarning = new QgsSettingsEntryBool( u"show-install-from-zip-warning"_s, sTreePluginManager, true );


QgsPluginManager::QgsPluginManager( QWidget *parent, bool pluginsAreEnabled, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( u"PluginManager"_s, parent, fl )
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
  mPluginsDetailsSplitter->restoreState( settings.value( u"Windows/PluginManager/secondSplitterState"_s ).toByteArray() );
  // 2) The current mOptionsListWidget index (it will overwrite the "tab" setting of QgsOptionsDialogBase that handles the stackedWidget page
  // instead of the mOptionsListWidget index). Then the signal connected above will update the relevant page as well.
  const int option = settings.value( u"Windows/PluginManager/option"_s, 0 ).toInt();
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
  connect( voteSubmit, &QPushButton::clicked, this, &QgsPluginManager::submitVote );

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
  settings.setValue( u"Windows/PluginManager/secondSplitterState"_s, mPluginsDetailsSplitter->saveState() );
  settings.setValue( u"Windows/PluginManager/option"_s, mOptionsListWidget->currentRow() );
}


void QgsPluginManager::setPythonUtils( QgsPythonUtils *pythonUtils )
{
  mPythonUtils = pythonUtils;

  // Now enable Python support:
  // Show and preset widgets only suitable when Python support active
  mOptionsListWidget->item( PLUGMAN_TAB_INSTALL_FROM_ZIP )->setHidden( false );
  buttonUpgradeAll->show();
  buttonInstall->show();
  buttonInstallExperimental->setVisible( settingsAllowExperimental->value() );
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
  mZipFileWidget->setDefaultRoot( settingsLastZipDirectory->value() );
  mZipFileWidget->setFilter( tr( "Plugin packages (*.zip *.ZIP)" ) );
  connect( mZipFileWidget, &QgsFileWidget::fileChanged, this, &QgsPluginManager::mZipFileWidget_fileChanged );
  connect( buttonInstallFromZip, &QPushButton::clicked, this, &QgsPluginManager::buttonInstallFromZip_clicked );

  // Initialize the "Settings" tab widgets
  if ( settingsAutomaticallyCheckForPluginUpdates->value() )
  {
    ckbCheckUpdates->setChecked( true );
  }

  if ( settingsAllowExperimental->value() )
  {
    ckbExperimental->setChecked( true );
  }

  if ( settingsAllowDeprecated->value() )
  {
    ckbDeprecated->setChecked( true );
  }
}

void QgsPluginManager::loadPlugin( const QString &id )
{
  const QMap<QString, QString> *plugin = pluginMetadata( id );

  if ( !plugin )
  {
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString library = plugin->value( u"library"_s );
  if ( plugin->value( u"pythonic"_s ) == "true"_L1 )
  {
    library = plugin->value( u"id"_s );
    QgsDebugMsgLevel( "Loading Python plugin: " + library, 2 );
    pRegistry->loadPythonPlugin( library );
  }
  else // C++ plugin
  {
    QgsDebugMsgLevel( "Loading C++ plugin: " + library, 2 );
    pRegistry->loadCppPlugin( library );
  }

  QgsDebugMsgLevel( "Plugin loaded: " + library, 2 );
  QApplication::restoreOverrideCursor();
}


void QgsPluginManager::unloadPlugin( const QString &id )
{
  const QMap<QString, QString> *plugin = pluginMetadata( id );

  if ( !plugin )
  {
    return;
  }

  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
  QString library = plugin->value( u"library"_s );

  if ( plugin->value( u"pythonic"_s ) == "true"_L1 )
  {
    library = plugin->value( u"id"_s );
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
  if ( !plugin )
  {
    return;
  }

  QgsSettings settings;
  if ( plugin->value( u"pythonic"_s ) == "true"_L1 )
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
#if defined( Q_OS_WIN ) || defined( __CYGWIN__ )
  sharedLibExtension = "*.dll";
#else
  sharedLibExtension = u"*.so*"_s;
#endif

  // check all libs in the current ans user plugins directories, and get name and descriptions
  // First, the qgis install directory/lib (this info is available from the provider registry so we use it here)
  QgsProviderRegistry *pr = QgsProviderRegistry::instance();
  QStringList myPathList( pr->libraryDirectory().path() );

  const QgsSettings settings;
  const QStringList myPaths = settings.value( u"plugins/searchPathsForPlugins"_s ).toStringList();
  if ( !myPaths.isEmpty() )
  {
    myPathList.append( myPaths );
    myPathList.removeDuplicates();
  }

  for ( int j = 0; j < myPathList.size(); ++j )
  {
    const QString myPluginDir = myPathList.at( j );
    const QDir pluginDir( myPluginDir, sharedLibExtension, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks );

    if ( pluginDir.count() == 0 )
    {
      QMessageBox::information( this, tr( "No Plugins" ), tr( "No QGIS plugins found in %1" ).arg( myPluginDir ) );
      continue;
    }

    for ( uint i = 0; i < pluginDir.count(); i++ )
    {
      const QString lib = u"%1/%2"_s.arg( myPluginDir, pluginDir[i] );

#ifdef TESTLIB
      // This doesn't work on windows and causes problems with plugins
      // on OS X (the code doesn't cause a problem but including dlfcn.h
      // renders plugins unloadable)
#if !defined( Q_OS_WIN ) && !defined( Q_OS_MACOS )
      // test code to help debug loading problems
      // This doesn't work on windows and causes problems with plugins
      // on OS X (the code doesn't cause a problem but including dlfcn.h
      // renders plugins unloadable)

      //void *handle = dlopen( (const char *) lib, RTLD_LAZY);
      void *handle = dlopen( lib.toLocal8Bit().data(), RTLD_LAZY | RTLD_GLOBAL );
      if ( !handle )
      {
        QgsDebugError( u"Error in dlopen: "_s );
        QgsDebugError( dlerror() );
      }
      else
      {
        QgsDebugMsgLevel( "dlopen succeeded for " + lib, 2 );
        dlclose( handle );
      }
#endif //#ifndef Q_OS_WIN && Q_OS_MACOS
#endif //#ifdef TESTLIB

      QgsDebugMsgLevel( "Examining: " + lib, 2 );
      try
      {
        auto myLib = std::make_unique<QLibrary>( lib );
        const bool loaded = myLib->load();
        if ( !loaded )
        {
          QgsDebugMsgLevel( u"Failed to load: %1 (%2)"_s.arg( myLib->fileName(), myLib->errorString() ), 2 );
          continue;
        }

        QgsDebugMsgLevel( "Loaded library: " + myLib->fileName(), 2 );
        //Type is only used in non-provider plugins, so data providers are not picked
        if ( !myLib->resolve( "type" ) )
        {
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
          QgsDebugMsgLevel( u"Plugin name not returned when queried"_s, 2 );
        }
        if ( pDesc )
        {
          QgsDebugMsgLevel( "Plugin description: " + *pDesc(), 2 );
        }
        else
        {
          QgsDebugMsgLevel( u"Plugin description not returned when queried"_s, 2 );
        }
        if ( pCat )
        {
          QgsDebugMsgLevel( "Plugin category: " + *pCat(), 2 );
        }
        else
        {
          QgsDebugMsgLevel( u"Plugin category not returned when queried"_s, 2 );
        }
        if ( pVersion )
        {
          QgsDebugMsgLevel( "Plugin version: " + *pVersion(), 2 );
        }
        else
        {
          QgsDebugMsgLevel( u"Plugin version not returned when queried"_s, 2 );
        }
        if ( pIcon )
        {
          QgsDebugMsgLevel( "Plugin icon: " + *pIcon(), 2 );
        }
        else
        {
          QgsDebugMsgLevel( u"Plugin icon not returned when queried"_s, 2 );
        }
        if ( pCreateDate )
        {
          QgsDebugMsgLevel( "Plugin create date: " + *pCreateDate(), 2 );
        }
        else
        {
          QgsDebugMsgLevel( u"Plugin create date not returned when queried"_s, 2 );
        }
        if ( pUpdateDate )
        {
          QgsDebugMsgLevel( "Plugin update date: " + *pUpdateDate(), 2 );
        }
        else
        {
          QgsDebugMsgLevel( u"Plugin update date not returned when queried"_s, 2 );
        }

        if ( !pName || !pDesc || !pVersion )
        {
          QgsDebugMsgLevel( "Failed to get name, description, or type for " + myLib->fileName(), 2 );
          continue;
        }

        // Add "cpp:" prefix in case of two: Python and C++ plugins with the same name
        const QString baseName = "cpp:" + QFileInfo( lib ).baseName();

        QMap<QString, QString> metadata;
        metadata[u"id"_s] = baseName;
        metadata[u"name"_s] = *pName();
        metadata[u"description"_s] = *pDesc();
        metadata[u"category"_s] = ( pCat ? *pCat() : tr( "Plugins" ) );
        metadata[u"version_installed"_s] = *pVersion();
        metadata[u"icon"_s] = ( pIcon ? *pIcon() : QString() );
        metadata[u"library"_s] = myLib->fileName();
        metadata[u"pythonic"_s] = u"false"_s;
        metadata[u"installed"_s] = u"true"_s;
        metadata[u"readonly"_s] = u"true"_s;
        metadata[u"status"_s] = u"orphan"_s;
        metadata[u"experimental"_s] = ( pExperimental ? *pExperimental() : QString() );
        metadata[u"create_date"_s] = ( pCreateDate ? *pCreateDate() : QString() );
        metadata[u"update_date"_s] = ( pUpdateDate ? *pUpdateDate() : QString() );
        mPlugins.insert( baseName, metadata );
      }
      catch ( QgsSettingsException &ex )
      {
        QgsDebugError( u"Unhandled settings exception loading %1: %2"_s.arg( lib, ex.what() ) );
        continue;
      }
      catch ( ... )
      {
        QgsDebugError( u"Unhandled exception loading %1"_s.arg( lib ) );
        continue;
      }
    }
  }
  QgsDebugMsgLevel( u"Loaded cpp plugins"_s, 2 );
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

  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( !it->value( u"id"_s ).isEmpty() )
    {
      const QString baseName = it->value( u"id"_s );
      const QString pluginName = it->value( u"name"_s );
      const QString description = it->value( u"description"_s );
      const QString author = it->value( u"author_name"_s );
      const QString createDate = it->value( u"create_date"_s );
      const QString updateDate = it->value( u"update_date"_s );
      const QString iconPath = it->value( u"icon"_s );
      const QString status = it->value( u"status"_s );
      const QString status_exp = it->value( u"status_exp"_s );
      const QString error = it->value( u"error"_s );

      QStandardItem *mypDetailItem = new QStandardItem( pluginName );

      mypDetailItem->setData( baseName, PLUGIN_BASE_NAME_ROLE );
      mypDetailItem->setData( status, PLUGIN_STATUS_ROLE );
      mypDetailItem->setData( status_exp, PLUGIN_STATUSEXP_ROLE );
      mypDetailItem->setData( error, PLUGIN_ERROR_ROLE );
      mypDetailItem->setData( description, PLUGIN_DESCRIPTION_ROLE );
      mypDetailItem->setData( author, PLUGIN_AUTHOR_ROLE );
      mypDetailItem->setData( createDate, PLUGIN_CREATE_DATE );
      mypDetailItem->setData( updateDate, PLUGIN_UPDATE_DATE );
      mypDetailItem->setData( it->value( u"tags"_s ), PLUGIN_TAGS_ROLE );
      mypDetailItem->setData( it->value( u"downloads"_s ).rightJustified( 10, '0' ), PLUGIN_DOWNLOADS_ROLE );
      mypDetailItem->setData( it->value( u"average_vote"_s ), PLUGIN_VOTE_ROLE );
      mypDetailItem->setData( it->value( u"deprecated"_s ), PLUGIN_ISDEPRECATED_ROLE );

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
      // Broken plugins are checkable to allow disabling them
      mypDetailItem->setCheckable( it->value( u"installed"_s ) == "true"_L1 && it->value( u"error"_s ) != "incompatible"_L1 );

      // Set ckeckState depending on the plugin is loaded or not.
      // Initially mark all unchecked, then overwrite state of loaded ones with checked.
      // Only do it with installed plugins, do not initialize checkboxes of not installed plugins at all.
      if ( it->value( u"installed"_s ) == "true"_L1 )
      {
        mypDetailItem->setCheckState( Qt::Unchecked );
      }

      if ( isPluginEnabled( it->value( u"id"_s ) ) )
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
    mModelPlugins->appendRow( createSpacerItem( tr( "Only locally available", "category: plugins that are only locally available" ), u"orphanZ"_s ) );
    if ( hasReinstallablePlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Reinstallable", "category: plugins that are installed and available" ), u"installedZ"_s ) );
    if ( hasUpgradeablePlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Upgradeable", "category: plugins that are installed and there is a newer version available" ), u"upgradeableZ"_s ) );
    if ( hasNewerPlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Downgradeable", "category: plugins that are installed and there is an OLDER version available" ), u"newerZ"_s ) );
    if ( hasAvailablePlugins() )
      mModelPlugins->appendRow( createSpacerItem( tr( "Installable", "category: plugins that are available for installation" ), u"not installedZ"_s ) );
  }
#endif

  updateWindowTitle();

  buttonUpgradeAll->setEnabled( hasUpgradeablePlugins() );

  // Disable tabs that are empty because of no suitable plugins in the model.
  mOptionsListWidget->item( PLUGMAN_TAB_NOT_INSTALLED )->setHidden( !hasAvailablePlugins() );
  mOptionsListWidget->item( PLUGMAN_TAB_UPGRADEABLE )->setHidden( !hasUpgradeablePlugins() );
  mOptionsListWidget->item( PLUGMAN_TAB_NEW )->setHidden( !hasNewPlugins() );
  mOptionsListWidget->item( PLUGMAN_TAB_INVALID )->setHidden( !hasInvalidPlugins() );
}


void QgsPluginManager::pluginItemChanged( QStandardItem *item )
{
  const QString id = item->data( PLUGIN_BASE_NAME_ROLE ).toString();

  if ( item->checkState() )
  {
    if ( mPluginsAreEnabled && !isPluginEnabled( id ) )
    {
      QgsDebugMsgLevel( " Loading plugin: " + id, 2 );
      loadPlugin( id );
    }
    else
    {
      // only enable the plugin, as we're in --noplugins mode
      QgsDebugMsgLevel( " Enabling plugin: " + id, 2 );
      savePluginState( id, true );
    }
  }
  else if ( !item->checkState() )
  {
    QgsDebugMsgLevel( " Unloading plugin: " + id, 2 );
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
                 "    color:"
                 + palette().color( QPalette::ColorRole::Text ).name() + ";"
                                                                         "    background-color:"
                 + palette().color( QPalette::ColorRole::Base ).name() + ";"
                                                                         "  }"
                                                                         "  body, table {"
                                                                         "    padding:0px;"
                                                                         "    margin:0px;"
                                                                         "    font-family:Verdana, Sans-serif;"
                                                                         "    font-size:10pt;"
                                                                         "  }"
                                                                         "  a {"
                                                                         "    color: "
                 + palette().color( QPalette::ColorRole::Link ).name() + ";"
                                                                         "    text-decoration:none;"
                                                                         "  }"
                                                                         "  a:hover,a:focus {"
                                                                         "    color: "
                 + palette().color( QPalette::ColorRole::Link ).name() + ";"
                                                                         "    text-decoration:underline;"
                                                                         "  }"
                                                                         "  a:visited {"
                                                                         "    color: "
                 + palette().color( QPalette::ColorRole::LinkVisited ).name() + ";"
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

  if ( !metadata->value( u"plugin_id"_s ).isEmpty() )
  {
    voteRating->show();
    voteLabel->show();
    voteSlider->show();
    voteSubmit->show();
    QgsDebugMsgLevel( u"vote slider:%1"_s.arg( std::round( metadata->value( "average_vote" ).toFloat() ) ), 2 );
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
  }

  html += "<body>";


  // First prepare message box(es)
  if ( !metadata->value( u"error"_s ).isEmpty() )
  {
    QString errorMsg;
    if ( metadata->value( u"error"_s ) == "incompatible"_L1 )
    {
      errorMsg = u"<b>%1</b><br/>%2"_s.arg( tr( "This plugin is incompatible with this version of QGIS" ), metadata->value( u"error_details"_s ) );
    }
    else if ( metadata->value( u"error"_s ) == "dependent"_L1 )
    {
      errorMsg = u"<b>%1:</b><br/>%2"_s.arg( tr( "This plugin requires a missing module" ), metadata->value( u"error_details"_s ) );
    }
    else
    {
      errorMsg = u"<b>%1</b><br/>%2"_s.arg( tr( "This plugin is broken" ), metadata->value( u"error_details"_s ) );
    }
    html += QString( "<table cellspacing=\"2\" cellpadding=\"6\" width=\"100%\" style=\"background-color: rgba(238, 144, 0, 0.25)\">"
                     "  <tr><td width=\"100%\">%1</td></tr>"
                     "</table>" )
              .arg( errorMsg );
  }

  if ( metadata->value( u"status"_s ) == "upgradeable"_L1 || metadata->value( u"status_exp"_s ) == "upgradeable"_L1 )
  {
    html += QString( "<table cellspacing=\"2\" cellpadding=\"6\" width=\"100%\" style=\"background-color: rgba(170, 0, 238, 0.25)\">"
                     "  <tr><td width=\"100%\"><b>%1</b></td></tr>"
                     "</table>" )
              .arg( tr( "There is a new version available" ) );
  }

  if ( metadata->value( u"status"_s ) == "new"_L1 || metadata->value( u"status_exp"_s ) == "new"_L1 )
  {
    html += QString( "<table cellspacing=\"2\" cellpadding=\"6\" width=\"100%\" style=\"background-color: rgba(0, 238, 0, 0.25)\">"
                     "  <tr><td width=\"100%\"><b>%1</b></td></tr>"
                     "</table>" )
              .arg( tr( "This is a new plugin" ) );
  }

  if ( metadata->value( u"status"_s ) == "newer"_L1 && metadata->value( u"status_exp"_s ) == "newer"_L1 )
  {
    html += QString( "<table cellspacing=\"2\" cellpadding=\"6\" width=\"100%\" style=\"background-color: rgba(238, 133, 0, 0.25)\">"
                     "  <tr><td width=\"100%\"><b>%1</b></td></tr>"
                     "</table>" )
              .arg( tr( "Installed version of this plugin is higher than any version found in repository" ) );
  }

  if ( !metadata->value( u"version_available_experimental"_s ).isEmpty() )
  {
    html += QString( "<table cellspacing=\"2\" cellpadding=\"2\" width=\"100%\" style=\"background-color: rgba(238, 238, 10, 0.25)\">"
                     "  <tr><td width=\"100%\">"
                     "    <img src=\"qrc:/images/themes/default/pluginExperimental.png\" width=\"32\"><b>%1</b>"
                     "  </td></tr>"
                     "</table>" )
              .arg( tr( "This plugin has an experimental version available" ) );
  }

  if ( metadata->value( u"deprecated"_s ) == "true"_L1 )
  {
    html += QString( "<table cellspacing=\"2\" cellpadding=\"2\" width=\"100%\" style=\"background-color: rgba(238, 0, 80, 0.25)\">"
                     "  <tr><td width=\"100%\">"
                     "    <img src=\"qrc:/images/themes/default/pluginDeprecated.svg\" width=\"32\"><b>%1</b>"
                     "  </td></tr>"
                     "</table>" )
              .arg( tr( "This plugin is deprecated" ) );
  }

  if ( metadata->value( u"readonly"_s ) == "true"_L1 )
  {
    html += QString( "<table cellspacing=\"2\" cellpadding=\"2\" width=\"100%\" style=\"background-color: rgba(0, 133, 238, 0.25)\">"
                     "  <tr><td width=\"100%\"><b>%1</b></td></tr>"
                     "</table>" )
              .arg( tr( "This is a core plugin, so you can't uninstall it" ) );
  }

  // Now the metadata

  html += "<table cellspacing='4' width='100%'>"_L1;
  html += "<tr><td colspan='2'>"_L1;

  QString iconPath = metadata->value( u"icon"_s );

  if ( QFileInfo( iconPath ).isFile() || iconPath.startsWith( "http"_L1 ) )
  {
    if ( iconPath.startsWith( ":/"_L1 ) )
    {
      iconPath = "qrc" + iconPath;
    }
    else if ( !iconPath.startsWith( "http"_L1 ) )
    {
#if defined( Q_OS_WIN )
      iconPath = "file:///" + iconPath;
#else
      iconPath = "file://" + iconPath;
#endif
    }
    html += u"<img src=\"%1\" style=\"float:right;max-width:64px;max-height:64px;\">"_s.arg( iconPath );
  }

  const thread_local QRegularExpression stripHtml = QRegularExpression( u"&lt;[^\\s].*?&gt;"_s );

  QString name = metadata->value( u"name"_s );
  name = name.remove( stripHtml );
  html += u"<h1>%1</h1>"_s.arg( name );

  QString description = metadata->value( u"description"_s );
  description = description.remove( stripHtml );
  html += u"<h3>%1</h3>"_s.arg( description );

  if ( !metadata->value( u"about"_s ).isEmpty() )
  {
    QString about = metadata->value( u"about"_s );
    // The regular expression ensures that a new line will be present after the closure of a paragraph tag (i.e. </p>)
    const thread_local QRegularExpression pTagRe( u"&lt;\\/p&gt;([^\\n])"_s );
    about = about.replace( pTagRe, u"&lt;/p&gt;\n\\1"_s ).remove( stripHtml );
    html += about.replace( '\n', "<br/>"_L1 );
    html += "<br/><br/>"_L1;
  }

  QString votes;
  votes += tr( "Average rating %1" ).arg( metadata->value( "average_vote" ).toFloat(), 0, 'f', 1 );
  if ( !metadata->value( u"rating_votes"_s ).isEmpty() )
  {
    if ( !votes.isEmpty() )
      votes += ", "_L1;
    votes += tr( "%1 rating vote(s)" ).arg( metadata->value( u"rating_votes"_s ) );
  }
  if ( !metadata->value( u"downloads"_s ).isEmpty() )
  {
    if ( !votes.isEmpty() )
      votes += ", "_L1;
    votes += tr( "%1 downloads" ).arg( metadata->value( u"downloads"_s ) );
  }

  voteRating->setText( votes );

  html += "</td></tr>"_L1;
  html += "<tr><td width='1%'> </td><td width='99%'> </td></tr>"_L1;

  if ( !metadata->value( u"category"_s ).isEmpty() )
  {
    html += u"<tr><td class='key'>%1 </td><td>%2</td></tr>"_s.arg( tr( "Category" ), metadata->value( u"category"_s ) );
  }
  if ( !metadata->value( u"tags"_s ).isEmpty() )
  {
    QStringList tags = metadata->value( u"tags"_s ).toLower().split( ',' );
    for ( auto tag = tags.begin(); tag != tags.end(); ++tag )
    {
      *tag = u"<a href='rpc2://search.tag/%1/'>%1</a>"_s.arg( ( *tag ).trimmed() );
    }
    html += u"<tr><td class='key'>%1 </td><td>%2</td></tr>"_s.arg( tr( "Tags" ), tags.join( ", "_L1 ) );
  }

  if ( !metadata->value( u"homepage"_s ).isEmpty() || !metadata->value( u"tracker"_s ).isEmpty() || !metadata->value( u"code_repository"_s ).isEmpty() )
  {
    html += u"<tr><td class='key'>%1 </td><td>"_s.arg( tr( "More info" ) );
    if ( !metadata->value( u"homepage"_s ).isEmpty() )
    {
      html += u"<a href='%1'>%2</a> &nbsp; "_s.arg( metadata->value( u"homepage"_s ), tr( "homepage" ) );
    }
    if ( !metadata->value( u"tracker"_s ).isEmpty() )
    {
      html += u"<a href='%1'>%2</a> &nbsp; "_s.arg( metadata->value( u"tracker"_s ), tr( "bug tracker" ) );
    }
    if ( !metadata->value( u"code_repository"_s ).isEmpty() )
    {
      html += u"<a href='%1'>%2</a>"_s.arg( metadata->value( u"code_repository"_s ), tr( "code repository" ) );
    }
    html += "</td></tr>"_L1;
  }

  if ( !metadata->value( u"author_email"_s ).isEmpty() )
  {
    html += u"<tr><td class='key'>%1 </td><td><a href='mailto:%2'>%3</a></td></tr>"_s.arg( tr( "Author" ), metadata->value( u"author_email"_s ), metadata->value( u"author_name"_s ) );
  }
  else if ( !metadata->value( u"author_name"_s ).isEmpty() )
  {
    html += u"<tr><td class='key'>%1 </td><td>%2</td></tr>"_s.arg( tr( "Author" ), metadata->value( u"author_name"_s ) );
  }

  if ( !metadata->value( u"version_installed"_s ).isEmpty() )
  {
    QString ver = metadata->value( u"version_installed"_s );
    if ( ver == "-1"_L1 )
    {
      ver = '?';
    }
    QString localDir = metadata->value( u"library"_s );
    if ( QFileInfo( localDir ).isFile() )
    {
      localDir = QFileInfo( localDir ).canonicalFilePath();
    }
    else
    {
      localDir = QDir( localDir ).canonicalPath();
    }
    html += QStringLiteral( "<tr><td class='key'>%1 </td><td title='%2'><a href='%3'>%4</a></td></tr>"
    )
              .arg( tr( "Installed version" ), QDir::toNativeSeparators( localDir ), QUrl::fromLocalFile( localDir ).toString(), ver );
  }

  // use a localized date/time short format string
  QString dateTimeFormat = QLocale().dateTimeFormat( QLocale::FormatType::ShortFormat );
  if ( !dateTimeFormat.contains( "yyyy" ) )
  {
    // enforce year with 4 digits
    dateTimeFormat.replace( "yy", "yyyy" );
  }
  // if we allow experimental, we show both stable and experimental versions
  if ( !metadata->value( u"version_available_stable"_s ).isEmpty() )
  {
    QString downloadUrl = metadata->value( u"download_url_stable"_s );
    if ( downloadUrl.contains( u"plugins.qgis.org"_s ) )
    {
      // For the main repo, open the plugin version page instead of the download link. For other repositories the download link is the only known endpoint.
      downloadUrl = downloadUrl.replace( "download/"_L1, QString() );
    }

    QString dateUpdatedStr;
    if ( !metadata->value( u"update_date_stable"_s ).isEmpty() )
    {
      const QDateTime dateUpdatedUtc = QDateTime::fromString( metadata->value( u"update_date_stable"_s ).trimmed(), Qt::ISODate );
      if ( dateUpdatedUtc.isValid() )
      {
        const QDateTime dateUpdatedLocal = dateUpdatedUtc.toLocalTime();
        dateUpdatedStr += tr( "updated at %1 %2" ).arg( QLocale().toString( dateUpdatedLocal, dateTimeFormat ), dateUpdatedLocal.timeZoneAbbreviation() );
      }
    }

    html += QStringLiteral( "<tr><td class='key'>%1 </td><td title='%2'><a href='%2'>%3</a> %4</td></tr>"
    )
              .arg( tr( "Available version (stable)" ), downloadUrl, metadata->value( u"version_available_stable"_s ), dateUpdatedStr );
  }

  if ( !metadata->value( u"version_available_experimental"_s ).isEmpty() )
  {
    QString downloadUrl = metadata->value( u"download_url_experimental"_s );
    if ( downloadUrl.contains( u"plugins.qgis.org"_s ) )
    {
      // For the main repo, open the plugin version page instead of the download link. For other repositories the download link is the only known endpoint.
      downloadUrl = downloadUrl.replace( "download/"_L1, QString() );
    }

    QString dateUpdatedStr;
    if ( !metadata->value( u"update_date_experimental"_s ).isEmpty() )
    {
      const QDateTime dateUpdated = QDateTime::fromString( metadata->value( u"update_date_experimental"_s ).trimmed(), Qt::ISODate );
      if ( dateUpdated.isValid() )
        dateUpdatedStr += tr( "updated at %1" ).arg( QLocale().toString( dateUpdated, dateTimeFormat ) );
    }

    html += QStringLiteral( "<tr><td class='key'>%1 </td><td title='%2'><a href='%2'>%3</a> %4</td></tr>"
    )
              .arg( tr( "Available version (experimental)" ), downloadUrl, metadata->value( u"version_available_experimental"_s ), dateUpdatedStr );
  }

  if ( !metadata->value( u"changelog"_s ).isEmpty() )
  {
    QString changelog = metadata->value( u"changelog"_s );
    changelog = changelog.trimmed().replace( '\n', "<br/>"_L1 );
    html += u"<tr><td class='key'>%1 </td><td>%2</td></tr>"_s.arg( tr( "Changelog" ), changelog );
  }

  if ( !metadata->value( u"plugin_dependencies"_s ).isEmpty() )
  {
    QString pluginDependencies = metadata->value( u"plugin_dependencies"_s );
    pluginDependencies = pluginDependencies.trimmed();
    html += u"<tr><td class='key'>%1 </td><td>%2</td></tr>"_s.arg( tr( "Plugin dependencies" ), pluginDependencies );
  }

  html += "</table>"_L1;

  html += "</body>"_L1;

  wvDetails->setHtml( html );

  // Set buttonInstall text (and sometimes focus)
  buttonInstall->setDefault( false );
  if ( metadata->value( u"status"_s ) == "upgradeable"_L1 )
  {
    buttonInstall->setText( tr( "Upgrade Plugin" ) );
    buttonInstall->setToolTip( tr( "Upgrades selected plugin to latest stable version" ) );
    buttonInstall->setDefault( true );
  }
  else if ( metadata->value( u"status"_s ) == "newer"_L1 )
  {
    buttonInstall->setText( tr( "Downgrade Plugin" ) );
    buttonInstall->setToolTip( tr( "Downgrades selected plugin to latest stable version" ) );
  }
  else if ( metadata->value( u"status"_s ) == "not installed"_L1 || metadata->value( u"status"_s ) == "new"_L1 )
  {
    buttonInstall->setText( tr( "Install Plugin" ) );
    buttonInstall->setToolTip( tr( "Installs latest stable version of the selected plugin" ) );
  }
  else
  {
    // Default (will be grayed out if not available for reinstallation)
    buttonInstall->setText( tr( "Reinstall Plugin" ) );
    buttonInstall->setToolTip( tr( "Reinstalls latest stable version of the selected plugin" ) );
  }

  // Set buttonInstall text (and sometimes focus)
  buttonInstallExperimental->setDefault( false );
  if ( metadata->value( u"status_exp"_s ) == "upgradeable"_L1 )
  {
    buttonInstallExperimental->setText( tr( "Upgrade Experimental Plugin" ) );
    buttonInstallExperimental->setToolTip( tr( "Upgrades selected plugin to the experimental version" ) );
  }
  else if ( metadata->value( u"status_exp"_s ) == "newer"_L1 )
  {
    buttonInstallExperimental->setText( tr( "Downgrade Experimental Plugin" ) );
    buttonInstallExperimental->setToolTip( tr( "Downgrades selected plugin to the experimental version" ) );
  }
  else if ( metadata->value( u"status_exp"_s ) == "not installed"_L1 || metadata->value( u"status"_s ) == "new"_L1 )
  {
    buttonInstallExperimental->setText( tr( "Install Experimental Plugin" ) );
    buttonInstallExperimental->setToolTip( tr( "Installs experimental version of the selected plugin" ) );
  }
  else
  {
    // Default (will be grayed out if not available for reinstallation)
    buttonInstallExperimental->setText( tr( "Reinstall Experimental Plugin" ) );
    buttonInstallExperimental->setToolTip( tr( "Reinstalls experimental version of the selected plugin" ) );
  }

  // Enable/disable buttons
  const bool expAllowed = settingsAllowExperimental->value();

  const bool installEnabled = metadata->value( u"pythonic"_s ).toUpper() == "TRUE"_L1 && metadata->value( u"status"_s ) != "orphan"_L1 && metadata->value( u"status"_s ) != "none available"_L1;
  const bool installExpEnabled = metadata->value( u"pythonic"_s ).toUpper() == "TRUE"_L1 && metadata->value( u"status_exp"_s ) != "orphan"_L1 && metadata->value( u"status_exp"_s ) != "none available"_L1;

  buttonInstall->setEnabled( installEnabled );
  buttonInstall->setVisible( installEnabled || !installExpEnabled );
  buttonInstallExperimental->setEnabled( expAllowed && installExpEnabled );
  buttonInstallExperimental->setVisible( expAllowed && installExpEnabled );

  buttonUninstall->setEnabled( metadata->value( u"pythonic"_s ).toUpper() == "TRUE"_L1 && metadata->value( u"readonly"_s ) != "true"_L1 && !metadata->value( u"version_installed"_s ).isEmpty() );

  buttonUninstall->setHidden( metadata->value( u"version_installed"_s ).isEmpty() );

  // Store the id of the currently displayed plugin
  mCurrentlyDisplayedPlugin = metadata->value( u"id"_s );
}


void QgsPluginManager::selectTabItem( int idx )
{
  mOptionsListWidget->setCurrentRow( idx );
}


void QgsPluginManager::clearPythonPluginMetadata()
{
  for ( QMap<QString, QMap<QString, QString>>::iterator it = mPlugins.begin();
        it != mPlugins.end(); )
  {
    if ( it->value( u"pythonic"_s ) == "true"_L1 )
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
  const QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.find( key );
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
    actionClearFilter->setEnabled( repository.value( u"inspection_filter"_s ) == "true"_L1 );
    treeRepositories->addAction( actionClearFilter );
    connect( actionClearFilter, &QAction::triggered, this, &QgsPluginManager::clearRepositoryFilter );
  }

  const QString key = repository.value( u"name"_s );
  if ( !key.isEmpty() )
  {
    QTreeWidgetItem *a = new QTreeWidgetItem( treeRepositories );
    a->setText( 1, key );
    a->setText( 2, repository.value( u"url"_s ) );
    if ( repository.value( u"enabled"_s ) == "true"_L1 && repository.value( u"valid"_s ) == "true"_L1 )
    {
      if ( repository.value( u"state"_s ) == "2"_L1 )
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
      if ( repository.value( u"valid"_s ) == "true"_L1 )
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


// Slots ///////////////////////////////////////////////////////////////////


// "Close" button clicked
void QgsPluginManager::reject()
{
#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    settingsAutomaticallyCheckForPluginUpdates->setValue( ckbCheckUpdates->isChecked() );
    QgsPythonRunner::run( u"pyplugin_installer.instance().onManagerClose()"_s );
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
        acceptedStatuses << u"installed"_s << u"not installed"_s << u"new"_s << u"orphan"_s << u"none available"_s << u"newer"_s << u"upgradeable"_s << u"not installedZ"_s << u"installedZ"_s << u"upgradeableZ"_s << u"orphanZ"_s << u"newerZZ"_s << QString();
        tabTitle = u"all_plugins"_s;
        break;
      case PLUGMAN_TAB_INSTALLED:
        // installed (statuses ends with Z are for spacers to always sort properly)
        acceptedStatuses << u"installed"_s << u"orphan"_s << u"newer"_s << u"upgradeable"_s << u"installedZ"_s << u"upgradeableZ"_s << u"orphanZ"_s << u"newerZZ"_s << QString();
        tabTitle = u"installed_plugins"_s;
        break;
      case PLUGMAN_TAB_NOT_INSTALLED:
        // not installed (get more)
        acceptedStatuses << u"not installed"_s << u"new"_s;
        tabTitle = u"not_installed_plugins"_s;
        break;
      case PLUGMAN_TAB_UPGRADEABLE:
        // upgradeable
        acceptedStatuses << u"upgradeable"_s;
        tabTitle = u"upgradeable_plugins"_s;
        break;
      case PLUGMAN_TAB_NEW:
        // new
        acceptedStatuses << u"new"_s;
        tabTitle = u"new_plugins"_s;
        break;
      case PLUGMAN_TAB_INVALID:
        // invalid
        acceptedStatuses << u"invalid"_s;
        tabTitle = u"invalid_plugins"_s;
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
                     "      color: "
                     + palette().color( QPalette::ColorRole::Text ).name() + ";"
                                                                             "      background-color:"
                     + palette().color( QPalette::ColorRole::Base ).name() + ";"
                                                                             "      margin: 2px;"
                                                                             "      font-family: Verdana, Sans-serif;"
                                                                             "      font-size: 10pt;"
                                                                             "  }"
                                                                             "  a, a:hover {"
                                                                             "      color: "
                     + palette().color( QPalette::ColorRole::Link ).name() + ";"
                                                                             "  }"
                                                                             "  a:visited {"
                                                                             "      color: "
                     + palette().color( QPalette::ColorRole::LinkVisited ).name() + ";"
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

void QgsPluginManager::submitVote()
{
  if ( mCurrentPluginId < 0 )
    return;

  sendVote( mCurrentPluginId, voteSlider->value() );
}

void QgsPluginManager::sendVote( int pluginId, int vote )
{
  QString response;
  QgsPythonRunner::eval( u"pyplugin_installer.instance().sendVote('%1', '%2')"_s.arg( pluginId ).arg( vote ), response );
  if ( response == "True"_L1 )
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
  if ( url.scheme() == "rpc2"_L1 )
  {
    if ( url.host() == "plugin.vote"_L1 )
    {
      QStringList params = url.path().split( '/' );
      sendVote( params[1].toInt(), params[2].toInt() );
    }
    else if ( url.host() == "search.tag"_L1 )
    {
      QStringList params = url.path().split( '/' );
      leFilter->setText( u"tag:%1"_s.arg( params[1] ) );
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
  if ( text.startsWith( "tag:"_L1, Qt::CaseInsensitive ) )
  {
    text = text.remove( u"tag:"_s );
    mModelProxy->setFilterRole( PLUGIN_TAGS_ROLE );
    QgsDebugMsgLevel( "PluginManager TAG filter changed to :" + text, 3 );
  }
  else
  {
    mModelProxy->setFilterRole( 0 );
    QgsDebugMsgLevel( "PluginManager filter changed to :" + text, 3 );
  }

  const QRegularExpression filterRegExp( text, QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( filterRegExp );
}

void QgsPluginManager::buttonUpgradeAll_clicked()
{
  QgsPythonRunner::run( u"pyplugin_installer.instance().upgradeAllUpgradeable()"_s );
}

void QgsPluginManager::buttonInstall_clicked()
{
  QgsPythonRunner::run( u"pyplugin_installer.instance().installPlugin('%1', stable=True)"_s.arg( mCurrentlyDisplayedPlugin ) );
}


void QgsPluginManager::buttonInstallExperimental_clicked()
{
  QgsPythonRunner::run( u"pyplugin_installer.instance().installPlugin('%1', stable=False)"_s.arg( mCurrentlyDisplayedPlugin ) );
}


void QgsPluginManager::buttonUninstall_clicked()
{
  QgsPythonRunner::run( u"pyplugin_installer.instance().uninstallPlugin('%1')"_s.arg( mCurrentlyDisplayedPlugin ) );
}


void QgsPluginManager::mZipFileWidget_fileChanged( const QString &filePath )
{
  buttonInstallFromZip->setEnabled( QFileInfo( filePath ).isFile() );
}


void QgsPluginManager::buttonInstallFromZip_clicked()
{
  const bool showInstallFromZipWarning = settingsShowInstallFromZipWarning->value();

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
    settingsShowInstallFromZipWarning->setValue( !msgbox.checkBox()->isChecked() );
  }

  if ( !showInstallFromZipWarning || msgbox.result() == QMessageBox::Yes )
  {
    QgsPythonRunner::run( u"pyplugin_installer.instance().installFromZipFile(r'''%1''')"_s.arg( mZipFileWidget->filePath() ) );
    mZipFileWidget->setFilePath( QString() );
  }
}


void QgsPluginManager::treeRepositories_itemSelectionChanged()
{
  buttonEditRep->setEnabled( !treeRepositories->selectedItems().isEmpty() );
  buttonDeleteRep->setEnabled( !treeRepositories->selectedItems().isEmpty() );
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
    key = key.replace( '\'', "\\\'"_L1 ).replace( '\"', "\\\""_L1 );
    QgsDebugMsgLevel( "Disabling all repositories but selected: " + key, 2 );
    QgsPythonRunner::run( u"pyplugin_installer.instance().setRepositoryInspectionFilter('%1')"_s.arg( key ) );
  }
}


void QgsPluginManager::clearRepositoryFilter()
{
  QgsDebugMsgLevel( u"Enabling all repositories back"_s, 2 );
  QgsPythonRunner::run( u"pyplugin_installer.instance().setRepositoryInspectionFilter()"_s );
}


void QgsPluginManager::buttonRefreshRepos_clicked()
{
  QgsDebugMsgLevel( u"Refreshing repositories..."_s, 2 );
  QgsPythonRunner::run( u"pyplugin_installer.instance().reloadAndExportData()"_s );
}


void QgsPluginManager::buttonAddRep_clicked()
{
  QgsDebugMsgLevel( u"Adding repository connection..."_s, 2 );
  QgsPythonRunner::run( u"pyplugin_installer.instance().addRepository()"_s );
}


void QgsPluginManager::buttonEditRep_clicked()
{
  QTreeWidgetItem *current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( '\'', "\\\'"_L1 ).replace( '\"', "\\\""_L1 );
    QgsDebugMsgLevel( "Editing repository connection: " + key, 2 );
    QgsPythonRunner::run( u"pyplugin_installer.instance().editRepository('%1')"_s.arg( key ) );
  }
}


void QgsPluginManager::buttonDeleteRep_clicked()
{
  QTreeWidgetItem *current = treeRepositories->currentItem();
  if ( current )
  {
    QString key = current->text( 1 );
    key = key.replace( '\'', "\\\'"_L1 ).replace( '\"', "\\\""_L1 );
    QgsDebugMsgLevel( "Deleting repository connection: " + key, 2 );
    QgsPythonRunner::run( u"pyplugin_installer.instance().deleteRepository('%1')"_s.arg( key ) );
  }
}


void QgsPluginManager::ckbExperimental_toggled( bool state )
{
  settingsAllowExperimental->setValue( state );
  QgsPythonRunner::run( u"pyplugin_installer.installer_data.plugins.rebuild()"_s );
  QgsPythonRunner::run( u"pyplugin_installer.instance().exportPluginsToManager()"_s );
}

void QgsPluginManager::ckbDeprecated_toggled( bool state )
{
  settingsAllowDeprecated->setValue( state );
  QgsPythonRunner::run( u"pyplugin_installer.installer_data.plugins.rebuild()"_s );
  QgsPythonRunner::run( u"pyplugin_installer.instance().exportPluginsToManager()"_s );
}


// PRIVATE METHODS ///////////////////////////////////////////////////////////////////


bool QgsPluginManager::isPluginEnabled( const QString &key )
{
  const QMap<QString, QString> *plugin = pluginMetadata( key );
  if ( !plugin || plugin->isEmpty() )
  {
    // No such plugin in the metadata registry
    return false;
  }

  const QgsSettings mySettings;
  if ( plugin->value( u"pythonic"_s ) != "true"_L1 )
  {
    // Trim "cpp:" prefix from cpp plugin id
    const QString trimmedKey = key.mid( 4 );
    return ( mySettings.value( "/Plugins/" + trimmedKey, QVariant( false ) ).toBool() );
  }
  else
  {
    return ( plugin->value( u"installed"_s ) == "true"_L1 && mySettings.value( "/PythonPlugins/" + key, QVariant( false ) ).toBool() );
  }
}


bool QgsPluginManager::hasAvailablePlugins()
{
  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( u"status"_s ) == "not installed"_L1 || it->value( u"status"_s ) == "new"_L1 )
    {
      return true;
    }
  }

  return false;
}


bool QgsPluginManager::hasReinstallablePlugins()
{
  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    // plugins marked as "installed" are available for download (otherwise they are marked "orphans")
    if ( it->value( u"status"_s ) == "installed"_L1 )
    {
      return true;
    }
  }

  return false;
}


bool QgsPluginManager::hasUpgradeablePlugins()
{
  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( u"status"_s ) == "upgradeable"_L1 )
    {
      return true;
    }
  }

  return false;
}


bool QgsPluginManager::hasNewPlugins()
{
  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( u"status"_s ) == "new"_L1 )
    {
      return true;
    }
  }

  return false;
}


bool QgsPluginManager::hasNewerPlugins()
{
  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( it->value( u"status"_s ) == "newer"_L1 )
    {
      return true;
    }
  }

  return false;
}


bool QgsPluginManager::hasInvalidPlugins()
{
  for ( QMap<QString, QMap<QString, QString>>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    if ( !it->value( u"error"_s ).isEmpty() )
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
    QString title = u"%1 | %2"_s.arg( tr( "Plugins" ), curitem->text() );
    if ( mOptionsListWidget->currentRow() < mOptionsListWidget->count() - 2 && mModelPlugins )
    {
      // if it's not the Settings tab, add the plugin count
      title += u" (%3)"_s.arg( mModelProxy->countWithCurrentStatus() );
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
  QgsHelp::openHelp( u"plugins/plugins.html"_s );
}
