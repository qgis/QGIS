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
#include "qgstest.h"

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
    void insertLinks();
    void titleCase_data();
    void titleCase();
    void ampersandEncode_data();
    void ampersandEncode();
    void wordWrap_data();
    void wordWrap();

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

void TestQgsStringUtils::insertLinks()
{
  QCOMPARE( QgsStringUtils::insertLinks( QString() ), QString() );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "not a link!" ) ), QString( "not a link!" ) );
  bool found = true;
  QCOMPARE( QgsStringUtils::insertLinks( QString( "not a link!" ), &found ), QString( "not a link!" ) );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this www.north-road.com is a link" ), &found ), QString( "this <a href=\"http://www.north-road.com\">www.north-road.com</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this www.north-road.com.au is a link" ), &found ), QString( "this <a href=\"http://www.north-road.com.au\">www.north-road.com.au</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this www.north-road.sucks is not a good link" ), &found ), QString( "this <a href=\"http://www.north-road.sucks\">www.north-road.sucks</a> is not a good link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this http://www.north-road.com is a link" ), &found ), QString( "this <a href=\"http://www.north-road.com\">http://www.north-road.com</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this http://north-road.com is a link" ), &found ), QString( "this <a href=\"http://north-road.com\">http://north-road.com</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this http://north-road.com is a link, so is http://qgis.org, OK?" ), &found ), QString( "this <a href=\"http://north-road.com\">http://north-road.com</a> is a link, so is <a href=\"http://qgis.org\">http://qgis.org</a>, OK?" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "this north-road.com might not be a link" ), &found ), QString( "this north-road.com might not be a link" ) );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "please ftp to ftp://droopbox.ru and submit stuff" ), &found ), QString( "please ftp to <a href=\"ftp://droopbox.ru\">ftp://droopbox.ru</a> and submit stuff" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "please visit https://fsociety.org" ), &found ), QString( "please visit <a href=\"https://fsociety.org\">https://fsociety.org</a>" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "send your credit card number to qgis@qgis.org today!" ), &found ), QString( "send your credit card number to <a href=\"mailto:qgis@qgis.org\">qgis@qgis.org</a> today!" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "send your credit card number to qgis@qgis.org.nz today!" ), &found ), QString( "send your credit card number to <a href=\"mailto:qgis@qgis.org.nz\">qgis@qgis.org.nz</a> today!" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "visit http://qgis.org or email qgis@qgis.org" ), &found ), QString( "visit <a href=\"http://qgis.org\">http://qgis.org</a> or email <a href=\"mailto:qgis@qgis.org\">qgis@qgis.org</a>" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QString( "is a@a an email?" ), &found ), QString( "is a@a an email?" ) );
  QVERIFY( !found );
}

void TestQgsStringUtils::titleCase_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );

  // invalid strings
  QTest::newRow( "empty string" ) << "" << "";
  QTest::newRow( "single character" ) << "a" << "A";
  QTest::newRow( "string 1" ) << "follow step-by-step instructions" << "Follow Step-by-Step Instructions";
  QTest::newRow( "string 2" ) << "this sub-phrase is nice" << "This Sub-Phrase Is Nice";
  QTest::newRow( "" ) << "catchy title: a subtitle" << "Catchy Title: A Subtitle";
  QTest::newRow( "string 3" ) << "all words capitalized" << "All Words Capitalized";
  QTest::newRow( "string 4" ) << "small words are for by and of lowercase" << "Small Words Are for by and of Lowercase";
  QTest::newRow( "string 5" ) << "a small word starts" << "A Small Word Starts";
  QTest::newRow( "last word" ) << "a small word it ends on" << "A Small Word It Ends On";
  QTest::newRow( "last word2" ) << "Ends with small word of" << "Ends With Small Word Of";
  QTest::newRow( "string 6" ) << "Merge VRT(s)" << "Merge VRT(s)";
  QTest::newRow( "string 6" ) << "multiple sentences. more than one." << "Multiple Sentences. More Than One.";
  QTest::newRow( "accented" ) << "extraer vértices" << "Extraer Vértices";
}

void TestQgsStringUtils::titleCase()
{
  QFETCH( QString, input );
  QFETCH( QString, expected );
  QCOMPARE( QgsStringUtils::capitalize( input, QgsStringUtils::TitleCase ), expected );
}

void TestQgsStringUtils::ampersandEncode_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "empty string" ) << "" << "";
  QTest::newRow( "amp" ) << "a & b" << "a &amp; b";
  QTest::newRow( "gt" ) << "a > b" << "a &gt; b";
  QTest::newRow( "lt" ) << "a < b" << "a &lt; b";
  QTest::newRow( "mix" ) << "a <²> b" << "a &lt;&#178;&gt; b";
}

void TestQgsStringUtils::ampersandEncode()
{
  QFETCH( QString, input );
  QFETCH( QString, expected );
  QCOMPARE( QgsStringUtils::ampersandEncode( input ), expected );

}

void TestQgsStringUtils::wordWrap_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<int>( "length" );
  QTest::addColumn<bool>( "isMax" );
  QTest::addColumn<QString>( "delimiter" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "wordwrap" ) << "university of qgis" << 13 << true << QString() << "university of\nqgis";
  QTest::newRow( "optional parameters unspecified" ) << "test string" << 5 << true << QString() << "test\nstring";
  QTest::newRow( "wordwrap with delim" ) << "university of qgis" << 13 << true << QStringLiteral( " " ) << "university of\nqgis";
  QTest::newRow( "wordwrap min" ) << "university of qgis" << 3 << false << QStringLiteral( " " ) << "university\nof qgis";
  QTest::newRow( "wordwrap min with delim" ) << "university of qgis" << 3 << false << QStringLiteral( " " ) << "university\nof qgis";
  QTest::newRow( "wordwrap on multi line" ) << "university of qgis\nsupports many multiline" << 5 << false << QStringLiteral( " " ) << "university\nof qgis\nsupports\nmany multiline";
  QTest::newRow( "wordwrap on zero-space width" ) << QStringLiteral( "test%1zero-width space" ).arg( QChar( 8203 ) ) << 4 << false << QString() << "test\nzero-width\nspace";
  QTest::newRow( "optional parameters specified" ) << "testxstring" << 5 << true << "x" << "test\nstring";
}

void TestQgsStringUtils::wordWrap()
{
  QFETCH( QString, input );
  QFETCH( int, length );
  QFETCH( bool, isMax );
  QFETCH( QString, delimiter );
  QFETCH( QString, expected );

  QCOMPARE( QgsStringUtils::wordWrap( input, length, isMax, delimiter ), expected );
}


QGSTEST_MAIN( TestQgsStringUtils )
#include "testqgsstringutils.moc"
