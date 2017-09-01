/***************************************************************************
    qgsxyzconnectiondialog.cpp
    ---------------------
    begin                : February 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthconfigselect.h"
#include "qgsxyzconnectiondialog.h"
#include "qgsxyzconnection.h"

QgsXyzConnectionDialog::QgsXyzConnectionDialog( QWidget *parent )
  : QDialog( parent )
  , mAuthConfigSelect( nullptr )
{
  setupUi( this );

  mAuthConfigSelect = new QgsAuthConfigSelect( this );
  mTabAuth->insertTab( 1, mAuthConfigSelect, tr( "Configurations" ) );
  // Behavior for min and max zoom checkbox
  connect( mCheckBoxZMin, &QCheckBox::toggled, mSpinZMin, &QSpinBox::setEnabled );
  connect( mCheckBoxZMax, &QCheckBox::toggled, mSpinZMax, &QSpinBox::setEnabled );
}

void QgsXyzConnectionDialog::setConnection( const QgsXyzConnection &conn )
{
  mEditName->setText( conn.name );
  mEditUrl->setText( conn.url );
  mCheckBoxZMin->setChecked( conn.zMin != -1 );
  mSpinZMin->setValue( conn.zMin != -1 ? conn.zMin : 0 );
  mCheckBoxZMax->setChecked( conn.zMax != -1 );
  mSpinZMax->setValue( conn.zMax != -1 ? conn.zMax : 18 );
  mEditUsername->setText( conn.username );
  mEditPassword->setText( conn.password );
  mEditReferer->setText( conn.referer );
  mAuthConfigSelect->setConfigId( conn.authCfg );
  if ( ! conn.authCfg.isEmpty( ) )
  {
    mTabAuth->setCurrentIndex( mTabAuth->indexOf( mAuthConfigSelect ) );
  }
  else
  {
    mTabAuth->setCurrentIndex( 0 );
  }
}

QgsXyzConnection QgsXyzConnectionDialog::connection() const
{
  QgsXyzConnection conn;
  conn.name = mEditName->text();
  conn.url = mEditUrl->text();
  if ( mCheckBoxZMin->isChecked() )
    conn.zMin = mSpinZMin->value();
  if ( mCheckBoxZMax->isChecked() )
    conn.zMax = mSpinZMax->value();
  conn.username = mEditUsername->text();
  conn.password = mEditPassword->text();
  conn.referer = mEditReferer->text();
  conn.authCfg = mAuthConfigSelect->configId( );
  return conn;
}
