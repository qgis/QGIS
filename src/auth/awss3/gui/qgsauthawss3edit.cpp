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

#include "ui_qgsauthawss3edit.h"
#include "qgsauthawss3edit.h"

#include "moc_qgsauthawss3edit.cpp"

QgsAuthAwsS3Edit::QgsAuthAwsS3Edit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( leUsername, &QLineEdit::textChanged, this, &QgsAuthAwsS3Edit::leUsername_textChanged );
  connect( lePassword, &QLineEdit::textChanged, this, &QgsAuthAwsS3Edit::lePassword_textChanged );
  connect( leRegion, &QLineEdit::textChanged, this, &QgsAuthAwsS3Edit::leRegion_textChanged );
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
  config.insert( u"username"_s, leUsername->text() );
  config.insert( u"password"_s, lePassword->text() );
  config.insert( u"region"_s, leRegion->text() );

  return config;
}

void QgsAuthAwsS3Edit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  leUsername->setText( configmap.value( u"username"_s ) );
  lePassword->setText( configmap.value( u"password"_s ) );
  leRegion->setText( configmap.value( u"region"_s ) );

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
