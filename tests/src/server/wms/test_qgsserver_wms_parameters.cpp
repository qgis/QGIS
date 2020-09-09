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

  QgsWms::QgsWmsParameters parameters( query );

  QList<QgsWms::QgsWmsParametersLayer> layers_params = parameters.layersParameters();
  QCOMPARE( layers_params.size(), 3 );

  QgsWms::QgsWmsParametersLayer layer_params = layers_params[0];
  QCOMPARE( layer_params.mNickname, QString( "external_layer_1" ) );
  QCOMPARE( layer_params.mExternalUri, QString( "layers=layer_1_name&url=http://url_1" ) );

  layer_params = layers_params[1];
  QCOMPARE( layer_params.mNickname, QString( "layer" ) );

  layer_params = layers_params[2];
  QCOMPARE( layer_params.mNickname, QString( "external_layer_2" ) );
  QCOMPARE( layer_params.mExternalUri, QString( "layers=layer_2_name&url=http://url_2" ) );
}

QGSTEST_MAIN( TestQgsServerWmsParameters )
#include "test_qgsserver_wms_parameters.moc"
