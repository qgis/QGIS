/***************************************************************************
    testqgspostgresprovider.cpp
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick dot valsecchi at camptocamp dot com
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

#include <qgspostgresprovider.h>

class TestQgsPostgresProvider: public QObject
{
    Q_OBJECT
  private slots:
    void decodeHstore()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "\"1\"=>\"2\", \"a\"=>\"b, \\\"c'\", \"backslash\"=>\"\\\\\"" ), QStringLiteral( "hstore" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "1" )] = "2";
      expected[QStringLiteral( "a" )] = "b, \"c'";
      expected[QStringLiteral( "backslash" )] = "\\";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void decodeHstoreNoQuote()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "1=>2, a=>b c" ), QStringLiteral( "hstore" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "1" )] = "2";
      expected[QStringLiteral( "a" )] = "b c";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void decodeArray2StringList()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{\"1\",\"2\", \"a\\\\1\" , \"\\\\\",\"b, \\\"c'\"}" ), QStringLiteral( "hstore" ) );
      QCOMPARE( decoded.type(), QVariant::StringList );

      QStringList expected;
      expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a\\1" ) << QStringLiteral( "\\" ) << QStringLiteral( "b, \"c'" );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toStringList(), expected );
    }

    void decodeArray2StringListNoQuote()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{1,2, a ,b, c}" ), QStringLiteral( "hstore" ) );
      QCOMPARE( decoded.type(), QVariant::StringList );

      QStringList expected;
      expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toStringList(), expected );
    }

    void decodeArray2IntList()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{1, 2, 3,-5,10}" ), QStringLiteral( "hstore" ) );
      QCOMPARE( decoded.type(), QVariant::StringList );

      QVariantList expected;
      expected << QVariant( 1 ) << QVariant( 2 ) << QVariant( 3 ) << QVariant( -5 ) << QVariant( 10 );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toList(), expected );
    }
    void decodeJsonList()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "[1,2,3]" ), QStringLiteral( "json" ) );
      QCOMPARE( decoded.type(), QVariant::List );

      QVariantList expected;
      expected.append( 1 );
      expected.append( 2 );
      expected.append( 3 );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toList(), expected );
    }
    void decodeJsonbList()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "[1,2,3]" ), QStringLiteral( "jsonb" ) );
      QCOMPARE( decoded.type(), QVariant::List );

      QVariantList expected;
      expected.append( 1 );
      expected.append( 2 );
      expected.append( 3 );
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toList(), expected );
    }
    void decodeJsonMap()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "json" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "a" )] = "1";
      expected[QStringLiteral( "b" )] = "2";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }
    void decodeJsonbMap()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "jsonb" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "a" )] = "1";
      expected[QStringLiteral( "b" )] = "2";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }
    /*
    void decodeJsonInt()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "'123'" ), QStringLiteral( "json" ) );
      QCOMPARE( decoded.type(), QVariant::Int );

      int expected;
      expected="'123'";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded., expected );
    }
    void decodeJsonbInt()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "'123'" ), QStringLiteral( "jsonb" ) );
      QCOMPARE( decoded.type(), QVariant::Int );

      int expected;
      expected=123;
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toInt(), expected );
    }
    */
};

QGSTEST_MAIN( TestQgsPostgresProvider )
#include "testqgspostgresprovider.moc"
