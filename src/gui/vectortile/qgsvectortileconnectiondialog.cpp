/***************************************************************************
    qgsvectortileconnectiondialog.cpp
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortileconnectiondialog.h"
#include "qgsvectortileconnection.h"
#include "qgsgui.h"
#include <QMessageBox>

QgsVectorTileConnectionDialog::QgsVectorTileConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  // Behavior for min and max zoom checkbox
  connect( mCheckBoxZMin, &QCheckBox::toggled, mSpinZMin, &QSpinBox::setEnabled );
  connect( mCheckBoxZMax, &QCheckBox::toggled, mSpinZMax, &QSpinBox::setEnabled );
}

void QgsVectorTileConnectionDialog::setConnection( const QgsVectorTileConnection &conn )
{
  mEditName->setText( conn.name );
  mEditUrl->setText( conn.url );
  mCheckBoxZMin->setChecked( conn.zMin != -1 );
  mSpinZMin->setValue( conn.zMin != -1 ? conn.zMin : 0 );
  mCheckBoxZMax->setChecked( conn.zMax != -1 );
  mSpinZMax->setValue( conn.zMax != -1 ? conn.zMax : 14 );
}

QgsVectorTileConnection QgsVectorTileConnectionDialog::connection() const
{
  QgsVectorTileConnection conn;
  conn.name = mEditName->text();
  conn.url = mEditUrl->text();
  if ( mCheckBoxZMin->isChecked() )
    conn.zMin = mSpinZMin->value();
  if ( mCheckBoxZMax->isChecked() )
    conn.zMax = mSpinZMax->value();
  return conn;
}

void QgsVectorTileConnectionDialog::accept()
{
  if ( mCheckBoxZMin->isChecked() && mCheckBoxZMax->isChecked() && mSpinZMax->value() < mSpinZMin->value() )
  {
    QMessageBox::warning( this, tr( "Connection Properties" ), tr( "The maximum zoom level (%1) cannot be lower than the minimum zoom level (%2)." ).arg( mSpinZMax->value() ).arg( mSpinZMin->value() ) );
    return;
  }
  QDialog::accept();
}
