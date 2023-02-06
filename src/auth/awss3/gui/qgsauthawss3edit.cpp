/***************************************************************************
  qgsauthawss3edit.cpp
  --------------------------------------
  Date                 : December 2022
  Copyright            : (C) 2022 by Jacky Volpes
  Email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthawss3edit.h"
#include "ui_qgsauthawss3edit.h"


QgsAuthAwsS3Edit::QgsAuthAwsS3Edit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( leUsername, &QLineEdit::textChanged, this, &QgsAuthAwsS3Edit::leUsername_textChanged );
  connect( lePassword, &QLineEdit::textChanged, this, &QgsAuthAwsS3Edit::lePassword_textChanged );
  connect( leRegion, &QLineEdit::textChanged, this, &QgsAuthAwsS3Edit::leRegion_textChanged );
  connect( chkPasswordShow, &QCheckBox::stateChanged, this, &QgsAuthAwsS3Edit::chkPasswordShow_stateChanged );
}

bool QgsAuthAwsS3Edit::validateConfig()
{
  const bool curvalid = !leUsername->text().isEmpty() && !lePassword->text().isEmpty() && !leRegion->text().isEmpty();
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthAwsS3Edit::configMap() const
{
  QgsStringMap config;
  config.insert( QStringLiteral( "username" ), leUsername->text() );
  config.insert( QStringLiteral( "password" ), lePassword->text() );
  config.insert( QStringLiteral( "region" ), leRegion->text() );

  return config;
}

void QgsAuthAwsS3Edit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  leUsername->setText( configmap.value( QStringLiteral( "username" ) ) );
  lePassword->setText( configmap.value( QStringLiteral( "password" ) ) );
  leRegion->setText( configmap.value( QStringLiteral( "region" ) ) );

  validateConfig();
}

void QgsAuthAwsS3Edit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthAwsS3Edit::clearConfig()
{
  leUsername->clear();
  lePassword->clear();
  leRegion->clear();
  chkPasswordShow->setChecked( false );
}

void QgsAuthAwsS3Edit::leUsername_textChanged( const QString &txt )
{
  Q_UNUSED( txt )
  validateConfig();
}

void QgsAuthAwsS3Edit::lePassword_textChanged( const QString &txt )
{
  Q_UNUSED( txt )
  validateConfig();
}

void QgsAuthAwsS3Edit::leRegion_textChanged( const QString &txt )
{
  Q_UNUSED( txt )
  validateConfig();
}

void QgsAuthAwsS3Edit::chkPasswordShow_stateChanged( int state )
{
  lePassword->setEchoMode( ( state > 0 ) ? QLineEdit::Normal : QLineEdit::Password );
}
