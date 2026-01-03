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
#include "qgstest.h"

#include <QObject>

class TestQgsStringUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

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
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"abc"_s, QString() ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( QString(), u"abc"_s ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"abc"_s, u"abc"_s ), 0 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"abc"_s, u"aBc"_s, true ), 1 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"abc"_s, u"xec"_s ), 2 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"abc"_s, u"abd"_s ), 1 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"abc"_s, u"ebg"_s ), 2 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"kitten"_s, u"sitting"_s ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"kItten"_s, u"sitting"_s ), 3 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"kitten"_s, u"sitTing"_s, true ), 4 );
  QCOMPARE( QgsStringUtils::levenshteinDistance( u"kitten"_s, u"xkitte"_s ), 2 );
}

void TestQgsStringUtils::longestCommonSubstring()
{
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QString(), QString() ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"abc"_s, QString() ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( QString(), u"abc"_s ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"abc"_s, u"def"_s ), QString() );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"abc"_s, u"abd"_s ), u"ab"_s );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"abc"_s, u"xbc"_s ), u"bc"_s );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"abc"_s, u"xbd"_s ), u"b"_s );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"longer test"_s, u"inger task"_s ), u"nger t"_s );
  QCOMPARE( QgsStringUtils::longestCommonSubstring( u"lonGer test"_s, u"inger task"_s, true ), u"er t"_s );
}

void TestQgsStringUtils::hammingDistance()
{
  QCOMPARE( QgsStringUtils::hammingDistance( QString(), QString() ), 0 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, QString() ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( QString(), u"abc"_s ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, u"abcd"_s ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abcd"_s, u"abc"_s ), -1 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, u"abc"_s ), 0 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, u"aBc"_s, true ), 1 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, u"xec"_s ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, u"abd"_s ), 1 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"abc"_s, u"ebg"_s ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"kitten"_s, u"sittin"_s ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"kItten"_s, u"sittin"_s ), 2 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"kitten"_s, u"sitTin"_s, true ), 3 );
  QCOMPARE( QgsStringUtils::hammingDistance( u"kitten"_s, u"xkitte"_s ), 5 );
}

void TestQgsStringUtils::soundex()
{
  QCOMPARE( QgsStringUtils::soundex( QString() ), QString() );
  //test data from jellyfish & fuzzycomp python libraries
  QCOMPARE( QgsStringUtils::soundex( u"Washington"_s ), u"W252"_s );
  QCOMPARE( QgsStringUtils::soundex( u"Lee"_s ), u"L000"_s );
  QCOMPARE( QgsStringUtils::soundex( u"Gutierrez"_s ), u"G362"_s );
  QCOMPARE( QgsStringUtils::soundex( u"Jackson"_s ), u"J250"_s );
  QCOMPARE( QgsStringUtils::soundex( u"a"_s ), u"A000"_s );
  QCOMPARE( QgsStringUtils::soundex( u"herman"_s ), u"H650"_s );
  QCOMPARE( QgsStringUtils::soundex( u"robert"_s ), u"R163"_s );
  QCOMPARE( QgsStringUtils::soundex( u"RuperT"_s ), u"R163"_s );
  QCOMPARE( QgsStringUtils::soundex( u"rubin"_s ), u"R150"_s );
  QCOMPARE( QgsStringUtils::soundex( u"ashcraft"_s ), u"A261"_s );
  QCOMPARE( QgsStringUtils::soundex( u"ashcroft"_s ), u"A261"_s );
}

void TestQgsStringUtils::insertLinks()
{
  QCOMPARE( QgsStringUtils::insertLinks( QString() ), QString() );
  QCOMPARE( QgsStringUtils::insertLinks( u"not a link!"_s ), u"not a link!"_s );
  bool found = true;
  QCOMPARE( QgsStringUtils::insertLinks( u"not a link!"_s, &found ), u"not a link!"_s );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this www.north-road.com is a link"_s, &found ), u"this <a href=\"http://www.north-road.com\">www.north-road.com</a> is a link"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this www.north-road.com.au is a link"_s, &found ), u"this <a href=\"http://www.north-road.com.au\">www.north-road.com.au</a> is a link"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this www.north-road.sucks is not a good link"_s, &found ), u"this <a href=\"http://www.north-road.sucks\">www.north-road.sucks</a> is not a good link"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this http://www.north-road.com is a link"_s, &found ), u"this <a href=\"http://www.north-road.com\">http://www.north-road.com</a> is a link"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this http://north-road.com is a link"_s, &found ), u"this <a href=\"http://north-road.com\">http://north-road.com</a> is a link"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this http://north-road.com is a link, so is http://qgis.org, OK?"_s, &found ), u"this <a href=\"http://north-road.com\">http://north-road.com</a> is a link, so is <a href=\"http://qgis.org\">http://qgis.org</a>, OK?"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"this north-road.com might not be a link"_s, &found ), u"this north-road.com might not be a link"_s );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( u"please ftp to ftp://droopbox.ru and submit stuff"_s, &found ), u"please ftp to <a href=\"ftp://droopbox.ru\">ftp://droopbox.ru</a> and submit stuff"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"please visit https://fsociety.org"_s, &found ), u"please visit <a href=\"https://fsociety.org\">https://fsociety.org</a>"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"send your credit card number to qgis@qgis.org today!"_s, &found ), u"send your credit card number to <a href=\"mailto:qgis@qgis.org\">qgis@qgis.org</a> today!"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"send your credit card number to qgis@qgis.org.nz today!"_s, &found ), u"send your credit card number to <a href=\"mailto:qgis@qgis.org.nz\">qgis@qgis.org.nz</a> today!"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"visit http://qgis.org or email qgis@qgis.org"_s, &found ), u"visit <a href=\"http://qgis.org\">http://qgis.org</a> or email <a href=\"mailto:qgis@qgis.org\">qgis@qgis.org</a>"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"is a@a an email?"_s, &found ), u"is a@a an email?"_s );
  QVERIFY( !found );
  QCOMPARE( QgsStringUtils::insertLinks( u"Load file:///this/is/path/to.file?query=1#anchor"_s, &found ), u"Load <a href=\"file:///this/is/path/to.file?query=1#anchor\">file:///this/is/path/to.file?query=1#anchor</a>"_s );
  QVERIFY( found );
  QCOMPARE( QgsStringUtils::insertLinks( u"Load https://iot.comune.fe.it/FROST-Server/v1.1/Observations('b1d12280-ac1f-11ee-94c7-cf46c7a21b9f')"_s, &found ), u"Load <a href=\"https://iot.comune.fe.it/FROST-Server/v1.1/Observations('b1d12280-ac1f-11ee-94c7-cf46c7a21b9f')\">https://iot.comune.fe.it/FROST-Server/v1.1/Observations('b1d12280-ac1f-11ee-94c7-cf46c7a21b9f')</a>"_s );
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
  QCOMPARE( QgsStringUtils::capitalize( u" abc def"_s, Qgis::Capitalization::UpperCamelCase ), u"AbcDef"_s );
  QCOMPARE( QgsStringUtils::capitalize( u"ABC DEF"_s, Qgis::Capitalization::UpperCamelCase ), u"AbcDef"_s );
  QCOMPARE( QgsStringUtils::capitalize( u"àbc def"_s, Qgis::Capitalization::UpperCamelCase ), u"ÀbcDef"_s );
  QCOMPARE( QgsStringUtils::capitalize( u"àbc dÉf"_s, Qgis::Capitalization::UpperCamelCase ), u"ÀbcDéf"_s );
}

void TestQgsStringUtils::htmlToMarkdown()
{
  QCOMPARE( QgsStringUtils::htmlToMarkdown( u"<b>Visit</b> <a href=\"http://qgis.org\">!</a>"_s ), u"**Visit** [!](http://qgis.org)"_s );
  QCOMPARE( QgsStringUtils::htmlToMarkdown( u"<b>Visit</b><br><a href='http://qgis.org'>QGIS</a>"_s ), u"**Visit**\n[QGIS](http://qgis.org)"_s );

  // convert PRE
  QCOMPARE( QgsStringUtils::htmlToMarkdown( u"<b>My code</b><pre>a = 1\nb=2\nc=3</pre>"_s ), u"**My code**\n```\na = 1\nb=2\nc=3```\n"_s );
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
  QTest::newRow( "wordwrap with delim" ) << "university of qgis" << 13 << true << u" "_s << "university of\nqgis";
  QTest::newRow( "wordwrap min" ) << "university of qgis" << 3 << false << u" "_s << "university\nof qgis";
  QTest::newRow( "wordwrap min with delim" ) << "university of qgis" << 3 << false << u" "_s << "university\nof qgis";
  QTest::newRow( "wordwrap on multi line" ) << "university of qgis\nsupports many multiline" << 5 << false << u" "_s << "university\nof qgis\nsupports\nmany multiline";
  QTest::newRow( "wordwrap on zero-space width" ) << u"test%1zero-width space"_s.arg( QChar( 8203 ) ) << 4 << false << QString() << "test\nzero-width\nspace";
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
  QVERIFY( QgsStringUtils::isUrl( u"http://example.com"_s ) );
  QVERIFY( QgsStringUtils::isUrl( u"https://example.com"_s ) );
  QVERIFY( QgsStringUtils::isUrl( u"ftp://example.com"_s ) );
  QVERIFY( QgsStringUtils::isUrl( u"file:///path/to/file"_s ) );
  QVERIFY( QgsStringUtils::isUrl( u"file://C:\\path\\to\\file"_s ) );
  QVERIFY( !QgsStringUtils::isUrl( QString() ) );
  QVERIFY( !QgsStringUtils::isUrl( u"some:random/string"_s ) );
  QVERIFY( !QgsStringUtils::isUrl( u"bla"_s ) );
}


QGSTEST_MAIN( TestQgsStringUtils )
#include "testqgsstringutils.moc"
