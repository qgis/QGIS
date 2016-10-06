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

  //note - have to use this method to build up unicode QStrings to avoid issues with Windows
  //builds and invalid codepages
  QString str1;
  str1 += QChar( 0x179F );
  str1 += QChar( 0x17D2 );
  str1 += QChar( 0x178F );
  str1 += QChar( 0x17D2 );
  str1 += QChar( 0x179A );
  str1 += QChar( 0x17B8 );
  str1 += QChar( 0x179B );
  str1 += QChar( 0x17D2 );
  QString expected1Pt1;
  expected1Pt1 += QChar( 0x179F );
  expected1Pt1 += QChar( 0x17D2 );
  expected1Pt1 += QChar( 0x178F );
  expected1Pt1 += QChar( 0x17D2 );
  expected1Pt1 += QChar( 0x179A );
  expected1Pt1 += QChar( 0x17B8 );
  QString expected1Pt2;
  expected1Pt2 += QChar( 0x179B );
  expected1Pt2 += QChar( 0x17D2 );

  QCOMPARE( QgsPalLabeling::splitToGraphemes( str1 ), QStringList() << expected1Pt1 << expected1Pt2 );

  QString str2;
  str2 += QChar( 0x1780 );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x179A );
  str2 += QChar( 0x17BB );
  str2 += QChar( 0x1798 );
  str2 += QChar( 0x17A2 );
  str2 += QChar( 0x1784 );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x1782 );
  str2 += QChar( 0x1780 );
  str2 += QChar( 0x17B6 );
  str2 += QChar( 0x179A );
  str2 += QChar( 0x179F );
  str2 += QChar( 0x17B7 );
  str2 += QChar( 0x1791 );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x1792 );
  str2 += QChar( 0x17B7 );
  str2 += QChar( 0x1798 );
  str2 += QChar( 0x1793 );
  str2 += QChar( 0x17BB );
  str2 += QChar( 0x179F );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x179F );

  QString expected2Pt1;
  expected2Pt1 += QChar( 0x1780 );
  expected2Pt1 += QChar( 0x17D2 );
  expected2Pt1 += QChar( 0x179A );
  expected2Pt1 += QChar( 0x17BB );
  QString expected2Pt2;
  expected2Pt2 += QChar( 0x1798 );
  QString expected2Pt3;
  expected2Pt3 += QChar( 0x17A2 );
  QString expected2Pt4;
  expected2Pt4 += QChar( 0x1784 );
  expected2Pt4 += QChar( 0x17D2 );
  expected2Pt4 += QChar( 0x1782 );
  QString expected2Pt5;
  expected2Pt5 += QChar( 0x1780 );
  expected2Pt5 += QChar( 0x17B6 );
  QString expected2Pt6;
  expected2Pt6 += QChar( 0x179A );
  QString expected2Pt7;
  expected2Pt7 += QChar( 0x179F );
  expected2Pt7 += QChar( 0x17B7 );
  QString expected2Pt8;
  expected2Pt8 += QChar( 0x1791 );
  expected2Pt8 += QChar( 0x17D2 );
  expected2Pt8 += QChar( 0x1792 );
  expected2Pt8 += QChar( 0x17B7 );
  QString expected2Pt9;
  expected2Pt9 += QChar( 0x1798 );
  QString expected2Pt10;
  expected2Pt10 += QChar( 0x1793 );
  expected2Pt10 += QChar( 0x17BB );
  QString expected2Pt11;
  expected2Pt11 += QChar( 0x179F );
  expected2Pt11 += QChar( 0x17D2 );
  expected2Pt11 += QChar( 0x179F );

  QCOMPARE( QgsPalLabeling::splitToGraphemes( str2 ), QStringList() << expected2Pt1
            << expected2Pt2
            << expected2Pt3
            << expected2Pt4
            << expected2Pt5
            << expected2Pt6
            << expected2Pt7
            << expected2Pt8
            << expected2Pt9
            << expected2Pt10
            << expected2Pt11 );
}

QTEST_MAIN( TestQgsPalLabeling )
#include "testqgspallabeling.moc"
