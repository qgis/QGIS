/***************************************************************************
    qgsauthutils.cpp
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

#include "qgsauthguiutils.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "qgssettings.h"
#include "qgsauthmanager.h"
#include "qgsauthmasterpassresetdialog.h"
#include "qgslogger.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"


QColor QgsAuthGuiUtils::greenColor()
{
  return QColor( 0, 170, 0 );
}

QColor QgsAuthGuiUtils::orangeColor()
{
  return QColor( 255, 128, 0 );
}

QColor QgsAuthGuiUtils::redColor()
{
  return QColor( 200, 0, 0 );
}

QColor QgsAuthGuiUtils::yellowColor()
{
  return QColor( 255, 255, 125 );
}

QString QgsAuthGuiUtils::greenTextStyleSheet( const QString &selector )
{
  return QStringLiteral( "%1{color: %2;}" ).arg( selector, QgsAuthGuiUtils::greenColor().name() );
}

QString QgsAuthGuiUtils::orangeTextStyleSheet( const QString &selector )
{
  return QStringLiteral( "%1{color: %2;}" ).arg( selector, QgsAuthGuiUtils::orangeColor().name() );
}

QString QgsAuthGuiUtils::redTextStyleSheet( const QString &selector )
{
  return QStringLiteral( "%1{color: %2;}" ).arg( selector, QgsAuthGuiUtils::redColor().name() );
}

bool QgsAuthGuiUtils::isDisabled( QgsMessageBar *msgbar )
{
  if ( QgsApplication::authManager()->isDisabled() )
  {
    msgbar->pushMessage( QObject::tr( "Authentication System" ),
                         QObject::tr( "DISABLED. Resources authenticating via the system can not be accessed" ),
                         Qgis::MessageLevel::Critical );
    return true;
  }
  return false;
}

void QgsAuthGuiUtils::exportSelectedAuthenticationConfigs( QStringList authenticationConfigIds, QgsMessageBar *msgbar )
{
  const QString password = QInputDialog::getText( msgbar, QObject::tr( "Export Authentication Configurations" ),
                           QObject::tr( "Enter a password encrypt the configuration file:" ), QLineEdit::Password );
  if ( password.isEmpty() )
  {
    if ( QMessageBox::warning( msgbar,
                               QObject::tr( "Export Authentication Configurations" ),
                               QObject::tr( "Exporting authentication configurations with a blank password will result in a plain text file which may contain sensitive information. Are you sure you want to do this?" ),
                               QMessageBox::Ok | QMessageBox::Cancel,
                               QMessageBox::Cancel ) == QMessageBox::Cancel )
    {
      return;
    }
  }

  const QString filename = QFileDialog::getSaveFileName( msgbar, QObject::tr( "Export Authentication Configurations" ), QDir::homePath(),
                           QObject::tr( "XML files (*.xml *.XML)" ) );
  if ( filename.isEmpty() )
    return;

  const bool ok = QgsApplication::authManager()->exportAuthenticationConfigsToXml( filename, authenticationConfigIds, password );
  if ( !ok )
  {
    msgbar->pushMessage( QgsApplication::authManager()->authManTag(),
                         QObject::tr( "Export of authentication configurations failed." ),
                         Qgis::MessageLevel::Critical );
  }
}

void QgsAuthGuiUtils::importAuthenticationConfigs( QgsMessageBar *msgbar )
{

  const QString filename = QFileDialog::getOpenFileName( msgbar, QObject::tr( "Export Authentication Configurations" ), QDir::homePath(),
                           QObject::tr( "XML files (*.xml *.XML)" ) );
  if ( filename.isEmpty() )
    return;


  QFile file( filename );
  if ( !file.open( QFile::ReadOnly ) )
  {
    return;
  }

  QDomDocument document( QStringLiteral( "qgis_authentication" ) );
  if ( !document.setContent( &file ) )
  {
    file.close();
    return;
  }
  file.close();

  const QDomElement root = document.documentElement();
  if ( root.tagName() != QLatin1String( "qgis_authentication" ) )
  {
    return;
  }

  QString password;
  if ( root.hasAttribute( QStringLiteral( "salt" ) ) )
  {
    password = QInputDialog::getText( msgbar, QObject::tr( "Import Authentication Configurations" ),
                                      QObject::tr( "Enter the password to decrypt the configurations file:" ), QLineEdit::Password );
  }

  const bool ok = QgsApplication::authManager()->importAuthenticationConfigsFromXml( filename, password );
  if ( !ok )
  {
    msgbar->pushMessage( QgsApplication::authManager()->authManTag(),
                         QObject::tr( "Import of authentication configurations failed." ),
                         Qgis::MessageLevel::Critical );
  }
}

void QgsAuthGuiUtils::setMasterPassword( QgsMessageBar *msgbar )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar ) )
    return;

  if ( QgsApplication::authManager()->masterPasswordIsSet() )
  {
    msgbar->pushMessage( QgsApplication::authManager()->authManTag(),
                         QObject::tr( "Master password already set." ),
                         Qgis::MessageLevel::Info );
    return;
  }
  ( void )QgsApplication::authManager()->setMasterPassword( true );
}

void QgsAuthGuiUtils::clearCachedMasterPassword( QgsMessageBar *msgbar )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar ) )
    return;

  QString msg( QObject::tr( "Master password not cleared because it is not set." ) );
  Qgis::MessageLevel level( Qgis::MessageLevel::Info );

  if ( QgsApplication::authManager()->masterPasswordIsSet() )
  {
    QgsApplication::authManager()->clearMasterPassword();
    msg = QObject::tr( "Master password cleared (NOTE: network connections may be cached)." );
    if ( QgsApplication::authManager()->masterPasswordIsSet() )
    {
      msg = QObject::tr( "Master password FAILED to be cleared." );
      level = Qgis::MessageLevel::Warning;
    }
  }

  msgbar->pushMessage( QgsApplication::authManager()->authManTag(), msg, level );
}

void QgsAuthGuiUtils::resetMasterPassword( QgsMessageBar *msgbar,  QWidget *parent )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar ) )
    return;

  QString msg( QObject::tr( "Master password reset" ) );
  Qgis::MessageLevel level( Qgis::MessageLevel::Info );

  // check that a master password is even set in auth db
  if ( !QgsApplication::authManager()->masterPasswordHashInDatabase() )
  {
    msg = QObject::tr( "Master password reset: NO current password hash in database" );
    msgbar->pushMessage( QgsApplication::authManager()->authManTag(), msg, Qgis::MessageLevel::Warning );
    return;
  }

  // get new password via dialog; do current password verification in-dialog
  QString newpass;
  QString oldpass;
  bool keepbackup = false;
  QgsMasterPasswordResetDialog dlg( parent );

  if ( !dlg.requestMasterPasswordReset( &newpass, &oldpass, &keepbackup ) )
  {
    QgsDebugMsg( QStringLiteral( "Master password reset: input canceled by user" ) );
    return;
  }

  QString backuppath;
  if ( !QgsApplication::authManager()->resetMasterPassword( newpass, oldpass, keepbackup, &backuppath ) )
  {
    msg = QObject::tr( "Master password FAILED to be reset" );
    level = Qgis::MessageLevel::Warning;
  }

  if ( !backuppath.isEmpty() )
  {
    msg += QObject::tr( " (database backup: %1)" ).arg( backuppath );
  }

  msgbar->pushMessage( QgsApplication::authManager()->authManTag(), msg, level );
}

void QgsAuthGuiUtils::clearCachedAuthenticationConfigs( QgsMessageBar *msgbar )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar ) )
    return;

  QgsApplication::authManager()->clearAllCachedConfigs();
  const QString msg = QObject::tr( "Cached authentication configurations for session cleared" );
  msgbar->pushMessage( QgsApplication::authManager()->authManTag(), msg, Qgis::MessageLevel::Info );
}

void QgsAuthGuiUtils::removeAuthenticationConfigs( QgsMessageBar *msgbar, QWidget *parent )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar ) )
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

  QString msg( QObject::tr( "Authentication configurations removed." ) );
  Qgis::MessageLevel level( Qgis::MessageLevel::Info );

  if ( !QgsApplication::authManager()->removeAllAuthenticationConfigs() )
  {
    msg = QObject::tr( "Authentication configurations FAILED to be removed." );
    level = Qgis::MessageLevel::Warning;
  }

  msgbar->pushMessage( QgsApplication::authManager()->authManTag(), msg, level );
}

void QgsAuthGuiUtils::eraseAuthenticationDatabase( QgsMessageBar *msgbar, QWidget *parent )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar ) )
    return;

  const QMessageBox::StandardButton btn = QMessageBox::warning(
      parent,
      QObject::tr( "Erase Database" ),
      QObject::tr( "Are you sure you want to ERASE the entire authentication database?\n\n"
                   "Operation can NOT be undone!\n\n"
                   "(Current database will be backed up and new one created.)" ),
      QMessageBox::Ok | QMessageBox::Cancel,
      QMessageBox::Cancel );

  QgsApplication::authManager()->setScheduledAuthDatabaseErase( false );

  if ( btn == QMessageBox::Cancel )
  {
    return;
  }

  QString msg( QObject::tr( "Active authentication database erased." ) );
  Qgis::MessageLevel level( Qgis::MessageLevel::Warning );

  QString backuppath;
  if ( !QgsApplication::authManager()->eraseAuthenticationDatabase( true, &backuppath ) )
  {
    msg = QObject::tr( "Authentication database FAILED to be erased." );
    level = Qgis::MessageLevel::Warning;
  }
  else
  {
    if ( !backuppath.isEmpty() )
    {
      msg += QObject::tr( " (backup: %1)" ).arg( backuppath );
    }
    level = Qgis::MessageLevel::Critical;
  }

  msgbar->pushMessage( QObject::tr( "RESTART QGIS" ), msg, level );
}

void QgsAuthGuiUtils::fileFound( bool found, QWidget *widget )
{
  if ( !found )
  {
    widget->setStyleSheet( QgsAuthGuiUtils::redTextStyleSheet( QStringLiteral( "QLineEdit" ) ) );
    widget->setToolTip( QObject::tr( "File not found" ) );
  }
  else
  {
    widget->setStyleSheet( QString() );
    widget->setToolTip( QString() );
  }
}

QString QgsAuthGuiUtils::getOpenFileName( QWidget *parent, const QString &title, const QString &extfilter )
{
  QgsSettings settings;
  const QString recentdir = settings.value( QStringLiteral( "UI/lastAuthOpenFileDir" ), QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( parent, title, recentdir, extfilter );
  if ( !f.isEmpty() )
  {
    settings.setValue( QStringLiteral( "UI/lastAuthOpenFileDir" ), QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}

void QgsAuthGuiUtils::passwordHelperDelete( QgsMessageBar *msgbar, QWidget *parent )
{
  if ( QMessageBox::warning( parent,
                             QObject::tr( "Delete Password" ),
                             QObject::tr( "Do you really want to delete the master password from your %1?" )
                             .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }
  QString msg;
  Qgis::MessageLevel level;
  if ( ! QgsApplication::authManager()->passwordHelperDelete() )
  {
    msg = QgsApplication::authManager()->passwordHelperErrorMessage();
    level = Qgis::MessageLevel::Warning;
  }
  else
  {
    msg = QObject::tr( "Master password was successfully deleted from your %1" )
          .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME );

    level = Qgis::MessageLevel::Info;
  }
  msgbar->pushMessage( QObject::tr( "Password helper delete" ), msg, level );
}

void QgsAuthGuiUtils::passwordHelperSync( QgsMessageBar *msgbar )
{
  QString msg;
  Qgis::MessageLevel level;
  if ( ! QgsApplication::authManager()->masterPasswordIsSet() )
  {
    msg = QObject::tr( "Master password is not set and cannot be stored in your %1." )
          .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME );
    level = Qgis::MessageLevel::Warning;
  }
  else if ( ! QgsApplication::authManager()->passwordHelperSync() )
  {
    msg = QgsApplication::authManager()->passwordHelperErrorMessage();
    level = Qgis::MessageLevel::Warning;
  }
  else
  {
    msg = QObject::tr( "Master password has been successfully stored in your %1." )
          .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME );

    level = Qgis::MessageLevel::Info;
  }
  msgbar->pushMessage( QObject::tr( "Password helper write" ), msg, level );
}

void QgsAuthGuiUtils::passwordHelperEnable( bool enabled, QgsMessageBar *msgbar )
{
  QgsApplication::authManager()->setPasswordHelperEnabled( enabled );
  const QString msg = enabled ? QObject::tr( "Your %1 will be <b>used from now</b> on to store and retrieve the master password." )
                      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ) :
                      QObject::tr( "Your %1 will <b>not be used anymore</b> to store and retrieve the master password." )
                      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME );
  msgbar->pushMessage( QObject::tr( "Password helper write" ), msg, Qgis::MessageLevel::Info );
}

void QgsAuthGuiUtils::passwordHelperLoggingEnable( bool enabled, QgsMessageBar *msgbar, int timeout )
{
  Q_UNUSED( msgbar )
  Q_UNUSED( timeout )
  QgsApplication::authManager()->setPasswordHelperLoggingEnabled( enabled );
}
