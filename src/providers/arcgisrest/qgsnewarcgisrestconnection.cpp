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
#include "qgsauthsettingswidget.h"
#include "qgshttpheaderwidget.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "fromencodedcomponenthelper.h"

#include <QMessageBox>
#include <QUrl>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QUrlQuery>

QgsNewArcGisRestConnectionDialog::QgsNewArcGisRestConnectionDialog( QWidget *parent, const QString &baseKey, const QString &connectionName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mBaseKey( baseKey )
  , mOriginalConnName( connectionName )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewArcGisRestConnectionDialog::showHelp );

  const QRegularExpression rx( QStringLiteral( "/connections-([^/]+)/" ) );
  const QRegularExpressionMatch match = rx.match( baseKey );
  if ( match.hasMatch() )
  {
    const QString connectionType( match.captured( 1 ).toUpper() );
    setWindowTitle( tr( "Create a New %1 Connection" ).arg( connectionType ) );
  }

  mCredentialsBaseKey = mBaseKey.split( '-' ).last().toUpper();

  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( QStringLiteral( "[^\\/]+" ) ), txtName ) );

  if ( !connectionName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    const QgsSettings settings;

    const QString key = mBaseKey + connectionName;
    const QString credentialsKey = "qgis/" + mCredentialsBaseKey + '/' + connectionName;
    txtName->setText( connectionName );
    txtUrl->setText( settings.value( key + "/url" ).toString() );
    mHttpHeaders->setFromSettings( settings, key );

    // portal
    mContentEndPointLineEdit->setText( settings.value( key + "/content_endpoint" ).toString() );
    mCommunityEndPointLineEdit->setText( settings.value( key + "/community_endpoint" ).toString() );

    // Authentication
    mAuthSettings->setUsername( settings.value( credentialsKey + "/username" ).toString() );
    mAuthSettings->setPassword( settings.value( credentialsKey + "/password" ).toString() );
    mAuthSettings->setConfigId( settings.value( credentialsKey + "/authcfg" ).toString() );
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
  const QgsSettings settings;
  const QString key = mBaseKey + txtName->text();

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
       settings.contains( key + "/url" ) &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  if ( ! mAuthSettings->password().isEmpty() &&
       QMessageBox::question( this,
                              tr( "Saving Passwords" ),
                              tr( "WARNING: You have entered a password. It will be stored in unsecured plain text in your project files and your home directory (Unix-like OS) or user profile (Windows). If you want to avoid this, press Cancel and either:\n\na) Don't provide a password in the connection settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return false;
  }

  return true;
}

QUrl QgsNewArcGisRestConnectionDialog::urlTrimmed() const
{
  QUrl url( txtUrl->text().trimmed() );
  const QUrlQuery query( url );
  const QList<QPair<QString, QString> > items = query.queryItems( QUrl::FullyEncoded );
  QHash< QString, QPair<QString, QString> > params;
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
  QgsSettings settings;
  const QString key = mBaseKey + txtName->text();
  const QString credentialsKey = "qgis/" + mCredentialsBaseKey + '/' + txtName->text();

  if ( !validate() )
    return;

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != key )
  {
    settings.remove( mBaseKey + mOriginalConnName );
    settings.remove( "qgis/" + mCredentialsBaseKey + '/' + mOriginalConnName );
    settings.sync();
  }

  const QUrl url( urlTrimmed() );
  settings.setValue( key + "/url", url.toString() );

  settings.setValue( credentialsKey + "/username", mAuthSettings->username() );
  settings.setValue( credentialsKey + "/password", mAuthSettings->password() );

  settings.setValue( key + "/content_endpoint", mContentEndPointLineEdit->text() );
  settings.setValue( key + "/community_endpoint", mCommunityEndPointLineEdit->text() );

  settings.setValue( credentialsKey + "/authcfg", mAuthSettings->configId() );

  mHttpHeaders->updateSettings( settings, key );

  settings.setValue( mBaseKey + "/selected", txtName->text() );

  QDialog::accept();
}

void QgsNewArcGisRestConnectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/index.html" ) );
}
