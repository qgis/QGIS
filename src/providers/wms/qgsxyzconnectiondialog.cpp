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

#include "qgsxyzconnectiondialog.h"
#include "qgsxyzconnection.h"
#include "qgsgui.h"
#include <QMessageBox>

QgsXyzConnectionDialog::QgsXyzConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

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
  mAuthSettings->setUsername( conn.username );
  mAuthSettings->setPassword( conn.password );
  mEditReferer->setText( conn.referer );
  int index = 0;  // default is "unknown"
  if ( conn.tilePixelRatio == 2. )
    index = 2;  // high-res
  else if ( conn.tilePixelRatio == 1. )
    index = 1;  // normal-res
  mComboTileResolution->setCurrentIndex( index );
  mAuthSettings->setConfigId( conn.authCfg );
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
  conn.username = mAuthSettings->username();
  conn.password = mAuthSettings->password();
  conn.referer = mEditReferer->text();
  if ( mComboTileResolution->currentIndex() == 1 )
    conn.tilePixelRatio = 1.;  // normal-res
  else if ( mComboTileResolution->currentIndex() == 2 )
    conn.tilePixelRatio = 2.;  // high-res
  else
    conn.tilePixelRatio = 0;  // unknown
  conn.authCfg = mAuthSettings->configId( );
  return conn;
}

void QgsXyzConnectionDialog::accept()
{
  if ( mCheckBoxZMin->isChecked() && mCheckBoxZMax->isChecked() && mSpinZMax->value() < mSpinZMin->value() )
  {
    QMessageBox::warning( this, tr( "Connection Properties" ), tr( "The maximum zoom level (%1) cannot be lower than the minimum zoom level (%2)." ).arg( mSpinZMax->value() ).arg( mSpinZMin->value() ) );
    return;
  }
  QDialog::accept();
}
