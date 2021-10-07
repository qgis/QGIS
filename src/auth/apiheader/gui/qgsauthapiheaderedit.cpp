/***************************************************************************
    qgsauthapiheaderedit.cpp
    ------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Tom Cummins
    author               : Tom Cummins
    email                : tom cumminsc9 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthapiheaderedit.h"
#include "ui_qgsauthapiheaderedit.h"


QgsAuthApiHeaderEdit::QgsAuthApiHeaderEdit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( headerKeyEdit, &QLineEdit::textChanged, this, &QgsAuthApiHeaderEdit::textChanged );
}

bool QgsAuthApiHeaderEdit::validateConfig()
{
  const bool curvalid = !headerKeyEdit->text().isEmpty();
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthApiHeaderEdit::configMap() const
{
  QgsStringMap config;
  config.insert( QStringLiteral( "headerKey" ), headerKeyEdit->text() );
  config.insert( QStringLiteral( "headerValue" ), headerValueEdit->toPlainText() );

  return config;
}

void QgsAuthApiHeaderEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  headerKeyEdit->setText( configmap.value( QStringLiteral( "headerKey" ) ) );
  headerValueEdit->setPlainText( configmap.value( QStringLiteral( "headerValue" ) ) );

  validateConfig();
}

void QgsAuthApiHeaderEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthApiHeaderEdit::clearConfig()
{
  headerKeyEdit->clear();
  headerValueEdit->clear();
}

void QgsAuthApiHeaderEdit::textChanged( const QString &txt )
{
  Q_UNUSED( txt )
  validateConfig();
}
