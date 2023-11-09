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
#include "qgswmsrequest.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS parameters class
 */
class TestQgsServerWmsRequest : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void cst();
    void update();
};

void TestQgsServerWmsRequest::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsServerWmsRequest::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerWmsRequest::cst()
{
  QgsServerRequest request;
  request.setParameter( "VERSION", "1.0.0" );

  // the base class for parameters provides raw values...
  QCOMPARE( request.serverParameters().version(), "1.0.0" );

  // ...whereas the wms parameters class interprets values to be valid in the
  // WMS context
  const QgsWms::QgsWmsRequest wmsRequest( request );
  QCOMPARE( wmsRequest.wmsParameters().version(), "1.1.1" );
}

void TestQgsServerWmsRequest::update()
{
  // init request with parameters
  QgsServerRequest request;
  request.setParameter( "PARAM_0", "0" );
  request.setParameter( "PARAM_1", "1" );

  QgsWms::QgsWmsRequest wmsRequest( request );
  QCOMPARE( wmsRequest.wmsParameters().value( "PARAM_0" ), "0" );

  wmsRequest.setParameter( "PARAM_3", "3" );
  QCOMPARE( wmsRequest.wmsParameters().value( "PARAM_3" ), "3" );

  // init request by loading a query
  const QUrl url( "http://qgisserver?PARAM_0=0&PARAM_1=1" );

  const QgsServerRequest request2( url );

  QCOMPARE( request2.serverParameters().value( "PARAM_0" ), "0" );

  QgsWms::QgsWmsRequest wmsRequest2( request2 );
  QCOMPARE( wmsRequest2.wmsParameters().value( "PARAM_0" ), "0" );

  wmsRequest2.setParameter( "PARAM_3", "3" );
  QCOMPARE( wmsRequest2.wmsParameters().value( "PARAM_3" ), "3" );
}

QGSTEST_MAIN( TestQgsServerWmsRequest )
#include "test_qgsserver_wms_request.moc"
