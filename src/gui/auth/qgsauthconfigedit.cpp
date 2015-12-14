/***************************************************************************
    qgsauthconfigedit.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthconfigedit.h"

#include <QPushButton>

#include "qgsauthconfig.h"
#include "qgsauthconfigidedit.h"
#include "qgsauthmanager.h"
#include "qgsauthmethodedit.h"
#include "qgslogger.h"


QgsAuthConfigEdit::QgsAuthConfigEdit( QWidget *parent , const QString& authcfg , const QString &dataprovider )
    : QDialog( parent )
    , mAuthCfg( authcfg )
    , mDataProvider( dataprovider )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
{
  bool disabled = QgsAuthManager::instance()->isDisabled();
  bool idok = true;

  if ( !disabled && !authcfg.isEmpty() )
  {
    idok = QgsAuthManager::instance()->configIds().contains( authcfg );
  }

  if ( disabled || !idok )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );

    QString msg( disabled ? QgsAuthManager::instance()->disabledMessage() : "" );
    if ( !authcfg.isEmpty() )
    {
      msg += "\n\n" + tr( "Authentication config id not loaded: %1" ).arg( authcfg );
    }
    mAuthNotify = new QLabel( msg, this );
    mAuthNotifyLayout->addWidget( mAuthNotify );

    mAuthCfg.clear(); // otherwise will contiue to try authenticate (and fail) after save
    buttonBox->button( QDialogButtonBox::Save )->setEnabled( false );
  }
  else
  {
    setupUi( this );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );
    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveConfig() ) );
    connect( buttonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ), this, SLOT( resetConfig() ) );

    populateAuthMethods();

    connect( cmbAuthMethods, SIGNAL( currentIndexChanged( int ) ),
             stkwAuthMethods, SLOT( setCurrentIndex( int ) ) );
    connect( cmbAuthMethods, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( validateAuth() ) );

    connect( authCfgEdit, SIGNAL( validityChanged( bool ) ), this, SLOT( validateAuth() ) );

    // needed (if only combobox is ever changed)?
    // connect( stkwAuthMethods, SIGNAL( currentChanged( int ) ),
    //          cmbAuthMethods, SLOT( setCurrentIndex( int ) ) );

    // connect( stkwAuthMethods, SIGNAL( currentChanged( int ) ),
    //          this, SLOT( validateAuth() ) );

    if ( cmbAuthMethods->count() > 0 )
    {
      cmbAuthMethods->setCurrentIndex( 0 );
      stkwAuthMethods->setCurrentIndex( 0 );
    }

    loadConfig();
    validateAuth();

    leName->setFocus();
  }
}

QgsAuthConfigEdit::~QgsAuthConfigEdit()
{
}

void QgsAuthConfigEdit::populateAuthMethods()
{
  QStringList authMethodKeys = QgsAuthManager::instance()->authMethodsKeys( mDataProvider );

  // sort by auth method description attribute, then populate
  QMap<QString, QgsAuthMethod *> descmap;
  Q_FOREACH ( const QString &authMethodKey, authMethodKeys )
  {
    QgsAuthMethod *authmethod = QgsAuthManager::instance()->authMethod( authMethodKey );
    if ( !authmethod )
    {
      QgsDebugMsg( QString( "Load auth method instance FAILED for auth method key (%1)" ).arg( authMethodKey ) );
      continue;
    }
    descmap.insert( authmethod->displayDescription(), authmethod );
  }

  QMap<QString, QgsAuthMethod *>::iterator it = descmap.begin();
  for ( it = descmap.begin(); it != descmap.end(); ++it )
  {
    QgsAuthMethodEdit *editWidget = qobject_cast<QgsAuthMethodEdit*>(
                                      QgsAuthManager::instance()->authMethodEditWidget( it.value()->key(), this ) );
    if ( !editWidget )
    {
      QgsDebugMsg( QString( "Load auth method edit widget FAILED for auth method key (%1)" ).arg( it.value()->key() ) );
      continue;
    }
    connect( editWidget, SIGNAL( validityChanged( bool ) ), this, SLOT( validateAuth() ) );

    cmbAuthMethods->addItem( it.key(), QVariant( it.value()->key() ) );
    stkwAuthMethods->addWidget( editWidget );
  }
}

void QgsAuthConfigEdit::loadConfig()
{
  bool emptyAuthCfg = mAuthCfg.isEmpty();
  authCfgEdit->setAllowEmptyId( emptyAuthCfg );
  if ( emptyAuthCfg )
  {
    return;
  }

  // edit mode requires master password to have been set and verified against auth db
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
  {
    mAuthCfg.clear();
    return;
  }

  QgsAuthMethodConfig mconfig;
  if ( !QgsAuthManager::instance()->loadAuthenticationConfig( mAuthCfg, mconfig, true ) )
  {
    QgsDebugMsg( QString( "Loading FAILED for authcfg: %1" ).arg( mAuthCfg ) );
    return;
  }

  if ( !mconfig.isValid( true ) )
  {
    QgsDebugMsg( QString( "Loading FAILED for authcfg (%1): invalid config" ).arg( mAuthCfg ) );
    return;
  }

  // load basic info
  leName->setText( mconfig.name() );
  leResource->setText( mconfig.uri() );
  authCfgEdit->setAuthConfigId( mconfig.id() );

  QString authMethodKey = QgsAuthManager::instance()->configAuthMethodKey( mAuthCfg );

  QgsDebugMsg( QString( "Loading authcfg: %1" ).arg( mAuthCfg ) );
  QgsDebugMsg( QString( "Loading auth method: %1" ).arg( authMethodKey ) );

  if ( authMethodKey.isEmpty() )
  {
    QgsDebugMsg( QString( "Loading FAILED for authcfg (%1): no auth method found" ).arg( mAuthCfg ) );
    return;
  }

  if ( mconfig.method() != authMethodKey )
  {
    QgsDebugMsg( QString( "Loading FAILED for authcfg (%1): auth method and key mismatch" ).arg( mAuthCfg ) );
    return;
  }

  int indx = authMethodIndex( authMethodKey );
  if ( indx == -1 )
  {
    QgsDebugMsg( QString( "Loading FAILED for authcfg (%1): no edit widget loaded for auth method '%2'" )
                 .arg( mAuthCfg, authMethodKey ) );
    if ( cmbAuthMethods->count() > 0 )
    {
      cmbAuthMethods->setCurrentIndex( 0 );
      stkwAuthMethods->setCurrentIndex( 0 );
    }
    return;
  }

  cmbAuthMethods->setCurrentIndex( indx );
  stkwAuthMethods->setCurrentIndex( indx );

  QgsAuthMethodEdit *editWidget = currentEditWidget();
  if ( !editWidget )
  {
    QgsDebugMsg( QString( "Cast to edit widget FAILED for authcfg (%1) and auth method key (%2)" )
                 .arg( mAuthCfg, authMethodKey ) );
    return;
  }

  editWidget->loadConfig( mconfig.configMap() );
}

void QgsAuthConfigEdit::resetConfig()
{
  clearAll();
  loadConfig();
  validateAuth();
}

void QgsAuthConfigEdit::saveConfig()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QString authMethodKey = cmbAuthMethods->itemData( cmbAuthMethods->currentIndex() ).toString();

  QgsAuthMethodEdit *editWidget = currentEditWidget();
  if ( !editWidget )
  {
    QgsDebugMsg( QString( "Cast to edit widget FAILED)" ) );
    return;
  }

  QgsAuthMethod *authmethod = QgsAuthManager::instance()->authMethod( authMethodKey );
  if ( !authmethod )
  {
    QgsDebugMsg( QString( "Save auth config FAILED when loading auth method instance from key (%1)" ).arg( authMethodKey ) );
    return;
  }

  QgsAuthMethodConfig mconfig;
  mconfig.setName( leName->text() );
  mconfig.setUri( leResource->text() );
  mconfig.setMethod( authMethodKey );
  mconfig.setVersion( authmethod->version() );
  mconfig.setConfigMap( editWidget->configMap() );

  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( "Save auth config FAILED: config invalid" );
    return;
  }

  QString authCfgId( authCfgEdit->configId() );
  if ( !mAuthCfg.isEmpty() )
  {
    if ( authCfgId == mAuthCfg ) // update
    {
      mconfig.setId( mAuthCfg );
      if ( QgsAuthManager::instance()->updateAuthenticationConfig( mconfig ) )
      {
        emit authenticationConfigUpdated( mAuthCfg );
      }
      else
      {
        QgsDebugMsg( QString( "Updating auth config FAILED for authcfg: %1" ).arg( mAuthCfg ) );
      }
    }
    else // store new with unique ID, then delete previous
    {
      mconfig.setId( authCfgId );
      if ( QgsAuthManager::instance()->storeAuthenticationConfig( mconfig ) )
      {
        emit authenticationConfigStored( authCfgId );
        if ( !QgsAuthManager::instance()->removeAuthenticationConfig( mAuthCfg ) )
        {
          QgsDebugMsg( QString( "Removal of older auth config FAILED" ) );
        }
        mAuthCfg = authCfgId;
      }
      else
      {
        QgsDebugMsg( QString( "Storing new auth config with user-created unique ID FAILED" ) );
      }
    }
  }
  else if ( mAuthCfg.isEmpty() )
  {
    if ( authCfgId.isEmpty() ) // create new with generated ID
    {
      if ( QgsAuthManager::instance()->storeAuthenticationConfig( mconfig ) )
      {
        mAuthCfg = mconfig.id();
        emit authenticationConfigStored( mAuthCfg );
      }
      else
      {
        QgsDebugMsg( QString( "Storing new auth config FAILED" ) );
      }
    }
    else // create new with user-created unique ID
    {
      mconfig.setId( authCfgId );
      if ( QgsAuthManager::instance()->storeAuthenticationConfig( mconfig ) )
      {
        mAuthCfg = authCfgId;
        emit authenticationConfigStored( mAuthCfg );
      }
      else
      {
        QgsDebugMsg( QString( "Storing new auth config with user-created unique ID FAILED" ) );
      }
    }
  }

  this->accept();
}

void QgsAuthConfigEdit::on_btnClear_clicked()
{
  QgsAuthMethodEdit *editWidget = currentEditWidget();
  if ( !editWidget )
  {
    QgsDebugMsg( QString( "Cast to edit widget FAILED)" ) );
    return;
  }

  editWidget->clearConfig();

  validateAuth();
}

void QgsAuthConfigEdit::clearAll()
{
  leName->clear();
  leResource->clear();
  authCfgEdit->clear();

  for ( int i = 0; i < stkwAuthMethods->count(); i++ )
  {
    QgsAuthMethodEdit *editWidget = qobject_cast<QgsAuthMethodEdit*>( stkwAuthMethods->widget( i ) );
    if ( editWidget )
    {
      editWidget->clearConfig();
    }
  }

  validateAuth();
}

void QgsAuthConfigEdit::validateAuth()
{
  bool authok = !leName->text().isEmpty();

  QgsAuthMethodEdit *editWidget = currentEditWidget();
  if ( !editWidget )
  {
    QgsDebugMsg( QString( "Cast to edit widget FAILED" ) );
  }
  else
  {
    authok = authok && editWidget->validateConfig();
  }
  authok = authok && authCfgEdit->validate();

  buttonBox->button( QDialogButtonBox::Save )->setEnabled( authok );
}

void QgsAuthConfigEdit::on_leName_textChanged( const QString& txt )
{
  Q_UNUSED( txt );
  validateAuth();
}

int QgsAuthConfigEdit::authMethodIndex( const QString &authMethodKey )
{
  return cmbAuthMethods->findData( QVariant( authMethodKey ) );
}

QgsAuthMethodEdit *QgsAuthConfigEdit::currentEditWidget()
{
  return qobject_cast<QgsAuthMethodEdit*>( stkwAuthMethods->currentWidget() );
}
