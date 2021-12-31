/***************************************************************************
     test_qgsserver_wms_parameters.cpp
     ---------------------------------
    Date                 : 02 Sept 2020
    Copyright            : (C) 2020 by Paul Blottiere
    Email                : paul dot blottiere @ gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgswmsparameters.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS parameters class
 */
class TestQgsServerWmsParameters : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void external_layers();
    void percent_encoding();
    void version_negotiation();
    void get_capabilities_version();
};

void TestQgsServerWmsParameters::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsServerWmsParameters::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerWmsParameters::external_layers()
{
  QUrlQuery query;
  query.addQueryItem( "LAYERS", "EXTERNAL_WMS:external_layer_1,layer,EXTERNAL_WMS:external_layer_2" );
  query.addQueryItem( "external_layer_1:url", "http://url_1" );
  query.addQueryItem( "external_layer_1:layers", "layer_1_name" );
  query.addQueryItem( "external_layer_2:url", "http://url_2" );
  query.addQueryItem( "external_layer_2:layers", "layer_2_name" );
  query.addQueryItem( "external_layer_2:opacities", "100" );
  query.addQueryItem( "OPACITIES", "255,200,125" );

  const QgsWms::QgsWmsParameters parameters( query );

  QList<QgsWms::QgsWmsParametersLayer> layers_params = parameters.layersParameters();
  QCOMPARE( layers_params.size(), 3 );

  QgsWms::QgsWmsParametersLayer layer_params = layers_params[0];
  QCOMPARE( layer_params.mNickname, QString( "external_layer_1" ) );
  QCOMPARE( layer_params.mExternalUri, QString( "layers=layer_1_name&url=http://url_1" ) );

  layer_params = layers_params[1];
  QCOMPARE( layer_params.mNickname, QString( "layer" ) );

  layer_params = layers_params[2];
  QCOMPARE( layer_params.mNickname, QString( "external_layer_2" ) );
  QCOMPARE( layer_params.mExternalUri, QString( "layers=layer_2_name&opacities=100&url=http://url_2" ) );

  //test if opacities are also applied to external layers
  QCOMPARE( layers_params[0].mOpacity, 255 );
  QCOMPARE( layers_params[1].mOpacity, 200 );
  QCOMPARE( layers_params[2].mOpacity, 125 );
}

void TestQgsServerWmsParameters::percent_encoding()
{
  // '+' in its encoded ('%2B') form is transformed in '+' sign and
  // forwarded to parameters subclasses
  QUrlQuery query;
  query.addQueryItem( "MYPARAM", QString( "a%2Cb%2Cc%2C%C3%A4%C3%B6s+%2B+%25%26%23" ) );

  QgsServerParameters params;
  params.load( query );
  QCOMPARE( params.value( "MYPARAM" ), QString( "a,b,c,äös + %&#" ) );

  const QgsWms::QgsWmsParameters wmsParams( params );
  QCOMPARE( wmsParams.value( "MYPARAM" ), QString( "a,b,c,äös + %&#" ) );

  // back to urlQuery
  QgsServerParameters params2;
  params2.load( params.urlQuery() );
  QCOMPARE( params2.value( "MYPARAM" ), QString( "a,b,c,äös + %&#" ) );
}

void TestQgsServerWmsParameters::version_negotiation()
{
  QUrlQuery query;

  query.addQueryItem( "VERSION", "1.3.0" );
  QgsWms::QgsWmsParameters parameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.3.0" ) );

  query.clear();
  query.addQueryItem( "VERSION", "1.1.1" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.1.1" ) );

  query.clear();
  query.addQueryItem( "VERSION", "1.1.0" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.1.1" ) );

  query.clear();
  query.addQueryItem( "VERSION", "" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.3.0" ) );

  query.clear();
  query.addQueryItem( "VERSION", "33.33.33" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.3.0" ) );

  query.clear();
  query.addQueryItem( "VERSION", "1.1.1" );
  query.addQueryItem( "REQUEST", "GetProjectSettings" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.3.0" ) );

  query.clear();
  query.addQueryItem( "REQUEST", "GetProjectSettings" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.3.0" ) );

  query.clear();
  query.addQueryItem( "WMTVER", "1.1.1" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.1.1" ) );

  query.clear();
  query.addQueryItem( "WMTVER", "1.1.1" );
  query.addQueryItem( "VERSION", "1.3.0" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.version(), QStringLiteral( "1.3.0" ) );
}

void TestQgsServerWmsParameters::get_capabilities_version()
{
  QUrlQuery query;

  query.addQueryItem( "VERSION", "1.3.0" );
  query.addQueryItem( "REQUEST", "capabilities" );
  QgsWms::QgsWmsParameters parameters( query );
  QCOMPARE( parameters.request(), QStringLiteral( "capabilities" ) );

  query.clear();
  query.addQueryItem( "VERSION", "1.1.1" );
  query.addQueryItem( "REQUEST", "capabilities" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.request(), QStringLiteral( "GetCapabilities" ) );

  query.clear();
  query.addQueryItem( "VERSION", "1.1.0" );
  query.addQueryItem( "REQUEST", "capabilities" );
  parameters = QgsWms::QgsWmsParameters( query );
  QCOMPARE( parameters.request(), QStringLiteral( "GetCapabilities" ) );
}

QGSTEST_MAIN( TestQgsServerWmsParameters )
#include "test_qgsserver_wms_parameters.moc"
