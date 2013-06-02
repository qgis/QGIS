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

QgsCredentialDialog::QgsCredentialDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  setInstance( this );
  connect( this, SIGNAL( credentialsRequested( QString, QString *, QString *, QString, bool * ) ),
           this, SLOT( requestCredentials( QString, QString *, QString *, QString, bool * ) ),
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
  labelRealm->setText( realm );
  leUsername->setText( *username );
  lePassword->setText( *password );
  labelMessage->setText( message );
  labelMessage->setHidden( message.isEmpty() );

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
