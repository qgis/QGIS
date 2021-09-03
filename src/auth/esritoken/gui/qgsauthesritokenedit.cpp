/***************************************************************************
    qgsauthesritokenedit.cpp
    ------------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    author               : Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthesritokenedit.h"
#include "ui_qgsauthesritokenedit.h"


QgsAuthEsriTokenEdit::QgsAuthEsriTokenEdit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( mTokenEdit, &QPlainTextEdit::textChanged, this, &QgsAuthEsriTokenEdit::tokenChanged );
}

bool QgsAuthEsriTokenEdit::validateConfig()
{
  const bool curvalid = !mTokenEdit->toPlainText().isEmpty();
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthEsriTokenEdit::configMap() const
{
  QgsStringMap config;
  config.insert( QStringLiteral( "token" ), mTokenEdit->toPlainText() );

  return config;
}

void QgsAuthEsriTokenEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  mTokenEdit->setPlainText( configmap.value( QStringLiteral( "token" ) ) );

  validateConfig();
}

void QgsAuthEsriTokenEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthEsriTokenEdit::clearConfig()
{
  mTokenEdit->clear();
}

void QgsAuthEsriTokenEdit::tokenChanged()
{
  validateConfig();
}
