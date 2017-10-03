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

#include <QDateTime>

QgsAuthSettingsWidget::QgsAuthSettingsWidget( QWidget *parent,
    const QString &configId,
    const QString &username,
    const QString &password,
    const QString &dataprovider )
  : QWidget( parent )
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
    tabAuth->setCurrentIndex( tabAuth->indexOf( tabConfigurations ) );
  }
  else if ( !( username.isEmpty() && password.isEmpty( ) ) )
  {
    tabAuth->setCurrentIndex( tabAuth->indexOf( tabBasic ) );
  }
  connect( btnConvertToEncrypted, &QPushButton::clicked, this, &QgsAuthSettingsWidget::convertToEncrypted );
  updateConvertBtnState();
}

void QgsAuthSettingsWidget::setWarningText( const QString &warningText )
{
  lblWarning->setText( warningText );
}

void QgsAuthSettingsWidget::setBasicText( const QString &basicText )
{
  lblBasic->setText( basicText );
}

const QString QgsAuthSettingsWidget::username() const
{
  return txtUserName->text();
}

const QString QgsAuthSettingsWidget::password() const
{
  return txtPassword->text();
}

const QString QgsAuthSettingsWidget::configId() const
{
  return mAuthConfigSelect->configId();
}

int QgsAuthSettingsWidget::currentTabIndex() const
{
  return tabAuth->currentIndex( );
}

bool QgsAuthSettingsWidget::btnConvertToEncryptedIsEnabled() const
{
  return btnConvertToEncrypted->isEnabled( );
}

bool QgsAuthSettingsWidget::convertToEncrypted( )
{
  tabAuth->setCurrentIndex( tabAuth->indexOf( tabConfigurations ) );
  QgsAuthMethodConfig config( QStringLiteral( "Basic" ) );
  config.setName( tr( "Converted config %1" ).arg( QDateTime::currentDateTime().toString( ) ) );
  config.setConfig( QStringLiteral( "username" ), txtUserName->text() );
  config.setConfig( QStringLiteral( "password" ), txtPassword->text() );
  if ( ! QgsAuthManager::instance()->storeAuthenticationConfig( config ) )
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

void QgsAuthSettingsWidget::on_txtUserName_textChanged( const QString &text )
{
  Q_UNUSED( text );
  updateConvertBtnState();
}

void QgsAuthSettingsWidget::on_txtPassword_textChanged( const QString &text )
{
  Q_UNUSED( text );
  updateConvertBtnState();
}

void QgsAuthSettingsWidget::updateConvertBtnState()
{
  btnConvertToEncrypted->setEnabled( ! txtUserName->text().isEmpty() || ! txtPassword->text().isEmpty() );
}
