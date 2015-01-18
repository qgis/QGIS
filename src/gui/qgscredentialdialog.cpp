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
#include "qgslogger.h"

#include <QSettings>
#include <QThread>

QgsCredentialDialog::QgsCredentialDialog( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  setInstance( this );
  connect( this, SIGNAL( credentialsRequested( QString, QString *, QString *, QString, bool * ) ),
           this, SLOT( requestCredentials( QString, QString *, QString *, QString, bool * ) ),
           Qt::BlockingQueuedConnection );
  connect( this, SIGNAL( credentialsRequestedMasterPassword( QString *, bool, bool * ) ),
           this, SLOT( requestCredentialsMasterPassword( QString *, bool, bool * ) ),
           Qt::BlockingQueuedConnection );
}

QgsCredentialDialog::~QgsCredentialDialog()
{
}

bool QgsCredentialDialog::request( QString realm, QString &username, QString &password, QString message )
{
  bool ok;
  if ( qApp->thread() != QThread::currentThread() )
  {
    QgsDebugMsg( "emitting signal" );
    emit credentialsRequested( realm, &username, &password, message, &ok );
    QgsDebugMsg( QString( "signal returned %1 (username=%2, password=%3)" ).arg( ok ? "true" : "false" ).arg( username ).arg( password ) );
  }
  else
  {
    requestCredentials( realm, &username, &password, message, &ok );
  }
  return ok;
}

void QgsCredentialDialog::requestCredentials( QString realm, QString *username, QString *password, QString message, bool *ok )
{
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

  QString titletxt( stored ? tr( "Enter CURRENT master authentication password" ) : tr( "Set NEW master authentication password" ) );
  lblPasswordTitle->setText( titletxt );

  lblDontForget->setVisible( !stored );

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  while ( true )
  {
    QgsDebugMsg( "exec()" );
    *ok = exec() == QDialog::Accepted;
    QgsDebugMsg( QString( "exec(): %1" ).arg( *ok ? "true" : "false" ) );

    if ( *ok )
    {
      if ( !leMasterPass->text().isEmpty() )
      {
        *password = leMasterPass->text();
        break;
      }
    }
    else
    {
      break;
    }
  }

  // don't leave master password in singleton's text field, or the ability to show it
  leMasterPass->clear();
  chkMasterPassShow->setChecked( false );

  QApplication::restoreOverrideCursor();
}

void QgsCredentialDialog::on_chkMasterPassShow_stateChanged( int state )
{
  leMasterPass->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}

