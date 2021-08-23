/***************************************************************************
     testqgssqliteutils.cpp
     --------------------------------------
    Date                 : 2018-06-13
    Copyright            : (C) 2018 Alessandro Pasotti
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
#include <QStringList>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include "qgssqliteutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test to verify that QgsSqliteUtils are working correctly
 */
class TestQgsSqliteUtils : public QObject
{
    Q_OBJECT

  public:

    TestQgsSqliteUtils() = default;


  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.
    // void initStyles();

    void testPrintfAscii();
    void testPrintfUtf8();
    void testQuotedString_data();
    void testQuotedString();
    void testQuotedIdentifier_data();
    void testQuotedIdentifier();
    void testQuotedValue_data();
    void testQuotedValue();
};


// slots
void TestQgsSqliteUtils::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDatabase();

  // output test environment
  QgsApplication::showSettings();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );


}

void TestQgsSqliteUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSqliteUtils::testPrintfAscii()
{
  const QString tag( "Meteor" );
  const QString query( qgs_sqlite3_mprintf( "SELECT id FROM tag WHERE LOWER(name)='%q'", tag.toUtf8().toLower().constData() ) );
  QCOMPARE( query, QString( "SELECT id FROM tag WHERE LOWER(name)='%1'" ).arg( tag.toLower() ) );
}


void TestQgsSqliteUtils::testPrintfUtf8()
{
  const QString tag( "МЕТЕОР" );
  QCOMPARE( tag.toLower(), QString( "метеор" ) );
  const QString lowerTag( tag.toLower() );
  const QString query( qgs_sqlite3_mprintf( "SELECT id FROM tag WHERE LOWER(name)='%q'", lowerTag.toUtf8().constData() ) );
  QCOMPARE( query, QString( "SELECT id FROM tag WHERE LOWER(name)='%1'" ).arg( lowerTag ) );
}

void TestQgsSqliteUtils::testQuotedString_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "test 1" ) << "university of qgis" << "'university of qgis'";
  QTest::newRow( "test 2" ) << "university of 'qgis'" << "'university of ''qgis'''";
  QTest::newRow( "test NULL" ) << QString() << "NULL";
}

void TestQgsSqliteUtils::testQuotedString()
{
  QFETCH( QString, input );
  QFETCH( QString, expected );

  QCOMPARE( QgsSqliteUtils::quotedString( input ), expected );
}

void TestQgsSqliteUtils::testQuotedIdentifier_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "myColumn" ) << "myColumn" << "\"myColumn\"";
  QTest::newRow( "my column" ) << "my column" << "\"my column\"";
  QTest::newRow( "The \"Column\"" ) << "The \"Column\"" << "\"The \"\"Column\"\"\"";
}

void TestQgsSqliteUtils::testQuotedIdentifier()
{
  QFETCH( QString, input );
  QFETCH( QString, expected );

  QCOMPARE( QgsSqliteUtils::quotedIdentifier( input ), expected );
}

void TestQgsSqliteUtils::testQuotedValue_data()
{
  QTest::addColumn<QVariant>( "input" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "String" ) << QVariant( "Test string" ) << "'Test string'";
  QTest::newRow( "Integer" ) << QVariant( 5 ) << "5";
  QTest::newRow( "Double" ) << QVariant( 3.2 ) << "3.2";
  QTest::newRow( "Boolean" ) << QVariant( true ) << "1";
  QTest::newRow( "Escaped string" ) << QVariant( "It's a test string" ) << "'It''s a test string'";
  QTest::newRow( "NULL" ) << QVariant() << "NULL";
}

void TestQgsSqliteUtils::testQuotedValue()
{
  QFETCH( QVariant, input );
  QFETCH( QString, expected );

  QCOMPARE( QgsSqliteUtils::quotedValue( input ), expected );
}


QGSTEST_MAIN( TestQgsSqliteUtils )
#include "testqgssqliteutils.moc"
