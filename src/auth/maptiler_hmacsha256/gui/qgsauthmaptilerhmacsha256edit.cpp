/***************************************************************************
    qgsauthmaptilerhmacsha256edit.cpp
    ------------------------
    begin                : January 2022
    copyright            : (C) 2022 by Vincent Cloarec
    author               : Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthmaptilerhmacsha256edit.h"
#include "ui_qgsauthmaptilerhmacsha256edit.h"


QgsAuthMapTilerHmacSha256Edit::QgsAuthMapTilerHmacSha256Edit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( mTokenEdit, &QPlainTextEdit::textChanged, this, &QgsAuthMapTilerHmacSha256Edit::configChanged );
}

bool QgsAuthMapTilerHmacSha256Edit::validateConfig()
{
  const bool curvalid = !mTokenEdit->toPlainText().isEmpty();
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthMapTilerHmacSha256Edit::configMap() const
{
  QgsStringMap config;
  config.insert( QStringLiteral( "token" ), mTokenEdit->toPlainText() );

  return config;
}

void QgsAuthMapTilerHmacSha256Edit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  mTokenEdit->setPlainText( configmap.value( QStringLiteral( "token" ) ) );

  validateConfig();
}

void QgsAuthMapTilerHmacSha256Edit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthMapTilerHmacSha256Edit::clearConfig()
{
  mTokenEdit->clear();
}

void QgsAuthMapTilerHmacSha256Edit::configChanged()
{
  validateConfig();
}
