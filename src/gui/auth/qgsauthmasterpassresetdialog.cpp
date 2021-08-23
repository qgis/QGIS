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
#include "qgsapplication.h"


QgsMasterPasswordResetDialog::QgsMasterPasswordResetDialog( QWidget *parent )
  : QDialog( parent )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    mAuthNotifyLayout = new QVBoxLayout;
    this->setLayout( mAuthNotifyLayout );
    mAuthNotify = new QLabel( QgsApplication::authManager()->disabledMessage(), this );
    mAuthNotifyLayout->addWidget( mAuthNotify );
  }
  else
  {
    setupUi( this );
    connect( leMasterPassCurrent, &QgsPasswordLineEdit::textChanged, this, &QgsMasterPasswordResetDialog::leMasterPassCurrent_textChanged );
    connect( leMasterPassNew, &QgsPasswordLineEdit::textChanged, this, &QgsMasterPasswordResetDialog::leMasterPassNew_textChanged );
  }
}

bool QgsMasterPasswordResetDialog::requestMasterPasswordReset( QString *newpass, QString *oldpass, bool *keepbackup )
{
  if ( !QgsApplication::authManager()->isDisabled() )
  {
    validatePasswords();
    leMasterPassCurrent->setFocus();

    const bool ok = ( exec() == QDialog::Accepted );
    //QgsDebugMsg( QStringLiteral( "exec(): %1" ).arg( ok ? "true" : "false" ) );

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

void QgsMasterPasswordResetDialog::leMasterPassCurrent_textChanged( const QString &pass )
{
  // since this is called on every keystroke, block signals emitted during verification of password
  QgsApplication::authManager()->blockSignals( true );
  mPassCurOk = !pass.isEmpty();
  QgsApplication::authManager()->blockSignals( false );
  validatePasswords();
}

void QgsMasterPasswordResetDialog::leMasterPassNew_textChanged( const QString &pass )
{
  mPassNewOk = !pass.isEmpty();
  validatePasswords();
}

void QgsMasterPasswordResetDialog::validatePasswords()
{
  const QString ss1 = mPassCurOk ? QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QLineEdit" ) )
                      : QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) );
  leMasterPassCurrent->setStyleSheet( ss1 );
  const QString ss2 = mPassNewOk ? QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QLineEdit" ) )
                      : QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) );
  leMasterPassNew->setStyleSheet( ss2 );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mPassCurOk && mPassNewOk );
}

