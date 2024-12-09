/***************************************************************************
    qgssensorthingsconnectiondialog.cpp
    ---------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsconnectiondialog.h"
#include "moc_qgssensorthingsconnectiondialog.cpp"
#include "qgssensorthingsconnection.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgssensorthingsconnectionwidget.h"

#include <QPushButton>
#include <QMessageBox>

///@cond PRIVATE

QgsSensorThingsConnectionDialog::QgsSensorThingsConnectionDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mConnectionWidget = new QgsSensorThingsConnectionWidget();
  QHBoxLayout *hlayout = new QHBoxLayout();
  hlayout->addWidget( mConnectionWidget );
  mConnectionGroupBox->setLayout( hlayout );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html" ) );
  } );
  connect( mEditName, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionDialog::updateOkButtonState );
  connect( mConnectionWidget, &QgsSensorThingsConnectionWidget::validChanged, this, &QgsSensorThingsConnectionDialog::updateOkButtonState );
}

void QgsSensorThingsConnectionDialog::setConnection( const QString &name, const QString &uri )
{
  mEditName->setText( name );

  const QgsSensorThingsProviderConnection::Data conn = QgsSensorThingsProviderConnection::decodedUri( uri );

  mConnectionWidget->setUrl( conn.url );
  mConnectionWidget->setUsername( conn.username );
  mConnectionWidget->setPassword( conn.password );
  mConnectionWidget->setReferer( conn.httpHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  mConnectionWidget->setAuthCfg( conn.authCfg );
}

QString QgsSensorThingsConnectionDialog::connectionUri() const
{
  QgsSensorThingsProviderConnection::Data conn;
  conn.url = mConnectionWidget->url();

  conn.username = mConnectionWidget->username();
  conn.password = mConnectionWidget->password();
  conn.httpHeaders[QgsHttpHeaders::KEY_REFERER] = mConnectionWidget->referer();
  conn.authCfg = mConnectionWidget->authcfg();

  return QgsSensorThingsProviderConnection::encodedUri( conn );
}

QString QgsSensorThingsConnectionDialog::connectionName() const
{
  return mEditName->text();
}

void QgsSensorThingsConnectionDialog::updateOkButtonState()
{
  const bool enabled = !mEditName->text().isEmpty() && !mConnectionWidget->url().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

void QgsSensorThingsConnectionDialog::accept()
{
  // validate here if required
  QDialog::accept();
}

///@endcond PRIVATE
