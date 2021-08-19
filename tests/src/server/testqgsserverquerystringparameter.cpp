/***************************************************************************

   testqgsserverquerystringparameter.cpp
     --------------------------------------
    Date                 : Jul 10 2019
    Copyright            : (C) 2019 by Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>

//qgis includes...
#include "qgsserverquerystringparameter.h"
#include "qgsserverapicontext.h"
#include "qgsserverrequest.h"
#include "qgsserverexception.h"
#include "qgsrequesthandler.h"
#include "qgsbufferserverrequest.h"
#include "qgsbufferserverresponse.h"

/**
 * \ingroup UnitTests
 * Unit tests for the server query string parameter
 */
class TestQgsServerQueryStringParameter : public QObject
{
    Q_OBJECT

  public:
    TestQgsServerQueryStringParameter() = default;

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase();

    // will be called after the last testfunction was executed.
    void cleanupTestCase();

    // will be called before each testfunction is executed
    void init();

    // will be called after every testfunction.
    void cleanup();

    // Basic test on types and constraints
    void testArguments();

    // Test custom validators
    void testCustomValidators();

    // Test default values
    void testDefaultValues();

    // Test QgsRequestHandler::parseInput (i.e. POST requests) with special chars
    void testParseInput();
};


void TestQgsServerQueryStringParameter::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsServerQueryStringParameter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerQueryStringParameter::init()
{
}

void TestQgsServerQueryStringParameter::cleanup()
{
}

void TestQgsServerQueryStringParameter::testArguments()
{
  QgsServerQueryStringParameter p { QStringLiteral( "parameter1" ) };
  QgsServerRequest request;
  const QgsServerApiContext ctx { "/wfs3", &request, nullptr, nullptr, nullptr };

  // Test string (default)
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ).toString(), QString( "123" ) );
  QCOMPARE( p.value( ctx ).type(), QVariant::String );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=a%20string" ) );
  QCOMPARE( p.value( ctx ).toString(), QString( "a string" ) );
  QCOMPARE( p.value( ctx ).type(), QVariant::String );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/" ) );
  QCOMPARE( p.value( ctx ).toString(), QString() );

  // Test required
  p.mRequired = true;
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/" ) );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestException );

  // Test int
  p.mType = QgsServerQueryStringParameter::Type::Integer;
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ).toInt(), 123 );
  QCOMPARE( p.value( ctx ).type(), QVariant::LongLong );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=a%20string" ) );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestException );

  // Test double
  p.mType = QgsServerQueryStringParameter::Type::Double;
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ).toDouble(), 123.0 );
  QCOMPARE( p.value( ctx ).type(), QVariant::Double );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123.456" ) );
  QCOMPARE( p.value( ctx ).toDouble(), 123.456 );
  QCOMPARE( p.value( ctx ).type(), QVariant::Double );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=a%20string" ) );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestException );
  QCOMPARE( QString::fromStdString( p.data()["schema"]["type"] ), QString( "number" ) );

  // Test list
  p.mType = QgsServerQueryStringParameter::Type::List;
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123,a%20value" ) );
  QCOMPARE( p.value( ctx ).toStringList(), QStringList() << QStringLiteral( "123" ) << QStringLiteral( "a value" ) );
  QCOMPARE( p.value( ctx ).type(), QVariant::StringList );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=a%20value" ) );
  QCOMPARE( p.value( ctx ).toStringList(), QStringList() << QStringLiteral( "a value" ) );
  QCOMPARE( p.value( ctx ).type(), QVariant::StringList );

}

void TestQgsServerQueryStringParameter::testCustomValidators()
{
  QgsServerQueryStringParameter p { QStringLiteral( "parameter1" ), true, QgsServerQueryStringParameter::Type::Integer };
  QgsServerRequest request;
  const QgsServerApiContext ctx { "/wfs3", &request, nullptr, nullptr, nullptr };

  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ).toInt(), 123 );

  // Test a range validator that increments the value
  const QgsServerQueryStringParameter::customValidator validator = [ ]( const QgsServerApiContext &, QVariant & value ) -> bool
  {
    const auto v { value.toLongLong() };
    // Change the value by adding 1
    value.setValue( v + 1 );
    return v > 500 && v < 1000;
  };
  p.setCustomValidator( validator );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestException );

  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=501" ) );
  QCOMPARE( p.value( ctx ).toInt(), 502 );
  QCOMPARE( p.value( ctx ).type(), QVariant::LongLong );

}

void TestQgsServerQueryStringParameter::testDefaultValues()
{
  // Set a default AND required, verify it's ignored
  const QgsServerQueryStringParameter p { QStringLiteral( "parameter1" ), true, QgsServerQueryStringParameter::Type::Integer, QStringLiteral( "Paramerer 1" ), 10 };
  QgsServerRequest request;
  const QgsServerApiContext ctx { "/wfs3", &request, nullptr, nullptr, nullptr };

  request.setUrl( QStringLiteral( "http://www.qgis.org/api/" ) );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestException );

  const QgsServerQueryStringParameter p2 { QStringLiteral( "parameter1" ), false, QgsServerQueryStringParameter::Type::Integer, QStringLiteral( "Paramerer 1" ), 10 };
  QCOMPARE( p2.value( ctx ).toInt(), 10 );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=501" ) );
  QCOMPARE( p2.value( ctx ).toInt(), 501 );

}

void TestQgsServerQueryStringParameter::testParseInput()
{
  // Request with layers "a", "b", "c" and "äös + %&#"
  QByteArray data( "SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&LAYERS=a%2Cb%2Cc%2C%C3%A4%C3%B6s+%2B+%25%26%23" );
  QgsBufferServerRequest request( QStringLiteral( "http://localhost/wms/test" ), QgsServerRequest::PostMethod, QgsServerRequest::Headers(), &data );
  QgsBufferServerResponse response;

  QgsRequestHandler requestHandler( request, response );
  requestHandler.parseInput();

  const QgsServerParameters params = request.serverParameters();
  QMap<QString, QString> paramsMap = params.toMap();
  QCOMPARE( paramsMap["SERVICE"], QStringLiteral( "WMS" ) );
  QCOMPARE( paramsMap["VERSION"], QStringLiteral( "1.3.0" ) );
  QCOMPARE( paramsMap["REQUEST"], QStringLiteral( "GetMap" ) );
  QCOMPARE( paramsMap["LAYERS"], QStringLiteral( "a,b,c,äös + %&#" ) );
}


QGSTEST_MAIN( TestQgsServerQueryStringParameter )
#include "testqgsserverquerystringparameter.moc"
