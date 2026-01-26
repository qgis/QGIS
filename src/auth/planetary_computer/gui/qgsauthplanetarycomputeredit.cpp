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

const QString QgsAuthPlanetaryComputerEdit::REQUEST_URL_TEMPLATE = u"https://login.microsoftonline.com/%1/oauth2/v2.0/authorize"_s;
const QString QgsAuthPlanetaryComputerEdit::TOKEN_URL_TEMPLATE = u"https://login.microsoftonline.com/%1/oauth2/v2.0/token"_s;
const QString QgsAuthPlanetaryComputerEdit::SCOPE = u"https://geocatalog.spatio.azure.com/.default"_s;

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
    const QString requestUrl = REQUEST_URL_TEMPLATE.arg( tenantId.isEmpty() ? u"organizations"_s : tenantId );
    const QString tokenUrl = TOKEN_URL_TEMPLATE.arg( tenantId.isEmpty() ? u"organizations"_s : tenantId );
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


    config.insert( u"oauth2config"_s, json );
    config.insert( u"serverType"_s, u"pro"_s );
    config.insert( u"clientId"_s, clientId );
    config.insert( u"rootUrl"_s, rootUrl );
    config.insert( u"tenantId"_s, tenantId );
  }
  else
  {
    config.insert( u"serverType"_s, u"open"_s );
  }
  return config;
}


void QgsAuthPlanetaryComputerEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  whileBlocking( leClientId )->setText( configmap.value( u"clientId"_s ) );
  whileBlocking( leRootUrl )->setText( configmap.value( u"rootUrl"_s ) );
  whileBlocking( leTenantId )->setText( configmap.value( u"tenantId"_s ) );
  updateServerType( configmap.value( u"serverType"_s ) == "pro"_L1 ? 1 : 0 );
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

  const QString openHelp = tr( "Use this server type for %1 - the data are publicly accessible and do not require an account." ).arg( "<a href=\"https://planetarycomputer.microsoft.com/\">https://planetarycomputer.microsoft.com/</a>"_L1 );
  const QString proHelp = u"%1<br/>%2: <a href=\"%3\">%3</a>"_s.arg( tr( "Contact your Microsoft Entra admin for App Registration details." ), tr( "Setup guide" ), u"http://aka.ms/qgis"_s );

  lblHelp->setText( u"<html><head/><body><p><span style=\" font-style:italic;\">%1</span></p></body></html>"_s.arg( isPro ? proHelp : openHelp ) );

  validateConfig(); // NOLINT
}
