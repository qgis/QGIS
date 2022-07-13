/***************************************************************************
    begin                : July 13, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthoauth2edit.h"
#include "ui_qgsauthoauth2edit.h"

#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

#include "Json.h"

#include "qgsapplication.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"

QgsAuthOAuth2Edit::QgsAuthOAuth2Edit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
  , mDefinedConfigsCache( QgsStringMap() )
{
  setupUi( this );

  initGui();

  initConfigObjs();

  populateGrantFlows();
  updateGrantFlow( static_cast<int>( QgsAuthOAuth2Config::AuthCode ) ); // first index: Authorization Code

  populateAccessMethods();

  queryTableSelectionChanged();

  loadDefinedConfigs();

  setupConnections();

  loadFromOAuthConfig( mOAuthConfigCustom.get() );
  updatePredefinedLocationsTooltip();

  pteDefinedDesc->setOpenLinks( false );
  connect( pteDefinedDesc, &QTextBrowser::anchorClicked, this, [ = ]( const QUrl & url )
  {
    QDesktopServices::openUrl( url );
  } );
}


void QgsAuthOAuth2Edit::initGui()
{
  mParentName = parentNameField();

  frameNotify->setVisible( false );

  // TODO: add messagebar to notify frame?

  tabConfigs->setCurrentIndex( customTab() );

  btnExport->setEnabled( false );

  chkbxTokenPersist->setChecked( false );

  grpbxAdvanced->setCollapsed( true );
  grpbxAdvanced->setFlat( false );

  btnTokenClear = new QToolButton( this );
  btnTokenClear->setObjectName( QStringLiteral( "btnTokenClear" ) );
  btnTokenClear->setMaximumHeight( 20 );
  btnTokenClear->setText( tr( "Tokens" ) );
  btnTokenClear->setToolTip( tr( "Remove cached tokens" ) );
  btnTokenClear->setIcon( QIcon( QStringLiteral( ":/oauth2method/svg/close.svg" ) ) );
  btnTokenClear->setIconSize( QSize( 12, 12 ) );
  btnTokenClear->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  btnTokenClear->setEnabled( hasTokenCacheFile() );

  connect( btnTokenClear, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::removeTokenCacheFile );
  tabConfigs->setCornerWidget( btnTokenClear, Qt::TopRightCorner );
}

QWidget *QgsAuthOAuth2Edit::parentWidget() const
{
  if ( !window() )
  {
    return nullptr;
  }

  const QMetaObject *metaObject = window()->metaObject();
  const QString parentclass = metaObject->className();
  //QgsDebugMsg( QStringLiteral( "parent class: %1" ).arg( parentclass ) );
  if ( parentclass != QLatin1String( "QgsAuthConfigEdit" ) )
  {
    QgsDebugMsg( QStringLiteral( "Parent widget not QgsAuthConfigEdit instance" ) );
    return nullptr;
  }

  return window();
}

QLineEdit *QgsAuthOAuth2Edit::parentNameField() const
{
  return parentWidget() ? parentWidget()->findChild<QLineEdit *>( QStringLiteral( "leName" ) ) : nullptr;
}

QString QgsAuthOAuth2Edit::parentConfigId() const
{
  if ( !parentWidget() )
  {
    return QString();
  }

  QgsAuthConfigEdit *cie = qobject_cast<QgsAuthConfigEdit *>( parentWidget() );
  if ( !cie )
  {
    QgsDebugMsg( QStringLiteral( "Could not cast to QgsAuthConfigEdit" ) );
    return QString();
  }

  if ( cie->configId().isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "QgsAuthConfigEdit->configId() is empty" ) );
  }

  return cie->configId();
}


void QgsAuthOAuth2Edit::setupConnections()
{
  // Action and interaction connections
  connect( tabConfigs, &QTabWidget::currentChanged, this, &QgsAuthOAuth2Edit::tabIndexChanged );

  connect( btnExport, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::exportOAuthConfig );
  connect( btnImport, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::importOAuthConfig );

  connect( tblwdgQueryPairs, &QTableWidget::itemSelectionChanged, this, &QgsAuthOAuth2Edit::queryTableSelectionChanged );

  connect( btnAddQueryPair, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::addQueryPair );
  connect( btnRemoveQueryPair, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::removeQueryPair );

  connect( lstwdgDefinedConfigs, &QListWidget::currentItemChanged, this, &QgsAuthOAuth2Edit::currentDefinedItemChanged );

  connect( btnGetDefinedDirPath, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::getDefinedCustomDir );
  connect( leDefinedDirPath, &QLineEdit::textChanged, this, &QgsAuthOAuth2Edit::definedCustomDirChanged );

  connect( btnSoftStatementDir, &QToolButton::clicked, this, &QgsAuthOAuth2Edit::getSoftStatementDir );
  connect( leSoftwareStatementJwtPath, &QLineEdit::textChanged, this, &QgsAuthOAuth2Edit::softwareStatementJwtPathChanged );
  connect( leSoftwareStatementConfigUrl, &QLineEdit::textChanged, this, [ = ]( const QString & txt )
  {
    btnRegister->setEnabled( ! leSoftwareStatementJwtPath->text().isEmpty()
                             && ( QUrl( txt ).isValid() || ! mRegistrationEndpoint.isEmpty() ) );
  } );
  connect( btnRegister, &QPushButton::clicked, this, &QgsAuthOAuth2Edit::getSoftwareStatementConfig );

  // Custom config editing connections
  connect( cmbbxGrantFlow, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsAuthOAuth2Edit::updateGrantFlow ); // also updates GUI
  connect( pteDescription, &QPlainTextEdit::textChanged, this, &QgsAuthOAuth2Edit::descriptionChanged );
  connect( leRequestUrl, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setRequestUrl );
  connect( leTokenUrl, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setTokenUrl );
  connect( leRefreshTokenUrl, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setRefreshTokenUrl );
  connect( leRedirectUrl, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setRedirectUrl );
  connect( spnbxRedirectPort, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ),
           mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setRedirectPort );
  connect( leClientId, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setClientId );
  connect( leClientSecret, &QgsPasswordLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setClientSecret );
  connect( leUsername, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setUsername );
  connect( lePassword, &QgsPasswordLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setPassword );
  connect( leScope, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setScope );
  connect( leApiKey, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setApiKey );
  connect( chkbxTokenPersist, &QCheckBox::toggled, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setPersistToken );
  connect( cmbbxAccessMethod, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsAuthOAuth2Edit::updateConfigAccessMethod );
  connect( spnbxRequestTimeout, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ),
           mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setRequestTimeout );

  connect( mTokenHeaderLineEdit, &QLineEdit::textChanged, mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::setCustomHeader );

  connect( mOAuthConfigCustom.get(), &QgsAuthOAuth2Config::validityChanged, this, &QgsAuthOAuth2Edit::configValidityChanged );

  if ( mParentName )
  {
    connect( mParentName, &QLineEdit::textChanged, this, &QgsAuthOAuth2Edit::configValidityChanged );
  }
}


void QgsAuthOAuth2Edit::configValidityChanged()
{
  validateConfig();
  const bool parentname = mParentName && !mParentName->text().isEmpty();
  btnExport->setEnabled( mValid && parentname );
}

bool QgsAuthOAuth2Edit::validateConfig()
{
  const bool curvalid = ( onCustomTab() ? mOAuthConfigCustom->isValid() : !mDefinedId.isEmpty() );
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthOAuth2Edit::configMap() const
{
  QgsStringMap configmap;
  bool ok = false;

  if ( onCustomTab() )
  {
    if ( !mOAuthConfigCustom || !mOAuthConfigCustom->isValid() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to serialize OAuth config object: null or invalid object" ) );
      return configmap;
    }

    mOAuthConfigCustom->setQueryPairs( queryPairs() );

    const QByteArray configtxt = mOAuthConfigCustom->saveConfigTxt( QgsAuthOAuth2Config::JSON, false, &ok );

    if ( !ok )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to serialize OAuth config object" ) );
      return configmap;
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to serialize OAuth config object: content empty" ) );
      return configmap;
    }

    //###################### DO NOT LEAVE ME UNCOMMENTED #####################
    //QgsDebugMsg( QStringLiteral( "SAVE oauth2config configtxt: \n\n%1\n\n" ).arg( QString( configtxt ) ) );
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################

    configmap.insert( QStringLiteral( "oauth2config" ), QString( configtxt ) );

    updateTokenCacheFile( mOAuthConfigCustom->persistToken() );
  }
  else if ( onDefinedTab() && !mDefinedId.isEmpty() )
  {
    configmap.insert( QStringLiteral( "definedid" ), mDefinedId );
    configmap.insert( QStringLiteral( "defineddirpath" ), leDefinedDirPath->text() );
    configmap.insert( QStringLiteral( "querypairs" ),
                      QgsAuthOAuth2Config::serializeFromVariant(
                        queryPairs(), QgsAuthOAuth2Config::JSON, false ) );
  }

  return configmap;
}

void QgsAuthOAuth2Edit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  bool ok = false;

  //QgsDebugMsg( QStringLiteral( "oauth2config: " ).arg( configmap.value( QStringLiteral( "oauth2config" ) ) ) );

  if ( configmap.contains( QStringLiteral( "oauth2config" ) ) )
  {
    tabConfigs->setCurrentIndex( customTab() );
    const QByteArray configtxt = configmap.value( QStringLiteral( "oauth2config" ) ).toUtf8();
    if ( !configtxt.isEmpty() )
    {
      //###################### DO NOT LEAVE ME UNCOMMENTED #####################
      //QgsDebugMsg( QStringLiteral( "LOAD oauth2config configtxt: \n\n%1\n\n" ).arg( QString( configtxt ) ) );
      //###################### DO NOT LEAVE ME UNCOMMENTED #####################

      if ( !mOAuthConfigCustom->loadConfigTxt( configtxt, QgsAuthOAuth2Config::JSON ) )
      {
        QgsDebugMsg( QStringLiteral( "FAILED to load OAuth2 config into object" ) );
      }

      //###################### DO NOT LEAVE ME UNCOMMENTED #####################
      //QVariantMap vmap = mOAuthConfigCustom->mappedProperties();
      //QByteArray vmaptxt = QgsAuthOAuth2Config::serializeFromVariant(vmap, QgsAuthOAuth2Config::JSON, true );
      //QgsDebugMsg( QStringLiteral( "LOAD oauth2config vmaptxt: \n\n%1\n\n" ).arg( QString( vmaptxt ) ) );
      //###################### DO NOT LEAVE ME UNCOMMENTED #####################

      // could only be loading defaults at this point
      loadFromOAuthConfig( mOAuthConfigCustom.get() );

      mPrevPersistToken = mOAuthConfigCustom->persistToken();
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load OAuth2 config: empty config txt" ) );
    }
  }
  else if ( configmap.contains( QStringLiteral( "definedid" ) ) )
  {
    tabConfigs->setCurrentIndex( definedTab() );
    const QString definedid = configmap.value( QStringLiteral( "definedid" ) );
    setCurrentDefinedConfig( definedid );
    if ( !definedid.isEmpty() )
    {
      if ( !configmap.value( QStringLiteral( "defineddirpath" ) ).isEmpty() )
      {
        // this will trigger a reload of dirs and a reselection of any existing defined id
        leDefinedDirPath->setText( configmap.value( QStringLiteral( "defineddirpath" ) ) );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "No custom defined dir path to load OAuth2 config" ) );
        selectCurrentDefinedConfig();
      }

      const QByteArray querypairstxt = configmap.value( QStringLiteral( "querypairs" ) ).toUtf8();
      if ( !querypairstxt.isNull() && !querypairstxt.isEmpty() )
      {
        const QVariantMap querypairsmap =
          QgsAuthOAuth2Config::variantFromSerialized( querypairstxt, QgsAuthOAuth2Config::JSON, &ok );
        if ( ok )
        {
          populateQueryPairs( querypairsmap );
        }
        else
        {
          QgsDebugMsg( QStringLiteral( "No query pairs to load OAuth2 config: failed to parse" ) );
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "No query pairs to load OAuth2 config: empty text" ) );
      }
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load a defined ID for OAuth2 config" ) );
    }
  }

  validateConfig();
}

void QgsAuthOAuth2Edit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthOAuth2Edit::clearConfig()
{
  // restore defaults to config objs
  mOAuthConfigCustom->setToDefaults();

  mDefinedId.clear();

  clearQueryPairs();

  // clear any set predefined location
  leDefinedDirPath->clear();

  // reload predefined table
  loadDefinedConfigs();

  loadFromOAuthConfig( mOAuthConfigCustom.get() );
}

void QgsAuthOAuth2Edit::loadFromOAuthConfig( const QgsAuthOAuth2Config *config )
{
  if ( !config )
  {
    return;
  }

  // load relative to config type
  if ( config->configType() == QgsAuthOAuth2Config::Custom )
  {
    if ( config->isValid() )
    {
      tabConfigs->setCurrentIndex( customTab() );
    }
    pteDescription->setPlainText( config->description() );
    leRequestUrl->setText( config->requestUrl() );
    leTokenUrl->setText( config->tokenUrl() );
    leRefreshTokenUrl->setText( config->refreshTokenUrl() );
    leRedirectUrl->setText( config->redirectUrl() );
    spnbxRedirectPort->setValue( config->redirectPort() );
    leClientId->setText( config->clientId() );
    leClientSecret->setText( config->clientSecret() );
    leUsername->setText( config->username() );
    lePassword->setText( config->password() );
    leScope->setText( config->scope() );
    leApiKey->setText( config->apiKey() );
    mTokenHeaderLineEdit->setText( config->customHeader() );

    // advanced
    chkbxTokenPersist->setChecked( config->persistToken() );
    cmbbxAccessMethod->setCurrentIndex( static_cast<int>( config->accessMethod() ) );
    spnbxRequestTimeout->setValue( config->requestTimeout() );

    populateQueryPairs( config->queryPairs() );

    updateGrantFlow( static_cast<int>( config->grantFlow() ) );
  }

  validateConfig();
}

void QgsAuthOAuth2Edit::updateTokenCacheFile( bool curpersist ) const
{
  // default for unset persistToken in config and edit GUI is false
  if ( mPrevPersistToken == curpersist )
  {
    return;
  }

  if ( !parent() )
  {
    QgsDebugMsg( QStringLiteral( "Edit widget has no parent" ) );
    return;
  }

  const QString authcfg = parentConfigId();
  if ( authcfg.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Auth config ID empty in ID widget of parent" ) );
    return;
  }

  const QString localcachefile = QgsAuthOAuth2Config::tokenCachePath( authcfg, false );

  const QString tempcachefile = QgsAuthOAuth2Config::tokenCachePath( authcfg, true );

  //QgsDebugMsg( QStringLiteral( "localcachefile: %1" ).arg( localcachefile ) );
  //QgsDebugMsg( QStringLiteral( "tempcachefile: %1" ).arg( tempcachefile ) );

  if ( curpersist )
  {
    // move cache file from temp dir to local
    if ( QFile::exists( localcachefile ) && !QFile::remove( localcachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to delete local token cache file: %1" ).arg( localcachefile ) );
      return;
    }
    if ( QFile::exists( tempcachefile ) && !QFile::copy( tempcachefile, localcachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to copy temp to local token cache file: %1 -> %2" ).arg( tempcachefile, localcachefile ) );
      return;
    }
    if ( QFile::exists( tempcachefile ) && !QFile::remove( tempcachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to delete temp token cache file after copy: %1" ).arg( tempcachefile ) );
      return;
    }
  }
  else
  {
    // move cache file from local to temp
    if ( QFile::exists( tempcachefile ) && !QFile::remove( tempcachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to delete temp token cache file: %1" ).arg( tempcachefile ) );
      return;
    }
    if ( QFile::exists( localcachefile ) && !QFile::copy( localcachefile, tempcachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to copy local to temp token cache file: %1 -> %2" ).arg( localcachefile, tempcachefile ) );
      return;
    }
    if ( QFile::exists( localcachefile ) && !QFile::remove( localcachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to delete temp token cache file after copy: %1" ).arg( localcachefile ) );
      return;
    }
  }
}


void QgsAuthOAuth2Edit::tabIndexChanged( int indx )
{
  mCurTab = indx;
  validateConfig();
}


void QgsAuthOAuth2Edit::populateGrantFlows()
{
  cmbbxGrantFlow->addItem( QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::AuthCode ),
                           static_cast<int>( QgsAuthOAuth2Config::AuthCode ) );
  cmbbxGrantFlow->addItem( QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::Implicit ),
                           static_cast<int>( QgsAuthOAuth2Config::Implicit ) );
  cmbbxGrantFlow->addItem( QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::ResourceOwner ),
                           static_cast<int>( QgsAuthOAuth2Config::ResourceOwner ) );
}


void QgsAuthOAuth2Edit::definedCustomDirChanged( const QString &path )
{
  const QFileInfo pinfo( path );
  const bool ok = pinfo.exists() || pinfo.isDir();

  leDefinedDirPath->setStyleSheet( ok ? QString() : QgsAuthGuiUtils::redTextStyleSheet() );
  updatePredefinedLocationsTooltip();

  if ( ok )
  {
    loadDefinedConfigs();
  }
}


void QgsAuthOAuth2Edit::softwareStatementJwtPathChanged( const QString &path )
{
  const QFileInfo pinfo( path );
  const bool ok = pinfo.exists() || pinfo.isFile();

  leSoftwareStatementJwtPath->setStyleSheet( ok ? QString() : QgsAuthGuiUtils::redTextStyleSheet() );

  if ( ok )
  {
    parseSoftwareStatement( path );
  }
}


void QgsAuthOAuth2Edit::setCurrentDefinedConfig( const QString &id )
{
  mDefinedId = id;
  QgsDebugMsg( QStringLiteral( "Set defined ID: %1" ).arg( id ) );
  validateConfig();
}

void QgsAuthOAuth2Edit::currentDefinedItemChanged( QListWidgetItem *cur, QListWidgetItem *prev )
{
  Q_UNUSED( prev )

  QgsDebugMsg( QStringLiteral( "Entered" ) );

  const QString id = cur->data( Qt::UserRole ).toString();
  if ( !id.isEmpty() )
  {
    setCurrentDefinedConfig( id );
  }
}


void QgsAuthOAuth2Edit::selectCurrentDefinedConfig()
{
  if ( mDefinedId.isEmpty() )
  {
    return;
  }

  if ( !onDefinedTab() )
  {
    tabConfigs->setCurrentIndex( definedTab() );
  }

  lstwdgDefinedConfigs->selectionModel()->clearSelection();

  for ( int i = 0; i < lstwdgDefinedConfigs->count(); ++i )
  {
    QListWidgetItem *itm = lstwdgDefinedConfigs->item( i );

    if ( itm->data( Qt::UserRole ).toString() == mDefinedId )
    {
      lstwdgDefinedConfigs->setCurrentItem( itm, QItemSelectionModel::Select );
      break;
    }
  }
}

void QgsAuthOAuth2Edit::getDefinedCustomDir()
{
  const QString extradir = QFileDialog::getExistingDirectory( this, tr( "Select extra directory to parse" ),
                           QDir::homePath(), QFileDialog::DontResolveSymlinks );
  this->raise();
  this->activateWindow();

  if ( extradir.isEmpty() )
  {
    return;
  }
  leDefinedDirPath->setText( extradir );
}

void QgsAuthOAuth2Edit::getSoftStatementDir()
{
  const QString softStatementFile = QFileDialog::getOpenFileName( this, tr( "Select software statement file" ),
                                    QDir::homePath(), tr( "JSON Web Token (*.jwt)" ) );
  this->raise();
  this->activateWindow();

  if ( softStatementFile.isEmpty() )
  {
    return;
  }
  leSoftwareStatementJwtPath->setText( softStatementFile );
}

void QgsAuthOAuth2Edit::initConfigObjs()
{
  mOAuthConfigCustom = std::make_unique<QgsAuthOAuth2Config>( nullptr );
  mOAuthConfigCustom->setConfigType( QgsAuthOAuth2Config::Custom );
  mOAuthConfigCustom->setToDefaults();
}


bool QgsAuthOAuth2Edit::hasTokenCacheFile()
{
  const QString authcfg = parentConfigId();
  if ( authcfg.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Auth config ID empty in ID widget of parent" ) );
    return false;
  }

  return ( QFile::exists( QgsAuthOAuth2Config::tokenCachePath( authcfg, false ) )
           || QFile::exists( QgsAuthOAuth2Config::tokenCachePath( authcfg, true ) ) );
}

//slot
void QgsAuthOAuth2Edit::removeTokenCacheFile()
{
  const QString authcfg = parentConfigId();
  if ( authcfg.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Auth config ID empty in ID widget of parent" ) );
    return;
  }

  const QStringList cachefiles = QStringList()
                                 << QgsAuthOAuth2Config::tokenCachePath( authcfg, false )
                                 << QgsAuthOAuth2Config::tokenCachePath( authcfg, true );

  for ( const QString &cachefile : cachefiles )
  {
    if ( QFile::exists( cachefile ) && !QFile::remove( cachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "Remove token cache file FAILED for authcfg %1: %2" ).arg( authcfg, cachefile ) );
    }
  }
  btnTokenClear->setEnabled( hasTokenCacheFile() );
}

void QgsAuthOAuth2Edit::updateDefinedConfigsCache()
{
  const QString extradir = leDefinedDirPath->text();
  mDefinedConfigsCache.clear();
  mDefinedConfigsCache = QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( this, extradir );
}

void QgsAuthOAuth2Edit::loadDefinedConfigs()
{
  whileBlocking( lstwdgDefinedConfigs )->clear();
  updateDefinedConfigsCache();
  updatePredefinedLocationsTooltip();

  QgsStringMap::const_iterator i = mDefinedConfigsCache.constBegin();
  while ( i != mDefinedConfigsCache.constEnd() )
  {
    QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( this );
    if ( !config->loadConfigTxt( i.value().toUtf8(), QgsAuthOAuth2Config::JSON ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load config for ID: %1" ).arg( i.key() ) );
      config->deleteLater();
      continue;
    }

    const QString grantflow = QgsAuthOAuth2Config::grantFlowString( config->grantFlow() );

    const QString name = QStringLiteral( "%1 (%2): %3" )
                         .arg( config->name(), grantflow, config->description() );

    const QString tip = tr( "ID: %1\nGrant flow: %2\nDescription: %3" )
                        .arg( i.key(), grantflow, config->description() );

    QListWidgetItem *itm = new QListWidgetItem( lstwdgDefinedConfigs );
    itm->setText( name );
    itm->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    itm->setData( Qt::UserRole, QVariant( i.key() ) );
    itm->setData( Qt::ToolTipRole, QVariant( tip ) );
    lstwdgDefinedConfigs->addItem( itm );

    config->deleteLater();
    ++i;
  }

  if ( lstwdgDefinedConfigs->count() == 0 )
  {
    QListWidgetItem *itm = new QListWidgetItem( lstwdgDefinedConfigs );
    itm->setText( tr( "No predefined configurations found on disk" ) );
    QFont f( itm->font() );
    f.setItalic( true );
    itm->setFont( f );
    itm->setFlags( Qt::NoItemFlags );
    lstwdgDefinedConfigs->addItem( itm );
  }

  selectCurrentDefinedConfig();
}

bool QgsAuthOAuth2Edit::onCustomTab() const
{
  return mCurTab == customTab();
}

bool QgsAuthOAuth2Edit::onDefinedTab() const
{
  return mCurTab == definedTab();
}

bool QgsAuthOAuth2Edit::onStatementTab() const
{
  return mCurTab == statementTab();
}

void QgsAuthOAuth2Edit::updateGrantFlow( int indx )
{
  if ( cmbbxGrantFlow->currentIndex() != indx )
  {
    whileBlocking( cmbbxGrantFlow )->setCurrentIndex( indx );
  }

  const QgsAuthOAuth2Config::GrantFlow flow =
    static_cast<QgsAuthOAuth2Config::GrantFlow>( cmbbxGrantFlow->itemData( indx ).toInt() );
  mOAuthConfigCustom->setGrantFlow( flow );

  // bool authcode = ( flow == QgsAuthOAuth2Config::AuthCode );
  const bool implicit = ( flow == QgsAuthOAuth2Config::Implicit );
  const bool resowner = ( flow == QgsAuthOAuth2Config::ResourceOwner );

  lblRequestUrl->setVisible( !resowner );
  leRequestUrl->setVisible( !resowner );
  if ( resowner )
    leRequestUrl->setText( QString() );

  lblRedirectUrl->setVisible( !resowner );
  frameRedirectUrl->setVisible( !resowner );

  lblClientSecret->setVisible( !implicit );
  leClientSecret->setVisible( !implicit );
  if ( implicit )
    leClientSecret->setText( QString() );

  leClientId->setPlaceholderText( resowner ? tr( "Optional" ) : tr( "Required" ) );
  leClientSecret->setPlaceholderText( resowner ? tr( "Optional" ) : tr( "Required" ) );


  lblUsername->setVisible( resowner );
  leUsername->setVisible( resowner );
  if ( !resowner )
    leUsername->setText( QString() );
  lblPassword->setVisible( resowner );
  lePassword->setVisible( resowner );
  if ( !resowner )
    lePassword->setText( QString() );
}


void QgsAuthOAuth2Edit::exportOAuthConfig()
{
  if ( !onCustomTab() || !mValid )
  {
    return;
  }

  QSettings settings;
  const QString recentdir = settings.value( QStringLiteral( "UI/lastAuthSaveFileDir" ), QDir::homePath() ).toString();
  const QString configpath = QFileDialog::getSaveFileName(
                               this, tr( "Save OAuth2 Config File" ), recentdir, QStringLiteral( "OAuth2 config files (*.json)" ) );
  this->raise();
  this->activateWindow();

  if ( configpath.isEmpty() )
  {
    return;
  }
  settings.setValue( QStringLiteral( "UI/lastAuthSaveFileDir" ), QFileInfo( configpath ).absoluteDir().path() );

  // give it a kind of random id for re-importing
  mOAuthConfigCustom->setId( QgsApplication::authManager()->uniqueConfigId() );

  mOAuthConfigCustom->setQueryPairs( queryPairs() );

  if ( mParentName && !mParentName->text().isEmpty() )
  {
    mOAuthConfigCustom->setName( mParentName->text() );
  }

  if ( !QgsAuthOAuth2Config::writeOAuth2Config( configpath, mOAuthConfigCustom.get(),
       QgsAuthOAuth2Config::JSON, true ) )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to export OAuth2 config file" ) );
  }
  // clear temp changes
  mOAuthConfigCustom->setId( QString() );
  mOAuthConfigCustom->setName( QString() );
}


void QgsAuthOAuth2Edit::importOAuthConfig()
{
  if ( !onCustomTab() )
  {
    return;
  }

  const QString configfile =
    QgsAuthGuiUtils::getOpenFileName( this, tr( "Select OAuth2 Config File" ), QStringLiteral( "OAuth2 config files (*.json)" ) );
  this->raise();
  this->activateWindow();

  const QFileInfo importinfo( configfile );
  if ( configfile.isEmpty() || !importinfo.exists() )
  {
    return;
  }

  QByteArray configtxt;
  QFile cfile( configfile );
  const bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
  if ( ret )
  {
    configtxt = cfile.readAll();
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "FAILED to open config for reading: %1" ).arg( configfile ) );
    cfile.close();
    return;
  }
  cfile.close();

  if ( configtxt.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "EMPTY read of config: %1" ).arg( configfile ) );
    return;
  }

  QgsStringMap configmap;
  configmap.insert( QStringLiteral( "oauth2config" ), QString( configtxt ) );
  loadConfig( configmap );
}


void QgsAuthOAuth2Edit::descriptionChanged()
{
  mOAuthConfigCustom->setDescription( pteDescription->toPlainText() );
}


void QgsAuthOAuth2Edit::populateAccessMethods()
{
  cmbbxAccessMethod->addItem( QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::Header ),
                              static_cast<int>( QgsAuthOAuth2Config::Header ) );
  cmbbxAccessMethod->addItem( QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::Form ),
                              static_cast<int>( QgsAuthOAuth2Config::Form ) );
  cmbbxAccessMethod->addItem( QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::Query ),
                              static_cast<int>( QgsAuthOAuth2Config::Query ) );
}


void QgsAuthOAuth2Edit::updateConfigAccessMethod( int indx )
{
  mOAuthConfigCustom->setAccessMethod( static_cast<QgsAuthOAuth2Config::AccessMethod>( indx ) );
  switch ( static_cast<QgsAuthOAuth2Config::AccessMethod>( indx ) )
  {
    case QgsAuthOAuth2Config::Header:
      mTokenHeaderLineEdit->setVisible( true );
      mTokenHeaderLabel->setVisible( true );
      break;
    case QgsAuthOAuth2Config::Form:
    case QgsAuthOAuth2Config::Query:
      mTokenHeaderLineEdit->setVisible( false );
      mTokenHeaderLabel->setVisible( false );
      break;
  }
}

void QgsAuthOAuth2Edit::addQueryPairRow( const QString &key, const QString &val )
{
  const int rowCnt = tblwdgQueryPairs->rowCount();
  tblwdgQueryPairs->insertRow( rowCnt );

  const Qt::ItemFlags itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
                                 | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

  QTableWidgetItem *keyItm = new QTableWidgetItem( key );
  keyItm->setFlags( itmFlags );
  tblwdgQueryPairs->setItem( rowCnt, 0, keyItm );

  QTableWidgetItem *valItm = new QTableWidgetItem( val );
  keyItm->setFlags( itmFlags );
  tblwdgQueryPairs->setItem( rowCnt, 1, valItm );
}


void QgsAuthOAuth2Edit::populateQueryPairs( const QVariantMap &querypairs, bool append )
{
  if ( !append )
  {
    clearQueryPairs();
  }

  QVariantMap::const_iterator i = querypairs.constBegin();
  while ( i != querypairs.constEnd() )
  {
    addQueryPairRow( i.key(), i.value().toString() );
    ++i;
  }
}


void QgsAuthOAuth2Edit::queryTableSelectionChanged()
{
  const bool hassel = tblwdgQueryPairs->selectedItems().count() > 0;
  btnRemoveQueryPair->setEnabled( hassel );
}

void QgsAuthOAuth2Edit::updateConfigQueryPairs()
{
  mOAuthConfigCustom->setQueryPairs( queryPairs() );
}

QVariantMap QgsAuthOAuth2Edit::queryPairs() const
{
  QVariantMap querypairs;
  for ( int i = 0; i < tblwdgQueryPairs->rowCount(); ++i )
  {
    if ( tblwdgQueryPairs->item( i, 0 )->text().isEmpty() )
    {
      continue;
    }
    querypairs.insert( tblwdgQueryPairs->item( i, 0 )->text(),
                       QVariant( tblwdgQueryPairs->item( i, 1 )->text() ) );
  }
  return querypairs;
}


void QgsAuthOAuth2Edit::addQueryPair()
{
  addQueryPairRow( QString(), QString() );
  tblwdgQueryPairs->setFocus();
  tblwdgQueryPairs->setCurrentCell( tblwdgQueryPairs->rowCount() - 1, 0 );
  tblwdgQueryPairs->edit( tblwdgQueryPairs->currentIndex() );
}


void QgsAuthOAuth2Edit::removeQueryPair()
{
  tblwdgQueryPairs->removeRow( tblwdgQueryPairs->currentRow() );
}


void QgsAuthOAuth2Edit::clearQueryPairs()
{
  for ( int i = tblwdgQueryPairs->rowCount(); i > 0 ; --i )
  {
    tblwdgQueryPairs->removeRow( i - 1 );
  }
}

void QgsAuthOAuth2Edit::parseSoftwareStatement( const QString &path )
{
  QFile file( path );
  QByteArray softwareStatementBase64;
  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    softwareStatementBase64 = file.readAll();
  }
  if ( softwareStatementBase64.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Error software statement is empty: %1" ).arg( path ) );
    file.close();
    return;
  }
  mRegistrationEndpoint = QString();
  file.close();
  mSoftwareStatement.insert( QStringLiteral( "software_statement" ), softwareStatementBase64 );
  QList<QByteArray> payloadParts( softwareStatementBase64.split( '.' ) );
  if ( payloadParts.count() < 2 )
  {
    QgsDebugMsg( QStringLiteral( "Error parsing JSON: base64 decode returned less than 2 parts" ) );
    return;
  }
  const QByteArray payload = payloadParts[1];
  QByteArray decoded = QByteArray::fromBase64( payload/*, QByteArray::Base64UrlEncoding*/ );
  QByteArray errStr;
  bool res = false;
  const QMap<QString, QVariant> jsonData = QJsonWrapper::parseJson( decoded, &res, &errStr ).toMap();
  if ( !res )
  {
    QgsDebugMsg( QStringLiteral( "Error parsing JSON: %1" ).arg( QString( errStr ) ) );
    return;
  }
  if ( jsonData.contains( QStringLiteral( "grant_types" ) ) && jsonData.contains( QStringLiteral( "redirect_uris" ) ) )
  {
    const QStringList grantTypes( jsonData[QStringLiteral( "grant_types" ) ].toStringList() );
    if ( !grantTypes.isEmpty( ) )
    {
      const QString grantType = grantTypes[0];
      if ( grantType == QLatin1String( "authorization_code" ) )
      {
        updateGrantFlow( static_cast<int>( QgsAuthOAuth2Config::AuthCode ) );
      }
      else
      {
        updateGrantFlow( static_cast<int>( QgsAuthOAuth2Config::ResourceOwner ) );
      }
    }
    //Set redirect_uri
    const QStringList  redirectUris( jsonData[QStringLiteral( "redirect_uris" ) ].toStringList() );
    if ( !redirectUris.isEmpty( ) )
    {
      const QString redirectUri = redirectUris[0];
      leRedirectUrl->setText( redirectUri );
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Error software statement is invalid: %1" ).arg( path ), 4 );
    return;
  }
  if ( jsonData.contains( QStringLiteral( "registration_endpoint" ) ) )
  {
    mRegistrationEndpoint = jsonData[QStringLiteral( "registration_endpoint" )].toString();
    leSoftwareStatementConfigUrl->setText( mRegistrationEndpoint );
  }
  QgsDebugMsgLevel( QStringLiteral( "JSON: %1" ).arg( QString::fromLocal8Bit( decoded.data() ) ), 4 );
}

void QgsAuthOAuth2Edit::configReplyFinished()
{
  qDebug() << "QgsAuthOAuth2Edit::onConfigReplyFinished";
  QNetworkReply *configReply = qobject_cast<QNetworkReply *>( sender() );
  if ( configReply->error() == QNetworkReply::NoError )
  {
    const QByteArray replyData = configReply->readAll();
    QByteArray errStr;
    bool res = false;
    const QVariantMap config = QJsonWrapper::parseJson( replyData, &res, &errStr ).toMap();

    if ( !res )
    {
      QgsDebugMsg( QStringLiteral( "Error parsing JSON: %1" ).arg( QString( errStr ) ) );
      return;
    }
    // I haven't found any docs about the content of this confg JSON file
    // I assume that registration_endpoint is all that it MUST contain.
    // But we also MAY have other optional information here
    if ( config.contains( QStringLiteral( "registration_endpoint" ) ) )
    {
      if ( config.contains( QStringLiteral( "authorization_endpoint" ) ) )
        leRequestUrl->setText( config.value( QStringLiteral( "authorization_endpoint" ) ).toString() );
      if ( config.contains( QStringLiteral( "token_endpoint" ) ) )
        leTokenUrl->setText( config.value( QStringLiteral( "token_endpoint" ) ).toString() );

      registerSoftStatement( config.value( QStringLiteral( "registration_endpoint" ) ).toString() );
    }
    else
    {
      const QString errorMsg = tr( "Downloading configuration failed with error: %1" ).arg( configReply->errorString() );
      QgsMessageLog::logMessage( errorMsg, QStringLiteral( "OAuth2" ), Qgis::MessageLevel::Critical );
    }
  }
  mDownloading = false;
  configReply->deleteLater();
}

void QgsAuthOAuth2Edit::registerReplyFinished()
{
  //JSV todo
  //better error handling
  qDebug() << "QgsAuthOAuth2Edit::onRegisterReplyFinished";
  QNetworkReply *registerReply = qobject_cast<QNetworkReply *>( sender() );
  if ( registerReply->error() == QNetworkReply::NoError )
  {
    const QByteArray replyData = registerReply->readAll();
    QByteArray errStr;
    bool res = false;
    const QVariantMap clientInfo = QJsonWrapper::parseJson( replyData, &res, &errStr ).toMap();

    // According to RFC 7591 sec. 3.2.1.  Client Information Response the only
    // required field is client_id
    leClientId->setText( clientInfo.value( QStringLiteral( "client_id" ) ).toString() );
    if ( clientInfo.contains( QStringLiteral( "client_secret" ) ) )
      leClientSecret->setText( clientInfo.value( QStringLiteral( "client_secret" ) ).toString() );
    if ( clientInfo.contains( QStringLiteral( "authorization_endpoint" ) ) )
      leRequestUrl->setText( clientInfo.value( QStringLiteral( "authorization_endpoint" ) ).toString() );
    if ( clientInfo.contains( QStringLiteral( "token_endpoint" ) ) )
      leTokenUrl->setText( clientInfo.value( QStringLiteral( "token_endpoint" ) ).toString() );
    if ( clientInfo.contains( QStringLiteral( "scopes" ) ) )
      leScope->setText( clientInfo.value( QStringLiteral( "scopes" ) ).toString() );

    tabConfigs->setCurrentIndex( 0 );
  }
  else
  {
    const QString errorMsg = QStringLiteral( "Client registration failed with error: %1" ).arg( registerReply->errorString() );
    QgsMessageLog::logMessage( errorMsg, QStringLiteral( "OAuth2" ), Qgis::MessageLevel::Critical );
  }
  mDownloading = false;
  registerReply->deleteLater();
}

void QgsAuthOAuth2Edit::networkError( QNetworkReply::NetworkError error )
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  qWarning() << "QgsAuthOAuth2Edit::onNetworkError: " << error << ": " << reply->errorString();
  const QString errorMsg = QStringLiteral( "Network error: %1" ).arg( reply->errorString() );
  QgsMessageLog::logMessage( errorMsg, QStringLiteral( "OAuth2" ), Qgis::MessageLevel::Critical );
  qDebug() << "QgsAuthOAuth2Edit::onNetworkError: " << reply->readAll();
}


void QgsAuthOAuth2Edit::registerSoftStatement( const QString &registrationUrl )
{
  const QUrl regUrl( registrationUrl );
  if ( !regUrl.isValid() )
  {
    qWarning() << "Registration url is not valid";
    return;
  }
  QByteArray errStr;
  bool res = false;
  const QByteArray json = QJsonWrapper::toJson( QVariant( mSoftwareStatement ), &res, &errStr );
  QNetworkRequest registerRequest( regUrl );
  QgsSetRequestInitiatorClass( registerRequest, QStringLiteral( "QgsAuthOAuth2Edit" ) );
  registerRequest.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/json" ) );
  QNetworkReply *registerReply;
  // For testability: use GET if protocol is file://
  if ( regUrl.scheme() == QLatin1String( "file" ) )
    registerReply = QgsNetworkAccessManager::instance()->get( registerRequest );
  else
    registerReply = QgsNetworkAccessManager::instance()->post( registerRequest, json );
  mDownloading = true;
  connect( registerReply, &QNetworkReply::finished, this, &QgsAuthOAuth2Edit::registerReplyFinished, Qt::QueuedConnection );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  connect( registerReply, qOverload<QNetworkReply::NetworkError>( &QNetworkReply::error ), this, &QgsAuthOAuth2Edit::networkError, Qt::QueuedConnection );
#else
  connect( registerReply, &QNetworkReply::errorOccurred, this, &QgsAuthOAuth2Edit::networkError, Qt::QueuedConnection );
#endif
}

void QgsAuthOAuth2Edit::getSoftwareStatementConfig()
{
  if ( !mRegistrationEndpoint.isEmpty() )
  {
    registerSoftStatement( mRegistrationEndpoint );
  }
  else
  {
    const QString config = leSoftwareStatementConfigUrl->text();
    const QUrl configUrl( config );
    QNetworkRequest configRequest( configUrl );
    QgsSetRequestInitiatorClass( configRequest, QStringLiteral( "QgsAuthOAuth2Edit" ) );
    QNetworkReply *configReply = QgsNetworkAccessManager::instance()->get( configRequest );
    mDownloading = true;
    connect( configReply, &QNetworkReply::finished, this, &QgsAuthOAuth2Edit::configReplyFinished, Qt::QueuedConnection );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect( configReply, qOverload<QNetworkReply::NetworkError>( &QNetworkReply::error ), this, &QgsAuthOAuth2Edit::networkError, Qt::QueuedConnection );
#else
    connect( configReply, &QNetworkReply::errorOccurred, this, &QgsAuthOAuth2Edit::networkError, Qt::QueuedConnection );
#endif
  }
}

void QgsAuthOAuth2Edit::updatePredefinedLocationsTooltip()
{
  const QStringList dirs = QgsAuthOAuth2Config::configLocations( leDefinedDirPath->text() );
  QString locationList;
  QString locationListHtml;
  for ( const QString &dir : dirs )
  {
    if ( !locationList.isEmpty() )
      locationList += '\n';
    if ( locationListHtml.isEmpty() )
      locationListHtml = QStringLiteral( "<ul>" );
    locationList += QStringLiteral( "• %1" ).arg( dir );
    locationListHtml += QStringLiteral( "<li><a href=\"%1\">%2</a></li>" ).arg( QUrl::fromLocalFile( dir ).toString(), dir );
  }
  if ( !locationListHtml.isEmpty() )
    locationListHtml += QLatin1String( "</ul>" );

  const QString tip = QStringLiteral( "<p>" ) + tr( "Defined configurations are JSON-formatted files, with a single configuration per file. "
                      "This allows configurations to be swapped out via filesystem tools without affecting user "
                      "configurations. It is recommended to use the Configure tab’s export function, then edit the "
                      "resulting file. See QGIS documentation for further details." ) + QStringLiteral( "</p><p>" ) +
                      tr( "Configurations files can be placed in the directories:" ) + QStringLiteral( "</p>" ) + locationListHtml;
  pteDefinedDesc->setHtml( tip );

  lstwdgDefinedConfigs->setToolTip( tr( "Configuration files can be placed in the directories:\n\n%1" ).arg( locationList ) );
}

