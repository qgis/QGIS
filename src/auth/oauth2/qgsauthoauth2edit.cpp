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

#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgsauthconfigedit.h"
#include "qgslogger.h"


QgsAuthOAuth2Edit::QgsAuthOAuth2Edit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
  , mOAuthConfigCustom( nullptr )
  , mDefinedConfigsCache( QgsStringMap() )
  , mParentName( nullptr )
  , mValid( false )
  , mCurTab( 0 )
  , mPrevPersistToken( false )
  , btnTokenClear( nullptr )
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

  loadFromOAuthConfig( mOAuthConfigCustom );
}

QgsAuthOAuth2Edit::~QgsAuthOAuth2Edit()
{
  deleteConfigObjs();
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
  btnTokenClear->setIcon( QIcon( QStringLiteral( ":/oauth2method/oauth2_resources/close.svg" ) ) );
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
  QString parentclass = metaObject->className();
  //QgsDebugMsg( QStringLiteral( "parent class: %1" ).arg( parentclass ) );
  if ( parentclass != QStringLiteral( "QgsAuthConfigEdit" ) )
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

// slot
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

  // Custom config editing connections
  connect( cmbbxGrantFlow, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsAuthOAuth2Edit::updateGrantFlow ); // also updates GUI
  connect( pteDescription, &QPlainTextEdit::textChanged, this, &QgsAuthOAuth2Edit::descriptionChanged );
  connect( leRequestUrl, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setRequestUrl );
  connect( leTokenUrl, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setTokenUrl );
  connect( leRefreshTokenUrl, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setRefreshTokenUrl );
  connect( leRedirectUrl, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setRedirectUrl );
  connect( spnbxRedirectPort, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ),
           mOAuthConfigCustom, &QgsAuthOAuth2Config::setRedirectPort );
  connect( leClientId, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setClientId );
  connect( leClientSecret, &QgsPasswordLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setClientSecret );
  connect( leUsername, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setUsername );
  connect( lePassword, &QgsPasswordLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setPassword );
  connect( leScope, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setScope );
  connect( leState, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setState );
  connect( leApiKey, &QLineEdit::textChanged, mOAuthConfigCustom, &QgsAuthOAuth2Config::setApiKey );
  connect( chkbxTokenPersist, &QCheckBox::toggled, mOAuthConfigCustom, &QgsAuthOAuth2Config::setPersistToken );
  connect( cmbbxAccessMethod, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
           this, &QgsAuthOAuth2Edit::updateConfigAccessMethod );
  connect( spnbxRequestTimeout, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ),
           mOAuthConfigCustom, &QgsAuthOAuth2Config::setRequestTimeout );

  connect( mOAuthConfigCustom, &QgsAuthOAuth2Config::validityChanged, this, &QgsAuthOAuth2Edit::configValidityChanged );

  if ( mParentName )
  {
    connect( mParentName, &QLineEdit::textChanged, this, &QgsAuthOAuth2Edit::configValidityChanged );
  }
}

// slot
void QgsAuthOAuth2Edit::configValidityChanged()
{
  validateConfig();
  bool parentname = mParentName && !mParentName->text().isEmpty();
  btnExport->setEnabled( mValid && parentname );
}

bool QgsAuthOAuth2Edit::validateConfig()
{
  bool curvalid = ( onCustomTab() ? mOAuthConfigCustom->isValid() : !mDefinedId.isEmpty() );
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

    QByteArray configtxt = mOAuthConfigCustom->saveConfigTxt( QgsAuthOAuth2Config::JSON, false, &ok );

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
    QByteArray configtxt = configmap.value( QStringLiteral( "oauth2config" ) ).toUtf8();
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
      loadFromOAuthConfig( mOAuthConfigCustom );

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
    QString definedid = configmap.value( QStringLiteral( "definedid" ) );
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

      QByteArray querypairstxt = configmap.value( QStringLiteral( "querypairs" ) ).toUtf8();
      if ( !querypairstxt.isNull() && !querypairstxt.isEmpty() )
      {
        QVariantMap querypairsmap =
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

  loadFromOAuthConfig( mOAuthConfigCustom );
}

// slot
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
    leState->setText( config->state() );
    leApiKey->setText( config->apiKey() );

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

  QString authcfg = parentConfigId();
  if ( authcfg.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Auth config ID empty in ID widget of parent" ) );
    return;
  }

  QString localcachefile = QgsAuthOAuth2Config::tokenCachePath( authcfg, false );

  QString tempcachefile = QgsAuthOAuth2Config::tokenCachePath( authcfg, true );

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

// slot
void QgsAuthOAuth2Edit::tabIndexChanged( int indx )
{
  mCurTab = indx;
  validateConfig();
}

// slot
void QgsAuthOAuth2Edit::populateGrantFlows()
{
  cmbbxGrantFlow->addItem( QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::AuthCode ),
                           static_cast<int>( QgsAuthOAuth2Config::AuthCode ) );
  cmbbxGrantFlow->addItem( QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::Implicit ),
                           static_cast<int>( QgsAuthOAuth2Config::Implicit ) );
  cmbbxGrantFlow->addItem( QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::ResourceOwner ),
                           static_cast<int>( QgsAuthOAuth2Config::ResourceOwner ) );
}

// slot
void QgsAuthOAuth2Edit::definedCustomDirChanged( const QString &path )
{
  QFileInfo pinfo( path );
  bool ok = pinfo.exists() || pinfo.isDir();

  leDefinedDirPath->setStyleSheet( ok ? "" : QgsAuthGuiUtils::redTextStyleSheet() );

  if ( ok )
  {
    loadDefinedConfigs();
  }
}

// slot
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

  QString id = cur->data( Qt::UserRole ).toString();
  if ( !id.isEmpty() )
  {
    setCurrentDefinedConfig( id );
  }
}

// slot
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

// slot
void QgsAuthOAuth2Edit::getDefinedCustomDir()
{
  QString extradir = QFileDialog::getExistingDirectory( this, tr( "Select extra directory to parse" ),
                     QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
  this->raise();
  this->activateWindow();

  if ( extradir.isEmpty() )
  {
    return;
  }
  leDefinedDirPath->setText( extradir );
}

void QgsAuthOAuth2Edit::initConfigObjs()
{
  mOAuthConfigCustom = new QgsAuthOAuth2Config( this );
  mOAuthConfigCustom->setConfigType( QgsAuthOAuth2Config::Custom );
  mOAuthConfigCustom->setToDefaults();
}

void QgsAuthOAuth2Edit::deleteConfigObjs()
{
  delete mOAuthConfigCustom;
  mOAuthConfigCustom = nullptr;
}

bool QgsAuthOAuth2Edit::hasTokenCacheFile()
{
  QString authcfg = parentConfigId();
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
  QString authcfg = parentConfigId();
  if ( authcfg.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Auth config ID empty in ID widget of parent" ) );
    return;
  }

  QStringList cachefiles;
  cachefiles << QgsAuthOAuth2Config::tokenCachePath( authcfg, false )
             << QgsAuthOAuth2Config::tokenCachePath( authcfg, true );

  Q_FOREACH ( const QString &cachefile, cachefiles )
  {
    if ( QFile::exists( cachefile ) && !QFile::remove( cachefile ) )
    {
      QgsDebugMsg( QStringLiteral( "Remove token cache file FAILED for authcfg %1: %2" ).arg( authcfg, cachefile ) );
    }
  }
  btnTokenClear->setEnabled( hasTokenCacheFile() );
}

// slot
void QgsAuthOAuth2Edit::updateDefinedConfigsCache()
{
  QString extradir = leDefinedDirPath->text();
  mDefinedConfigsCache.clear();
  mDefinedConfigsCache = QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( this, extradir );
}

// slot
void QgsAuthOAuth2Edit::loadDefinedConfigs()
{
  lstwdgDefinedConfigs->blockSignals( true );
  lstwdgDefinedConfigs->clear();
  lstwdgDefinedConfigs->blockSignals( false );

  updateDefinedConfigsCache();

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

    QString grantflow = QgsAuthOAuth2Config::grantFlowString( config->grantFlow() );

    QString name = QStringLiteral( "%1 (%2): %3" )
                   .arg( config->name(), grantflow, config->description() );

    QString tip = tr( "ID: %1\nGrant flow: %2\nDescription: %3" )
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

// slot
void QgsAuthOAuth2Edit::updateGrantFlow( int indx )
{
  if ( cmbbxGrantFlow->currentIndex() != indx )
  {
    cmbbxGrantFlow->blockSignals( true );
    cmbbxGrantFlow->setCurrentIndex( indx );
    cmbbxGrantFlow->blockSignals( false );
  }

  QgsAuthOAuth2Config::GrantFlow flow =
    static_cast<QgsAuthOAuth2Config::GrantFlow>( cmbbxGrantFlow->itemData( indx ).toInt() );
  mOAuthConfigCustom->setGrantFlow( flow );

  // bool authcode = ( flow == QgsAuthOAuth2Config::AuthCode );
  bool implicit = ( flow == QgsAuthOAuth2Config::Implicit );
  bool resowner = ( flow == QgsAuthOAuth2Config::ResourceOwner );

  lblRequestUrl->setVisible( !resowner );
  leRequestUrl->setVisible( !resowner );
  if ( resowner ) leRequestUrl->setText( QString() );

  lblRedirectUrl->setVisible( !resowner );
  frameRedirectUrl->setVisible( !resowner );

  lblClientSecret->setVisible( !implicit );
  leClientSecret->setVisible( !implicit );
  if ( implicit ) leClientSecret->setText( QString() );

  leClientId->setPlaceholderText( resowner ? tr( "Optional" ) : tr( "Required" ) );
  leClientSecret->setPlaceholderText( resowner ? tr( "Optional" ) : tr( "Required" ) );


  lblUsername->setVisible( resowner );
  leUsername->setVisible( resowner );
  if ( !resowner ) leUsername->setText( QString() );
  lblPassword->setVisible( resowner );
  lePassword->setVisible( resowner );
  if ( !resowner ) lePassword->setText( QString() );
}

// slot
void QgsAuthOAuth2Edit::exportOAuthConfig()
{
  if ( !onCustomTab() || !mValid )
  {
    return;
  }

  QSettings settings;
  QString recentdir = settings.value( QStringLiteral( "UI/lastAuthSaveFileDir" ), QDir::homePath() ).toString();
  QString configpath = QFileDialog::getSaveFileName(
                         this, tr( "Save OAuth2 Config File" ), recentdir, QStringLiteral( "OAuth2 config files (*.json)" ) );
  this->raise();
  this->activateWindow();

  if ( configpath.isEmpty() )
  {
    return;
  }
  settings.setValue( QStringLiteral( "UI/lastAuthSaveFileDir" ), QFileInfo( configpath ).absoluteDir().path() );

  // give it a kind of random id for re-importing
  mOAuthConfigCustom->setId( QgsAuthManager::instance()->uniqueConfigId() );

  mOAuthConfigCustom->setQueryPairs( queryPairs() );

  if ( mParentName && !mParentName->text().isEmpty() )
  {
    mOAuthConfigCustom->setName( mParentName->text() );
  }

  if ( !QgsAuthOAuth2Config::writeOAuth2Config( configpath, mOAuthConfigCustom,
       QgsAuthOAuth2Config::JSON, true ) )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to export OAuth2 config file" ) );
  }
  // clear temp changes
  mOAuthConfigCustom->setId( QString::null );
  mOAuthConfigCustom->setName( QString::null );
}

// slot
void QgsAuthOAuth2Edit::importOAuthConfig()
{
  if ( !onCustomTab() )
  {
    return;
  }

  QString configfile =
    QgsAuthGuiUtils::getOpenFileName( this, tr( "Select OAuth2 Config File" ), QStringLiteral( "OAuth2 config files (*.json)" ) );
  this->raise();
  this->activateWindow();

  QFileInfo importinfo( configfile );
  if ( configfile.isEmpty() || !importinfo.exists() )
  {
    return;
  }

  QByteArray configtxt;
  QFile cfile( configfile );
  bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
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

// slot
void QgsAuthOAuth2Edit::descriptionChanged()
{
  mOAuthConfigCustom->setDescription( pteDescription->toPlainText() );
}

// slot
void QgsAuthOAuth2Edit::populateAccessMethods()
{
  cmbbxAccessMethod->addItem( QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::Header ),
                              static_cast<int>( QgsAuthOAuth2Config::Header ) );
  cmbbxAccessMethod->addItem( QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::Form ),
                              static_cast<int>( QgsAuthOAuth2Config::Form ) );
  cmbbxAccessMethod->addItem( QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::Query ),
                              static_cast<int>( QgsAuthOAuth2Config::Query ) );
}

// slot
void QgsAuthOAuth2Edit::updateConfigAccessMethod( int indx )
{
  mOAuthConfigCustom->setAccessMethod( static_cast<QgsAuthOAuth2Config::AccessMethod>( indx ) );
}

void QgsAuthOAuth2Edit::addQueryPairRow( const QString &key, const QString &val )
{
  int rowCnt = tblwdgQueryPairs->rowCount();
  tblwdgQueryPairs->insertRow( rowCnt );

  Qt::ItemFlags itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
                           | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

  QTableWidgetItem *keyItm = new QTableWidgetItem( key );
  keyItm->setFlags( itmFlags );
  tblwdgQueryPairs->setItem( rowCnt, 0, keyItm );

  QTableWidgetItem *valItm = new QTableWidgetItem( val );
  keyItm->setFlags( itmFlags );
  tblwdgQueryPairs->setItem( rowCnt, 1, valItm );
}

// slot
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

// slot
void QgsAuthOAuth2Edit::queryTableSelectionChanged()
{
  bool hassel = tblwdgQueryPairs->selectedItems().count() > 0;
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

// slot
void QgsAuthOAuth2Edit::addQueryPair()
{
  addQueryPairRow( QString(), QString() );
  tblwdgQueryPairs->setFocus();
  tblwdgQueryPairs->setCurrentCell( tblwdgQueryPairs->rowCount() - 1, 0 );
  tblwdgQueryPairs->edit( tblwdgQueryPairs->currentIndex() );
}

// slot
void QgsAuthOAuth2Edit::removeQueryPair()
{
  tblwdgQueryPairs->removeRow( tblwdgQueryPairs->currentRow() );
}

// slot
void QgsAuthOAuth2Edit::clearQueryPairs()
{
  for ( int i = tblwdgQueryPairs->rowCount(); i > 0 ; --i )
  {
    tblwdgQueryPairs->removeRow( i - 1 );
  }
}
