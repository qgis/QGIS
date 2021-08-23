/***************************************************************************
                          qgscredentialdialog.cpp  -  description
                             -------------------
    begin                : February 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscredentialdialog.h"

#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsapplication.h"

#include <QPushButton>
#include <QMenu>
#include <QToolButton>
#include <QThread>
#include <QTimer>
#include <QGlobalStatic>

QMutex QgsCredentialDialog::sIgnoredConnectionsCacheMutex;
typedef QSet<QString> IgnoredConnectionsSet;

//! Temporary cache for ignored connections, to avoid GUI freezing by multiple credentials requests to the same connection
Q_GLOBAL_STATIC( IgnoredConnectionsSet, sIgnoredConnectionsCache );


static QString invalidStyle_( const QString &selector = QStringLiteral( "QLineEdit" ) )
{
  return QStringLiteral( "%1{color: rgb(200, 0, 0);}" ).arg( selector );
}

QgsCredentialDialog::QgsCredentialDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )

{
  setupUi( this );
  connect( leMasterPass, &QgsPasswordLineEdit::textChanged, this, &QgsCredentialDialog::leMasterPass_textChanged );
  connect( leMasterPassVerify, &QgsPasswordLineEdit::textChanged, this, &QgsCredentialDialog::leMasterPassVerify_textChanged );
  connect( chkbxEraseAuthDb, &QCheckBox::toggled, this, &QgsCredentialDialog::chkbxEraseAuthDb_toggled );
  setInstance( this );
  connect( this, &QgsCredentialDialog::credentialsRequested,
           this, &QgsCredentialDialog::requestCredentials,
           Qt::BlockingQueuedConnection );
  connect( this, &QgsCredentialDialog::credentialsRequestedMasterPassword,
           this, &QgsCredentialDialog::requestCredentialsMasterPassword,
           Qt::BlockingQueuedConnection );

  // Setup ignore button
  mIgnoreButton->setToolTip( tr( "All requests for this connection will be automatically rejected" ) );
  QMenu *menu = new QMenu( mIgnoreButton );
  QAction *ignoreTemporarily = new QAction( tr( "Ignore for 10 Seconds" ), menu );
  ignoreTemporarily->setToolTip( tr( "All requests for this connection will be automatically rejected for 10 seconds" ) );
  QAction *ignoreForSession = new QAction( tr( "Ignore for Session" ), menu );
  ignoreForSession->setToolTip( tr( "All requests for this connection will be automatically rejected for the duration of the current session" ) );
  menu->addAction( ignoreTemporarily );
  menu->addAction( ignoreForSession );
  connect( ignoreTemporarily, &QAction::triggered, this, [ = ]
  {
    mIgnoreMode = IgnoreTemporarily;
    mIgnoreButton->setText( ignoreTemporarily->text() );
    mIgnoreButton->setToolTip( ignoreTemporarily->toolTip() );
  } );
  connect( ignoreForSession, &QAction::triggered, this, [ = ]
  {
    mIgnoreMode = IgnoreForSession;
    mIgnoreButton->setText( ignoreForSession->text() );
    mIgnoreButton->setToolTip( ignoreForSession->toolTip() );
  } );
  mIgnoreButton->setText( mIgnoreMode == IgnoreTemporarily ? ignoreTemporarily->text() : ignoreForSession->text() );
  mIgnoreButton->setToolTip( mIgnoreMode == IgnoreTemporarily ? ignoreTemporarily->toolTip() : ignoreForSession->toolTip() );
  mIgnoreButton->setMenu( menu );
  mIgnoreButton->setMaximumHeight( mOkButton->sizeHint().height() );

  // Connect ok and cancel buttons
  connect( mOkButton, &QPushButton::clicked, this, &QgsCredentialDialog::accept );
  connect( mCancelButton, &QPushButton::clicked, this, &QgsCredentialDialog::reject );

  // Keep a cache of ignored connections, and ignore them for 10 seconds.
  connect( mIgnoreButton, &QPushButton::clicked, this, [ = ]( bool )
  {
    const QString realm { labelRealm->text() };
    {
      const QMutexLocker locker( &sIgnoredConnectionsCacheMutex );
      // Insert the realm in the cache of ignored connections
      sIgnoredConnectionsCache->insert( realm );
    }
    if ( mIgnoreMode == IgnoreTemporarily )
    {
      QTimer::singleShot( 10000, nullptr, [ = ]()
      {
        QgsDebugMsgLevel( QStringLiteral( "Removing ignored connection from cache: %1" ).arg( realm ), 4 );
        const QMutexLocker locker( &sIgnoredConnectionsCacheMutex );
        sIgnoredConnectionsCache->remove( realm );
      } );
    }
    accept( );
  } );

  leMasterPass->setPlaceholderText( tr( "Required" ) );
  chkbxPasswordHelperEnable->setText( tr( "Store/update the master password in your %1" )
                                      .arg( QgsAuthManager::AUTH_PASSWORD_HELPER_DISPLAY_NAME ) );
  leUsername->setFocus();
}

bool QgsCredentialDialog::request( const QString &realm, QString &username, QString &password, const QString &message )
{
  bool ok;
  if ( qApp->thread() != QThread::currentThread() )
  {
    QgsDebugMsg( QStringLiteral( "emitting signal" ) );
    emit credentialsRequested( realm, &username, &password, message, &ok );
    QgsDebugMsg( QStringLiteral( "signal returned %1 (username=%2)" ).arg( ok ? "true" : "false", username ) );
  }
  else
  {
    requestCredentials( realm, &username, &password, message, &ok );
  }
  return ok;
}

void QgsCredentialDialog::requestCredentials( const QString &realm, QString *username, QString *password, const QString &message, bool *ok )
{
  Q_ASSERT( qApp->thread() == thread() && thread() == QThread::currentThread() );
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 4 );
  {
    const QMutexLocker locker( &sIgnoredConnectionsCacheMutex );
    if ( sIgnoredConnectionsCache->contains( realm ) )
    {
      QgsDebugMsg( QStringLiteral( "Skipping ignored connection: " ) + realm );
      *ok = false;
      return;
    }
  }
  stackedWidget->setCurrentIndex( 0 );
  mIgnoreButton->show();
  chkbxPasswordHelperEnable->setChecked( QgsApplication::authManager()->passwordHelperEnabled() );
  labelRealm->setText( realm );
  leUsername->setText( *username );
  lePassword->setText( *password );
  labelMessage->setText( message );
  labelMessage->setHidden( message.isEmpty() );

  if ( leUsername->text().isEmpty() )
    leUsername->setFocus();
  else
    lePassword->setFocus();

  QWidget *activeWindow = qApp->activeWindow();

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  QgsDebugMsgLevel( QStringLiteral( "exec()" ), 4 );
  *ok = exec() == QDialog::Accepted;
  QgsDebugMsgLevel( QStringLiteral( "exec(): %1" ).arg( *ok ? "true" : "false" ), 4 );

  QApplication::restoreOverrideCursor();

  if ( activeWindow )
    activeWindow->raise();

  if ( *ok )
  {
    *username = leUsername->text();
    *password = lePassword->text();
  }
}

bool QgsCredentialDialog::requestMasterPassword( QString &password, bool stored )
{
  bool ok;
  if ( qApp->thread() != QThread::currentThread() )
  {
    QgsDebugMsgLevel( QStringLiteral( "emitting signal" ), 4 );
    emit credentialsRequestedMasterPassword( &password, stored, &ok );
  }
  else
  {
    requestCredentialsMasterPassword( &password, stored, &ok );
  }
  return ok;
}

void QgsCredentialDialog::requestCredentialsMasterPassword( QString *password, bool stored, bool *ok )
{
  QgsDebugMsgLevel( QStringLiteral( "Entering." ), 4 );
  stackedWidget->setCurrentIndex( 1 );

  mIgnoreButton->hide();
  leMasterPass->setFocus();

  const QString titletxt( stored ? tr( "Enter CURRENT master authentication password" ) : tr( "Set NEW master authentication password" ) );
  lblPasswordTitle->setText( titletxt );

  chkbxPasswordHelperEnable->setChecked( QgsApplication::authManager()->passwordHelperEnabled() );

  leMasterPassVerify->setVisible( !stored );
  lblDontForget->setVisible( !stored );

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  grpbxPassAttempts->setVisible( false );
  int passfailed = 0;
  while ( true )
  {
    mOkButton->setEnabled( false );
    // TODO: have the number of attempted passwords configurable in auth settings?
    if ( passfailed >= 3 )
    {
      lblSavedForSession->setVisible( false );
      grpbxPassAttempts->setTitle( tr( "Password attempts: %1" ).arg( passfailed ) );
      grpbxPassAttempts->setVisible( true );
    }

    // resize vertically to fit contents
    QSize s = sizeHint();
    s.setWidth( width() );
    resize( s );

    QgsDebugMsgLevel( QStringLiteral( "exec()" ), 4 );
    *ok = exec() == QDialog::Accepted;
    QgsDebugMsgLevel( QStringLiteral( "exec(): %1" ).arg( *ok ? "true" : "false" ), 4 );

    if ( *ok )
    {
      bool passok = !leMasterPass->text().isEmpty();
      if ( passok && stored && !chkbxEraseAuthDb->isChecked() )
      {
        passok = QgsApplication::authManager()->verifyMasterPassword( leMasterPass->text() );
      }

      if ( passok && !stored )
      {
        passok = ( leMasterPass->text() == leMasterPassVerify->text() );
      }

      if ( passok || chkbxEraseAuthDb->isChecked() )
      {
        if ( stored && chkbxEraseAuthDb->isChecked() )
        {
          QgsApplication::authManager()->setScheduledAuthDatabaseErase( true );
        }
        else
        {
          *password = leMasterPass->text();
          // Let's store user's preferences to use the password helper
          if ( chkbxPasswordHelperEnable->isChecked() != QgsApplication::authManager()->passwordHelperEnabled() )
          {
            QgsApplication::authManager()->setPasswordHelperEnabled( chkbxPasswordHelperEnable->isChecked() );
          }
        }
        break;
      }
      else
      {
        if ( stored )
          ++passfailed;

        leMasterPass->setStyleSheet( invalidStyle_() );
        if ( leMasterPassVerify->isVisible() )
        {
          leMasterPassVerify->setStyleSheet( invalidStyle_() );
        }
      }
    }
    else
    {
      break;
    }

    if ( passfailed >= 5 )
    {
      break;
    }
  }

  // don't leave master password in singleton's text field, or the ability to show it
  leMasterPass->clear();
  leMasterPassVerify->clear();

  chkbxEraseAuthDb->setChecked( false );
  lblSavedForSession->setVisible( true );

  // re-enable OK button or non-master-password requests will be blocked
  // needs to come after leMasterPass->clear() or textChanged auto-slot with disable it again
  mOkButton->setEnabled( true );

  QApplication::restoreOverrideCursor();

  if ( passfailed >= 5 )
  {
    close();
  }
}

void QgsCredentialDialog::leMasterPass_textChanged( const QString &pass )
{
  leMasterPass->setStyleSheet( QString() );
  bool passok = !pass.isEmpty(); // regardless of new or comparing existing, empty password disallowed
  if ( leMasterPassVerify->isVisible() )
  {
    leMasterPassVerify->setStyleSheet( QString() );
    passok = passok && ( leMasterPass->text() == leMasterPassVerify->text() );
  }
  mOkButton->setEnabled( passok );

  if ( leMasterPassVerify->isVisible() && !passok )
  {
    leMasterPass->setStyleSheet( invalidStyle_() );
    leMasterPassVerify->setStyleSheet( invalidStyle_() );
  }
}

void QgsCredentialDialog::leMasterPassVerify_textChanged( const QString &pass )
{
  if ( leMasterPassVerify->isVisible() )
  {
    leMasterPass->setStyleSheet( QString() );
    leMasterPassVerify->setStyleSheet( QString() );

    // empty password disallowed
    const bool passok = !pass.isEmpty() && ( leMasterPass->text() == leMasterPassVerify->text() );
    mOkButton->setEnabled( passok );
    if ( !passok )
    {
      leMasterPass->setStyleSheet( invalidStyle_() );
      leMasterPassVerify->setStyleSheet( invalidStyle_() );
    }
  }
}

void QgsCredentialDialog::chkbxEraseAuthDb_toggled( bool checked )
{
  if ( checked )
    mOkButton->setEnabled( true );
}

