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
#include "qgsmarkersymbollayer.h"
#include "qgsrulebasedrenderer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsstyle.h"
#include "qgsproject.h"
#include "qgsstyleentityvisitor.h"
#include "qgsrasterlayer.h"
#include "qgsrastershader.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsprintlayout.h"
#include "qgslayoutitemscalebar.h"
#include "qgsfontutils.h"
#include "qgslayoutmanager.h"
#include "qgsannotationmanager.h"
#include "qgstextannotation.h"

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

    void testCreateSymbols();
    void testCreateColorRamps();
    void testCreateTextFormats();
    void testCreateLabelSettings();
    void testLoadColorRamps();
    void testSaveLoad();
    void testFavorites();
    void testTags();
    void testSmartGroup();
    void testIsStyleXml();
    void testVisitor();

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

void TestStyle::testCreateSymbols()
{
  // add some symbols to favorites
  QgsStyle s;
  s.createMemoryDatabase();

  std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym4( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  s.addSymbol( QStringLiteral( "symbolA" ), sym1.release(), true );
  s.addSymbol( QStringLiteral( "symbolB" ), sym2.release(), true );
  s.addSymbol( QStringLiteral( "symbolC" ), sym3.release(), true );
  QgsStyleSymbolEntity symbolEntity( sym4.get() );
  s.addEntity( QStringLiteral( "symbolD" ),  &symbolEntity, true );

  QCOMPARE( s.allNames( QgsStyle::SymbolEntity ),
            QStringList() << QStringLiteral( "symbolA" )
            << QStringLiteral( "symbolB" )
            << QStringLiteral( "symbolC" )
            << QStringLiteral( "symbolD" ) );
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

  QCOMPARE( mStyle->allNames( QgsStyle::ColorrampEntity ), QStringList() << QStringLiteral( "test_cb1" )
            << QStringLiteral( "test_cb2" )
            << QStringLiteral( "test_cc1" )
            << QStringLiteral( "test_cc2" )
            << QStringLiteral( "test_cc3" )
            << QStringLiteral( "test_gradient" )
            << QStringLiteral( "test_random" ) );

  std::unique_ptr< QgsCptCityColorRamp > cc4Ramp = qgis::make_unique< QgsCptCityColorRamp >( QStringLiteral( "grass/byr" ), QString() );
  QgsStyleColorRampEntity entity( cc4Ramp.get() );
  QVERIFY( mStyle->addEntity( "test_cc4", &entity, true ) );

  QCOMPARE( mStyle->allNames( QgsStyle::ColorrampEntity ), QStringList() << QStringLiteral( "test_cb1" )
            << QStringLiteral( "test_cb2" )
            << QStringLiteral( "test_cc1" )
            << QStringLiteral( "test_cc2" )
            << QStringLiteral( "test_cc3" )
            << QStringLiteral( "test_cc4" )
            << QStringLiteral( "test_gradient" )
            << QStringLiteral( "test_random" ) );
}

void TestStyle::testCreateTextFormats()
{
  QVERIFY( mStyle->textFormatNames().isEmpty() );
  QCOMPARE( mStyle->textFormatCount(), 0 );
  // non existent format, should be default
  QCOMPARE( mStyle->textFormat( QString( "blah" ) ).color().name(), QStringLiteral( "#000000" ) );

  QSignalSpy spy( mStyle, &QgsStyle::textFormatAdded );
  QSignalSpy spyChanged( mStyle, &QgsStyle::textFormatChanged );
  // add a format
  QgsTextFormat format;
  format.setColor( QColor( 255, 0, 0 ) );
  QVERIFY( mStyle->addTextFormat( "test_format", format, true ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 0 );

  QVERIFY( mStyle->textFormatNames().contains( QStringLiteral( "test_format" ) ) );
  QCOMPARE( mStyle->textFormatCount(), 1 );
  QCOMPARE( mStyle->textFormat( QString( "test_format" ) ).color().name(), QStringLiteral( "#ff0000" ) );

  format.setColor( QColor( 255, 255, 0 ) );
  QVERIFY( mStyle->addTextFormat( "test_format", format, true ) );
  QVERIFY( mStyle->textFormatNames().contains( QStringLiteral( "test_format" ) ) );
  QCOMPARE( mStyle->textFormatCount(), 1 );
  QCOMPARE( mStyle->textFormat( QString( "test_format" ) ).color().name(), QStringLiteral( "#ffff00" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 1 );

  format.setColor( QColor( 255, 255, 255 ) );
  QVERIFY( mStyle->addTextFormat( "test_format2", format, true ) );
  QVERIFY( mStyle->textFormatNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( mStyle->textFormatCount(), 2 );
  QCOMPARE( mStyle->textFormat( QString( "test_format" ) ).color().name(), QStringLiteral( "#ffff00" ) );
  QCOMPARE( mStyle->textFormat( QString( "test_format2" ) ).color().name(), QStringLiteral( "#ffffff" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spyChanged.count(), 1 );

  // save and restore
  QVERIFY( mStyle->exportXml( QDir::tempPath() + "/text_style.xml" ) );

  QgsStyle style2;
  QVERIFY( style2.importXml( QDir::tempPath() + "/text_style.xml" ) );

  QVERIFY( style2.textFormatNames().contains( QStringLiteral( "test_format" ) ) );
  QVERIFY( style2.textFormatNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( style2.textFormatCount(), 2 );
  QCOMPARE( style2.textFormat( QString( "test_format" ) ).color().name(), QStringLiteral( "#ffff00" ) );
  QCOMPARE( style2.textFormat( QString( "test_format2" ) ).color().name(), QStringLiteral( "#ffffff" ) );

  QCOMPARE( mStyle->allNames( QgsStyle::TextFormatEntity ), QStringList() << QStringLiteral( "test_format" )
            << QStringLiteral( "test_format2" ) );


  format.setColor( QColor( 255, 255, 205 ) );
  QgsStyleTextFormatEntity entity( format );
  QVERIFY( mStyle->addEntity( "test_format4", &entity, true ) );
  QVERIFY( mStyle->textFormatNames().contains( QStringLiteral( "test_format4" ) ) );
}

void TestStyle::testCreateLabelSettings()
{
  QVERIFY( mStyle->labelSettingsNames().isEmpty() );
  QCOMPARE( mStyle->labelSettingsCount(), 0 );
  // non existent settings, should be default
  QVERIFY( mStyle->labelSettings( QString( "blah" ) ).fieldName.isEmpty() );

  QSignalSpy spy( mStyle, &QgsStyle::labelSettingsAdded );
  QSignalSpy spyChanged( mStyle, &QgsStyle::labelSettingsChanged );
  // add settings
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "a_field_of_dreams" );
  QVERIFY( mStyle->addLabelSettings( "test_settings", settings, true ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 0 );

  QVERIFY( mStyle->labelSettingsNames().contains( QStringLiteral( "test_settings" ) ) );
  QCOMPARE( mStyle->labelSettingsCount(), 1 );
  QCOMPARE( mStyle->labelSettings( QString( "test_settings" ) ).fieldName, QStringLiteral( "a_field_of_dreams" ) );

  settings.fieldName = QStringLiteral( "actually_no_its_a_nightmare" );
  QVERIFY( mStyle->addLabelSettings( "test_settings", settings, true ) );
  QVERIFY( mStyle->labelSettingsNames().contains( QStringLiteral( "test_settings" ) ) );
  QCOMPARE( mStyle->labelSettingsCount(), 1 );
  QCOMPARE( mStyle->labelSettings( QString( "test_settings" ) ).fieldName, QStringLiteral( "actually_no_its_a_nightmare" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 1 );

  settings.fieldName = QStringLiteral( "phew_it_was_just_a_dream_all_along" );
  QVERIFY( mStyle->addLabelSettings( "test_format2", settings, true ) );
  QVERIFY( mStyle->labelSettingsNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( mStyle->labelSettingsCount(), 2 );
  QCOMPARE( mStyle->labelSettings( QString( "test_settings" ) ).fieldName, QStringLiteral( "actually_no_its_a_nightmare" ) );
  QCOMPARE( mStyle->labelSettings( QString( "test_format2" ) ).fieldName, QStringLiteral( "phew_it_was_just_a_dream_all_along" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spyChanged.count(), 1 );

  // save and restore
  QVERIFY( mStyle->exportXml( QDir::tempPath() + "/text_style.xml" ) );

  QgsStyle style2;
  QVERIFY( style2.importXml( QDir::tempPath() + "/text_style.xml" ) );

  QVERIFY( style2.labelSettingsNames().contains( QStringLiteral( "test_settings" ) ) );
  QVERIFY( style2.labelSettingsNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( style2.labelSettingsCount(), 2 );
  QCOMPARE( style2.labelSettings( QString( "test_settings" ) ).fieldName, QStringLiteral( "actually_no_its_a_nightmare" ) );
  QCOMPARE( style2.labelSettings( QString( "test_format2" ) ).fieldName, QStringLiteral( "phew_it_was_just_a_dream_all_along" ) );

  QCOMPARE( mStyle->allNames( QgsStyle::LabelSettingsEntity ), QStringList() << QStringLiteral( "test_format2" )
            << QStringLiteral( "test_settings" ) );

  QgsStyleLabelSettingsEntity entity( settings );
  QVERIFY( mStyle->addEntity( "test_settings2", &entity, true ) );
  QVERIFY( mStyle->labelSettingsNames().contains( QStringLiteral( "test_settings2" ) ) );
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

  QVERIFY( !mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "AaaaaaaaaA" ) ) );

  // add some symbols to favorites
  std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  mStyle->saveSymbol( QStringLiteral( "symbolA" ), sym1.get(), true, QStringList() );
  mStyle->saveSymbol( QStringLiteral( "symbolB" ), sym2.get(), false, QStringList() );
  mStyle->saveSymbol( QStringLiteral( "symbolC" ), sym3.get(), true, QStringList() );

  QVERIFY( mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolB" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolC" ) ) );

  // check for added symbols to favorites
  favorites = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
  QCOMPARE( favorites.count(), count + 2 );
  QVERIFY( favorites.contains( QStringLiteral( "symbolA" ) ) );
  QVERIFY( favorites.contains( QStringLiteral( "symbolC" ) ) );

  QSignalSpy favoriteChangedSpy( mStyle, &QgsStyle::favoritedChanged );

  // remove one symbol from favorites
  mStyle->removeFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolA" ) );
  QCOMPARE( favoriteChangedSpy.count(), 1 );
  QCOMPARE( favoriteChangedSpy.at( 0 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::SymbolEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 0 ).at( 1 ).toString(), QStringLiteral( "symbolA" ) );
  QCOMPARE( favoriteChangedSpy.at( 0 ).at( 2 ).toBool(), false );

  QVERIFY( !mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolB" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolC" ) ) );

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

  QVERIFY( mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolB" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "symbolC" ) ) );

  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::red ), QColor( Qt::blue ) );
  QVERIFY( mStyle->addColorRamp( "gradient_1", gradientRamp, true ) );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  QCOMPARE( favorites.count(), 0 );

  QVERIFY( !mStyle->isFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) ) );

  mStyle->addFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 3 );
  QCOMPARE( favoriteChangedSpy.at( 2 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::ColorrampEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 2 ).at( 1 ).toString(), QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 2 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  QCOMPARE( favorites.count(), 1 );
  QVERIFY( favorites.contains( "gradient_1" ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) ) );

  mStyle->removeFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 4 );
  QCOMPARE( favoriteChangedSpy.at( 3 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::ColorrampEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 3 ).at( 1 ).toString(), QStringLiteral( "gradient_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 3 ).at( 2 ).toBool(), false );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::ColorrampEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "gradient_1" ) ) );

  // text formats
  QgsTextFormat format1;
  QVERIFY( mStyle->addTextFormat( QStringLiteral( "format_1" ), format1, true ) );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::TextFormatEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "format_1" ) ) );

  mStyle->addFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "format_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 5 );
  QCOMPARE( favoriteChangedSpy.at( 4 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::TextFormatEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 4 ).at( 1 ).toString(), QStringLiteral( "format_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 4 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::TextFormatEntity );
  QCOMPARE( favorites.count(), 1 );
  QVERIFY( favorites.contains( QStringLiteral( "format_1" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "format_1" ) ) );

  mStyle->removeFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "format_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 6 );
  QCOMPARE( favoriteChangedSpy.at( 5 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::TextFormatEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 5 ).at( 1 ).toString(), QStringLiteral( "format_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 5 ).at( 2 ).toBool(), false );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::TextFormatEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "format_1" ) ) );

  // label settings
  QgsPalLayerSettings settings1;
  QVERIFY( mStyle->addLabelSettings( QStringLiteral( "settings_1" ), settings1, true ) );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::LabelSettingsEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings_1" ) ) );

  mStyle->addFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 7 );
  QCOMPARE( favoriteChangedSpy.at( 6 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 6 ).at( 1 ).toString(), QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 6 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::LabelSettingsEntity );
  QCOMPARE( favorites.count(), 1 );
  QVERIFY( favorites.contains( QStringLiteral( "settings_1" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings_1" ) ) );

  mStyle->removeFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 8 );
  QCOMPARE( favoriteChangedSpy.at( 7 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 7 ).at( 1 ).toString(), QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 7 ).at( 2 ).toBool(), false );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::LabelSettingsEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings_1" ) ) );
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

  QCOMPARE( mStyle->allNames( QgsStyle::TagEntity ),
            QStringList() << QStringLiteral( "red" )
            << QStringLiteral( "starry" )
            << QStringLiteral( "circle" )
            << QStringLiteral( "blue" )
            << QStringLiteral( "purple" ) );

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

  // text formats

  // tag format
  QgsTextFormat format1;
  QVERIFY( mStyle->addTextFormat( "format1", format1, true ) );
  QgsTextFormat format2;
  QVERIFY( mStyle->addTextFormat( "format2", format2, true ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::TextFormatEntity, "format1", QStringList() << "blue" << "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 17 );
  QCOMPARE( tagsChangedSpy.at( 16 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::TextFormatEntity ) );
  QCOMPARE( tagsChangedSpy.at( 16 ).at( 1 ).toString(), QStringLiteral( "format1" ) );
  QCOMPARE( tagsChangedSpy.at( 16 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "blue" ) << QStringLiteral( "starry" ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::TextFormatEntity, "format2", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 18 );
  QCOMPARE( tagsChangedSpy.at( 17 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::TextFormatEntity ) );
  QCOMPARE( tagsChangedSpy.at( 17 ).at( 1 ).toString(), QStringLiteral( "format2" ) );
  QCOMPARE( tagsChangedSpy.at( 17 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) );

  //bad format name
  QVERIFY( !mStyle->tagSymbol( QgsStyle::TextFormatEntity, "no format", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 18 );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyle::TextFormatEntity, "format2", QStringList() << "red text" ) );
  QCOMPARE( tagsChangedSpy.count(), 19 );
  QCOMPARE( tagsChangedSpy.at( 18 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::TextFormatEntity ) );
  QCOMPARE( tagsChangedSpy.at( 18 ).at( 1 ).toString(), QStringLiteral( "format2" ) );
  QCOMPARE( tagsChangedSpy.at( 18 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) << QStringLiteral( "red text" ) );

  tags = mStyle->tags();
  QVERIFY( tags.contains( QStringLiteral( "red text" ) ) );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyle::TextFormatEntity, QStringLiteral( "format1" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::TextFormatEntity, QStringLiteral( "format2" ) );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "red text" ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyle::TextFormatEntity, "format1", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::TextFormatEntity, QStringLiteral( "format1" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 20 );
  QCOMPARE( tagsChangedSpy.at( 19 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::TextFormatEntity ) );
  QCOMPARE( tagsChangedSpy.at( 19 ).at( 1 ).toString(), QStringLiteral( "format1" ) );
  QCOMPARE( tagsChangedSpy.at( 19 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "starry" ) );

  // completely detag symbol
  QVERIFY( mStyle->detagSymbol( QgsStyle::TextFormatEntity, QStringLiteral( "format1" ) ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::TextFormatEntity, QStringLiteral( "format1" ) );
  QCOMPARE( tags.count(), 0 );
  QCOMPARE( tagsChangedSpy.count(), 21 );
  QCOMPARE( tagsChangedSpy.at( 20 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::TextFormatEntity ) );
  QCOMPARE( tagsChangedSpy.at( 20 ).at( 1 ).toString(), QStringLiteral( "format1" ) );
  QCOMPARE( tagsChangedSpy.at( 20 ).at( 2 ).toStringList(), QStringList() );


  // label settings

  // tag format
  QgsPalLayerSettings settings1;
  QVERIFY( mStyle->addLabelSettings( "settings1", settings1, true ) );
  QgsPalLayerSettings settings2;
  QVERIFY( mStyle->addLabelSettings( "settings2", settings2, true ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::LabelSettingsEntity, "settings1", QStringList() << "blue" << "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 24 );
  QCOMPARE( tagsChangedSpy.at( 23 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( tagsChangedSpy.at( 23 ).at( 1 ).toString(), QStringLiteral( "settings1" ) );
  QCOMPARE( tagsChangedSpy.at( 23 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "blue" ) << QStringLiteral( "starry" ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::LabelSettingsEntity, "settings2", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 25 );
  QCOMPARE( tagsChangedSpy.at( 24 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( tagsChangedSpy.at( 24 ).at( 1 ).toString(), QStringLiteral( "settings2" ) );
  QCOMPARE( tagsChangedSpy.at( 24 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) );

  //bad format name
  QVERIFY( !mStyle->tagSymbol( QgsStyle::LabelSettingsEntity, "no format", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 25 );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyle::LabelSettingsEntity, "settings2", QStringList() << "red labels" ) );
  QCOMPARE( tagsChangedSpy.count(), 26 );
  QCOMPARE( tagsChangedSpy.at( 25 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( tagsChangedSpy.at( 25 ).at( 1 ).toString(), QStringLiteral( "settings2" ) );
  QCOMPARE( tagsChangedSpy.at( 25 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) << QStringLiteral( "red labels" ) );

  tags = mStyle->tags();
  QVERIFY( tags.contains( QStringLiteral( "red labels" ) ) );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings1" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings2" ) );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "red labels" ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyle::LabelSettingsEntity, "settings1", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings1" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 27 );
  QCOMPARE( tagsChangedSpy.at( 26 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( tagsChangedSpy.at( 26 ).at( 1 ).toString(), QStringLiteral( "settings1" ) );
  QCOMPARE( tagsChangedSpy.at( 26 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "starry" ) );

  // completely detag symbol
  QVERIFY( mStyle->detagSymbol( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings1" ) ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::LabelSettingsEntity, QStringLiteral( "settings1" ) );
  QCOMPARE( tags.count(), 0 );
  QCOMPARE( tagsChangedSpy.count(), 28 );
  QCOMPARE( tagsChangedSpy.at( 27 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LabelSettingsEntity ) );
  QCOMPARE( tagsChangedSpy.at( 27 ).at( 1 ).toString(), QStringLiteral( "settings1" ) );
  QCOMPARE( tagsChangedSpy.at( 27 ).at( 2 ).toStringList(), QStringList() );
}

void TestStyle::testSmartGroup()
{
  QgsStyle style;
  style.createMemoryDatabase();

  QSignalSpy groupModifiedSpy( &style, &QgsStyle::groupsModified );

  std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QgsStringMap() ) );
  style.addSymbol( QStringLiteral( "symbolA" ), sym1->clone(), true );
  style.addSymbol( QStringLiteral( "symbolB" ), sym2->clone(), true );
  style.addSymbol( QStringLiteral( "symbolC" ), sym3->clone(), true );
  QgsLimitedRandomColorRamp *randomRamp = new QgsLimitedRandomColorRamp();
  QVERIFY( style.addColorRamp( "ramp a", randomRamp, true ) );
  randomRamp = new QgsLimitedRandomColorRamp();
  QVERIFY( style.addColorRamp( "different bbb", randomRamp, true ) );

  QgsTextFormat format1;
  QVERIFY( style.addTextFormat( "format a", format1, true ) );
  QgsTextFormat format2;
  QVERIFY( style.addTextFormat( "different text bbb", format2, true ) );

  QgsPalLayerSettings settings1;
  QVERIFY( style.addLabelSettings( "settings a", settings1, true ) );
  QgsPalLayerSettings settings2;
  QVERIFY( style.addLabelSettings( "different l bbb", settings2, true ) );

  QVERIFY( style.smartgroupNames().empty() );
  QVERIFY( style.smartgroup( 5 ).isEmpty() );
  QCOMPARE( style.smartgroupId( QStringLiteral( "no exist" ) ), 0 );

  int res = style.addSmartgroup( QStringLiteral( "mine" ), QStringLiteral( "AND" ), QStringList(), QStringList(), QStringList() << QStringLiteral( "a" ), QStringList() );
  QCOMPARE( res, 1 );
  QCOMPARE( style.smartgroupNames(), QStringList() << QStringLiteral( "mine" ) );
  QCOMPARE( style.smartgroup( 1 ).values( QStringLiteral( "name" ) ), QList< QString >() << QStringLiteral( "a" ) );
  QCOMPARE( style.smartgroupId( QStringLiteral( "mine" ) ), 1 );
  QCOMPARE( groupModifiedSpy.count(), 1 );

  QCOMPARE( style.allNames( QgsStyle::SmartgroupEntity ),
            QStringList() << QStringLiteral( "mine" ) );

  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::SymbolEntity, 1 ), QStringList() << QStringLiteral( "symbolA" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::ColorrampEntity, 1 ), QStringList() << QStringLiteral( "ramp a" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::TextFormatEntity, 1 ), QStringList() << QStringLiteral( "format a" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LabelSettingsEntity, 1 ), QStringList() << QStringLiteral( "settings a" ) );

  res = style.addSmartgroup( QStringLiteral( "tag" ), QStringLiteral( "OR" ), QStringList(), QStringList(), QStringList() << "c", QStringList() << "a" );
  QCOMPARE( res, 2 );
  QCOMPARE( style.smartgroupNames(), QStringList() << QStringLiteral( "mine" ) << QStringLiteral( "tag" ) );
  QCOMPARE( style.smartgroup( 2 ).values( QStringLiteral( "name" ) ), QList< QString >() << QStringLiteral( "c" ) );
  QCOMPARE( style.smartgroup( 2 ).values( QStringLiteral( "!name" ) ), QList< QString >() << QStringLiteral( "a" ) );
  QCOMPARE( style.smartgroupId( QStringLiteral( "tag" ) ), 2 );
  QCOMPARE( groupModifiedSpy.count(), 2 );

  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::SymbolEntity, 2 ), QStringList() << QStringLiteral( "symbolB" ) << QStringLiteral( "symbolC" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::ColorrampEntity, 2 ), QStringList() << QStringLiteral( "different bbb" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::TextFormatEntity, 2 ), QStringList() << QStringLiteral( "different text bbb" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LabelSettingsEntity, 2 ), QStringList() << QStringLiteral( "different l bbb" ) );

  // tag some symbols
  style.tagSymbol( QgsStyle::SymbolEntity, "symbolA", QStringList() << "red" << "blue" );
  style.tagSymbol( QgsStyle::SymbolEntity, "symbolB", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::ColorrampEntity, "ramp a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::ColorrampEntity, "different bbb", QStringList() << "blue" << "red" );
  style.tagSymbol( QgsStyle::TextFormatEntity, "format a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::TextFormatEntity, "different text bbb", QStringList() << "blue" << "red" );
  style.tagSymbol( QgsStyle::LabelSettingsEntity, "settings a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::LabelSettingsEntity, "different l bbb", QStringList() << "blue" << "red" );

  // adding tags modifies groups!
  QCOMPARE( groupModifiedSpy.count(), 4 );

  res = style.addSmartgroup( QStringLiteral( "tags" ), QStringLiteral( "AND" ), QStringList() << "blue", QStringList() << "red", QStringList(), QStringList() );
  QCOMPARE( res, 3 );
  QCOMPARE( style.smartgroupNames(), QStringList() << QStringLiteral( "mine" ) << QStringLiteral( "tag" ) << QStringLiteral( "tags" ) );
  QCOMPARE( style.smartgroup( 3 ).values( QStringLiteral( "tag" ) ), QList< QString >() << QStringLiteral( "blue" ) );
  QCOMPARE( style.smartgroup( 3 ).values( QStringLiteral( "!tag" ) ), QList< QString >() << QStringLiteral( "red" ) );
  QCOMPARE( style.smartgroupId( QStringLiteral( "tags" ) ), 3 );
  QCOMPARE( groupModifiedSpy.count(), 5 );

  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::SymbolEntity, 3 ), QStringList() << QStringLiteral( "symbolB" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::ColorrampEntity, 3 ), QStringList() << QStringLiteral( "ramp a" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::TextFormatEntity, 3 ), QStringList() << QStringLiteral( "format a" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LabelSettingsEntity, 3 ), QStringList() << QStringLiteral( "settings a" ) );

  res = style.addSmartgroup( QStringLiteral( "combined" ), QStringLiteral( "AND" ), QStringList() << "blue", QStringList(), QStringList(), QStringList() << "a" );
  QCOMPARE( res, 4 );
  QCOMPARE( style.smartgroupNames(), QStringList() << QStringLiteral( "mine" ) << QStringLiteral( "tag" ) << QStringLiteral( "tags" )  << QStringLiteral( "combined" ) );
  QCOMPARE( style.smartgroup( 4 ).values( QStringLiteral( "tag" ) ), QList< QString >() << QStringLiteral( "blue" ) );
  QCOMPARE( style.smartgroup( 4 ).values( QStringLiteral( "!name" ) ), QList< QString >() << QStringLiteral( "a" ) );
  QCOMPARE( style.smartgroupId( QStringLiteral( "combined" ) ), 4 );
  QCOMPARE( groupModifiedSpy.count(), 6 );

  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::SymbolEntity, 4 ), QStringList() << QStringLiteral( "symbolB" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::ColorrampEntity, 4 ), QStringList() << QStringLiteral( "different bbb" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::TextFormatEntity, 4 ), QStringList() << QStringLiteral( "different text bbb" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LabelSettingsEntity, 4 ), QStringList() << QStringLiteral( "different l bbb" ) );

  style.remove( QgsStyle::SmartgroupEntity, 1 );
  QCOMPARE( style.smartgroupNames(), QStringList() << QStringLiteral( "tag" ) << QStringLiteral( "tags" )  << QStringLiteral( "combined" ) );
  QCOMPARE( groupModifiedSpy.count(), 7 );

  style.remove( QgsStyle::SmartgroupEntity, 4 );
  QCOMPARE( style.smartgroupNames(), QStringList() << QStringLiteral( "tag" ) << QStringLiteral( "tags" ) );
  QCOMPARE( groupModifiedSpy.count(), 8 );
}

void TestStyle::testIsStyleXml()
{
  QVERIFY( !QgsStyle::isXmlStyleFile( QString() ) );
  QVERIFY( !QgsStyle::isXmlStyleFile( QStringLiteral( "blah" ) ) );
  QVERIFY( QgsStyle::isXmlStyleFile( mTestDataDir + QStringLiteral( "categorized.xml" ) ) );
  QVERIFY( !QgsStyle::isXmlStyleFile( mTestDataDir + QStringLiteral( "openstreetmap/testdata.xml" ) ) );
}


class TestVisitor : public QgsStyleEntityVisitorInterface
{
  public:

    TestVisitor( QStringList &found )
      : mFound( found )
    {}

    bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
    {
      mFound << QStringLiteral( "enter: %1 %2" ).arg( node.identifier, node.description );
      return true;
    }

    bool visitExit( const QgsStyleEntityVisitorInterface::Node &node ) override
    {
      mFound << QStringLiteral( "exit: %1 %2" ).arg( node.identifier, node.description );
      return true;
    }

    bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &entity ) override
    {
      switch ( entity.entity->type() )
      {
        case QgsStyle::SymbolEntity:
        {
          mFound << QStringLiteral( "symbol: %1 %2 %3" ).arg( entity.description, entity.identifier, static_cast< const QgsStyleSymbolEntity * >( entity.entity )->symbol()->color().name() );
          break;
        }
        case QgsStyle::ColorrampEntity:
          mFound << QStringLiteral( "ramp: %1 %2 %3" ).arg( entity.description, entity.identifier, static_cast< const QgsStyleColorRampEntity * >( entity.entity )->ramp()->color( 0 ).name() );
          break;

        case QgsStyle::TextFormatEntity:
          mFound << QStringLiteral( "text format: %1 %2 %3" ).arg( entity.description, entity.identifier, static_cast< const QgsStyleTextFormatEntity * >( entity.entity )->format().font().family() );
          break;

        case QgsStyle::LabelSettingsEntity:
          mFound << QStringLiteral( "labels: %1 %2 %3" ).arg( entity.description, entity.identifier, static_cast< const QgsStyleLabelSettingsEntity * >( entity.entity )->settings().fieldName );
          break;

        case QgsStyle::TagEntity:
        case QgsStyle::SmartgroupEntity:
          break;
      }
      return true;
    }

    QStringList &mFound;
};

void TestStyle::testVisitor()
{
  // test style visitor
  QgsProject p;

  QStringList found;
  TestVisitor visitor( found );
  QVERIFY( p.accept( &visitor ) );
  QVERIFY( found.isEmpty() );

  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( vl->isValid() );
  p.addMapLayer( vl );

  // with renderer
  QgsSimpleMarkerSymbolLayer *simpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol();
  markerSymbol->changeSymbolLayer( 0, simpleMarkerLayer );
  vl->setRenderer( new QgsSingleSymbolRenderer( markerSymbol ) );

  QVERIFY( p.accept( &visitor ) );
  QCOMPARE( found, QStringList() << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "symbol:   #ff0000" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() ) );

  // rule based renderer
  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( vl2->isValid() );
  p.addMapLayer( vl2 );
  QgsSymbol *s1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  s1->setColor( QColor( 0, 255, 0 ) );
  QgsSymbol *s2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  s2->setColor( QColor( 0, 255, 255 ) );
  QgsRuleBasedRenderer::Rule *rootRule = new QgsRuleBasedRenderer::Rule( nullptr );
  QgsRuleBasedRenderer::Rule *rule2 = new QgsRuleBasedRenderer::Rule( s1, 0, 0, QStringLiteral( "fld >= 5 and fld <= 20" ) );
  rootRule->appendChild( rule2 );
  QgsRuleBasedRenderer::Rule *rule3 = new QgsRuleBasedRenderer::Rule( s2, 0, 0, QStringLiteral( "fld <= 10" ) );
  rule2->appendChild( rule3 );
  vl2->setRenderer( new QgsRuleBasedRenderer( rootRule ) );

  found.clear();
  QVERIFY( p.accept( &visitor ) );
  QCOMPARE( found, QStringList()
            << QStringLiteral( "enter: %1 vl2" ).arg( vl2->id() )
            << QStringLiteral( "enter: %1 " ).arg( rule2->ruleKey() )
            << QStringLiteral( "symbol:   #00ff00" )
            << QStringLiteral( "enter: %1 " ).arg( rule3->ruleKey() )
            << QStringLiteral( "symbol:   #00ffff" )
            << QStringLiteral( "exit: %1 " ).arg( rule3->ruleKey() )
            << QStringLiteral( "exit: %1 " ).arg( rule2->ruleKey() )
            << QStringLiteral( "exit: %1 vl2" ).arg( vl2->id() )
            << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "symbol:   #ff0000" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() ) );

  p.removeMapLayer( vl2 );

  // labeling
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  found.clear();
  QVERIFY( p.accept( &visitor ) );

  QCOMPARE( found, QStringList() << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "symbol:   #ff0000" )
            << QStringLiteral( "labels:   Class" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() ) );

  // raster layer
  QgsRasterLayer *rl = new QgsRasterLayer( QStringLiteral( TEST_DATA_DIR ) + "/tenbytenraster.asc",
      QStringLiteral( "rl" ) );
  QVERIFY( rl->isValid() );
  p.addMapLayer( rl );

  QgsRasterShader *rasterShader = new QgsRasterShader();
  QgsColorRampShader *colorRampShader = new QgsColorRampShader();
  colorRampShader->setColorRampType( QgsColorRampShader::Interpolated );
  colorRampShader->setSourceColorRamp( new QgsGradientColorRamp( QColor( 255, 255, 0 ), QColor( 255, 0, 255 ) ) );
  rasterShader->setRasterShaderFunction( colorRampShader );
  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( rl->dataProvider(), 1, rasterShader );
  rl->setRenderer( r );

  found.clear();
  QVERIFY( p.accept( &visitor ) );

  QCOMPARE( found, QStringList()
            << QStringLiteral( "enter: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "ramp:   #ffff00" )
            << QStringLiteral( "exit: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "symbol:   #ff0000" )
            << QStringLiteral( "labels:   Class" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() ) );

  // with layout
  QgsPrintLayout *l = new QgsPrintLayout( &p );
  l->setName( QStringLiteral( "test layout" ) );
  l->initializeDefaults();
  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l->addLayoutItem( scalebar );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );

  p.layoutManager()->addLayout( l );

  found.clear();
  QVERIFY( p.accept( &visitor ) );

  QCOMPARE( found, QStringList()
            << QStringLiteral( "enter: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "ramp:   #ffff00" )
            << QStringLiteral( "exit: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "symbol:   #ff0000" )
            << QStringLiteral( "labels:   Class" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "enter: layouts Layouts" )
            << QStringLiteral( "enter: layout test layout" )
            << QStringLiteral( "text format: <Scalebar> %1 QGIS Vera Sans" ).arg( scalebar->uuid() )
            << QStringLiteral( "symbol: Page page #ffffff" )
            << QStringLiteral( "exit: layout test layout" )
            << QStringLiteral( "exit: layouts Layouts" ) );

  // with annotations
  QgsTextAnnotation *annotation = new QgsTextAnnotation();
  QgsSymbol *a1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
  a1->setColor( QColor( 0, 200, 0 ) );
  annotation->setMarkerSymbol( static_cast< QgsMarkerSymbol * >( a1 ) );
  QgsSymbol *a2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry );
  a2->setColor( QColor( 200, 200, 0 ) );
  annotation->setFillSymbol( static_cast< QgsFillSymbol * >( a2 ) );
  p.annotationManager()->addAnnotation( annotation );

  found.clear();
  QVERIFY( p.accept( &visitor ) );

  QCOMPARE( found, QStringList()
            << QStringLiteral( "enter: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "ramp:   #ffff00" )
            << QStringLiteral( "exit: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "symbol:   #ff0000" )
            << QStringLiteral( "labels:   Class" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "enter: layouts Layouts" )
            << QStringLiteral( "enter: layout test layout" )
            << QStringLiteral( "text format: <Scalebar> %1 QGIS Vera Sans" ).arg( scalebar->uuid() )
            << QStringLiteral( "symbol: Page page #ffffff" )
            << QStringLiteral( "exit: layout test layout" )
            << QStringLiteral( "exit: layouts Layouts" )
            << QStringLiteral( "enter: annotations Annotations" )
            << QStringLiteral( "enter: annotation Annotation" )
            << QStringLiteral( "symbol: Marker marker #00c800" )
            << QStringLiteral( "symbol: Fill fill #c8c800" )
            << QStringLiteral( "exit: annotation Annotation" )
            << QStringLiteral( "exit: annotations Annotations" ) );
}

QGSTEST_MAIN( TestStyle )
#include "testqgsstyle.moc"
