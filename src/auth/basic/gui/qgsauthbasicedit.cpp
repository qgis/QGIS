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

#include "ui_qgsauthbasicedit.h"
#include "qgsauthbasicedit.h"

#include "moc_qgsauthbasicedit.cpp"

QgsAuthBasicEdit::QgsAuthBasicEdit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( leUsername, &QLineEdit::textChanged, this, &QgsAuthBasicEdit::leUsername_textChanged );
}

bool QgsAuthBasicEdit::validateConfig()
{
  const bool curvalid = !leUsername->text().isEmpty();
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
  config.insert( u"username"_s, leUsername->text() );
  config.insert( u"password"_s, lePassword->text() );
  config.insert( u"realm"_s, leRealm->text() );

  return config;
}

void QgsAuthBasicEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  leUsername->setText( configmap.value( u"username"_s ) );
  lePassword->setText( configmap.value( u"password"_s ) );
  leRealm->setText( configmap.value( u"realm"_s ) );

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
}

void QgsAuthBasicEdit::leUsername_textChanged( const QString &txt )
{
  Q_UNUSED( txt )
  validateConfig();
}
