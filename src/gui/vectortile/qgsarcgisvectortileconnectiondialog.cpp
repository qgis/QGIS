/***************************************************************************
    qgsarcgisvectortileconnectiondialog.cpp
    ---------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisvectortileconnectiondialog.h"
#include "qgsvectortileconnection.h"
#include "qgsgui.h"

#include <QMessageBox>
#include <QPushButton>

///@cond PRIVATE

QgsArcgisVectorTileConnectionDialog::QgsArcgisVectorTileConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( mEditName, &QLineEdit::textChanged, this, &QgsArcgisVectorTileConnectionDialog::updateOkButtonState );
  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsArcgisVectorTileConnectionDialog::updateOkButtonState );
}

void QgsArcgisVectorTileConnectionDialog::setConnection( const QString &name, const QString &uri )
{
  mEditName->setText( name );

  QgsVectorTileProviderConnection::Data conn = QgsVectorTileProviderConnection::decodedUri( uri );
  mEditUrl->setText( conn.url );
}

QString QgsArcgisVectorTileConnectionDialog::connectionUri() const
{
  QgsVectorTileProviderConnection::Data conn;
  conn.url = mEditUrl->text();
  conn.serviceType = QgsVectorTileProviderConnection::ArcgisVectorTileService;
  return QgsVectorTileProviderConnection::encodedUri( conn );
}

QString QgsArcgisVectorTileConnectionDialog::connectionName() const
{
  return mEditName->text();
}

void QgsArcgisVectorTileConnectionDialog::updateOkButtonState()
{
  bool enabled = !mEditName->text().isEmpty() && !mEditUrl->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

///@endcond
