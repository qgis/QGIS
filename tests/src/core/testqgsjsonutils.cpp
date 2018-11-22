/***************************************************************************
     testqgsjsonutils.cpp
     --------------------------------------
    Date                 : September 2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <qgsjsonutils.h>

class TestQgsJsonUtils : public QObject
{
    Q_OBJECT
  private slots:
    void testStringList()
    {
      QStringList list;

      {
        const QString json = QgsJsonUtils::encodeValue( list );
        QCOMPARE( json, QString( "[]" ) );
        const QVariant back = QgsJsonUtils::parseArray( json, QVariant::String );
        QCOMPARE( back.toStringList(), list );
      }

      {
        list << QStringLiteral( "one" ) << QStringLiteral( "<',\"\\>" ) << QStringLiteral( "two" );
        const QString json = QgsJsonUtils::encodeValue( list );
        QCOMPARE( json, QString( "[\"one\",\"<',\\\"\\\\>\",\"two\"]" ) );
        const QVariant back = QgsJsonUtils::parseArray( json, QVariant::String );
        QCOMPARE( back.toStringList(), list );
      }
    }

    void testIntList()
    {
      QVariantList list;

      {
        list << 1 << -2;
        const QString json = QgsJsonUtils::encodeValue( list );
        QCOMPARE( json, QString( "[1,-2]" ) );
        const QVariantList back = QgsJsonUtils::parseArray( json, QVariant::Int );
        QCOMPARE( back, list );
        QCOMPARE( back.at( 0 ).type(), QVariant::Int );
      }

      {
        // check invalid entries are ignored
        const QVariantList back = QgsJsonUtils::parseArray( QStringLiteral( "[1,\"a\",-2]" ), QVariant::Int );
        QCOMPARE( back, list );
      }
    }

    void testDoubleList()
    {
      QVariantList list;

      list << 1.0 << -2.2456;
      const QString json = QgsJsonUtils::encodeValue( list );
      QCOMPARE( json, QString( "[1,-2.2456]" ) );
      const QVariantList back = QgsJsonUtils::parseArray( json, QVariant::Double );
      QCOMPARE( back, list );
      QCOMPARE( back.at( 0 ).type(), QVariant::Double );
    }
};

QGSTEST_MAIN( TestQgsJsonUtils )
#include "testqgsjsonutils.moc"
