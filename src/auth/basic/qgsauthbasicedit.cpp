/***************************************************************************
    qgsauthbasicedit.cpp
    ---------------------
    begin                : September 1, 2015
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

#include "qgsauthbasicedit.h"
#include "ui_qgsauthbasicedit.h"


QgsAuthBasicEdit::QgsAuthBasicEdit( QWidget *parent )
    : QgsAuthMethodEdit( parent )
    , mValid( 0 )
{
  setupUi( this );
}

QgsAuthBasicEdit::~QgsAuthBasicEdit()
{
}

bool QgsAuthBasicEdit::validateConfig()
{
  bool curvalid = !leUsername->text().isEmpty();
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthBasicEdit::configMap() const
{
  QgsStringMap config;
  config.insert( "username", leUsername->text() );
  config.insert( "password", lePassword->text() );
  config.insert( "realm", leRealm->text() );

  return config;
}

void QgsAuthBasicEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  leUsername->setText( configmap.value( "username" ) );
  lePassword->setText( configmap.value( "password" ) );
  leRealm->setText( configmap.value( "realm" ) );

  validateConfig();
}

void QgsAuthBasicEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthBasicEdit::clearConfig()
{
  leUsername->clear();
  lePassword->clear();
  leRealm->clear();
  chkPasswordShow->setChecked( false );
}

void QgsAuthBasicEdit::on_leUsername_textChanged( const QString &txt )
{
  Q_UNUSED( txt );
  validateConfig();
}

void QgsAuthBasicEdit::on_chkPasswordShow_stateChanged( int state )
{
  lePassword->setEchoMode(( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}
