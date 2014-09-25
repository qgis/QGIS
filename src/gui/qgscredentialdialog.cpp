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

#include <QFileDialog>
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
  connect( this, SIGNAL( credentialsRequestedSslKey( QString *, QString *, bool, QString, QString, bool * ) ),
           this, SLOT( requestCredentialsSslKey( QString *, QString *, bool, QString, QString, bool * ) ),
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

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  QgsDebugMsg( "exec()" );
  *ok = exec() == QDialog::Accepted;
  QgsDebugMsg( QString( "exec(): %1" ).arg( *ok ? "true" : "false" ) );

  QApplication::restoreOverrideCursor();

  if ( *ok )
  {
    *username = leUsername->text();
    *password = lePassword->text();
  }
}

bool QgsCredentialDialog::requestSslKey( QString &password, QString &keypath,
    bool needskeypath, QString resource,
    QString message )
{
  bool ok;
  if ( qApp->thread() != QThread::currentThread() )
  {
    QgsDebugMsg( "emitting signal" );
    emit credentialsRequestedSslKey( &password, &keypath, needskeypath, resource, message, &ok );
  }
  else
  {
    requestCredentialsSslKey( &password, &keypath, needskeypath, resource, message, &ok );
  }
  return ok;
}

void QgsCredentialDialog::on_tbKeyPathSelect_clicked( bool chkd )
{
  Q_UNUSED( chkd );

  const QString& fn = QFileDialog::getOpenFileName( this, tr( "Open PEM Key File" ),
                      QDir::homePath(), tr( "PEM (*.pem *.key)" ) );
  if ( !fn.isEmpty() )
  {
    leKeyPath->setText( fn );
  }
}

void QgsCredentialDialog::requestCredentialsSslKey( QString *password, QString *keypath,
    bool needskeypath, QString resource,
    QString message, bool *ok )
{
  QgsDebugMsg( "Entering." );
  stackedWidget->setCurrentIndex( 1 );

  lblKeyResource->setText( resource );

  leKeyPath->setText( *keypath );
  lblKeyPath->setHidden( !needskeypath );
  frKeyPath->setHidden( !needskeypath );

  leKeyPassword->setText( *password );

  lblKeyMessage->setText( message );
  lblKeyMessage->setHidden( message.isEmpty() );

  if ( needskeypath )
  {
    tbKeyPathSelect->setFocus();
  }
  else
  {
    leKeyPassword->setPlaceholderText( "" );
    leKeyPassword->setFocus();
  }

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  QgsDebugMsg( "exec()" );
  *ok = exec() == QDialog::Accepted;
  QgsDebugMsg( QString( "exec(): %1" ).arg( *ok ? "true" : "false" ) );

  QApplication::restoreOverrideCursor();

  if ( *ok )
  {
    if ( needskeypath )
    {
      *keypath = leKeyPath->text();
    }
    *password = leKeyPassword->text();
  }
}

