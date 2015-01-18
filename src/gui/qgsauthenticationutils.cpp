/***************************************************************************
    qgsauthenticationutils.cpp
    ---------------------
    begin                : October 24, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

#include "qgsauthenticationutils.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>

#include "qgsauthenticationmanager.h"
#include "qgslogger.h"
#include "qgsmessagebar.h"


static QString validGreen_( const QString& selector = "QLineEdit" )
{
  return QString( "%1{color: rgb(0, 170, 0);}" ).arg( selector );
}
static QString validRed_( const QString& selector = "QLineEdit" )
{
  return QString( "%1{color: rgb(200, 0, 0);}" ).arg( selector );
}

QgsMasterPasswordResetDialog::QgsMasterPasswordResetDialog( QWidget *parent )
    : QDialog( parent )
    , mPassCurOk( false )
    , mPassNewOk( false )
    , mAuthNotifyLayout( 0 )
    , mAuthNotify( 0 )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsAuthManager::instance()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
  }
}

QgsMasterPasswordResetDialog::~QgsMasterPasswordResetDialog()
{
}

bool QgsMasterPasswordResetDialog::requestMasterPasswordReset( QString *newpass, QString *oldpass, bool *keepbackup )
{
  if ( !QgsAuthManager::instance()->isDisabled() )
  {
    validatePasswords();
    leMasterPassCurrent->setFocus();

    bool ok = ( exec() == QDialog::Accepted );
    //QgsDebugMsg( QString( "exec(): %1" ).arg( ok ? "true" : "false" ) );

    if ( ok )
    {
      *newpass = leMasterPassNew->text();
      *oldpass = leMasterPassCurrent->text();
      *keepbackup = chkKeepBackup->isChecked();
      return true;
    }
  }
  return false;
}

void QgsMasterPasswordResetDialog::on_leMasterPassCurrent_textChanged( const QString& pass )
{
  // since this is called on every keystroke, block signals emitted during verification of password
  QgsAuthManager::instance()->blockSignals( true );
  mPassCurOk = !pass.isEmpty() && QgsAuthManager::instance()->setMasterPassword( pass, true );
  QgsAuthManager::instance()->blockSignals( false );
  validatePasswords();
}

void QgsMasterPasswordResetDialog::on_leMasterPassNew_textChanged( const QString& pass )
{
  mPassNewOk = !pass.isEmpty() && !QgsAuthManager::instance()->masterPasswordSame( pass );
  validatePasswords();
}

void QgsMasterPasswordResetDialog::on_chkPassShowCurrent_stateChanged( int state )
{
  leMasterPassCurrent->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsMasterPasswordResetDialog::on_chkPassShowNew_stateChanged( int state )
{
  leMasterPassNew->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsMasterPasswordResetDialog::validatePasswords()
{
  leMasterPassCurrent->setStyleSheet( mPassCurOk ? validGreen_() : validRed_() );
  leMasterPassNew->setStyleSheet( mPassNewOk ? validGreen_() : validRed_() );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mPassCurOk && mPassNewOk );
}

///////////////////////////////////////////////

bool QgsAuthUtils::isDisabled( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    msgbar->pushMessage( QgsAuthManager::instance()->authManTag(),
                         QObject::tr( "DISABLED: QCA's OpenSSL plugin missing" ),
                         QgsMessageBar::WARNING, timeout );
    return true;
  }
  return false;
}

void QgsAuthUtils::setMasterPassword( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthUtils::isDisabled( msgbar, timeout ) )
    return;

  if ( QgsAuthManager::instance()->masterPasswordIsSet() )
  {
    msgbar->pushMessage( QgsAuthManager::instance()->authManTag(),
                         QObject::tr( "Master password already set" ),
                         QgsMessageBar::INFO, timeout );
    return;
  }
  QgsAuthManager::instance()->setMasterPassword( true );
}

void QgsAuthUtils::clearCachedMasterPassword( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthUtils::isDisabled( msgbar, timeout ) )
    return;

  QString msg( QObject::tr( "Master password not cleared because it is not set" ) );
  QgsMessageBar::MessageLevel level( QgsMessageBar::INFO );

  if ( QgsAuthManager::instance()->masterPasswordIsSet() )
  {
    QgsAuthManager::instance()->clearMasterPassword();
    msg = QObject::tr( "Master password cleared (NOTE: network connections may be cached)" );
    if ( QgsAuthManager::instance()->masterPasswordIsSet() )
    {
      msg = QObject::tr( "Master password FAILED to be cleared" );
      level = QgsMessageBar::WARNING;
    }
  }

  msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, level, timeout );
}

void QgsAuthUtils::resetMasterPassword( QgsMessageBar *msgbar, int timeout, QWidget *parent )
{
  if ( QgsAuthUtils::isDisabled( msgbar, timeout ) )
    return;

  QString msg( QObject::tr( "Master password reset" ) );
  QgsMessageBar::MessageLevel level( QgsMessageBar::INFO );

  // check that a master password is even set in auth db
  if ( !QgsAuthManager::instance()->masterPasswordHashInDb() )
  {
    msg = QObject::tr( "Master password reset: NO current password hash in database" );
    msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, QgsMessageBar::WARNING, 0 );
    return;
  }

  // get new password via dialog; do current password verification in-dialog
  QString newpass;
  QString oldpass;
  bool keepbackup = false;
  QgsMasterPasswordResetDialog dlg( parent );

  if ( !dlg.requestMasterPasswordReset( &newpass, &oldpass, &keepbackup ) )
  {
    QgsDebugMsg( "Master password reset: input canceled by user" );
    return;
  }

  QString backuppath;
  if ( !QgsAuthManager::instance()->resetMasterPassword( newpass, oldpass, keepbackup, &backuppath ) )
  {
    msg = QObject::tr( "Master password FAILED to be reset" );
    level = QgsMessageBar::WARNING;
  }

  if ( !backuppath.isEmpty() )
  {
    msg += QObject::tr( " (database backup: %1)" ).arg( backuppath );
    timeout = 0; // no timeout, so user can read backup message
  }

  msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, level, timeout );
}

void QgsAuthUtils::clearCachedAuthenticationConfigs( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthUtils::isDisabled( msgbar, timeout ) )
    return;

  QgsAuthManager::instance()->clearAllCachedConfigs();
  QString msg = QObject::tr( "Cached authentication configurations for session cleared" );
  msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, QgsMessageBar::INFO, timeout );
}

void QgsAuthUtils::removeAuthenticationConfigs( QgsMessageBar *msgbar, int timeout, QWidget *parent )
{
  if ( QgsAuthUtils::isDisabled( msgbar, timeout ) )
    return;

  if ( QMessageBox::warning( parent,
                             QObject::tr( "Remove Configurations" ),
                             QObject::tr( "Are you sure you want to remove ALL authentication configurations?\n\n"
                                          "Operation can NOT be undone!" ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QString msg( QObject::tr( "Authentication configurations removed" ) );
  QgsMessageBar::MessageLevel level( QgsMessageBar::INFO );

  if ( !QgsAuthManager::instance()->removeAllAuthenticationConfigs() )
  {
    msg = QObject::tr( "Authentication configurations FAILED to be removed" );
    level = QgsMessageBar::WARNING;
  }

  msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, level, timeout );
}

void QgsAuthUtils::eraseAuthenticationDatabase( QgsMessageBar *msgbar, int timeout, QWidget *parent )
{
  if ( QgsAuthUtils::isDisabled( msgbar, timeout ) )
    return;

  if ( QMessageBox::warning( parent,
                             QObject::tr( "Erase Database" ),
                             QObject::tr( "Are you sure you want to ERASE the entire authentication database?\n\n"
                                          "Operation can NOT be undone!" ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  QString msg( QObject::tr( "Authentication database erased" ) );
  QgsMessageBar::MessageLevel level( QgsMessageBar::INFO );

  if ( !QgsAuthManager::instance()->eraseAuthenticationDatabase() )
  {
    msg = QObject::tr( "Authentication database FAILED to be erased" );
    level = QgsMessageBar::WARNING;
  }

  msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, level, timeout );
}
