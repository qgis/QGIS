/***************************************************************************
     testqgsstyle.cpp
     --------------------------------------
    Date                 : Wed Aug  1 12:13:24 BRT 2012
    Copyright            : (C) 2012 Etienne Tourigny and Tim Sutton
    Email                : etourigny dot dev at gmail.com
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
#include "qgsmultirenderchecker.h"
#include <qgsapplication.h>
#include "qgsconfig.h"
#include "qgslogger.h"
#include "qgscolorramp.h"
#include "qgscptcityarchive.h"
#include "qgsvectorlayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgssinglesymbolrenderer.h"

#include "qgsstyle.h"

/**
 * \ingroup UnitTests
 * This is a unit test to verify that styles are working correctly
 */
class TestStyle : public QObject
{
    Q_OBJECT

  public:
    TestStyle();

  private:

    QString mReport;

    QgsStyle *mStyle = nullptr;
    QString mTestDataDir;

    bool testValidColor( QgsColorRamp *ramp, double value, const QColor &expected );
    bool imageCheck( QgsMapSettings &ms, const QString &testName );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.
    // void initStyles();

    void testCreateColorRamps();
    void testLoadColorRamps();
    void testSaveLoad();
    void testFavorites();
    void testTags();

};

TestStyle::TestStyle() = default;

// slots
void TestStyle::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDatabase();
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  // output test environment
  QgsApplication::showSettings();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  //initize a temporary memory-based style for tests to avoid clashing with shipped symbols
  mStyle = new QgsStyle();
  mStyle->createMemoryDatabase();

  // cpt-city ramp, small selection available in <testdir>/cpt-city
  QgsCptCityArchive::initArchives();

  mReport += QLatin1String( "<h1>Style Tests</h1>\n" );
}

void TestStyle::cleanupTestCase()
{
  // don't save
  // mStyle->save();
  delete mStyle;

  QgsCptCityArchive::clearArchives();
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
}

bool TestStyle::imageCheck( QgsMapSettings &ms, const QString &testName )
{
  QgsMultiRenderChecker checker;
  ms.setOutputDpi( 96 );
  checker.setControlName( "expected_" + testName );
  checker.setMapSettings( ms );
  bool result = checker.runTest( testName, 0 );
  mReport += checker.report();
  return result;
}

bool TestStyle::testValidColor( QgsColorRamp *ramp, double value, const QColor &expected )
{
  QColor result = ramp->color( value );
  //use int color components when testing (builds some fuzziness into test)
  if ( result.red() != expected.red() || result.green() != expected.green() || result.blue() != expected.blue()
       || result.alpha() != expected.alpha() )
  {
    QWARN( QString( "value = %1 result = %2 expected = %3" ).arg( value ).arg(
             result.name(), expected.name() ).toLocal8Bit().data() );
    return false;
  }
  return true;
}

void TestStyle::testCreateColorRamps()
{
  // gradient ramp
  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( Qt::white ) ) );
  gradientRamp->setStops( stops );
  QVERIFY( mStyle->addColorRamp( "test_gradient", gradientRamp, true ) );

  // random ramp
  QgsLimitedRandomColorRamp *randomRamp = new QgsLimitedRandomColorRamp();
  QVERIFY( mStyle->addColorRamp( "test_random", randomRamp, true ) );

  // color brewer ramp
  QgsColorBrewerColorRamp *cb1Ramp = new QgsColorBrewerColorRamp();
  QVERIFY( mStyle->addColorRamp( "test_cb1", cb1Ramp, true ) );
  QgsColorBrewerColorRamp *cb2Ramp = new QgsColorBrewerColorRamp( QStringLiteral( "RdYlGn" ), 6 );
  QVERIFY( mStyle->addColorRamp( "test_cb2", cb2Ramp, true ) );

  // discrete ramp with no variant
  QgsCptCityColorRamp *cc1Ramp = new QgsCptCityColorRamp( QStringLiteral( "cb/seq/PuBuGn_06" ), QString() );
  QVERIFY( mStyle->addColorRamp( "test_cc1", cc1Ramp, true ) );
  // discrete ramp with variant
  QgsCptCityColorRamp *cc2Ramp = new QgsCptCityColorRamp( QStringLiteral( "cb/div/PiYG" ), QStringLiteral( "_10" ) );
  QVERIFY( mStyle->addColorRamp( "test_cc2", cc2Ramp, true ) );
  // continuous ramp
  QgsCptCityColorRamp *cc3Ramp = new QgsCptCityColorRamp( QStringLiteral( "grass/byr" ), QString() );
  QVERIFY( mStyle->addColorRamp( "test_cc3", cc3Ramp, true ) );
}

void TestStyle::testLoadColorRamps()
{
  QStringList colorRamps = mStyle->colorRampNames();
  QStringList colorRampsTest = QStringList() << QStringLiteral( "test_gradient" ) << QStringLiteral( "test_random" )
                               << QStringLiteral( "test_cb1" ) << QStringLiteral( "test_cb2" );

  // values for color tests
  QMultiMap< QString, QPair< double, QColor> > colorTests;
  colorTests.insert( QStringLiteral( "test_gradient" ), qMakePair( 0.25, QColor( "#ff8080" ) ) );
  colorTests.insert( QStringLiteral( "test_gradient" ), qMakePair( 0.66, QColor( "#aeaeff" ) ) );
  // cannot test random colors!
  colorTests.insert( QStringLiteral( "test_cb1" ), qMakePair( 0.25, QColor( "#fdae61" ) ) );
  colorTests.insert( QStringLiteral( "test_cb1" ), qMakePair( 0.66, QColor( "#abdda4" ) ) );
  colorTests.insert( QStringLiteral( "test_cb2" ), qMakePair( 0.25, QColor( "#fc8d59" ) ) );
  colorTests.insert( QStringLiteral( "test_cb2" ), qMakePair( 0.66, QColor( "#d9ef8b" ) ) );

  // cpt-city
  colorRampsTest << QStringLiteral( "test_cc1" );
  colorTests.insert( QStringLiteral( "test_cc1" ), qMakePair( 0.25, QColor( "#d0d1e6" ) ) );
  colorTests.insert( QStringLiteral( "test_cc1" ), qMakePair( 0.66, QColor( "#67a9cf" ) ) );
  colorRampsTest << QStringLiteral( "test_cc2" );
  colorTests.insert( QStringLiteral( "test_cc2" ), qMakePair( 0.25, QColor( "#de77ae" ) ) );
  colorTests.insert( QStringLiteral( "test_cc2" ), qMakePair( 0.66, QColor( "#b8e186" ) ) );
  colorRampsTest << QStringLiteral( "test_cc3" );
  colorTests.insert( QStringLiteral( "test_cc3" ), qMakePair( 0.25, QColor( "#808080" ) ) );
  colorTests.insert( QStringLiteral( "test_cc3" ), qMakePair( 0.66, QColor( "#ffae00" ) ) );

  QgsDebugMsg( "loaded colorRamps: " + colorRamps.join( " " ) );

  Q_FOREACH ( const QString &name, colorRampsTest )
  {
    QgsDebugMsg( "colorRamp " + name );
    QVERIFY( colorRamps.contains( name ) );
    QgsColorRamp *ramp = mStyle->colorRamp( name );
    QVERIFY( ramp != nullptr );
    // test colors
    if ( colorTests.contains( name ) )
    {
      QList< QPair< double, QColor> > values = colorTests.values( name );
      for ( int i = 0; i < values.size(); ++i )
      {
        QVERIFY( testValidColor( ramp, values.at( i ).first, values.at( i ).second ) );
      }
    }
    if ( ramp )
      delete ramp;
  }
}

void TestStyle::testSaveLoad()
{
  // basic test to see that ramp is present
  QStringList colorRamps = mStyle->colorRampNames();
  QgsDebugMsg( "loaded colorRamps: " + colorRamps.join( " " ) );

  QStringList colorRampsTest = QStringList() << QStringLiteral( "test_gradient" );

  Q_FOREACH ( const QString &name, colorRampsTest )
  {
    QgsDebugMsg( "colorRamp " + name );
    QVERIFY( colorRamps.contains( name ) );
    QgsColorRamp *ramp = mStyle->colorRamp( name );
    QVERIFY( ramp != nullptr );
    if ( ramp )
      delete ramp;
  }
  // test content again
  testLoadColorRamps();
}

void TestStyle::testFavorites()
{
  // save initial number of favorites to compare against additions / subtractions
  QStringList favorites;
  favorites = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
  int count = favorites.count();

  // add some symbols to favorites
  std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  mStyle->saveSymbol( QStringLiteral( "symbolA" ), sym1.get(), true, QStringList() );
  mStyle->saveSymbol( QStringLiteral( "symbolB" ), sym2.get(), false, QStringList() );
  mStyle->saveSymbol( QStringLiteral( "symbolC" ), sym3.get(), true, QStringList() );

  // check for added symbols to favorites
  favorites = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
  QCOMPARE( favorites.count(), count + 2 );
  QVERIFY( favorites.contains( "symbolA" ) );
  QVERIFY( favorites.contains( "symbolC" ) );

  QSignalSpy favoriteChangedSpy( mStyle, &QgsStyle::favoritedChanged );

  // remove one symbol from favorites
  mStyle->removeFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolA" ) );
  QCOMPARE( favoriteChangedSpy.count(), 1 );
  QCOMPARE( favoriteChangedSpy.at( 0 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 0 ).at( 1 ).toString(), QStringLiteral( "symbolA" ) );
  QCOMPARE( favoriteChangedSpy.at( 0 ).at( 2 ).toBool(), false );

  // insure favorites updated after removal
  favorites = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
  QCOMPARE( favorites.count(), count + 1 );
  QVERIFY( favorites.contains( "symbolC" ) );

  mStyle->addFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolA" ) );
  QCOMPARE( favoriteChangedSpy.count(), 2 );
  QCOMPARE( favoriteChangedSpy.at( 1 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 1 ).at( 1 ).toString(), QStringLiteral( "symbolA" ) );
  QCOMPARE( favoriteChangedSpy.at( 1 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
  QCOMPARE( favorites.count(), count + 2 );
  QVERIFY( favorites.contains( "symbolA" ) );

  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QVERIFY( mStyle->addColorRamp( "gradient_1", gradientRamp, true ) );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  QCOMPARE( favorites.count(), 0 );

  mStyle->addFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 3 );
  QCOMPARE( favoriteChangedSpy.at( 2 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::ColorrampEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 2 ).at( 1 ).toString(), QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 2 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  QCOMPARE( favorites.count(), 1 );
  QVERIFY( favorites.contains( "gradient_1" ) );

  mStyle->removeFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 4 );
  QCOMPARE( favoriteChangedSpy.at( 3 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::ColorrampEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 3 ).at( 1 ).toString(), QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 3 ).at( 2 ).toBool(), false );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  QCOMPARE( favorites.count(), 0 );
}

void TestStyle::testTags()
{
  //add some tags
  int id = mStyle->addTag( QStringLiteral( "red" ) );
  QCOMPARE( id, mStyle->tagId( "red" ) );

  id = mStyle->addTag( QStringLiteral( "starry" ) );
  QCOMPARE( id, mStyle->tagId( "starry" ) );
  id = mStyle->addTag( QStringLiteral( "circle" ) );
  QCOMPARE( id, mStyle->tagId( "circle" ) );
  id = mStyle->addTag( QStringLiteral( "blue" ) );
  QCOMPARE( id, mStyle->tagId( "blue" ) );
  id = mStyle->addTag( QStringLiteral( "purple" ) );

  //check tagid and tag return values
  QCOMPARE( id, mStyle->tagId( "purple" ) );
  QCOMPARE( QStringLiteral( "purple" ), mStyle->tag( id ) );

  // Cyrillic
  id = mStyle->addTag( QStringLiteral( "МЕТЕОР" ) );
  QCOMPARE( id, mStyle->tagId( "МЕТЕОР" ) );

  QStringList tags = mStyle->tags();
  QCOMPARE( tags.count(), 6 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "starry" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "purple" ) );
  QVERIFY( tags.contains( "МЕТЕОР" ) );

  //remove tag
  mStyle->remove( QgsStyle::TagEntity, mStyle->tagId( QStringLiteral( "purple" ) ) );
  mStyle->remove( QgsStyle::TagEntity, -999 ); //bad id
  tags = mStyle->tags();
  QCOMPARE( tags.count(), 5 );
  QVERIFY( !tags.contains( "purple" ) );

  //add some symbols
  std::unique_ptr< QgsMarkerSymbol> sym1( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol> sym2( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol> sym3( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol> sym4( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  QVERIFY( mStyle->saveSymbol( "symbol1", sym1.get(), false, QStringList() << "red" << "starry" ) );
  mStyle->addSymbol( QStringLiteral( "blue starry" ), sym2.release(), true );
  mStyle->addSymbol( QStringLiteral( "red circle" ), sym3.release(), true );
  mStyle->addSymbol( QStringLiteral( "МЕТЕОР" ), sym4.release(), true );

  QSignalSpy tagsChangedSpy( mStyle, &QgsStyle::entityTagsChanged );

  //tag them
  QVERIFY( mStyle->tagSymbol( QgsStyle::SymbolEntity, "blue starry", QStringList() << "blue" << "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 1 );
  QCOMPARE( tagsChangedSpy.at( 0 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( tagsChangedSpy.at( 0 ).at( 1 ).toString(), QStringLiteral( "blue starry" ) );
  QCOMPARE( tagsChangedSpy.at( 0 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "blue" ) << QStringLiteral( "starry" ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::SymbolEntity, "red circle", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 2 );
  QCOMPARE( tagsChangedSpy.at( 1 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( tagsChangedSpy.at( 1 ).at( 1 ).toString(), QStringLiteral( "red circle" ) );
  QCOMPARE( tagsChangedSpy.at( 1 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) );

  //bad symbol name
  QVERIFY( !mStyle->tagSymbol( QgsStyle::SymbolEntity, "no symbol", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 2 );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyle::SymbolEntity, "red circle", QStringList() << "round" ) );
  QCOMPARE( tagsChangedSpy.count(), 3 );
  QCOMPARE( tagsChangedSpy.at( 2 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( tagsChangedSpy.at( 2 ).at( 1 ).toString(), QStringLiteral( "red circle" ) );
  QCOMPARE( tagsChangedSpy.at( 2 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) << QStringLiteral( "round" ) );

  tags = mStyle->tags();
  QVERIFY( tags.contains( "round" ) );

  // Cyrillic
  // Add twice (see issue #18281)
  QVERIFY( mStyle->tagSymbol( QgsStyle::SymbolEntity, "МЕТЕОР", QStringList() << "МЕТЕОР" ) );
  tags = mStyle->tags();
  QVERIFY( tags.contains( "МЕТЕОР" ) );
  QCOMPARE( tags.filter( "МЕТЕОР" ).count(), 1 );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "blue starry" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "red circle" ) );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "round" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "symbol1" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "starry" ) );

  tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "МЕТЕОР" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "МЕТЕОР" ) );

  //check that a given tag is attached to a symbol
  QVERIFY( mStyle->symbolHasTag( QgsStyle::SymbolEntity, QStringLiteral( "blue starry" ), QStringLiteral( "blue" ) ) );
  QVERIFY( mStyle->symbolHasTag( QgsStyle::SymbolEntity, QStringLiteral( "МЕТЕОР" ), QStringLiteral( "МЕТЕОР" ) ) );

  //check that a given tag is not attached to a symbol
  QCOMPARE( false, mStyle->symbolHasTag( QgsStyle::SymbolEntity, QStringLiteral( "blue starry" ), QStringLiteral( "notblue" ) ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyle::SymbolEntity, "blue starry", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "blue starry" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 5 );
  QCOMPARE( tagsChangedSpy.at( 4 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( tagsChangedSpy.at( 4 ).at( 1 ).toString(), QStringLiteral( "blue starry" ) );
  QCOMPARE( tagsChangedSpy.at( 4 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "starry" ) );

  // completely detag symbol
  QVERIFY( mStyle->detagSymbol( QgsStyle::SymbolEntity, QStringLiteral( "blue starry" ) ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, QStringLiteral( "blue starry" ) );
  QCOMPARE( tags.count(), 0 );
  QCOMPARE( tagsChangedSpy.count(), 6 );
  QCOMPARE( tagsChangedSpy.at( 5 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( tagsChangedSpy.at( 5 ).at( 1 ).toString(), QStringLiteral( "blue starry" ) );
  QCOMPARE( tagsChangedSpy.at( 5 ).at( 2 ).toStringList(), QStringList() );

  //try to remove tag from non-existing symbol
  QVERIFY( !mStyle->detagSymbol( QgsStyle::SymbolEntity, "no symbol!", QStringList() << "bad" << "blue" ) );
  QCOMPARE( tagsChangedSpy.count(), 6 );

  mStyle->tagSymbol( QgsStyle::SymbolEntity, "blue starry", QStringList() << QStringLiteral( "starry" ) );

  //check symbols with tag
  QStringList symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "red" ) ) );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "starry" ) ) );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "blue starry" ) );
  symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "circle" ) ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "round" ) ) );
  QCOMPARE( symbols.count(), 1 );
  symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "МЕТЕОР" ) ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "МЕТЕОР" ) );
  symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "blue" ) ) );
  QVERIFY( symbols.isEmpty() );
  symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, mStyle->tagId( QStringLiteral( "no tag" ) ) );
  QVERIFY( symbols.isEmpty() );

  //searching returns symbols with matching tags
  symbols = mStyle->findSymbols( QgsStyle::SymbolEntity, QStringLiteral( "red" ) );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->findSymbols( QgsStyle::SymbolEntity, QStringLiteral( "symbol1" ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "symbol1" ) );
  symbols = mStyle->findSymbols( QgsStyle::SymbolEntity, QStringLiteral( "starry" ) );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "blue starry" ) );
  symbols = mStyle->findSymbols( QgsStyle::SymbolEntity, QStringLiteral( "blue" ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "blue starry" ) );
  symbols = mStyle->findSymbols( QgsStyle::SymbolEntity, QStringLiteral( "round" ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->findSymbols( QgsStyle::SymbolEntity, QStringLiteral( "МЕТЕОР" ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "МЕТЕОР" ) );

  // tag ramp
  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QVERIFY( mStyle->addColorRamp( "gradient_tag1", gradientRamp, true ) );
  QgsGradientColorRamp *gradientRamp2 = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QVERIFY( mStyle->addColorRamp( "gradient_tag2", gradientRamp2, true ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::ColorrampEntity, "gradient_tag1", QStringList() << "blue" << "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 10 );
  QCOMPARE( tagsChangedSpy.at( 9 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::ColorrampEntity ) );
  QCOMPARE( tagsChangedSpy.at( 9 ).at( 1 ).toString(), QStringLiteral( "gradient_tag1" ) );
  QCOMPARE( tagsChangedSpy.at( 9 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "blue" ) << QStringLiteral( "starry" ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::ColorrampEntity, "gradient_tag2", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 11 );
  QCOMPARE( tagsChangedSpy.at( 10 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::ColorrampEntity ) );
  QCOMPARE( tagsChangedSpy.at( 10 ).at( 1 ).toString(), QStringLiteral( "gradient_tag2" ) );
  QCOMPARE( tagsChangedSpy.at( 10 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) );

  //bad ramp name
  QVERIFY( !mStyle->tagSymbol( QgsStyle::ColorrampEntity, "no ramp", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 11 );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyle::ColorrampEntity, "gradient_tag2", QStringList() << "round ramp" ) );
  QCOMPARE( tagsChangedSpy.count(), 12 );
  QCOMPARE( tagsChangedSpy.at( 11 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::ColorrampEntity ) );
  QCOMPARE( tagsChangedSpy.at( 11 ).at( 1 ).toString(), QStringLiteral( "gradient_tag2" ) );
  QCOMPARE( tagsChangedSpy.at( 11 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) << QStringLiteral( "round ramp" ) );

  tags = mStyle->tags();
  QVERIFY( tags.contains( QStringLiteral( "round ramp" ) ) );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_tag1" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_tag2" ) );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "round ramp" ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyle::ColorrampEntity, "gradient_tag1", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_tag1" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 13 );
  QCOMPARE( tagsChangedSpy.at( 12 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::ColorrampEntity ) );
  QCOMPARE( tagsChangedSpy.at( 12 ).at( 1 ).toString(), QStringLiteral( "gradient_tag1" ) );
  QCOMPARE( tagsChangedSpy.at( 12 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "starry" ) );

  // completely detag symbol
  QVERIFY( mStyle->detagSymbol( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_tag1" ) ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_tag1" ) );
  QCOMPARE( tags.count(), 0 );
  QCOMPARE( tagsChangedSpy.count(), 14 );
  QCOMPARE( tagsChangedSpy.at( 13 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::ColorrampEntity ) );
  QCOMPARE( tagsChangedSpy.at( 13 ).at( 1 ).toString(), QStringLiteral( "gradient_tag1" ) );
  QCOMPARE( tagsChangedSpy.at( 13 ).at( 2 ).toStringList(), QStringList() );
}

QGSTEST_MAIN( TestStyle )
#include "testqgsstyle.moc"
