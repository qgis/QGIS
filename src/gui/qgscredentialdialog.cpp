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

#include <QPushButton>
#include <QSettings>
#include <QThread>

static QString invalidStyle_( const QString& selector = "QLineEdit" )
{
  return QString( "%1{color: rgb(200, 0, 0);}" ).arg( selector );
}

QgsCredentialDialog::QgsCredentialDialog( QWidget *parent, const Qt::WindowFlags& fl )
    : QDialog( parent, fl )
    , mOkButton( nullptr )
{
  setupUi( this );
  setInstance( this );
  connect( this, SIGNAL( credentialsRequested( QString, QString *, QString *, QString, bool * ) ),
           this, SLOT( requestCredentials( QString, QString *, QString *, QString, bool * ) ),
           Qt::BlockingQueuedConnection );
  connect( this, SIGNAL( credentialsRequestedMasterPassword( QString *, bool, bool * ) ),
           this, SLOT( requestCredentialsMasterPassword( QString *, bool, bool * ) ),
           Qt::BlockingQueuedConnection );
  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  leMasterPass->setPlaceholderText( tr( "Required" ) );
  leUsername->setFocus();
}

QgsCredentialDialog::~QgsCredentialDialog()
{
}

bool QgsCredentialDialog::request( const QString& realm, QString &username, QString &password, const QString& message )
{
  bool ok;
  if ( qApp->thread() != QThread::currentThread() )
  {
    QgsDebugMsg( "emitting signal" );
    emit credentialsRequested( realm, &username, &password, message, &ok );
    QgsDebugMsg( QString( "signal returned %1 (username=%2, password=%3)" ).arg( ok ? "true" : "false", username, password ) );
  }
  else
  {
    requestCredentials( realm, &username, &password, message, &ok );
  }
  return ok;
}

void QgsCredentialDialog::requestCredentials( const QString& realm, QString *username, QString *password, const QString& message, bool *ok )
{
  Q_ASSERT( qApp->thread() == thread() && thread() == QThread::currentThread() );
  QgsDebugMsg( "Entering." );
  stackedWidget->setCurrentIndex( 0 );

  labelRealm->setText( realm );
  leUsername->setText( *username );
  lePassword->setText( *password );
  labelMessage->setText( message );
  labelMessage->setHidden( message.isEmpty() );

  if ( !leUsername->text().isEmpty() )
    lePassword->setFocus();

  QWidget *activeWindow = qApp->activeWindow();

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  QgsDebugMsg( "exec()" );
  *ok = exec() == QDialog::Accepted;
  QgsDebugMsg( QString( "exec(): %1" ).arg( *ok ? "true" : "false" ) );

  QApplication::restoreOverrideCursor();

  if ( activeWindow )
    activeWindow->raise();

  if ( *ok )
  {
    *username = leUsername->text();
    *password = lePassword->text();
  }
}

bool QgsCredentialDialog::requestMasterPassword( QString &password , bool stored )
{
  bool ok;
  if ( qApp->thread() != QThread::currentThread() )
  {
    QgsDebugMsg( "emitting signal" );
    emit credentialsRequestedMasterPassword( &password, stored, &ok );
  }
  else
  {
    requestCredentialsMasterPassword( &password, stored, &ok );
  }
  return ok;
}

void QgsCredentialDialog::requestCredentialsMasterPassword( QString * password, bool stored , bool *ok )
{
  QgsDebugMsg( "Entering." );
  stackedWidget->setCurrentIndex( 1 );
  leMasterPass->setFocus();

  QString titletxt( stored ? tr( "Enter CURRENT master authentication password" ) : tr( "Set NEW master authentication password" ) );
  lblPasswordTitle->setText( titletxt );

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

    QgsDebugMsg( "exec()" );
    *ok = exec() == QDialog::Accepted;
    QgsDebugMsg( QString( "exec(): %1" ).arg( *ok ? "true" : "false" ) );

    if ( *ok )
    {
      bool passok = !leMasterPass->text().isEmpty();
      if ( passok && stored && !chkbxEraseAuthDb->isChecked() )
      {
        passok = QgsAuthManager::instance()->verifyMasterPassword( leMasterPass->text() );
      }

      if ( passok && !stored )
      {
        passok = ( leMasterPass->text() == leMasterPassVerify->text() );
      }

      if ( passok || chkbxEraseAuthDb->isChecked() )
      {
        if ( stored && chkbxEraseAuthDb->isChecked() )
        {
          QgsAuthManager::instance()->setScheduledAuthDbErase( true );
        }
        else
        {
          *password = leMasterPass->text();
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
  chkMasterPassShow->setChecked( false );
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

void QgsCredentialDialog::on_chkMasterPassShow_stateChanged( int state )
{
  leMasterPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
  leMasterPassVerify->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

void QgsCredentialDialog::on_leMasterPass_textChanged( const QString &pass )
{
  leMasterPass->setStyleSheet( "" );
  bool passok = !pass.isEmpty(); // regardless of new or comparing existing, empty password disallowed
  if ( leMasterPassVerify->isVisible() )
  {
    leMasterPassVerify->setStyleSheet( "" );
    passok = passok && ( leMasterPass->text() == leMasterPassVerify->text() );
  }
  mOkButton->setEnabled( passok );

  if ( leMasterPassVerify->isVisible() && !passok )
  {
    leMasterPass->setStyleSheet( invalidStyle_() );
    leMasterPassVerify->setStyleSheet( invalidStyle_() );
  }
}

void QgsCredentialDialog::on_leMasterPassVerify_textChanged( const QString &pass )
{
  if ( leMasterPassVerify->isVisible() )
  {
    leMasterPass->setStyleSheet( "" );
    leMasterPassVerify->setStyleSheet( "" );

    // empty password disallowed
    bool passok = !pass.isEmpty() && ( leMasterPass->text() == leMasterPassVerify->text() );
    mOkButton->setEnabled( passok );
    if ( !passok )
    {
      leMasterPass->setStyleSheet( invalidStyle_() );
      leMasterPassVerify->setStyleSheet( invalidStyle_() );
    }
  }
}

void QgsCredentialDialog::on_chkbxEraseAuthDb_toggled( bool checked )
{
  if ( checked )
    mOkButton->setEnabled( true );
}

