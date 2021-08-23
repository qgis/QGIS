/***************************************************************************
     testqgsauthoauth2method.cpp
     ----------------------
    Date                 : August 2016
    Copyright            : (C) 2016 by Monsanto Company, USA
    Author               : Larry Shaffer, Boundless Spatial
    Email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <QtTest/QTest>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QSignalSpy>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthoauth2config.h"
#ifdef WITH_GUI
#include "qgsauthoauth2edit.h"
#endif

/**
 * \ingroup UnitTests
 * Unit tests for QgsAuthOAuth2Config
 */
class TestQgsAuthOAuth2Method: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testOAuth2Config();
    void testOAuth2ConfigIO();
    void testOAuth2ConfigUtils();
    void testDynamicRegistration();
    void testDynamicRegistrationJwt();
    void testDynamicRegistrationNoEndpoint();

  private:
    QgsAuthOAuth2Config *baseConfig( bool loaded = false );
    QByteArray baseConfigTxt( bool pretty = false );

    QVariantMap baseVariantMap();
    QByteArray baseVariantTxt();

    static QString smHashes;
    static QString sTestDataDir;
};


QString TestQgsAuthOAuth2Method::sTestDataDir = QStringLiteral( TEST_DATA_DIR ) + "/auth_system/oauth2";


QString TestQgsAuthOAuth2Method::smHashes = "#####################";
//QObject *TestQgsAuthOAuth2Method::smParentObj = new QObject();

void TestQgsAuthOAuth2Method::initTestCase()
{
  //setPrefixEnviron();
  QgsApplication::init();
  QgsApplication::initQgis();
  if ( QgsApplication::authManager()->isDisabled() )
    QSKIP( "Auth system is disabled, skipping test case", SkipAll );

  //qDebug() << QgsApplication::showSettings().toUtf8().constData();
}

void TestQgsAuthOAuth2Method::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAuthOAuth2Method::init()
{
  qDebug() << "\n************ Start " << QTest::currentTestFunction() << " ************";
}

void TestQgsAuthOAuth2Method::cleanup()
{
  qDebug() << "\n************ End " << QTest::currentTestFunction() << " ************";
}

QgsAuthOAuth2Config *TestQgsAuthOAuth2Method::baseConfig( bool loaded )
{
  QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( qApp );
  if ( loaded )
  {
    config->setId( "abc1234" );
    config->setVersion( 1 );
    config->setConfigType( QgsAuthOAuth2Config::Custom );
    config->setGrantFlow( QgsAuthOAuth2Config::AuthCode );
    config->setName( "MyConfig" );
    config->setDescription( "A test config" );
    config->setRequestUrl( "https://request.oauth2.test" );
    config->setTokenUrl( "https://token.oauth2.test" );
    config->setRefreshTokenUrl( "https://refreshtoken.oauth2.test" );
    config->setRedirectUrl( "subdir" );
    config->setRedirectPort( 7777 );
    config->setClientId( "myclientid" );
    config->setClientSecret( "myclientsecret" );
    config->setUsername( "myusername" );
    config->setPassword( "mypassword" );
    config->setScope( "scope_1 scope_2 scope_3" );
    config->setApiKey( "someapikey" );
    config->setPersistToken( false );
    config->setAccessMethod( QgsAuthOAuth2Config::Header );
    config->setCustomHeader( QStringLiteral( "x-auth" ) );
    config->setRequestTimeout( 30 ); // in seconds
    QVariantMap queryPairs;
    queryPairs.insert( "pf.username", "myusername" );
    queryPairs.insert( "pf.password", "mypassword" );
    config->setQueryPairs( queryPairs );
  }

  return config;
}

QByteArray TestQgsAuthOAuth2Method::baseConfigTxt( bool pretty )
{
  QByteArray out;
  if ( pretty )
  {
    out += "{\n"
           "    \"accessMethod\": 0,\n"
           "    \"apiKey\": \"someapikey\",\n"
           "    \"clientId\": \"myclientid\",\n"
           "    \"clientSecret\": \"myclientsecret\",\n"
           "    \"configType\": 1,\n"
           "    \"customHeader\": \"x-auth\",\n"
           "    \"description\": \"A test config\",\n"
           "    \"grantFlow\": 0,\n"
           "    \"id\": \"abc1234\",\n"
           "    \"name\": \"MyConfig\",\n"
           "    \"objectName\": \"\",\n"
           "    \"password\": \"mypassword\",\n"
           "    \"persistToken\": false,\n"
           "    \"queryPairs\": {\n"
           "        \"pf.password\": \"mypassword\",\n"
           "        \"pf.username\": \"myusername\"\n"
           "    },\n"
           "    \"redirectPort\": 7777,\n"
           "    \"redirectUrl\": \"subdir\",\n"
           "    \"refreshTokenUrl\": \"https://refreshtoken.oauth2.test\",\n"
           "    \"requestTimeout\": 30,\n"
           "    \"requestUrl\": \"https://request.oauth2.test\",\n"
           "    \"scope\": \"scope_1 scope_2 scope_3\",\n"
           "    \"tokenUrl\": \"https://token.oauth2.test\",\n"
           "    \"username\": \"myusername\",\n"
           "    \"version\": 1\n"
           "}\n";
  }
  else
  {
    out += "{\"accessMethod\":0,"
           "\"apiKey\":\"someapikey\","
           "\"clientId\":\"myclientid\","
           "\"clientSecret\":\"myclientsecret\","
           "\"configType\":1,"
           "\"customHeader\":\"x-auth\","
           "\"description\":\"A test config\","
           "\"grantFlow\":0,"
           "\"id\":\"abc1234\","
           "\"name\":\"MyConfig\","
           "\"objectName\":\"\","
           "\"password\":\"mypassword\","
           "\"persistToken\":false,"
           "\"queryPairs\":{\"pf.password\":\"mypassword\",\"pf.username\":\"myusername\"},"
           "\"redirectPort\":7777,"
           "\"redirectUrl\":\"subdir\","
           "\"refreshTokenUrl\":\"https://refreshtoken.oauth2.test\","
           "\"requestTimeout\":30,"
           "\"requestUrl\":\"https://request.oauth2.test\","
           "\"scope\":\"scope_1 scope_2 scope_3\","
           "\"tokenUrl\":\"https://token.oauth2.test\","
           "\"username\":\"myusername\","
           "\"version\":1}";
  }
  return out;
}

QVariantMap TestQgsAuthOAuth2Method::baseVariantMap()
{
  QVariantMap vmap;
  vmap.insert( "apiKey", "someapikey" );
  vmap.insert( "accessMethod", 0 );
  vmap.insert( "clientId", "myclientid" );
  vmap.insert( "clientSecret", "myclientsecret" );
  vmap.insert( "configType", 1 );
  vmap.insert( "description", "A test config" );
  vmap.insert( "grantFlow", 0 );
  vmap.insert( "id", "abc1234" );
  vmap.insert( "name", "MyConfig" );
  vmap.insert( "objectName", "" );
  vmap.insert( "password", "mypassword" );
  vmap.insert( "persistToken", false );
  vmap.insert( "customHeader", "x-auth" );
  QVariantMap qpairs;
  qpairs.insert( "pf.password", "mypassword" );
  qpairs.insert( "pf.username", "myusername" );
  vmap.insert( "queryPairs", qpairs );
  vmap.insert( "redirectPort", 7777 );
  vmap.insert( "redirectUrl", "subdir" );
  vmap.insert( "refreshTokenUrl", "https://refreshtoken.oauth2.test" );
  vmap.insert( "requestTimeout", 30 );
  vmap.insert( "requestUrl", "https://request.oauth2.test" );
  vmap.insert( "scope", "scope_1 scope_2 scope_3" );
  vmap.insert( "tokenUrl", "https://token.oauth2.test" );
  vmap.insert( "username", "myusername" );
  vmap.insert( "version", 1 );

  return vmap;
}

QByteArray TestQgsAuthOAuth2Method::baseVariantTxt()
{
  QByteArray out;
  return out;
}


void TestQgsAuthOAuth2Method::testOAuth2Config()
{
  qDebug() << "Verify base object config";
  QgsAuthOAuth2Config *config1 = new QgsAuthOAuth2Config( qApp );
  QVERIFY( !config1->isValid() );

  qDebug() << "Verify property interface";
  QObject *configo = config1;
  QCOMPARE( configo->property( "configType" ).toString(), QString( "1" ) ); // Custom
  config1->setConfigType( QgsAuthOAuth2Config::Predefined );
  QCOMPARE( configo->property( "configType" ).toString(), QString( "0" ) );
  QCOMPARE( QString( "%1" ).arg( config1->configType() ), QString( "0" ) );
  configo->setProperty( "configType", "Custom" );
  QCOMPARE( configo->property( "configType" ).toString(), QString( "1" ) );

  config1->deleteLater();

  qDebug() << "Verify base object validity";
  QgsAuthOAuth2Config *config2 = baseConfig();
  QVERIFY( !config2->isValid() );
  config2->deleteLater();

  QgsAuthOAuth2Config *config3 = baseConfig( true );
  QVERIFY( config3->isValid() );

  qDebug() << "Verify base object internal signals";
  const QSignalSpy spy_config( config3, SIGNAL( configChanged() ) );
  QSignalSpy spy_valid( config3, SIGNAL( validityChanged( bool ) ) );

  config3->setRedirectPort( 0 );
  // validity should now be false

  QCOMPARE( spy_config.count(), 1 );
  QCOMPARE( spy_valid.count(), 1 );
  const QList<QVariant> valid_args = spy_valid.takeAt( 0 );
  QVERIFY( valid_args.at( 0 ).toBool() == false );
  QVERIFY( !config3->isValid() );

  config3->setRedirectPort( 7777 );
  // validity should now be true

  QCOMPARE( spy_config.count(), 2 );
  QCOMPARE( spy_valid.count(), 1 );
  const QList<QVariant> valid_args2 = spy_valid.takeAt( 0 );
  QVERIFY( valid_args2.at( 0 ).toBool() == true );
  QVERIFY( config3->isValid() );

  QCOMPARE( spy_valid.count(), 0 );
  config3->setRedirectPort( 4444 );
  // validity should still be true, so no validityChanged fired

  QCOMPARE( spy_config.count(), 3 );
  QCOMPARE( spy_valid.count(), 0 );
  QVERIFY( config3->isValid() );

  config3->deleteLater();

  qDebug() << "Validate equality";
  QgsAuthOAuth2Config *config4 = baseConfig( true );
  QgsAuthOAuth2Config *config5 = baseConfig( true );
  QgsAuthOAuth2Config *config6 = baseConfig();
  QVERIFY( *config4 == *config5 );
  QVERIFY( *config4 != *config6 );

  //config5->setName( "Blah blah" );
  config5->setRedirectPort( 2222 );
  QVERIFY( *config4 != *config5 );
  config4->deleteLater();
  config5->deleteLater();
  config6->deleteLater();
}

void TestQgsAuthOAuth2Method::testOAuth2ConfigIO()
{
  qDebug() << "Verify saving config to text";
  QgsAuthOAuth2Config *config1 = baseConfig( true );
  bool ok = false;
  QByteArray cfgtxt = config1->saveConfigTxt( QgsAuthOAuth2Config::JSON, true, &ok );
  QVERIFY( ok );
  //qDebug() << "cfgtxt: \n" << cfgtxt;
  //qDebug() << "baseConfigTxt: \n" << baseConfigTxt( true );
  QCOMPARE( baseConfigTxt( true ), cfgtxt );
  cfgtxt.clear();

  ok = false;
  cfgtxt = config1->saveConfigTxt( QgsAuthOAuth2Config::JSON, false, &ok );
  QVERIFY( ok );
  //qDebug() << "cfgtxt: \n" << cfgtxt;
  //qDebug() << "baseConfigTxt: \n" << baseConfigTxt( false );
  QCOMPARE( baseConfigTxt( false ), cfgtxt );

  qDebug() << "Verify loading config from text";
  // from base
  QgsAuthOAuth2Config *config2 = new QgsAuthOAuth2Config( qApp );
  QVERIFY( config2->loadConfigTxt( baseConfigTxt( true ), QgsAuthOAuth2Config::JSON ) );
  QVERIFY( *config1 == *config2 );

  // roundtrip already saved text
  QgsAuthOAuth2Config *config3 = new QgsAuthOAuth2Config( qApp );
  QVERIFY( config3->loadConfigTxt( cfgtxt, QgsAuthOAuth2Config::JSON ) );
  QVERIFY( *config1 == *config3 );

  // roundtrip already loaded obj
  ok = false;
  cfgtxt.clear();
  cfgtxt = config2->saveConfigTxt( QgsAuthOAuth2Config::JSON, true, &ok );
  QVERIFY( ok );
  //qDebug() << "cfgtxt: \n" << cfgtxt;
  //qDebug() << "baseConfigTxt: \n" << baseConfigTxt( true );
  QCOMPARE( baseConfigTxt( true ), cfgtxt );

  qDebug() << "Verify writing config to file";
  const QString rndsuffix = QgsApplication::authManager()->uniqueConfigId();
  const QString dirname = QString( "oauth2_configs_%1" ).arg( rndsuffix );
  const QDir tmpdir = QDir::temp();
  tmpdir.mkdir( dirname );

  QgsAuthOAuth2Config *config4 = baseConfig( true );
  QgsAuthOAuth2Config *config5 = baseConfig( true );
  config5->setName( "Blah blah" );
  config5->setRedirectPort( 2222 );

  qDebug() << QDir::tempPath() + "/" + dirname;

  const QString config4path( QDir::tempPath() + "/" + dirname + "/config4.json" );
  const QString config5path( QDir::tempPath() + "/" + dirname + "/config5.json" );

  QVERIFY( QgsAuthOAuth2Config::writeOAuth2Config( config4path, config4,
           QgsAuthOAuth2Config::JSON, true ) );
  QVERIFY( QgsAuthOAuth2Config::writeOAuth2Config( config5path, config5,
           QgsAuthOAuth2Config::JSON, true ) );

  qDebug() << "Verify reading config files from directory";
  ok = false;
  QList<QgsAuthOAuth2Config *> configs =
    QgsAuthOAuth2Config::loadOAuth2Configs( QDir::tempPath() + "/" + dirname,
        qApp, QgsAuthOAuth2Config::JSON, &ok );
  QVERIFY( ok );
  QCOMPARE( configs.size(), 2 );
  QgsAuthOAuth2Config *config6 = configs.takeFirst();
  QgsAuthOAuth2Config *config7 = configs.takeFirst();
  QVERIFY( config6->isValid() );
  QVERIFY( config7->isValid() );
  if ( config6->redirectPort() == 2222 )
  {
    QVERIFY( *config6 == *config5 );
    QVERIFY( *config7 == *config4 );
  }
  else
  {
    QVERIFY( *config7 == *config5 );
    QVERIFY( *config6 == *config4 );
  }

  // TODO: add tests for mapOAuth2Configs and mappedOAuth2ConfigsCache
}

void TestQgsAuthOAuth2Method::testOAuth2ConfigUtils()
{
  const QVariantMap basevmap = baseVariantMap();
  bool ok = false;

  qDebug() << "Verify serializeFromVariant";
  const QByteArray vtxt = QgsAuthOAuth2Config::serializeFromVariant(
                            basevmap, QgsAuthOAuth2Config::JSON, true, &ok );
  QVERIFY( ok );
  //qDebug() << vtxt;
  //qDebug() << baseConfigTxt( true );
  QCOMPARE( vtxt, baseConfigTxt( true ) );

  qDebug() << "Verify variantFromSerialized";
  const QVariantMap vmap = QgsAuthOAuth2Config::variantFromSerialized(
                             baseConfigTxt( true ), QgsAuthOAuth2Config::JSON, &ok );
  QVERIFY( ok );
  QCOMPARE( vmap.value( "name" ).toString(), QString( "MyConfig" ) );
  QCOMPARE( vmap, basevmap );

}

void TestQgsAuthOAuth2Method::testDynamicRegistrationNoEndpoint()
{
#ifdef WITH_GUI
  QgsAuthOAuth2Config *config = baseConfig();
  config->setClientId( QString( ) );
  config->setClientSecret( QString( ) );
  QVariantMap configMap( config->mappedProperties() );
  QCOMPARE( configMap["clientId"].toString(), QString() );
  QCOMPARE( configMap["clientSecret"].toString(), QString() );
  QgsAuthOAuth2Edit dlg;
  QgsStringMap stringMap;
  for ( const auto &k : configMap.keys( ) )
  {
    stringMap[k] = configMap.value( k ).toString();
  }
  dlg.loadConfig( stringMap );
  QCOMPARE( dlg.leClientId->text(), QString() );
  QCOMPARE( dlg.leClientSecret->text(), QString() );

  // This JWT does not contain a registration_endpoint
  dlg.leSoftwareStatementJwtPath->setText( QStringLiteral( "%1/auth_code_grant_display_code.jwt" ).arg( sTestDataDir ) );
  QVERIFY( ! dlg.btnRegister->isEnabled() );
  QCOMPARE( dlg.leSoftwareStatementConfigUrl->text(), QString() );
#endif
}

void TestQgsAuthOAuth2Method::testDynamicRegistration()
{
#ifdef WITH_GUI
  QgsAuthOAuth2Config *config = baseConfig();
  config->setClientId( QString( ) );
  config->setClientSecret( QString( ) );
  QVariantMap configMap( config->mappedProperties() );
  QCOMPARE( configMap["clientId"].toString(), QString() );
  QCOMPARE( configMap["clientSecret"].toString(), QString() );
  QgsAuthOAuth2Edit dlg;
  QgsStringMap stringMap;
  for ( const auto &k : configMap.keys( ) )
  {
    stringMap[k] = configMap.value( k ).toString();
  }
  dlg.loadConfig( stringMap );
  QCOMPARE( dlg.leClientId->text(), QString() );
  QCOMPARE( dlg.leClientSecret->text(), QString() );

  // This JWT does not contain a registration_endpoint
  dlg.leSoftwareStatementJwtPath->setText( QStringLiteral( "%1/auth_code_grant_display_code.jwt" ).arg( sTestDataDir ) );
  QVERIFY( ! dlg.btnRegister->isEnabled() );
  QCOMPARE( dlg.leSoftwareStatementConfigUrl->text(), QString() );
  // Set the config url to something local
  dlg.leSoftwareStatementConfigUrl->setText( QUrl::fromLocalFile( QStringLiteral( "%1/auth_code_grant_display_code_get_config.json" ).arg( sTestDataDir ) ).toString( ) );
  QVERIFY( dlg.btnRegister->isEnabled() );
  // Change it to something local
  dlg.mRegistrationEndpoint = QUrl::fromLocalFile( QStringLiteral( "%1/client_information_registration_response.json" ).arg( sTestDataDir ) ).toString();
  QTest::mouseClick( dlg.btnRegister, Qt::MouseButton::LeftButton );
  while ( dlg.mDownloading )
  {
    qApp->processEvents();
  }
  QCOMPARE( dlg.leClientId->text(), QLatin1String( "___QGIS_ROCKS___@www.qgis.org" ) );
  QCOMPARE( dlg.leClientSecret->text(), QLatin1String( "___QGIS_ROCKS______QGIS_ROCKS______QGIS_ROCKS___" ) );
#endif
}


void TestQgsAuthOAuth2Method::testDynamicRegistrationJwt()
{
#ifdef WITH_GUI
  QgsAuthOAuth2Config *config = baseConfig();
  config->setClientId( QString( ) );
  config->setClientSecret( QString( ) );
  QVariantMap configMap( config->mappedProperties() );
  QCOMPARE( configMap["clientId"].toString(), QString() );
  QCOMPARE( configMap["clientSecret"].toString(), QString() );
  QgsAuthOAuth2Edit dlg;
  QgsStringMap stringMap;
  for ( const auto &k : configMap.keys( ) )
  {
    stringMap[k] = configMap.value( k ).toString();
  }
  dlg.loadConfig( stringMap );
  QCOMPARE( dlg.leClientId->text(), QString() );
  QCOMPARE( dlg.leClientSecret->text(), QString() );

  // Now set the config URL to the JWT that does contain a registration_endpoint
  dlg.leSoftwareStatementJwtPath->setText( QStringLiteral( "%1/auth_code_grant_display_code_registration_endpoint.jwt" ).arg( sTestDataDir ) );
  QCOMPARE( dlg.leSoftwareStatementConfigUrl->text(), QStringLiteral( "http://www.qgis.org/oauth2/registration" ) );
  QVERIFY( dlg.btnRegister->isEnabled() );
  // Change it to something local
  dlg.mRegistrationEndpoint = QUrl::fromLocalFile( QStringLiteral( "%1/client_information_registration_response.json" ).arg( sTestDataDir ) ).toString();
  QTest::mouseClick( dlg.btnRegister, Qt::MouseButton::LeftButton );
  while ( dlg.mDownloading )
  {
    qApp->processEvents();
  }
  QCOMPARE( dlg.leClientId->text(), QLatin1String( "___QGIS_ROCKS___@www.qgis.org" ) );
  QCOMPARE( dlg.leClientSecret->text(), QLatin1String( "___QGIS_ROCKS______QGIS_ROCKS______QGIS_ROCKS___" ) );
#endif
}


QGSTEST_MAIN( TestQgsAuthOAuth2Method )
#include "testqgsauthoauth2method.moc"
