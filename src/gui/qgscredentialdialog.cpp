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
#include <QSettings>

QgsCredentialDialog::QgsCredentialDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  setInstance( this );
}

QgsCredentialDialog::~QgsCredentialDialog()
{
}

bool QgsCredentialDialog::request( QString realm, QString &username, QString &password, QString message )
{
  labelRealm->setText( realm );
  leUsername->setText( username );
  lePassword->setText( password );
  labelMessage->setText( message );
  labelMessage->setHidden( message.isEmpty() );

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  int res = exec();

  QApplication::restoreOverrideCursor();

  if ( res == QDialog::Accepted )
  {
    username = leUsername->text();
    password = lePassword->text();
    return true;
  }
  else
  {
    return false;
  }
}
