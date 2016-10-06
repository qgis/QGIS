/***************************************************************************
    qgsauthmasterpassresetdialog.cpp
    ---------------------
    begin                : September 10, 2015
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

#include "qgsauthmasterpassresetdialog.h"

#include <QLineEdit>
#include <QPushButton>

#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


QgsMasterPasswordResetDialog::QgsMasterPasswordResetDialog( QWidget *parent )
    : QDialog( parent )
    , mPassCurOk( false )
    , mPassNewOk( false )
    , mAuthNotifyLayout( nullptr )
    , mAuthNotify( nullptr )
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
  mPassCurOk = !pass.isEmpty();
  QgsAuthManager::instance()->blockSignals( false );
  validatePasswords();
}

void QgsMasterPasswordResetDialog::on_leMasterPassNew_textChanged( const QString& pass )
{
  mPassNewOk = !pass.isEmpty();
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
  QString ss1 = mPassCurOk ? QgsAuthGuiUtils::greenTextStyleSheet( "QLineEdit" )
                : QgsAuthGuiUtils::redTextStyleSheet( "QLineEdit" );
  leMasterPassCurrent->setStyleSheet( ss1 );
  QString ss2 = mPassNewOk ? QgsAuthGuiUtils::greenTextStyleSheet( "QLineEdit" )
                : QgsAuthGuiUtils::redTextStyleSheet( "QLineEdit" );
  leMasterPassNew->setStyleSheet( ss2 );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mPassCurOk && mPassNewOk );
}

