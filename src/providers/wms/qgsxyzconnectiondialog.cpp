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
#include "qgshelp.h"
#include "qgsxyzsourcewidget.h"

#include <QMessageBox>

QgsXyzConnectionDialog::QgsXyzConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mSourceWidget = new QgsXyzSourceWidget();
  QHBoxLayout *hlayout = new QHBoxLayout();
  hlayout->addWidget( mSourceWidget );
  mConnectionGroupBox->setLayout( hlayout );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this,  [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#using-xyz-tile-services" ) );
  } );
  connect( mEditName, &QLineEdit::textChanged, this, &QgsXyzConnectionDialog::updateOkButtonState );
  connect( mSourceWidget, &QgsXyzSourceWidget::validChanged, this, &QgsXyzConnectionDialog::updateOkButtonState );
}

void QgsXyzConnectionDialog::setConnection( const QgsXyzConnection &conn )
{
  mEditName->setText( conn.name );
  mSourceWidget->setUrl( conn.url );
  mSourceWidget->setZMin( conn.zMin );
  mSourceWidget->setZMax( conn.zMax );
  mSourceWidget->setUsername( conn.username );
  mSourceWidget->setPassword( conn.password );
  mSourceWidget->setReferer( conn.httpHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  mSourceWidget->setTilePixelRatio( conn.tilePixelRatio );
  mSourceWidget->setAuthCfg( conn.authCfg );
  mSourceWidget->setInterpretation( conn.interpretation );
}

QgsXyzConnection QgsXyzConnectionDialog::connection() const
{
  QgsXyzConnection conn;
  conn.name = mEditName->text();
  conn.url = mSourceWidget->url();
  conn.zMin = mSourceWidget->zMin();
  conn.zMax = mSourceWidget->zMax();
  conn.username = mSourceWidget->username();
  conn.password = mSourceWidget->password();
  conn.httpHeaders[QgsHttpHeaders::KEY_REFERER] = mSourceWidget->referer();
  conn.tilePixelRatio = mSourceWidget->tilePixelRatio();
  conn.authCfg = mSourceWidget->authcfg( );
  conn.interpretation = mSourceWidget->interpretation();
  return conn;
}

void QgsXyzConnectionDialog::updateOkButtonState()
{
  const bool enabled = !mEditName->text().isEmpty() && !mSourceWidget->url().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsXyzConnectionDialog::accept()
{
  if ( mSourceWidget->zMin() != -1
       && mSourceWidget->zMax() != -1
       && mSourceWidget->zMax() < mSourceWidget->zMin() )
  {
    QMessageBox::warning( this, tr( "Connection Properties" ), tr( "The maximum zoom level (%1) cannot be lower than the minimum zoom level (%2)." ).arg( mSourceWidget->zMax() ).arg( mSourceWidget->zMin() ) );
    return;
  }
  QDialog::accept();
}
