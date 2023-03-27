/***************************************************************************
     testqgshttpheaders.cpp
     --------------------------------------
    Date                 : Tue 14 Sep 2012
    Copyright            : (C) 2012 by Benoit De Mezzo
    Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qgsapplication.h>
//header for class being tested
#include <qgshttpheaders.h>
#include <qgspoint.h>
#include "qgslogger.h"
#include "qgsowsconnection.h"
#include <QNetworkRequest>
#include <QUrlQuery>

class TestQgsHttpheaders: public QObject
{

    Q_OBJECT
  private:
    void setFromSettings( const QString &base );

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void sanitize();

    void setFromSettingsGoodKey();
    void setFromSettingsBadKey();
    void updateSettings();

    void createQgsOwsConnection();

    void updateNetworkRequest();
    void updateSetUrlQuery();
    void updateSetMap();
    void updateSetDomElement();
};

void TestQgsHttpheaders::initTestCase()
{

}

void TestQgsHttpheaders::sanitize()
{
  QgsHttpHeaders h;

  QCOMPARE( h.sanitizeKey( "qgis//mytest1/" ), "qgis/mytest1" ) ;
  QCOMPARE( h.sanitizeKey( "qgis/mytest1/" ), "qgis/mytest1" ) ;
}

void TestQgsHttpheaders::setFromSettingsGoodKey()
{
  setFromSettings( "qgis/mytest1/" );
}

void TestQgsHttpheaders::setFromSettingsBadKey()
{
  setFromSettings( "qgis//mytest1/" );
}

void TestQgsHttpheaders::setFromSettings( const QString &keyBase )
{
  QgsSettings settings;
  QString outOfHeaderKey = QStringLiteral( "outofheader" );
  settings.remove( keyBase ); // cleanup

  settings.setValue( keyBase + outOfHeaderKey, "value" );
  settings.setValue( keyBase + QgsHttpHeaders::PATH_PREFIX + "key1", "value1" );
  settings.setValue( keyBase + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER, "valueHH_R" );

  QgsHttpHeaders h( settings, keyBase );
  QVERIFY( ! h.keys().contains( outOfHeaderKey ) );
  QVERIFY( h.keys().contains( QStringLiteral( "key1" ) ) );
  QCOMPARE( h [ QStringLiteral( "key1" ) ].toString(), QStringLiteral( "value1" ) );
  QVERIFY( h.keys().contains( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( h [QgsHttpHeaders::KEY_REFERER ].toString(), QStringLiteral( "valueHH_R" ) );

  settings.setValue( keyBase + QgsHttpHeaders::KEY_REFERER, "value_R" );
  QgsHttpHeaders h2( settings, keyBase );
  QVERIFY( h2.keys().contains( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( h2 [QgsHttpHeaders::KEY_REFERER].toString(), QStringLiteral( "value_R" ) );
}

void TestQgsHttpheaders::updateSettings()
{
  QgsSettings settings;
  QString keyBase = QStringLiteral( "qgis/mytest2/" );
  settings.remove( keyBase ); // cleanup

  QgsHttpHeaders h( QVariantMap( { {QStringLiteral( "key1" ), "value1"}} ) );
  h.updateSettings( settings, keyBase );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::PATH_PREFIX + "key1" ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::PATH_PREFIX + "key1" ).toString(), "value1" );

  QVERIFY( ! settings.contains( keyBase + QgsHttpHeaders::KEY_REFERER ) );
  QVERIFY( ! settings.contains( keyBase + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER ) );

  // at old location
  settings.remove( keyBase + QgsHttpHeaders::KEY_REFERER );
  h [QgsHttpHeaders::KEY_REFERER] = QStringLiteral( "http://gg.com" );

  h.updateSettings( settings, keyBase );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER ).toString(), "http://gg.com" );
  QVERIFY( ! settings.contains( keyBase + QgsHttpHeaders::KEY_REFERER ) );

  // test backward compatibility
  settings.setValue( keyBase + QgsHttpHeaders::KEY_REFERER, "paf" ) ; // legacy referer, should be overridden
  h.updateSettings( settings, keyBase );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER ).toString(), "http://gg.com" );
  QVERIFY( settings.contains( keyBase + QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( settings.value( keyBase + QgsHttpHeaders::KEY_REFERER ).toString(), "http://gg.com" );
}


void TestQgsHttpheaders::createQgsOwsConnection()
{
  QgsSettings settings;
  settings.setValue( QString( QgsSettings::Prefix::QGIS ) + "/connections-service/name/" + QgsHttpHeaders::PATH_PREFIX + QgsHttpHeaders::KEY_REFERER,
                     "http://test.com" );
  settings.setValue( QString( QgsSettings::Prefix::QGIS ) + "/connections-service/name/" + QgsHttpHeaders::PATH_PREFIX + "other_http_header",
                     "value" );

  QgsOwsConnection ows( "service", "name" );
  QCOMPARE( ows.connectionInfo(), ",authcfg=,referer=http://test.com" );
  QCOMPARE( ows.uri().encodedUri(), "url&http-header:other_http_header=value&http-header:referer=http://test.com" );

  QgsDataSourceUri uri( QString( "https://www.ogc.org/?p1=v1" ) );
  QgsDataSourceUri uri2 = ows.addWmsWcsConnectionSettings( uri, "service", "name" );
  QCOMPARE( uri2.encodedUri(), "https://www.ogc.org/?p1=v1&http-header:other_http_header=value&http-header:referer=http://test.com" );

  // check space separated string
  QCOMPARE( uri2.uri(), " https://www.ogc.org/?p1='v1' http-header:other_http_header='value' http-header:referer='http://test.com' referer='http://test.com'" );
  // build new QgsDataSourceUri according to space separated string
  QgsDataSourceUri uri3( uri2.uri() );
  QCOMPARE( uri3.httpHeader( QgsHttpHeaders::KEY_REFERER ), "http://test.com" );
  QCOMPARE( uri3.httpHeader( "other_http_header" ), "value" );
  QCOMPARE( uri3.encodedUri(), "https://www.ogc.org/?p1=v1&referer=http://test.com&http-header:other_http_header=value&http-header:referer=http://test.com" );
}


void TestQgsHttpheaders::updateNetworkRequest()
{
  const QUrl url( "http://ogc.org" );
  QNetworkRequest request( url );
  QgsHttpHeaders h( QVariantMap( { {QStringLiteral( "key1" ), "value1"},  {QgsHttpHeaders::KEY_REFERER, "my_ref"}} ) );
  h.updateNetworkRequest( request );

  QVERIFY( request.hasRawHeader( "key1" ) );
  QCOMPARE( request.rawHeader( "key1" ), "value1" );

  QVERIFY( request.hasRawHeader( QByteArray::fromStdString( QgsHttpHeaders::KEY_REFERER.toStdString() ) ) );
  QCOMPARE( request.rawHeader( QByteArray::fromStdString( QgsHttpHeaders::KEY_REFERER.toStdString() ) ), "my_ref" );
}


void TestQgsHttpheaders::updateSetUrlQuery()
{
  QUrlQuery url( "http://ogc.org" );
  // === update
  QgsHttpHeaders h( QVariantMap( { {QStringLiteral( "key1" ), "value1"},  {QgsHttpHeaders::KEY_REFERER, "my_ref"}} ) );
  h.updateUrlQuery( url );

  QVERIFY( url.hasQueryItem( QgsHttpHeaders::PARAM_PREFIX +  "key1" ) );
  QCOMPARE( url.queryItemValue( QgsHttpHeaders::PARAM_PREFIX +  "key1" ), "value1" );

  QVERIFY( url.hasQueryItem( QgsHttpHeaders::PARAM_PREFIX +  QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( url.queryItemValue( QgsHttpHeaders::PARAM_PREFIX +  QgsHttpHeaders::KEY_REFERER ), "my_ref" );

  // TODO mandatory or not?
  /*QVERIFY( url.hasQueryItem( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( url.queryItemValue( QgsHttpHeaders::KEY_REFERER ), "my_ref" );*/

  // === setFrom
  QgsHttpHeaders h2;
  /*  url.removeQueryItem(QgsHttpHeaders::KEY_REFERER);
    url.addQueryItem( QgsHttpHeaders::KEY_REFERER, "my_ref_root" ); // overwrite root ref to ckeck backward compatibility
    */
  h2.setFromUrlQuery( url );
  QVERIFY( h2.keys().contains( QStringLiteral( "key1" ) ) );
  QCOMPARE( h2 [ QStringLiteral( "key1" ) ].toString(), QStringLiteral( "value1" ) );
  QVERIFY( h2.keys().contains( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( h2 [QgsHttpHeaders::KEY_REFERER ].toString(), QStringLiteral( "my_ref" ) );

}


void TestQgsHttpheaders::updateSetMap()
{
  QVariantMap map;
  // === update
  QgsHttpHeaders h( QVariantMap( { {QStringLiteral( "key1" ), "value1"},  {QgsHttpHeaders::KEY_REFERER, "my_ref"}} ) );
  h.updateMap( map );

  QVERIFY( map.contains( QgsHttpHeaders::PARAM_PREFIX +  "key1" ) );
  QCOMPARE( map[QgsHttpHeaders::PARAM_PREFIX +  "key1"], "value1" );

  QVERIFY( map.contains( QgsHttpHeaders::PARAM_PREFIX +  QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( map[QgsHttpHeaders::PARAM_PREFIX +  QgsHttpHeaders::KEY_REFERER], "my_ref" );

  QVERIFY( map.contains( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( map[QgsHttpHeaders::KEY_REFERER], "my_ref" );

  // === setFrom
  QgsHttpHeaders h2;
  map[QgsHttpHeaders::KEY_REFERER] = "my_ref_root"; // overwrite root ref to ckeck backward compatibility
  h2.setFromMap( map );
  QVERIFY( h2.keys().contains( QStringLiteral( "key1" ) ) );
  QCOMPARE( h2 [ QStringLiteral( "key1" ) ].toString(), QStringLiteral( "value1" ) );
  QVERIFY( h2.keys().contains( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( h2 [QgsHttpHeaders::KEY_REFERER ].toString(), QStringLiteral( "my_ref_root" ) );
}


void TestQgsHttpheaders::updateSetDomElement()
{
  QDomDocument doc( QStringLiteral( "connections" ) );
  QDomElement element = doc.createElement( "qgs" );
  // === update
  QgsHttpHeaders h( QVariantMap( { {QStringLiteral( "key1" ), "value1"},  {QgsHttpHeaders::KEY_REFERER, "my_ref"}} ) );
  h.updateDomElement( element );

  QVERIFY( element.hasAttribute( QgsHttpHeaders::PARAM_PREFIX +  "key1" ) );
  QCOMPARE( element.attribute( QgsHttpHeaders::PARAM_PREFIX +  "key1" ), "value1" );

  QVERIFY( element.hasAttribute( QgsHttpHeaders::PARAM_PREFIX +  QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( element.attribute( QgsHttpHeaders::PARAM_PREFIX +  QgsHttpHeaders::KEY_REFERER ), "my_ref" );

  // TODO mandatory or not?
  QVERIFY( element.hasAttribute( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( element.attribute( QgsHttpHeaders::KEY_REFERER ), "my_ref" );

  // === setFrom
  QgsHttpHeaders h2;
  element.setAttribute( QgsHttpHeaders::KEY_REFERER, "my_ref_root" ); // overwrite root ref to ckeck backward compatibility
  h2.setFromDomElement( element );
  QVERIFY( h2.keys().contains( QStringLiteral( "key1" ) ) );
  QCOMPARE( h2 [ QStringLiteral( "key1" ) ].toString(), QStringLiteral( "value1" ) );
  QVERIFY( h2.keys().contains( QgsHttpHeaders::KEY_REFERER ) );
  QCOMPARE( h2 [QgsHttpHeaders::KEY_REFERER ].toString(), QStringLiteral( "my_ref_root" ) );
}

QGSTEST_MAIN( TestQgsHttpheaders )
#include "testqgshttpheaders.moc"
