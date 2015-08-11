/***************************************************************************
                         testqgsstringutils.cpp
                         ----------------------
    begin                : June 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstringutils.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsStringUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void levenshtein();
    void longestCommonSubstring();
    void hammingDistance();
    void soundex();

};

void TestQgsStringUtils::initTestCase()
{

}

void TestQgsStringUtils::cleanupTestCase()
{

}

void TestQgsStringUtils::init()
{
}

void TestQgsStringUtils::cleanup()
{
}

void TestQgsStringUtils::levenshtein()
{
  QCOMPARE( QgsStringUtils::levenshteinDistance( QString(), QString() ), 0 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "abc", QString() ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QString(), "abc" ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "abc", "abc" ), 0 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "abc", "aBc", true ), 1 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "abc", "xec" ), 2 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "abc", "abd" ), 1 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "abc", "ebg" ), 2 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "kitten", "sitting" ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "kItten", "sitting" ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "kitten", "sitTing", true ), 4 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( "kitten", "xkitte" ), 2 );
}

void TestQgsStringUtils::longestCommonSubstring()
{
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QString(), QString() ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "abc", QString() ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QString(), "abc" ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "abc", "def" ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "abc", "abd" ), QString( "ab" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "abc", "xbc" ), QString( "bc" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "abc", "xbd" ), QString( "b" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "longer test", "inger task" ), QString( "nger t" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( "lonGer test", "inger task", true ), QString( "er t" ) );
}

void TestQgsStringUtils::hammingDistance()
{
  QCOMPARE( QgsStringUtils::hammingDistance( QString(), QString() ), 0 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", QString() ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QString(), "abc" ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", "abcd" ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abcd", "abc" ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", "abc" ), 0 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", "aBc", true ), 1 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", "xec" ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", "abd" ), 1 );
  QCOMPARE( QgsStringUtils::hammingDistance( "abc", "ebg" ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( "kitten", "sittin" ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( "kItten", "sittin" ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( "kitten", "sitTin", true ), 3 );
  QCOMPARE( QgsStringUtils::hammingDistance( "kitten", "xkitte" ), 5 );
}

void TestQgsStringUtils::soundex()
{
  QCOMPARE( QgsStringUtils::soundex( QString() ), QString() );
  //test data from jellyfish & fuzzycomp python libraries
  QCOMPARE( QgsStringUtils::soundex( "Washington" ), QString( "W252" ) );
  QCOMPARE( QgsStringUtils::soundex( "Lee" ), QString( "L000" ) );
  QCOMPARE( QgsStringUtils::soundex( "Gutierrez" ), QString( "G362" ) );
  QCOMPARE( QgsStringUtils::soundex( "Jackson" ), QString( "J250" ) );
  QCOMPARE( QgsStringUtils::soundex( "a" ), QString( "A000" ) );
  QCOMPARE( QgsStringUtils::soundex( "herman" ), QString( "H650" ) );
  QCOMPARE( QgsStringUtils::soundex( "robert" ), QString( "R163" ) );
  QCOMPARE( QgsStringUtils::soundex( "RuperT" ), QString( "R163" ) );
  QCOMPARE( QgsStringUtils::soundex( "rubin" ), QString( "R150" ) );
  QCOMPARE( QgsStringUtils::soundex( "ashcraft" ), QString( "A261" ) );
  QCOMPARE( QgsStringUtils::soundex( "ashcroft" ), QString( "A261" ) );
}


QTEST_MAIN( TestQgsStringUtils )
#include "testqgsstringutils.moc"
