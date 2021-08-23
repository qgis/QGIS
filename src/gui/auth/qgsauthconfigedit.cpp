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

#include "qgsauthmethodmetadata.h"
#include "qgsauthconfig.h"
#include "qgsauthconfigidedit.h"
#include "qgsauthmanager.h"
#include "qgsauthmethodedit.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsgui.h"

QgsAuthConfigEdit::QgsAuthConfigEdit( QWidget *parent, const QString &authcfg, const QString &dataprovider )
  : QDialog( parent )
  , mAuthCfg( authcfg )
  , mDataProvider( dataprovider )

{
  const bool disabled = QgsApplication::authManager()->isDisabled();
  bool idok = true;

  if ( !disabled && !authcfg.isEmpty() )
  {
    idok = QgsApplication::authManager()->configIds().contains( authcfg );
  }

  if ( disabled || !idok )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );

    QString msg( disabled ? QgsApplication::authManager()->disabledMessage() : QString() );
    if ( !authcfg.isEmpty() )
    {
      msg += "\n\n" + tr( "Authentication config id not loaded: %1" ).arg( authcfg );
    }
    mAuthNotify = new QLabel( msg, this );
    mAuthNotifyLayout->addWidget( mAuthNotify );

    mAuthCfg.clear(); // otherwise will continue to try authenticate (and fail) after save
    buttonBox->button( QDialogButtonBox::Save )->setEnabled( false );
  }
  else
  {
    setupUi( this );
    connect( btnClear, &QToolButton::clicked, this, &QgsAuthConfigEdit::btnClear_clicked );
    connect( leName, &QLineEdit::textChanged, this, &QgsAuthConfigEdit::leName_textChanged );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close );
    connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsAuthConfigEdit::saveConfig );
    connect( buttonBox->button( QDialogButtonBox::Reset ), &QAbstractButton::clicked, this, &QgsAuthConfigEdit::resetConfig );

    populateAuthMethods();

    connect( cmbAuthMethods, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             stkwAuthMethods, &QStackedWidget::setCurrentIndex );
    connect( cmbAuthMethods, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, [ = ] { validateAuth(); } );

    connect( authCfgEdit, &QgsAuthConfigIdEdit::validityChanged, this, &QgsAuthConfigEdit::validateAuth );

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

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsAuthConfigEdit::populateAuthMethods()
{
  const QStringList authMethodKeys = QgsApplication::authManager()->authMethodsKeys( mDataProvider );

  // sort by auth method description attribute, then populate
  QMap<QString, const QgsAuthMethodMetadata *> descmap;
  const auto constAuthMethodKeys = authMethodKeys;
  for ( const QString &authMethodKey : constAuthMethodKeys )
  {
    const QgsAuthMethodMetadata *meta = QgsApplication::authManager()->authMethodMetadata( authMethodKey );
    if ( !meta )
    {
      QgsDebugMsg( QStringLiteral( "Load auth method instance FAILED for auth method key (%1)" ).arg( authMethodKey ) );
      continue;
    }
    descmap.insert( meta->description(), meta );
  }

  QMap<QString, const QgsAuthMethodMetadata *>::iterator it = descmap.begin();
  for ( it = descmap.begin(); it != descmap.end(); ++it )
  {
    QgsAuthMethodEdit *editWidget = qobject_cast<QgsAuthMethodEdit *>(
                                      QgsApplication::authManager()->authMethodEditWidget( it.value()->key(), this ) );
    if ( !editWidget )
    {
      QgsDebugMsg( QStringLiteral( "Load auth method edit widget FAILED for auth method key (%1)" ).arg( it.value()->key() ) );
      continue;
    }
    connect( editWidget, &QgsAuthMethodEdit::validityChanged, this, &QgsAuthConfigEdit::validateAuth );

    cmbAuthMethods->addItem( it.key(), QVariant( it.value()->key() ) );
    stkwAuthMethods->addWidget( editWidget );
  }
}

void QgsAuthConfigEdit::loadConfig()
{
  const bool emptyAuthCfg = mAuthCfg.isEmpty();
  authCfgEdit->setAllowEmptyId( emptyAuthCfg );
  if ( emptyAuthCfg )
  {
    return;
  }

  // edit mode requires master password to have been set and verified against auth db
  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
  {
    mAuthCfg.clear();
    return;
  }

  QgsAuthMethodConfig mconfig;
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( mAuthCfg, mconfig, true ) )
  {
    QgsDebugMsg( QStringLiteral( "Loading FAILED for authcfg: %1" ).arg( mAuthCfg ) );
    return;
  }

  if ( !mconfig.isValid( true ) )
  {
    QgsDebugMsg( QStringLiteral( "Loading FAILED for authcfg (%1): invalid config" ).arg( mAuthCfg ) );
    return;
  }

  // load basic info
  leName->setText( mconfig.name() );
  leResource->setText( mconfig.uri() );
  authCfgEdit->setAuthConfigId( mconfig.id() );

  const QString authMethodKey = QgsApplication::authManager()->configAuthMethodKey( mAuthCfg );

  QgsDebugMsgLevel( QStringLiteral( "Loading authcfg: %1" ).arg( mAuthCfg ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Loading auth method: %1" ).arg( authMethodKey ), 2 );

  if ( authMethodKey.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Loading FAILED for authcfg (%1): no auth method found" ).arg( mAuthCfg ) );
    return;
  }

  if ( mconfig.method() != authMethodKey )
  {
    QgsDebugMsg( QStringLiteral( "Loading FAILED for authcfg (%1): auth method and key mismatch" ).arg( mAuthCfg ) );
    return;
  }

  const int indx = authMethodIndex( authMethodKey );
  if ( indx == -1 )
  {
    QgsDebugMsg( QStringLiteral( "Loading FAILED for authcfg (%1): no edit widget loaded for auth method '%2'" )
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
    QgsDebugMsg( QStringLiteral( "Cast to edit widget FAILED for authcfg (%1) and auth method key (%2)" )
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
  if ( !QgsApplication::authManager()->setMasterPassword( true ) )
    return;

  const QString authMethodKey = cmbAuthMethods->currentData().toString();

  QgsAuthMethodEdit *editWidget = currentEditWidget();
  if ( !editWidget )
  {
    QgsDebugMsg( QStringLiteral( "Cast to edit widget FAILED)" ) );
    return;
  }

  QgsAuthMethod *authmethod = QgsApplication::authManager()->authMethod( authMethodKey );
  if ( !authmethod )
  {
    QgsDebugMsg( QStringLiteral( "Save auth config FAILED when loading auth method instance from key (%1)" ).arg( authMethodKey ) );
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
    QgsDebugMsg( QStringLiteral( "Save auth config FAILED: config invalid" ) );
    return;
  }

  const QString authCfgId( authCfgEdit->configId() );
  if ( !mAuthCfg.isEmpty() )
  {
    if ( authCfgId == mAuthCfg ) // update
    {
      mconfig.setId( mAuthCfg );
      if ( QgsApplication::authManager()->updateAuthenticationConfig( mconfig ) )
      {
        emit authenticationConfigUpdated( mAuthCfg );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Updating auth config FAILED for authcfg: %1" ).arg( mAuthCfg ) );
      }
    }
    else // store new with unique ID, then delete previous
    {
      mconfig.setId( authCfgId );
      if ( QgsApplication::authManager()->storeAuthenticationConfig( mconfig ) )
      {
        emit authenticationConfigStored( authCfgId );
        if ( !QgsApplication::authManager()->removeAuthenticationConfig( mAuthCfg ) )
        {
          QgsDebugMsg( QStringLiteral( "Removal of older auth config FAILED" ) );
        }
        mAuthCfg = authCfgId;
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Storing new auth config with user-created unique ID FAILED" ) );
      }
    }
  }
  else if ( mAuthCfg.isEmpty() )
  {
    if ( authCfgId.isEmpty() ) // create new with generated ID
    {
      if ( QgsApplication::authManager()->storeAuthenticationConfig( mconfig ) )
      {
        mAuthCfg = mconfig.id();
        emit authenticationConfigStored( mAuthCfg );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Storing new auth config FAILED" ) );
      }
    }
    else // create new with user-created unique ID
    {
      mconfig.setId( authCfgId );
      if ( QgsApplication::authManager()->storeAuthenticationConfig( mconfig ) )
      {
        mAuthCfg = authCfgId;
        emit authenticationConfigStored( mAuthCfg );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Storing new auth config with user-created unique ID FAILED" ) );
      }
    }
  }

  this->accept();
}

void QgsAuthConfigEdit::btnClear_clicked()
{
  QgsAuthMethodEdit *editWidget = currentEditWidget();
  if ( !editWidget )
  {
    QgsDebugMsg( QStringLiteral( "Cast to edit widget FAILED)" ) );
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
    QgsAuthMethodEdit *editWidget = qobject_cast<QgsAuthMethodEdit *>( stkwAuthMethods->widget( i ) );
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
    QgsDebugMsg( QStringLiteral( "Cast to edit widget FAILED" ) );
  }
  else
  {
    authok = authok && editWidget->validateConfig();
  }
  authok = authok && authCfgEdit->validate();

  buttonBox->button( QDialogButtonBox::Save )->setEnabled( authok );
}

void QgsAuthConfigEdit::leName_textChanged( const QString &txt )
{
  Q_UNUSED( txt )
  validateAuth();
}

int QgsAuthConfigEdit::authMethodIndex( const QString &authMethodKey )
{
  return cmbAuthMethods->findData( QVariant( authMethodKey ) );
}

QgsAuthMethodEdit *QgsAuthConfigEdit::currentEditWidget()
{
  return qobject_cast<QgsAuthMethodEdit *>( stkwAuthMethods->currentWidget() );
}
