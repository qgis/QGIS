/***************************************************************************
    qgsnewarcgisrestconnection.cpp
                             -------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewarcgisrestconnection.h"
#include "moc_qgsnewarcgisrestconnection.cpp"
#include "qgsauthsettingswidget.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "qgsowsconnection.h"
#include "fromencodedcomponenthelper.h"
#include "qgssettingsentryimpl.h"

#include <QMessageBox>
#include <QUrl>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QUrlQuery>

QgsNewArcGisRestConnectionDialog::QgsNewArcGisRestConnectionDialog( QWidget *parent, const QString &connectionName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connectionName )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewArcGisRestConnectionDialog::showHelp );

  setWindowTitle( tr( "Create a New arcgisfeatureserver Connection" ) );

  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( QStringLiteral( "[^\\/]+" ) ), txtName ) );

  if ( !connectionName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    txtName->setText( connectionName );
    txtUrl->setText( QgsArcGisConnectionSettings::settingsUrl->value( connectionName ) );
    mHttpHeaders->setHeaders( QgsHttpHeaders( QgsArcGisConnectionSettings::settingsHeaders->value( connectionName ) ) );
    mUrlPrefix->setText( QgsArcGisConnectionSettings::settingsUrlPrefix->value( connectionName ) );

    // portal
    mContentEndPointLineEdit->setText( QgsArcGisConnectionSettings::settingsContentEndpoint->value( connectionName ) );
    mCommunityEndPointLineEdit->setText( QgsArcGisConnectionSettings::settingsCommunityEndpoint->value( connectionName ) );

    // Authentication
    mAuthSettings->setUsername( QgsArcGisConnectionSettings::settingsUsername->value( connectionName ) );
    mAuthSettings->setPassword( QgsArcGisConnectionSettings::settingsPassword->value( connectionName ) );
    mAuthSettings->setConfigId( QgsArcGisConnectionSettings::settingsAuthcfg->value( connectionName ) );
  }

  // Adjust height
  const int w = width();
  adjustSize();
  resize( w, height() );

  connect( txtName, &QLineEdit::textChanged, this, &QgsNewArcGisRestConnectionDialog::nameChanged );
  connect( txtUrl, &QLineEdit::textChanged, this, &QgsNewArcGisRestConnectionDialog::urlChanged );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsNewArcGisRestConnectionDialog::updateOkButtonState );
  connect( txtUrl, &QLineEdit::textChanged, this, &QgsNewArcGisRestConnectionDialog::updateOkButtonState );

  nameChanged( connectionName );
}

QString QgsNewArcGisRestConnectionDialog::name() const
{
  return txtName->text();
}

QString QgsNewArcGisRestConnectionDialog::url() const
{
  return txtUrl->text();
}

void QgsNewArcGisRestConnectionDialog::nameChanged( const QString &text )
{
  Q_UNUSED( text )
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
}

void QgsNewArcGisRestConnectionDialog::urlChanged( const QString &text )
{
  Q_UNUSED( text )
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
}

void QgsNewArcGisRestConnectionDialog::updateOkButtonState()
{
  const bool enabled = !txtName->text().isEmpty() && !txtUrl->text().isEmpty();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

bool QgsNewArcGisRestConnectionDialog::validate()
{
  const QString newName = txtName->text();
  bool newNameAlreadyExists = QgsArcGisConnectionSettings::sTreeConnectionArcgis->items().contains( newName );

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( newName, Qt::CaseInsensitive ) != 0 ) && newNameAlreadyExists && QMessageBox::question( this, tr( "Save Connection" ), tr( "Should the existing connection '%1' be overwritten?" ).arg( newName ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  if ( !mAuthSettings->password().isEmpty() && QMessageBox::question( this, tr( "Saving Passwords" ), tr( "WARNING: You have entered a password. It will be stored in unsecured plain text in your project files and your home directory (Unix-like OS) or user profile (Windows). If you want to avoid this, press Cancel and either:\n\na) Don't provide a password in the connection settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  return true;
}

QUrl QgsNewArcGisRestConnectionDialog::urlTrimmed() const
{
  QUrl url( txtUrl->text().trimmed() );
  const QUrlQuery query( url );
  const QList<QPair<QString, QString>> items = query.queryItems( QUrl::FullyEncoded );
  QHash<QString, QPair<QString, QString>> params;
  for ( const QPair<QString, QString> &it : items )
  {
    params.insert( it.first.toUpper(), it );
  }

  url.setQuery( query );

  if ( url.path( QUrl::FullyEncoded ).isEmpty() )
  {
    url.setPath( fromEncodedComponent_helper( "/" ) );
  }
  return url;
}

void QgsNewArcGisRestConnectionDialog::accept()
{
  const QString newName = txtName->text();

  if ( !validate() )
    return;

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != newName )
  {
    QgsArcGisConnectionSettings::sTreeConnectionArcgis->deleteItem( mOriginalConnName );
  }

  const QUrl url( urlTrimmed() );
  QgsArcGisConnectionSettings::settingsUrl->setValue( url.toString(), newName );

  QgsArcGisConnectionSettings::settingsUsername->setValue( mAuthSettings->username(), newName );
  QgsArcGisConnectionSettings::settingsPassword->setValue( mAuthSettings->password(), newName );

  QgsArcGisConnectionSettings::settingsContentEndpoint->setValue( mContentEndPointLineEdit->text(), newName );
  QgsArcGisConnectionSettings::settingsCommunityEndpoint->setValue( mCommunityEndPointLineEdit->text(), newName );

  QgsArcGisConnectionSettings::settingsAuthcfg->setValue( mAuthSettings->configId(), newName );

  QgsArcGisConnectionSettings::settingsHeaders->setValue( mHttpHeaders->httpHeaders().headers(), newName );
  QgsArcGisConnectionSettings::settingsUrlPrefix->setValue( mUrlPrefix->text(), newName );

  QgsArcGisConnectionSettings::sTreeConnectionArcgis->setSelectedItem( newName );

  QDialog::accept();
}

void QgsNewArcGisRestConnectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/index.html" ) );
}
