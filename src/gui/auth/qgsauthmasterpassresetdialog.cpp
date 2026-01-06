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

#include "qgsapplication.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgshelp.h"

#include <QLineEdit>
#include <QPushButton>

#include "moc_qgsauthmasterpassresetdialog.cpp"

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
    connect( buttonBox, &QDialogButtonBox::helpRequested, this, [] {
      QgsHelp::openHelp( u"auth_system/auth_overview.html#master-password"_s );
    } );

    if ( QgsApplication::authManager()->sqliteDatabasePath().isEmpty() )
    {
      chkKeepBackup->hide();
    }

    QString warning = tr( "The authentication store will be re-encrypted using the new password." );
    if ( QgsApplication::authManager()->passwordHelperEnabled() )
    {
      warning += u"<p><b>%1</b></p>"_s.arg( tr( "The new password will automatically be stored in the system %1." ).arg( QgsAuthManager::passwordHelperDisplayName() ) );
    }

    lblWarning->setText( warning );
  }
}

QgsPasswordLineEdit *QgsMasterPasswordResetDialog::oldPasswordLineEdit()
{
  return leMasterPassCurrent;
}

bool QgsMasterPasswordResetDialog::requestMasterPasswordReset( QString *newpass, QString *oldpass, bool *keepbackup )
{
  if ( !QgsApplication::authManager()->isDisabled() )
  {
    validatePasswords();
    leMasterPassCurrent->setFocus();

    const bool ok = ( exec() == QDialog::Accepted );
    //QgsDebugMsgLevel( u"exec(): %1"_s.arg( ok ? "true" : "false" ), 2 );

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

  if ( leMasterPassCurrent->isEnabled() )
  {
    const QString ss1 = currentPasswordOk ? QgsAuthGuiUtils::greenTextStyleSheet( u"QLineEdit"_s )
                                          : QgsAuthGuiUtils::redTextStyleSheet( u"QLineEdit"_s );
    leMasterPassCurrent->setStyleSheet( ss1 );
  }
  const QString ss2 = newPasswordOk ? QgsAuthGuiUtils::greenTextStyleSheet( u"QLineEdit"_s )
                                    : QgsAuthGuiUtils::redTextStyleSheet( u"QLineEdit"_s );
  leMasterPassNew->setStyleSheet( ss2 );
  const QString ss3 = confirmPasswordOk ? QgsAuthGuiUtils::greenTextStyleSheet( u"QLineEdit"_s )
                                        : QgsAuthGuiUtils::redTextStyleSheet( u"QLineEdit"_s );
  leMasterPassNew2->setStyleSheet( ss3 );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( currentPasswordOk && newPasswordOk && confirmPasswordOk );
}
