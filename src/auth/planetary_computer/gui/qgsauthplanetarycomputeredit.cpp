/***************************************************************************
    qgsauthplanetarycomputeredit.cpp
    ------------------------
    begin                : August 2025
    copyright            : (C) 2025 by Stefanos Natsis
    author               : Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ui_qgsauthplanetarycomputeredit.h"
#include "qgsauthplanetarycomputeredit.h"

#include "moc_qgsauthplanetarycomputeredit.cpp"

const QString QgsAuthPlanetaryComputerEdit::REQUEST_URL_TEMPLATE = QStringLiteral( "https://login.microsoftonline.com/%1/oauth2/v2.0/authorize" );
const QString QgsAuthPlanetaryComputerEdit::TOKEN_URL_TEMPLATE = QStringLiteral( "https://login.microsoftonline.com/%1/oauth2/v2.0/token" );
const QString QgsAuthPlanetaryComputerEdit::SCOPE = QStringLiteral( "https://geocatalog.spatio.azure.com/.default" );

QgsAuthPlanetaryComputerEdit::QgsAuthPlanetaryComputerEdit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );

  connect( cbType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsAuthPlanetaryComputerEdit::updateServerType ); // also updates GUI
  connect( leRootUrl, &QLineEdit::textChanged, this, [this] { validateConfig(); } );
  connect( leClientId, &QLineEdit::textChanged, this, [this] { validateConfig(); } );
  updateServerType( 0 );
}


bool QgsAuthPlanetaryComputerEdit::validateConfig()
{
  bool currentValid = false;
  if ( cbType->currentIndex() == 0 )
  {
    currentValid = true;
  }
  else if ( cbType->currentIndex() == 1 )
  {
    currentValid = !leClientId->text().isEmpty() && !leRootUrl->text().isEmpty();
  }
  if ( mValid != currentValid )
  {
    mValid = currentValid;
    emit validityChanged( currentValid );
  }
  return currentValid;
}


QgsStringMap QgsAuthPlanetaryComputerEdit::configMap() const
{
  const bool isPro = cbType->currentIndex() == 1;
  const QString clientId = leClientId->text();
  const QString rootUrl = leRootUrl->text();
  const QString tenantId = leTenantId->text();

  QgsStringMap config;

  if ( isPro )
  {
    const QString requestUrl = REQUEST_URL_TEMPLATE.arg( tenantId.isEmpty() ? QStringLiteral( "organizations" ) : tenantId );
    const QString tokenUrl = TOKEN_URL_TEMPLATE.arg( tenantId.isEmpty() ? QStringLiteral( "organizations" ) : tenantId );
    const QString json = QStringLiteral(
                           "{"
                           "\"accessMethod\": 0,"
                           "\"apiKey\": null,"
                           "\"clientId\": \"%1\","
                           "\"clientSecret\": null,"
                           "\"configType\": 1,"
                           "\"customHeader\": null,"
                           "\"description\": \"\","
                           "\"extraTokens\": {},"
                           "\"grantFlow\": 3,"
                           "\"id\": null,"
                           "\"name\": null,"
                           "\"objectName\": \"\","
                           "\"password\": null,"
                           "\"persistToken\": false,"
                           "\"queryPairs\": {},"
                           "\"redirectHost\": \"localhost\","
                           "\"redirectPort\": 7070,"
                           "\"redirectUrl\": null,"
                           "\"refreshTokenUrl\": null,"
                           "\"requestTimeout\": 30,"
                           "\"requestUrl\": \"%2\","
                           "\"scope\": \"%4\","
                           "\"tokenUrl\": \"%3\","
                           "\"username\": null,"
                           "\"version\": 1"
                           "}"
    )
                           .arg( clientId, requestUrl, tokenUrl, SCOPE );


    config.insert( QStringLiteral( "oauth2config" ), json );
    config.insert( QStringLiteral( "serverType" ), QStringLiteral( "pro" ) );
    config.insert( QStringLiteral( "clientId" ), clientId );
    config.insert( QStringLiteral( "rootUrl" ), rootUrl );
    config.insert( QStringLiteral( "tenantId" ), tenantId );
  }
  else
  {
    config.insert( QStringLiteral( "serverType" ), QStringLiteral( "open" ) );
  }
  return config;
}


void QgsAuthPlanetaryComputerEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  whileBlocking( leClientId )->setText( configmap.value( QStringLiteral( "clientId" ) ) );
  whileBlocking( leRootUrl )->setText( configmap.value( QStringLiteral( "rootUrl" ) ) );
  whileBlocking( leTenantId )->setText( configmap.value( QStringLiteral( "tenantId" ) ) );
  updateServerType( configmap.value( QStringLiteral( "serverType" ) ) == QLatin1String( "pro" ) ? 1 : 0 );
}


void QgsAuthPlanetaryComputerEdit::resetConfig()
{
  loadConfig( mConfigMap );
}


void QgsAuthPlanetaryComputerEdit::clearConfig()
{
  whileBlocking( leClientId )->clear();
  whileBlocking( leRootUrl )->clear();
  whileBlocking( leTenantId )->clear();
  updateServerType( 0 );
}


void QgsAuthPlanetaryComputerEdit::updateServerType( int indx )
{
  if ( cbType->currentIndex() != indx )
  {
    whileBlocking( cbType )->setCurrentIndex( indx );
  }

  const bool isPro = indx == 1;

  lblRootUrl->setVisible( isPro );
  leRootUrl->setVisible( isPro );
  lblClientId->setVisible( isPro );
  leClientId->setVisible( isPro );
  lblTenantId->setVisible( isPro );
  leTenantId->setVisible( isPro );

  const QString openHelp = tr( "Use this server type for %1 - the data are publicly accessible and do not require an account." ).arg( QLatin1String( "<a href=\"https://planetarycomputer.microsoft.com/\">https://planetarycomputer.microsoft.com/</a>" ) );
  const QString proHelp = QStringLiteral( "%1<br/>%2: <a href=\"%3\">%3</a>" ).arg( tr( "Contact your Microsoft Entra admin for App Registration details." ), tr( "Setup guide" ), QStringLiteral( "http://aka.ms/qgis" ) );

  lblHelp->setText( QStringLiteral( "<html><head/><body><p><span style=\" font-style:italic;\">%1</span></p></body></html>" ).arg( isPro ? proHelp : openHelp ) );

  validateConfig(); // NOLINT
}
