/***************************************************************************
     testqgsstylev2.cpp
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
#include <QtTest/QtTest>
#include <QObject>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include "qgsmultirenderchecker.h"
#include <qgsapplication.h>
#include "qgsconfig.h"
#include "qgslogger.h"
#include "qgsvectorcolorrampv2.h"
#include "qgscptcityarchive.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgssinglesymbolrendererv2.h"

#include "qgsstylev2.h"

/** \ingroup UnitTests
 * This is a unit test to verify that styles are working correctly
 */
class TestStyleV2 : public QObject
{
    Q_OBJECT

  public:
    TestStyleV2();

  private:

    QString mReport;

    QgsStyleV2 *mStyle;
    QString mTestDataDir;

    bool testValidColor( QgsVectorColorRampV2 *ramp, double value, const QColor& expected );
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
    void testTags();

};

TestStyleV2::TestStyleV2()
    : mStyle( nullptr )
{

}

// slots
void TestStyleV2::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDB();
  mTestDataDir = QString( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  // output test environment
  QgsApplication::showSettings();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  // initialize with a clean style
  QFile styleFile( QgsApplication::userStyleV2Path() );
  if ( styleFile.exists() )
  {
    styleFile.remove();
    QgsDebugMsg( "removed user style file " + styleFile.fileName() );
  }
  mStyle = QgsStyleV2::defaultStyle();
  // mStyle->clear();

  // cpt-city ramp, small selection available in <testdir>/cpt-city
  QgsCptCityArchive::initArchives();

  mReport += "<h1>StyleV2 Tests</h1>\n";
}

void TestStyleV2::cleanupTestCase()
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

bool TestStyleV2::imageCheck( QgsMapSettings& ms, const QString& testName )
{
  QgsMultiRenderChecker checker;
  ms.setOutputDpi( 96 );
  checker.setControlName( "expected_" + testName );
  checker.setMapSettings( ms );
  bool result = checker.runTest( testName, 0 );
  mReport += checker.report();
  return result;
}

bool TestStyleV2::testValidColor( QgsVectorColorRampV2 *ramp, double value, const QColor& expected )
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

void TestStyleV2::testCreateColorRamps()
{
  // gradient ramp
  QgsVectorGradientColorRampV2* gradientRamp = new QgsVectorGradientColorRampV2( QColor( Qt::red ), QColor( Qt::blue ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( Qt::white ) ) );
  gradientRamp->setStops( stops );
  QVERIFY( mStyle->addColorRamp( "test_gradient", gradientRamp, true ) );

  // random ramp
  QgsVectorRandomColorRampV2* randomRamp = new QgsVectorRandomColorRampV2();
  QVERIFY( mStyle->addColorRamp( "test_random", randomRamp, true ) );

  // color brewer ramp
  QgsVectorColorBrewerColorRampV2* cb1Ramp = new QgsVectorColorBrewerColorRampV2();
  QVERIFY( mStyle->addColorRamp( "test_cb1", cb1Ramp, true ) );
  QgsVectorColorBrewerColorRampV2* cb2Ramp = new QgsVectorColorBrewerColorRampV2( "RdYlGn", 6 );
  QVERIFY( mStyle->addColorRamp( "test_cb2", cb2Ramp, true ) );

  // discrete ramp with no variant
  QgsCptCityColorRampV2* cc1Ramp = new QgsCptCityColorRampV2( "cb/seq/PuBuGn_06", "" );
  QVERIFY( mStyle->addColorRamp( "test_cc1", cc1Ramp, true ) );
  // discrete ramp with variant
  QgsCptCityColorRampV2* cc2Ramp = new QgsCptCityColorRampV2( "cb/div/PiYG", "_10" );
  QVERIFY( mStyle->addColorRamp( "test_cc2", cc2Ramp, true ) );
  // continuous ramp
  QgsCptCityColorRampV2* cc3Ramp = new QgsCptCityColorRampV2( "grass/byr", "" );
  QVERIFY( mStyle->addColorRamp( "test_cc3", cc3Ramp, true ) );
}

void TestStyleV2::testLoadColorRamps()
{
  QStringList colorRamps = mStyle->colorRampNames();
  QStringList colorRampsTest = QStringList() << "BrBG" << "Spectral"
                               << "test_gradient" << "test_random"
                               << "test_cb1" << "test_cb2";

  // values for color tests
  QMultiMap< QString, QPair< double, QColor> > colorTests;
  colorTests.insert( "test_gradient", qMakePair( 0.25, QColor( "#ff8080" ) ) );
  colorTests.insert( "test_gradient", qMakePair( 0.66, QColor( "#aeaeff" ) ) );
  // cannot test random colors!
  colorTests.insert( "test_cb1", qMakePair( 0.25, QColor( "#fdae61" ) ) );
  colorTests.insert( "test_cb1", qMakePair( 0.66, QColor( "#abdda4" ) ) );
  colorTests.insert( "test_cb2", qMakePair( 0.25, QColor( "#fc8d59" ) ) );
  colorTests.insert( "test_cb2", qMakePair( 0.66, QColor( "#d9ef8b" ) ) );

  // cpt-city
  colorRampsTest << "test_cc1";
  colorTests.insert( "test_cc1", qMakePair( 0.25, QColor( "#d0d1e6" ) ) );
  colorTests.insert( "test_cc1", qMakePair( 0.66, QColor( "#67a9cf" ) ) );
  colorRampsTest << "test_cc2";
  colorTests.insert( "test_cc2", qMakePair( 0.25, QColor( "#de77ae" ) ) );
  colorTests.insert( "test_cc2", qMakePair( 0.66, QColor( "#b8e186" ) ) );
  colorRampsTest << "test_cc3";
  colorTests.insert( "test_cc3", qMakePair( 0.25, QColor( "#808080" ) ) );
  colorTests.insert( "test_cc3", qMakePair( 0.66, QColor( "#ffae00" ) ) );

  QgsDebugMsg( "loaded colorRamps: " + colorRamps.join( " " ) );

  Q_FOREACH ( const QString& name, colorRampsTest )
  {
    QgsDebugMsg( "colorRamp " + name );
    QVERIFY( colorRamps.contains( name ) );
    QgsVectorColorRampV2* ramp = mStyle->colorRamp( name );
    QVERIFY( ramp != 0 );
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

void TestStyleV2::testSaveLoad()
{
  // save not needed anymore, because used update=true in addColorRamp()
  // mStyle->save();
  mStyle->clear();
  mStyle->load( QgsApplication::userStyleV2Path() );

  // basic test to see that ramp is present
  QStringList colorRamps = mStyle->colorRampNames();
  QgsDebugMsg( "loaded colorRamps: " + colorRamps.join( " " ) );

  QStringList colorRampsTest = QStringList() << "test_gradient";

  Q_FOREACH ( const QString& name, colorRampsTest )
  {
    QgsDebugMsg( "colorRamp " + name );
    QVERIFY( colorRamps.contains( name ) );
    QgsVectorColorRampV2* ramp = mStyle->colorRamp( name );
    QVERIFY( ramp != 0 );
    if ( ramp )
      delete ramp;
  }
  // test content again
  testLoadColorRamps();
}

void TestStyleV2::testTags()
{
  mStyle->clear();
  //add some tags
  int id = mStyle->addTag( "red" );
  QCOMPARE( id, mStyle->tagId( "red" ) );
  id = mStyle->addTag( "starry" );
  QCOMPARE( id, mStyle->tagId( "starry" ) );
  id = mStyle->addTag( "circle" );
  QCOMPARE( id, mStyle->tagId( "circle" ) );
  id = mStyle->addTag( "blue" );
  QCOMPARE( id, mStyle->tagId( "blue" ) );
  id = mStyle->addTag( "purple" );
  QCOMPARE( id, mStyle->tagId( "purple" ) );

  QStringList tags = mStyle->tags();
  QCOMPARE( tags.count(), 5 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "starry" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "purple" ) );

  //remove tag
  mStyle->remove( QgsStyleV2::TagEntity, mStyle->tagId( "purple" ) );
  mStyle->remove( QgsStyleV2::TagEntity, -999 ); //bad id
  tags = mStyle->tags();
  QCOMPARE( tags.count(), 4 );
  QVERIFY( !tags.contains( "purple" ) );

  //add some symbols
  QVERIFY( mStyle->saveSymbol( "symbol1", QgsMarkerSymbolV2::createSimple( QgsStringMap() ), 0, QStringList() << "red" << "starry" ) );
  mStyle->addSymbol( "blue starry", QgsMarkerSymbolV2::createSimple( QgsStringMap() ), true );
  mStyle->addSymbol( "red circle", QgsMarkerSymbolV2::createSimple( QgsStringMap() ), true );

  //tag them
  QVERIFY( mStyle->tagSymbol( QgsStyleV2::SymbolEntity, "blue starry", QStringList() << "blue" << "starry" ) );
  QVERIFY( mStyle->tagSymbol( QgsStyleV2::SymbolEntity, "red circle", QStringList() << "red" << "circle" ) );
  //bad symbol name
  QVERIFY( !mStyle->tagSymbol( QgsStyleV2::SymbolEntity, "no symbol", QStringList() << "red" << "circle" ) );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyleV2::SymbolEntity, "red circle", QStringList() << "round" ) );
  tags = mStyle->tags();
  QVERIFY( tags.contains( "round" ) );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyleV2::SymbolEntity, "blue starry" );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyleV2::SymbolEntity, "red circle" );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "round" ) );
  tags = mStyle->tagsOfSymbol( QgsStyleV2::SymbolEntity, "symbol1" );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "starry" ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyleV2::SymbolEntity, "blue starry", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyleV2::SymbolEntity, "blue starry" );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );

  //try to remove tag from non-existing symbol
  QVERIFY( !mStyle->detagSymbol( QgsStyleV2::SymbolEntity, "no symbol!", QStringList() << "bad" << "blue" ) );

  //check symbols with tag
  QStringList symbols = mStyle->symbolsWithTag( QgsStyleV2::SymbolEntity, mStyle->tagId( "red" ) );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->symbolsWithTag( QgsStyleV2::SymbolEntity, mStyle->tagId( "starry" ) );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "blue starry" ) );
  symbols = mStyle->symbolsWithTag( QgsStyleV2::SymbolEntity, mStyle->tagId( "circle" ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->symbolsWithTag( QgsStyleV2::SymbolEntity, mStyle->tagId( "round" ) );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->symbolsWithTag( QgsStyleV2::SymbolEntity, mStyle->tagId( "blue" ) );
  QVERIFY( symbols.isEmpty() );
  symbols = mStyle->symbolsWithTag( QgsStyleV2::SymbolEntity, mStyle->tagId( "no tag" ) );
  QVERIFY( symbols.isEmpty() );

  //searching returns symbols with matching tags
  symbols = mStyle->findSymbols( QgsStyleV2::SymbolEntity, "red" );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "red circle" ) );
  symbols = mStyle->findSymbols( QgsStyleV2::SymbolEntity, "symbol1" );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "symbol1" ) );
  symbols = mStyle->findSymbols( QgsStyleV2::SymbolEntity, "starry" );
  QCOMPARE( symbols.count(), 2 );
  QVERIFY( symbols.contains( "symbol1" ) );
  QVERIFY( symbols.contains( "blue starry" ) );
  symbols = mStyle->findSymbols( QgsStyleV2::SymbolEntity, "blue" );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "blue starry" ) );
  symbols = mStyle->findSymbols( QgsStyleV2::SymbolEntity, "round" );
  QCOMPARE( symbols.count(), 1 );
  QVERIFY( symbols.contains( "red circle" ) );
}

QTEST_MAIN( TestStyleV2 )
#include "testqgsstylev2.moc"
