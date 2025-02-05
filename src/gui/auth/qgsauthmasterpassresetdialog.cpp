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
#include "moc_qgsauthmasterpassresetdialog.cpp"

#include <QLineEdit>
#include <QPushButton>

#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
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
    connect( leMasterPassCurrent, &QgsPasswordLineEdit::textChanged, this, &QgsMasterPasswordResetDialog::validatePasswords );
    connect( leMasterPassNew, &QgsPasswordLineEdit::textChanged, this, &QgsMasterPasswordResetDialog::validatePasswords );
    connect( leMasterPassNew2, &QgsPasswordLineEdit::textChanged, this, &QgsMasterPasswordResetDialog::validatePasswords );

    if ( QgsApplication::authManager()->sqliteDatabasePath().isEmpty() )
    {
      chkKeepBackup->hide();
    }
  }

  QString warning = tr( "Your authentication database will be duplicated and re-encrypted using the new password." );
  if ( QgsApplication::authManager()->passwordHelperEnabled() )
  {
    warning += QStringLiteral( "<p><b>%1</b></p>" ).arg( tr( "Your new password will automatically be stored in the system %1." ).arg( QgsAuthManager::passwordHelperDisplayName() ) );
  }

  lblWarning->setText( warning );
}

bool QgsMasterPasswordResetDialog::requestMasterPasswordReset( QString *newpass, QString *oldpass, bool *keepbackup )
{
  if ( !QgsApplication::authManager()->isDisabled() )
  {
    validatePasswords();
    leMasterPassCurrent->setFocus();

    const bool ok = ( exec() == QDialog::Accepted );
    //QgsDebugMsgLevel( QStringLiteral( "exec(): %1" ).arg( ok ? "true" : "false" ), 2 );

    if ( ok )
    {
      *newpass = leMasterPassNew->text();
      *oldpass = leMasterPassCurrent->text();
      *keepbackup = !chkKeepBackup->isHidden() && chkKeepBackup->isChecked();
      return true;
    }
  }
  return false;
}

void QgsMasterPasswordResetDialog::validatePasswords()
{
  const QString currentPassword = leMasterPassCurrent->text();
  const QString newPassword = leMasterPassNew->text();
  const QString confirmPassword = leMasterPassNew2->text();

  const bool currentPasswordOk = !currentPassword.isEmpty();
  const bool newPasswordOk = !newPassword.isEmpty();
  const bool confirmPasswordOk = !confirmPassword.isEmpty() && confirmPassword == newPassword;

  const QString ss1 = currentPasswordOk ? QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QLineEdit" ) )
                                        : QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) );
  leMasterPassCurrent->setStyleSheet( ss1 );
  const QString ss2 = newPasswordOk ? QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QLineEdit" ) )
                                    : QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) );
  leMasterPassNew->setStyleSheet( ss2 );
  const QString ss3 = confirmPasswordOk ? QgsAuthGuiUtils::greenTextStyleSheet( QStringLiteral( "QLineEdit" ) )
                                        : QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) );
  leMasterPassNew2->setStyleSheet( ss3 );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( currentPasswordOk && newPasswordOk && confirmPasswordOk );
}
