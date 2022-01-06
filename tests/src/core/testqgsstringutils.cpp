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
    void camelCase();
    void ampersandEncode_data();
    void ampersandEncode();
    void htmlToMarkdown();
    void wordWrap_data();
    void wordWrap();
    void testIsUrl();

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
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "abc" ), QString() ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QString(),  QStringLiteral( "abc" ) ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "abc" ),  QStringLiteral( "abc" ) ), 0 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "abc" ),  QStringLiteral( "aBc" ), true ), 1 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "abc" ),  QStringLiteral( "xec" ) ), 2 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "abc" ),  QStringLiteral( "abd" ) ), 1 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "abc" ),  QStringLiteral( "ebg" ) ), 2 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "kitten" ),  QStringLiteral( "sitting" ) ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "kItten" ),  QStringLiteral( "sitting" ) ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "kitten" ),  QStringLiteral( "sitTing" ), true ), 4 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QStringLiteral( "kitten" ),  QStringLiteral( "xkitte" ) ), 2 );
}

void TestQgsStringUtils::longestCommonSubstring()
{
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QString(), QString() ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "abc" ), QString() ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QString(), QStringLiteral( "abc" ) ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "abc" ),  QStringLiteral( "def" ) ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "abc" ),  QStringLiteral( "abd" ) ), QStringLiteral( "ab" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "abc" ),  QStringLiteral( "xbc" ) ), QStringLiteral( "bc" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "abc" ),  QStringLiteral( "xbd" ) ), QStringLiteral( "b" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "longer test" ),  QStringLiteral( "inger task" ) ), QStringLiteral( "nger t" ) );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QStringLiteral( "lonGer test" ),  QStringLiteral( "inger task" ), true ), QStringLiteral( "er t" ) );
}

void TestQgsStringUtils::hammingDistance()
{
  QCOMPARE( QgsStringUtils::hammingDistance( QString(), QString() ), 0 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QString() ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QString(), QStringLiteral( "abc" ) ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QStringLiteral( "abcd" ) ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abcd" ), QStringLiteral( "abc" ) ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QStringLiteral( "abc" ) ), 0 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QStringLiteral( "aBc" ), true ), 1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QStringLiteral( "xec" ) ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QStringLiteral( "abd" ) ), 1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "abc" ), QStringLiteral( "ebg" ) ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "kitten" ), QStringLiteral( "sittin" ) ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "kItten" ), QStringLiteral( "sittin" ) ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "kitten" ), QStringLiteral( "sitTin" ), true ), 3 );
  QCOMPARE( QgsStringUtils::hammingDistance( QStringLiteral( "kitten" ), QStringLiteral( "xkitte" ) ), 5 );
}

void TestQgsStringUtils::soundex()
{
  QCOMPARE( QgsStringUtils::soundex( QString() ), QString() );
  //test data from jellyfish & fuzzycomp python libraries
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "Washington" ) ), QStringLiteral( "W252" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "Lee" ) ), QStringLiteral( "L000" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "Gutierrez" ) ), QStringLiteral( "G362" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "Jackson" ) ), QStringLiteral( "J250" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "a" ) ), QStringLiteral( "A000" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "herman" ) ), QStringLiteral( "H650" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "robert" ) ), QStringLiteral( "R163" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "RuperT" ) ), QStringLiteral( "R163" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "rubin" ) ), QStringLiteral( "R150" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "ashcraft" ) ), QStringLiteral( "A261" ) );
  QCOMPARE( QgsStringUtils::soundex( QStringLiteral( "ashcroft" ) ), QStringLiteral( "A261" ) );
}

void TestQgsStringUtils::insertLinks()
{
  QCOMPARE( QgsStringUtils::insertLinks( QString() ), QString() );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "not a link!" ) ), QStringLiteral( "not a link!" ) );
  bool found = true;
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "not a link!" ), &found ), QStringLiteral( "not a link!" ) );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this www.north-road.com is a link" ), &found ), QStringLiteral( "this <a href=\"http://www.north-road.com\">www.north-road.com</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this www.north-road.com.au is a link" ), &found ), QStringLiteral( "this <a href=\"http://www.north-road.com.au\">www.north-road.com.au</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this www.north-road.sucks is not a good link" ), &found ), QStringLiteral( "this <a href=\"http://www.north-road.sucks\">www.north-road.sucks</a> is not a good link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this http://www.north-road.com is a link" ), &found ), QStringLiteral( "this <a href=\"http://www.north-road.com\">http://www.north-road.com</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this http://north-road.com is a link" ), &found ), QStringLiteral( "this <a href=\"http://north-road.com\">http://north-road.com</a> is a link" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this http://north-road.com is a link, so is http://qgis.org, OK?" ), &found ), QStringLiteral( "this <a href=\"http://north-road.com\">http://north-road.com</a> is a link, so is <a href=\"http://qgis.org\">http://qgis.org</a>, OK?" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "this north-road.com might not be a link" ), &found ), QStringLiteral( "this north-road.com might not be a link" ) );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "please ftp to ftp://droopbox.ru and submit stuff" ), &found ), QStringLiteral( "please ftp to <a href=\"ftp://droopbox.ru\">ftp://droopbox.ru</a> and submit stuff" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "please visit https://fsociety.org" ), &found ), QStringLiteral( "please visit <a href=\"https://fsociety.org\">https://fsociety.org</a>" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "send your credit card number to qgis@qgis.org today!" ), &found ), QStringLiteral( "send your credit card number to <a href=\"mailto:qgis@qgis.org\">qgis@qgis.org</a> today!" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "send your credit card number to qgis@qgis.org.nz today!" ), &found ), QStringLiteral( "send your credit card number to <a href=\"mailto:qgis@qgis.org.nz\">qgis@qgis.org.nz</a> today!" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "visit http://qgis.org or email qgis@qgis.org" ), &found ), QStringLiteral( "visit <a href=\"http://qgis.org\">http://qgis.org</a> or email <a href=\"mailto:qgis@qgis.org\">qgis@qgis.org</a>" ) );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "is a@a an email?" ), &found ), QStringLiteral( "is a@a an email?" ) );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( QStringLiteral( "Load file:///this/is/path/to.file?query=1#anchor" ), &found ), QStringLiteral( "Load <a href=\"file:///this/is/path/to.file?query=1#anchor\">file:///this/is/path/to.file?query=1#anchor</a>" ) );
  QVERIFY( found );
}

void TestQgsStringUtils::titleCase_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );

  // invalid strings
  QTest::newRow( "empty string" ) << "" << "";
  QTest::newRow( "single character" ) << "a" << "A";
  QTest::newRow( "string 1" ) << "follow step-by-step instructions" << "Follow Step-by-Step Instructions";
  QTest::newRow( "originally uppercase" ) << "FOLLOW STEP-BY-STEP INSTRUCTIONS" << "Follow Step-by-Step Instructions";
  QTest::newRow( "string 1" ) << "Follow step-by-step instructions" << "Follow Step-by-Step Instructions";
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
  QCOMPARE( QgsStringUtils::capitalize( input, Qgis::Capitalization::TitleCase ), expected );
}

void TestQgsStringUtils::camelCase()
{
  QCOMPARE( QgsStringUtils::capitalize( QString(), Qgis::Capitalization::UpperCamelCase ), QString() );
  QCOMPARE( QgsStringUtils::capitalize( QStringLiteral( " abc def" ), Qgis::Capitalization::UpperCamelCase ), QStringLiteral( "AbcDef" ) );
  QCOMPARE( QgsStringUtils::capitalize( QStringLiteral( "ABC DEF" ), Qgis::Capitalization::UpperCamelCase ), QStringLiteral( "AbcDef" ) );
  QCOMPARE( QgsStringUtils::capitalize( QStringLiteral( "àbc def" ), Qgis::Capitalization::UpperCamelCase ), QStringLiteral( "ÀbcDef" ) );
  QCOMPARE( QgsStringUtils::capitalize( QStringLiteral( "àbc dÉf" ), Qgis::Capitalization::UpperCamelCase ), QStringLiteral( "ÀbcDéf" ) );
}

void TestQgsStringUtils::htmlToMarkdown()
{
  QCOMPARE( QgsStringUtils::htmlToMarkdown( QStringLiteral( "<b>Visit</b> <a href=\"http://qgis.org\">!</a>" ) ), QStringLiteral( "**Visit** [!](http://qgis.org)" ) );
  QCOMPARE( QgsStringUtils::htmlToMarkdown( QStringLiteral( "<b>Visit</b><br><a href='http://qgis.org'>QGIS</a>" ) ), QStringLiteral( "**Visit**\n[QGIS](http://qgis.org)" ) );

  // convert PRE
  QCOMPARE( QgsStringUtils::htmlToMarkdown( QStringLiteral( "<b>My code</b><pre>a = 1\nb=2\nc=3</pre>" ) ), QStringLiteral( "**My code**\n```\na = 1\nb=2\nc=3```\n" ) );
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
  QTest::newRow( "wordwrap not possible" ) << "universityofqgis" << 13 << true << QString() << "universityofqgis";
  QTest::newRow( "wordwrap not required" ) << "uni of qgis" << 13 << true << QString() << "uni of qgis";
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

void TestQgsStringUtils::testIsUrl()
{
  QVERIFY( QgsStringUtils::isUrl( QStringLiteral( "http://example.com" ) ) );
  QVERIFY( QgsStringUtils::isUrl( QStringLiteral( "https://example.com" ) ) );
  QVERIFY( QgsStringUtils::isUrl( QStringLiteral( "ftp://example.com" ) ) );
  QVERIFY( QgsStringUtils::isUrl( QStringLiteral( "file:///path/to/file" ) ) );
  QVERIFY( QgsStringUtils::isUrl( QStringLiteral( "file://C:\\path\\to\\file" ) ) );
  QVERIFY( !QgsStringUtils::isUrl( QLatin1String( "" ) ) );
  QVERIFY( !QgsStringUtils::isUrl( QStringLiteral( "some:random/string" ) ) );
  QVERIFY( !QgsStringUtils::isUrl( QStringLiteral( "bla" ) ) );
}


QGSTEST_MAIN( TestQgsStringUtils )
#include "testqgsstringutils.moc"
