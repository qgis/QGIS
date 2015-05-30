/***************************************************************************
     testqgspallabeling.cpp
     ----------------------
    Date                 : May 2015
    Copyright            : (C) 2015 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSharedPointer>

#include "qgspallabeling.h"

class TestQgsPalLabeling: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void wrapChar();//test wrapping text lines
    void graphemes(); //test splitting strings to graphemes

  private:
};

void TestQgsPalLabeling::initTestCase()
{
}

void TestQgsPalLabeling::cleanupTestCase()
{

}

void TestQgsPalLabeling::init()
{

}

void TestQgsPalLabeling::cleanup()
{

}

void TestQgsPalLabeling::wrapChar()
{
  QCOMPARE( QgsPalLabeling::splitToLines( "nolines", QString() ) , QStringList() << "nolines" );
  QCOMPARE( QgsPalLabeling::splitToLines( "new line\nonly", QString() ), QStringList() << "new line" << "only" );
  QCOMPARE( QgsPalLabeling::splitToLines( "new line\nonly", QString( "\n" ) ), QStringList() << "new line" << "only" );
  QCOMPARE( QgsPalLabeling::splitToLines( "mixed new line\nand char", QString( " " ) ), QStringList() << "mixed" << "new" << "line" << "and" << "char" );
  QCOMPARE( QgsPalLabeling::splitToLines( "no matching chars", QString( "#" ) ), QStringList() << "no matching chars" );
  QCOMPARE( QgsPalLabeling::splitToLines( "no\nmatching\nchars", QString( "#" ) ), QStringList() << "no" << "matching" << "chars" );
}

void TestQgsPalLabeling::graphemes()
{
  QCOMPARE( QgsPalLabeling::splitToGraphemes( QString() ) , QStringList() );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( "abcd" ) , QStringList() << "a" << "b" << "c" << "d" );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( "ab cd" ) , QStringList() << "a" << "b" << " " << "c" << "d" );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( "ab cd " ) , QStringList() << "a" << "b" << " " << "c" << "d" << " " );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( QString::fromUtf8( "\u179F\u17D2\u178F\u17D2\u179A\u17B8\u179B\u17D2" ) ) , QStringList() << QString::fromUtf8( "\u179F\u17D2\u178F\u17D2\u179A\u17B8" ) << QString::fromUtf8( "\u179B\u17D2" ) );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( QString::fromUtf8( "\u1780\u17D2\u179A\u17BB\u1798\u17A2\u1784\u17D2\u1782\u1780\u17B6\u179A\u179F\u17B7\u1791\u17D2\u1792\u17B7\u1798\u1793\u17BB\u179F\u17D2\u179F" ) ) ,
            QStringList() << QString::fromUtf8( "\u1780\u17D2\u179A\u17BB" ) << QString::fromUtf8( "\u1798" ) << QString::fromUtf8( "\u17A2" )
            << QString::fromUtf8( "\u1784\u17D2\u1782" ) << QString::fromUtf8( "\u1780\u17B6" ) << QString::fromUtf8( "\u179A" )
            << QString::fromUtf8( "\u179F\u17B7" ) << QString::fromUtf8( "\u1791\u17D2\u1792\u17B7" ) << QString::fromUtf8( "\u1798" )
            << QString::fromUtf8( "\u1793\u17BB" ) << QString::fromUtf8( "\u179F\u17D2\u179F" ) );
}

QTEST_MAIN( TestQgsPalLabeling )
#include "testqgspallabeling.moc"
