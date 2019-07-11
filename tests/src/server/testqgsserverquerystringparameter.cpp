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
  QgsServerApiContext ctx { "/wfs3", &request, nullptr, nullptr, nullptr };

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
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestError );

  // Test int
  p.mType = QgsServerQueryStringParameter::Type::Int;
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ), 123 );
  QCOMPARE( p.value( ctx ).type(), QVariant::LongLong );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=a%20string" ) );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestError );

  // Test double
  p.mType = QgsServerQueryStringParameter::Type::Double;
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ), 123.0 );
  QCOMPARE( p.value( ctx ).type(), QVariant::Double );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123.456" ) );
  QCOMPARE( p.value( ctx ), 123.456 );
  QCOMPARE( p.value( ctx ).type(), QVariant::Double );
  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=a%20string" ) );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestError );

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
  QgsServerQueryStringParameter p { QStringLiteral( "parameter1" ), true, QgsServerQueryStringParameter::Type::Int };
  QgsServerRequest request;
  QgsServerApiContext ctx { "/wfs3", &request, nullptr, nullptr, nullptr };

  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=123" ) );
  QCOMPARE( p.value( ctx ), 123 );

  // Test a range validator that increments the value
  QgsServerQueryStringParameter::customValidator validator = [ ]( const QgsServerApiContext &, QVariant & value ) -> bool
  {
    const auto v { value.toLongLong() };
    // Change the value by adding 1
    value.setValue( v + 1 );
    return v > 500 && v < 1000;
  };
  p.setCustomValidator( validator );
  QVERIFY_EXCEPTION_THROWN( p.value( ctx ), QgsServerApiBadRequestError );

  request.setUrl( QStringLiteral( "http://www.qgis.org/api/?parameter1=501" ) );
  QCOMPARE( p.value( ctx ), 502 );
  QCOMPARE( p.value( ctx ).type(), QVariant::LongLong );

}

QGSTEST_MAIN( TestQgsServerQueryStringParameter )
#include "testqgsserverquerystringparameter.moc"
