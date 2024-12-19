/***************************************************************************
  qgsauthsettingswidget.cpp - QgsAuthSettingsWidget

 ---------------------
 begin                : 28.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsauthsettingswidget.h"
#include "qgsauthmanager.h"
#include "qgsauthconfig.h"
#include "qgsapplication.h"

#include <QDateTime>

QgsAuthSettingsWidget::QgsAuthSettingsWidget( QWidget *parent,
    const QString &configId,
    const QString &username,
    const QString &password,
    const QString &dataprovider )
  : QWidget( parent )
  , mDataprovider( dataprovider )
{
  setupUi( this );
  txtPassword->setText( password );
  txtUserName->setText( username );
  if ( ! dataprovider.isEmpty( ) )
  {
    mAuthConfigSelect->setDataProviderKey( dataprovider );
  }
  if ( ! configId.isEmpty( ) )
  {
    mAuthConfigSelect->setConfigId( configId );
  }
  setBasicText( "" );
  // default to warning about basic settings stored in project file
  setWarningText( formattedWarning( ProjectFile ) );
  connect( btnConvertToEncrypted, &QPushButton::clicked, this, &QgsAuthSettingsWidget::convertToEncrypted );
  connect( txtUserName, &QLineEdit::textChanged, this, &QgsAuthSettingsWidget::userNameTextChanged );
  connect( txtPassword, &QLineEdit::textChanged, this, &QgsAuthSettingsWidget::passwordTextChanged );
  connect( mAuthConfigSelect, &QgsAuthConfigSelect::selectedConfigIdChanged, this, &QgsAuthSettingsWidget::configIdChanged );

  // Hide store password and username by default
  showStoreCheckboxes( false );
  updateSelectedTab();
  updateConvertBtnState();
}

void QgsAuthSettingsWidget::setWarningText( const QString &warningText )
{
  lblWarning->setText( warningText );
}

void QgsAuthSettingsWidget::setBasicText( const QString &basicText )
{
  lblBasic->setText( basicText );
  // hide unused widget so its word wrapping does not add to parent widget's height
  lblBasic->setVisible( ! basicText.isEmpty() );
}

QString QgsAuthSettingsWidget::username() const
{
  return txtUserName->text();
}

void QgsAuthSettingsWidget::setUsername( const QString &username )
{
  txtUserName->setText( username );
  updateSelectedTab();
}

QString QgsAuthSettingsWidget::password() const
{
  return txtPassword->text();
}

void QgsAuthSettingsWidget::setPassword( const QString &password )
{
  txtPassword->setText( password );
  updateSelectedTab();
}

void QgsAuthSettingsWidget::setConfigId( const QString &configId )
{
  mAuthConfigSelect->setConfigId( configId );
  updateSelectedTab();
}

void QgsAuthSettingsWidget::setDataprovider( const QString &dataprovider )
{
  mDataprovider = dataprovider;
  mAuthConfigSelect->setDataProviderKey( dataprovider );
}

QString QgsAuthSettingsWidget::dataprovider() const
{
  return mDataprovider;
}

QString QgsAuthSettingsWidget::formattedWarning( WarningType warning )
{
  const QString out = tr( "Warning: credentials stored as plain text in %1." );
  switch ( warning )
  {
    case ProjectFile:
      return out.arg( tr( "project file" ) );
    case UserSettings:
      return out.arg( tr( "user settings" ) );
  }
  return QString(); // no build warnings
}

QString QgsAuthSettingsWidget::configId() const
{
  return mAuthConfigSelect->configId();
}

bool QgsAuthSettingsWidget::btnConvertToEncryptedIsEnabled() const
{
  return btnConvertToEncrypted->isEnabled( );
}

void QgsAuthSettingsWidget::showStoreCheckboxes( bool enabled )
{
  if ( enabled )
  {
    cbStorePassword->show();
    cbStoreUsername->show();
  }
  else
  {
    cbStorePassword->hide();
    cbStoreUsername->hide();
  }
}

void QgsAuthSettingsWidget::setStoreUsernameChecked( bool checked )
{
  cbStoreUsername->setChecked( checked );
}

void QgsAuthSettingsWidget::setStorePasswordChecked( bool checked )
{
  cbStorePassword->setChecked( checked );
}

bool QgsAuthSettingsWidget::storePasswordIsChecked() const
{
  return cbStorePassword->isChecked( );
}

bool QgsAuthSettingsWidget::storeUsernameIsChecked() const
{
  return cbStoreUsername->isChecked( );
}

bool QgsAuthSettingsWidget::configurationTabIsSelected()
{
  return tabAuth->currentIndex( ) == tabAuth->indexOf( tabConfigurations );
}

bool QgsAuthSettingsWidget::convertToEncrypted( )
{
  tabAuth->setCurrentIndex( tabAuth->indexOf( tabConfigurations ) );
  QgsAuthMethodConfig config( QStringLiteral( "Basic" ) );
  config.setName( tr( "Converted config %1" ).arg( QDateTime::currentDateTime().toString( ) ) );
  config.setConfig( QStringLiteral( "username" ), txtUserName->text() );
  config.setConfig( QStringLiteral( "password" ), txtPassword->text() );
  if ( ! QgsApplication::authManager()->storeAuthenticationConfig( config ) )
  {
    mAuthConfigSelect->showMessage( tr( "Couldn't create a Basic authentication configuration!" ) );
    return false;
  }
  else
  {
    txtUserName->setText( QString( ) );
    txtPassword->setText( QString( ) );
    mAuthConfigSelect->setConfigId( config.id( ) );
    return true;
  }
}

void QgsAuthSettingsWidget::userNameTextChanged( const QString &text )
{
  Q_UNUSED( text )
  updateConvertBtnState();
  emit usernameChanged();
}

void QgsAuthSettingsWidget::passwordTextChanged( const QString &text )
{
  Q_UNUSED( text )
  updateConvertBtnState();
  emit passwordChanged();
}

void QgsAuthSettingsWidget::updateConvertBtnState()
{
  btnConvertToEncrypted->setEnabled( ! txtUserName->text().isEmpty() || ! txtPassword->text().isEmpty() );
}

void QgsAuthSettingsWidget::updateSelectedTab()
{
  if ( ! mAuthConfigSelect->configId().isEmpty( ) )
  {
    tabAuth->setCurrentIndex( tabAuth->indexOf( tabConfigurations ) );
  }
  else if ( !( txtUserName->text( ).isEmpty() && txtPassword->text( ).isEmpty( ) ) )
  {
    tabAuth->setCurrentIndex( tabAuth->indexOf( tabBasic ) );
  }
}
