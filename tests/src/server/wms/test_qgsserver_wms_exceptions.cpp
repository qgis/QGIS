/***************************************************************************
     test_qgsserver_wms_exceptions.cpp
     ---------------------------------
    Date                 : 27 Mar 2019
    Copyright            : (C) 2019 by Paul Blottiere
    Email                : paul dot blottiere @ oslandia.com
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
#include "qgswmsserviceexception.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS Exceptions
 */
class TestQgsServerWmsExceptions : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void exception_code();
    void exception_message();
};

void TestQgsServerWmsExceptions::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsServerWmsExceptions::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerWmsExceptions::exception_code()
{
  QgsWms::QgsServiceException::ExceptionCode code = QgsWms::QgsServiceException::OGC_InvalidFormat;
  const QgsWms::QgsServiceException exception0( code, QString(), 400 );
  QCOMPARE( exception0.code(), QString( "InvalidFormat" ) );

  code = QgsWms::QgsServiceException::QGIS_InvalidParameterValue;
  const QgsWms::QgsServiceException exception1( code, QString(), 400 );
  QCOMPARE( exception1.code(), QString( "InvalidParameterValue" ) );
}

void TestQgsServerWmsExceptions::exception_message()
{
  QgsWms::QgsServiceException::ExceptionCode code = QgsWms::QgsServiceException::QGIS_MissingParameterValue;
  const QgsWms::QgsServiceException exception( code, QgsWms::QgsWmsParameter::LAYER, 400 );
  QCOMPARE( exception.message(), QString( "The LAYER parameter is missing." ) );

  code = QgsWms::QgsServiceException::OGC_InvalidSRS;
  const QgsWms::QgsServiceException exception_srs( code, QgsWms::QgsWmsParameter::SRS, 400 );
  QCOMPARE( exception_srs.message(), QString( "The SRS is not valid." ) );
  QCOMPARE( exception_srs.code(), QString( "InvalidSRS" ) );

  code = QgsWms::QgsServiceException::OGC_InvalidCRS;
  const QgsWms::QgsServiceException exception_crs( code, QgsWms::QgsWmsParameter::CRS, 400 );
  QCOMPARE( exception_crs.message(), QString( "The CRS is not valid." ) );
  QCOMPARE( exception_crs.code(), QString( "InvalidCRS" ) );
}

QGSTEST_MAIN( TestQgsServerWmsExceptions )
#include "test_qgsserver_wms_exceptions.moc"
