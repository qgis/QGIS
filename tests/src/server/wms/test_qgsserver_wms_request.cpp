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
  QgsWms::QgsWmsRequest wmsRequest( request );
  QCOMPARE( wmsRequest.wmsParameters().version(), "1.1.1" );
}

QGSTEST_MAIN( TestQgsServerWmsRequest )
#include "test_qgsserver_wms_request.moc"
