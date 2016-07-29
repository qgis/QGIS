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
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>

#include "qgsauthmanager.h"
#include "qgsauthmasterpassresetdialog.h"
#include "qgslogger.h"
#include "qgsmessagebar.h"


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
  return QString( "%1{color: %2;}" ).arg( selector, QgsAuthGuiUtils::greenColor().name() );
}

QString QgsAuthGuiUtils::orangeTextStyleSheet( const QString &selector )
{
  return QString( "%1{color: %2;}" ).arg( selector, QgsAuthGuiUtils::orangeColor().name() );
}

QString QgsAuthGuiUtils::redTextStyleSheet( const QString &selector )
{
  return QString( "%1{color: %2;}" ).arg( selector, QgsAuthGuiUtils::redColor().name() );
}

bool QgsAuthGuiUtils::isDisabled( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthManager::instance()->isDisabled() )
  {
    msgbar->pushMessage( QObject::tr( "Authentication System" ),
                         QObject::tr( "DISABLED. Resources authenticating via the system can not be accessed" ),
                         QgsMessageBar::CRITICAL, timeout );
    return true;
  }
  return false;
}

void QgsAuthGuiUtils::setMasterPassword( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar, timeout ) )
    return;

  if ( QgsAuthManager::instance()->masterPasswordIsSet() )
  {
    msgbar->pushMessage( QgsAuthManager::instance()->authManTag(),
                         QObject::tr( "Master password already set" ),
                         QgsMessageBar::INFO, timeout );
    return;
  }
  ( void )QgsAuthManager::instance()->setMasterPassword( true );
}

void QgsAuthGuiUtils::clearCachedMasterPassword( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar, timeout ) )
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

void QgsAuthGuiUtils::resetMasterPassword( QgsMessageBar *msgbar, int timeout, QWidget *parent )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar, timeout ) )
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

void QgsAuthGuiUtils::clearCachedAuthenticationConfigs( QgsMessageBar *msgbar, int timeout )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar, timeout ) )
    return;

  QgsAuthManager::instance()->clearAllCachedConfigs();
  QString msg = QObject::tr( "Cached authentication configurations for session cleared" );
  msgbar->pushMessage( QgsAuthManager::instance()->authManTag(), msg, QgsMessageBar::INFO, timeout );
}

void QgsAuthGuiUtils::removeAuthenticationConfigs( QgsMessageBar *msgbar, int timeout, QWidget *parent )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar, timeout ) )
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

void QgsAuthGuiUtils::eraseAuthenticationDatabase( QgsMessageBar *msgbar, int timeout, QWidget *parent )
{
  if ( QgsAuthGuiUtils::isDisabled( msgbar, timeout ) )
    return;

  QMessageBox::StandardButton btn =  QMessageBox::warning(
                                       parent,
                                       QObject::tr( "Erase Database" ),
                                       QObject::tr( "Are you sure you want to ERASE the entire authentication database?\n\n"
                                                    "Operation can NOT be undone!\n\n"
                                                    "(Current database will be backed up and new one created.)" ),
                                       QMessageBox::Ok | QMessageBox::Cancel,
                                       QMessageBox::Cancel );

  QgsAuthManager::instance()->setScheduledAuthDbErase( false );

  if ( btn == QMessageBox::Cancel )
  {
    return;
  }

  QString msg( QObject::tr( "Active authentication database erased" ) );
  QgsMessageBar::MessageLevel level( QgsMessageBar::WARNING );

  QString backuppath;
  if ( !QgsAuthManager::instance()->eraseAuthenticationDatabase( true, &backuppath ) )
  {
    msg = QObject::tr( "Authentication database FAILED to be erased" );
    level = QgsMessageBar::WARNING;
  }
  else
  {
    if ( !backuppath.isEmpty() )
    {
      msg += QObject::tr( " (backup: %1)" ).arg( backuppath );
    }
    level = QgsMessageBar::CRITICAL;
  }

  timeout = 0; // no timeout, so user can read restart message
  msgbar->pushMessage( QObject::tr( "RESTART QGIS" ), msg, level, timeout );
}

void QgsAuthGuiUtils::fileFound( bool found, QWidget *widget )
{
  if ( !found )
  {
    widget->setStyleSheet( QgsAuthGuiUtils::redTextStyleSheet( "QLineEdit" ) );
    widget->setToolTip( QObject::tr( "File not found" ) );
  }
  else
  {
    widget->setStyleSheet( "" );
    widget->setToolTip( "" );
  }
}

QString QgsAuthGuiUtils::getOpenFileName( QWidget *parent, const QString &title, const QString &extfilter )
{
  QSettings settings;
  QString recentdir = settings.value( "UI/lastAuthOpenFileDir", QDir::homePath() ).toString();
  QString f = QFileDialog::getOpenFileName( parent, title, recentdir, extfilter );
  if ( !f.isEmpty() )
  {
    settings.setValue( "UI/lastAuthOpenFileDir", QFileInfo( f ).absoluteDir().path() );
  }
  return f;
}
