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
#include <QSignalSpy>

// clazy:excludeall=qcolor-from-literal

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
#include "qgslayoutitemlegend.h"
#include "qgslayertreelayer.h"
#include "qgslayertreeutils.h"
#include "qgsmaplayerlegend.h"
#include "qgsabstract3dsymbol.h"
#include "qgs3dsymbolregistry.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

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

    static bool compareItemLists( QList<QgsColorRampShader::ColorRampItem> &itemsList1, QList<QgsColorRampShader::ColorRampItem> &itemsList2 )
    {
      if ( itemsList1.size() != itemsList2.size() ) return false;
      for ( int i = 0; i < itemsList1.size(); ++i )
      {
        if ( itemsList1[i].value != itemsList2[i].value )
          return false;
        if ( itemsList1[i].color.red() != itemsList2[i].color.red() )
          return false;
        if ( itemsList1[i].color.green() != itemsList2[i].color.green() )
          return false;
        if ( itemsList1[i].color.blue() != itemsList2[i].color.blue() )
          return false;
        if ( itemsList1[i].color.alpha() != itemsList2[i].color.alpha() )
          return false;
      }
      return true;
    }
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
    void testCreateLegendPatchShapes();
    void testCreate3dSymbol();
    void testLoadColorRamps();
    void testSaveLoad();
    void testFavorites();
    void testTags();
    void testSmartGroup();
    void testIsStyleXml();
    void testVisitor();
    void testColorRampShaderClassificationEqualInterval();
    void testColorRampShaderClassificationContinius();
    void testDefaultLabelTextFormat();
};


class Dummy3DSymbol : public QgsAbstract3DSymbol
{
  public:
    static QgsAbstract3DSymbol *create() { return new Dummy3DSymbol; }
    QString type() const override { return QStringLiteral( "dummy" ); }
    QgsAbstract3DSymbol *clone() const override { Dummy3DSymbol *res = new Dummy3DSymbol(); res->id = id; return res; }
    void readXml( const QDomElement &elem, const QgsReadWriteContext & ) override { id = elem.attribute( QStringLiteral( "id" ) ); }
    void writeXml( QDomElement &elem, const QgsReadWriteContext & ) const override { elem.setAttribute( QStringLiteral( "id" ), id ); }
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override { return QList< QgsWkbTypes::GeometryType >() << QgsWkbTypes::PointGeometry << QgsWkbTypes::LineGeometry; }

    QString id;

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

  // now cheat!
  QgsStyle::sDefaultStyle = mStyle;

  // cpt-city ramp, small selection available in <testdir>/cpt-city
  QgsCptCityArchive::initArchives();

  mReport += QLatin1String( "<h1>Style Tests</h1>\n" );

  QgsApplication::symbol3DRegistry()->addSymbolType( new Qgs3DSymbolMetadata( QStringLiteral( "dummy" ), QObject::tr( "Dummy" ),
      &Dummy3DSymbol::create, nullptr, nullptr ) );
}

void TestStyle::cleanupTestCase()
{
  // don't save
  // mStyle->save();

  // don't delete -- it's handled by exitQgis, cos we've set mStyle as the static default style instance
  // delete mStyle;

  QgsCptCityArchive::clearArchives();
  QgsApplication::exitQgis();

  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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

  std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  const std::unique_ptr< QgsMarkerSymbol > sym4( QgsMarkerSymbol::createSimple( QVariantMap() ) );
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
  const bool result = checker.runTest( testName, 0 );
  mReport += checker.report();
  return result;
}

bool TestStyle::testValidColor( QgsColorRamp *ramp, double value, const QColor &expected )
{
  const QColor result = ramp->color( value );
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

  const std::unique_ptr< QgsCptCityColorRamp > cc4Ramp = std::make_unique< QgsCptCityColorRamp >( QStringLiteral( "grass/byr" ), QString() );
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

  const QSignalSpy spy( mStyle, &QgsStyle::textFormatAdded );
  const QSignalSpy spyChanged( mStyle, &QgsStyle::textFormatChanged );
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

  const QSignalSpy spy( mStyle, &QgsStyle::labelSettingsAdded );
  const QSignalSpy spyChanged( mStyle, &QgsStyle::labelSettingsChanged );
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

void TestStyle::testCreateLegendPatchShapes()
{
  QVERIFY( mStyle->legendPatchShapeNames().isEmpty() );
  QCOMPARE( mStyle->legendPatchShapesCount(), 0 );
  // non existent settings, should be default
  QVERIFY( mStyle->legendPatchShape( QString( "blah" ) ).isNull() );

  const QSignalSpy spy( mStyle, &QgsStyle::entityAdded );
  const QSignalSpy spyChanged( mStyle, &QgsStyle::entityChanged );
  // add settings
  QgsLegendPatchShape settings;
  settings.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point ( 5 6 )" ) ) );
  QVERIFY( mStyle->addLegendPatchShape( "test_settings", settings, true ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 0 );

  QVERIFY( mStyle->legendPatchShapeNames().contains( QStringLiteral( "test_settings" ) ) );
  QCOMPARE( mStyle->legendPatchShapesCount(), 1 );
  QCOMPARE( mStyle->legendPatchShape( QString( "test_settings" ) ).geometry().asWkt(), QStringLiteral( "Point (5 6)" ) );

  settings.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point ( 15 16 )" ) ) );
  QVERIFY( mStyle->addLegendPatchShape( "test_settings", settings, true ) );
  QVERIFY( mStyle->legendPatchShapeNames().contains( QStringLiteral( "test_settings" ) ) );
  QCOMPARE( mStyle->legendPatchShapesCount(), 1 );
  QCOMPARE( mStyle->legendPatchShape( QString( "test_settings" ) ).geometry().asWkt(), QStringLiteral( "Point (15 16)" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 1 );

  settings.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point ( 25 26 )" ) ) );
  QVERIFY( mStyle->addLegendPatchShape( "test_format2", settings, true ) );
  QVERIFY( mStyle->legendPatchShapeNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( mStyle->legendPatchShapesCount(), 2 );
  QCOMPARE( mStyle->legendPatchShape( QString( "test_settings" ) ).geometry().asWkt(), QStringLiteral( "Point (15 16)" ) );
  QCOMPARE( mStyle->legendPatchShape( QString( "test_format2" ) ).geometry().asWkt(), QStringLiteral( "Point (25 26)" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spyChanged.count(), 1 );

  // save and restore
  QVERIFY( mStyle->exportXml( QDir::tempPath() + "/text_style.xml" ) );

  QgsStyle style2;
  QVERIFY( style2.importXml( QDir::tempPath() + "/text_style.xml" ) );

  QVERIFY( style2.legendPatchShapeNames().contains( QStringLiteral( "test_settings" ) ) );
  QVERIFY( style2.legendPatchShapeNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( style2.legendPatchShapesCount(), 2 );
  QCOMPARE( style2.legendPatchShape( QString( "test_settings" ) ).geometry().asWkt(), QStringLiteral( "Point (15 16)" ) );
  QCOMPARE( style2.legendPatchShape( QString( "test_format2" ) ).geometry().asWkt(), QStringLiteral( "Point (25 26)" ) );

  QCOMPARE( mStyle->allNames( QgsStyle::LegendPatchShapeEntity ), QStringList() << QStringLiteral( "test_format2" )
            << QStringLiteral( "test_settings" ) );

  QgsStyleLegendPatchShapeEntity entity( settings );
  QVERIFY( mStyle->addEntity( "test_settings2", &entity, true ) );
  QVERIFY( mStyle->legendPatchShapeNames().contains( QStringLiteral( "test_settings2" ) ) );
}

void TestStyle::testCreate3dSymbol()
{
  QVERIFY( mStyle->symbol3DNames().isEmpty() );
  QCOMPARE( mStyle->symbol3DCount(), 0 );
  // non existent settings, should be default
  QVERIFY( !mStyle->symbol3D( QString( "blah" ) ) );
  QVERIFY( mStyle->symbol3DCompatibleGeometryTypes( QStringLiteral( "blah" ) ).isEmpty() );

  const QSignalSpy spy( mStyle, &QgsStyle::entityAdded );
  const QSignalSpy spyChanged( mStyle, &QgsStyle::entityChanged );
  // add symbol
  Dummy3DSymbol symbol;
  symbol.id = QStringLiteral( "xxx" );
  QVERIFY( mStyle->addSymbol3D( "test_settings", symbol.clone(), true ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 0 );

  QVERIFY( mStyle->symbol3DNames().contains( QStringLiteral( "test_settings" ) ) );
  QCOMPARE( mStyle->symbol3DCount(), 1 );
  QVERIFY( mStyle->symbol3DCompatibleGeometryTypes( QStringLiteral( "blah" ) ).isEmpty() );
  QCOMPARE( mStyle->symbol3DCompatibleGeometryTypes( QStringLiteral( "test_settings" ) ), QList< QgsWkbTypes::GeometryType >() << QgsWkbTypes::PointGeometry << QgsWkbTypes::LineGeometry );
  std::unique_ptr< Dummy3DSymbol > retrieved( dynamic_cast< Dummy3DSymbol * >( mStyle->symbol3D( QStringLiteral( "test_settings" ) ) ) );
  QCOMPARE( retrieved->id, QStringLiteral( "xxx" ) );
  symbol.id = QStringLiteral( "yyy" );
  QVERIFY( mStyle->addSymbol3D( "test_settings", symbol.clone(), true ) );
  QVERIFY( mStyle->symbol3DNames().contains( QStringLiteral( "test_settings" ) ) );
  QCOMPARE( mStyle->symbol3DCount(), 1 );
  retrieved.reset( dynamic_cast< Dummy3DSymbol * >( mStyle->symbol3D( QStringLiteral( "test_settings" ) ) ) );
  QCOMPARE( retrieved->id, QStringLiteral( "yyy" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spyChanged.count(), 1 );

  symbol.id = QStringLiteral( "zzz" );
  QVERIFY( mStyle->addSymbol3D( "test_format2", symbol.clone(), true ) );
  QVERIFY( mStyle->symbol3DNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( mStyle->symbol3DCount(), 2 );
  retrieved.reset( dynamic_cast< Dummy3DSymbol * >( mStyle->symbol3D( QStringLiteral( "test_settings" ) ) ) );
  QCOMPARE( retrieved->id, QStringLiteral( "yyy" ) );
  retrieved.reset( dynamic_cast< Dummy3DSymbol * >( mStyle->symbol3D( QStringLiteral( "test_format2" ) ) ) );
  QCOMPARE( retrieved->id, QStringLiteral( "zzz" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spyChanged.count(), 1 );

  // save and restore
  QVERIFY( mStyle->exportXml( QDir::tempPath() + "/text_style.xml" ) );

  QgsStyle style2;
  QVERIFY( style2.importXml( QDir::tempPath() + "/text_style.xml" ) );

  QVERIFY( style2.symbol3DNames().contains( QStringLiteral( "test_settings" ) ) );
  QVERIFY( style2.symbol3DNames().contains( QStringLiteral( "test_format2" ) ) );
  QCOMPARE( style2.symbol3DCount(), 2 );
  retrieved.reset( dynamic_cast< Dummy3DSymbol * >( style2.symbol3D( QStringLiteral( "test_settings" ) ) ) );
  QCOMPARE( retrieved->id, QStringLiteral( "yyy" ) );
  retrieved.reset( dynamic_cast< Dummy3DSymbol * >( style2.symbol3D( QStringLiteral( "test_format2" ) ) ) );
  QCOMPARE( retrieved->id, QStringLiteral( "zzz" ) );

  QCOMPARE( mStyle->allNames( QgsStyle::Symbol3DEntity ), QStringList() << QStringLiteral( "test_format2" )
            << QStringLiteral( "test_settings" ) );

  QgsStyleSymbol3DEntity entity( &symbol );
  QVERIFY( mStyle->addEntity( "test_settings2", &entity, true ) );
  QVERIFY( mStyle->symbol3DNames().contains( QStringLiteral( "test_settings2" ) ) );
}

void TestStyle::testLoadColorRamps()
{
  const QStringList colorRamps = mStyle->colorRampNames();
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

  for ( const QString &name : colorRampsTest )
  {
    QgsDebugMsg( "colorRamp " + name );
    QVERIFY( colorRamps.contains( name ) );
    QgsColorRamp *ramp = mStyle->colorRamp( name );
    QVERIFY( ramp != nullptr );
    // test colors
    if ( colorTests.contains( name ) )
    {
      const QList< QPair< double, QColor> > values = colorTests.values( name );
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
  const QStringList colorRamps = mStyle->colorRampNames();
  QgsDebugMsg( "loaded colorRamps: " + colorRamps.join( " " ) );

  const QStringList colorRampsTest = QStringList() << QStringLiteral( "test_gradient" );

  for ( const QString &name : colorRampsTest )
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
  const int count = favorites.count();

  QVERIFY( !mStyle->isFavorite( QgsStyle::SymbolEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::TextFormatEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LabelSettingsEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::ColorrampEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "AaaaaaaaaA" ) ) );
  QVERIFY( !mStyle->isFavorite( QgsStyle::Symbol3DEntity, QStringLiteral( "AaaaaaaaaA" ) ) );

  // add some symbols to favorites
  const std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  const std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  const std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QVariantMap() ) );
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

  const QSignalSpy favoriteChangedSpy( mStyle, &QgsStyle::favoritedChanged );

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
  const QgsTextFormat format1;
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
  const QgsPalLayerSettings settings1;
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

  // legend patch shapes
  const QgsLegendPatchShape shape1;
  QVERIFY( mStyle->addLegendPatchShape( QStringLiteral( "settings_1" ), shape1, true ) );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::LegendPatchShapeEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "settings_1" ) ) );

  mStyle->addFavorite( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 9 );
  QCOMPARE( favoriteChangedSpy.at( 8 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 8 ).at( 1 ).toString(), QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 8 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::LegendPatchShapeEntity );
  QCOMPARE( favorites.count(), 1 );
  QVERIFY( favorites.contains( QStringLiteral( "settings_1" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "settings_1" ) ) );

  mStyle->removeFavorite( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 10 );
  QCOMPARE( favoriteChangedSpy.at( 9 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 9 ).at( 1 ).toString(), QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 9 ).at( 2 ).toBool(), false );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::LegendPatchShapeEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "settings_1" ) ) );

  // symbol 3d
  const Dummy3DSymbol symbol3d1;
  QVERIFY( mStyle->addSymbol3D( QStringLiteral( "settings_1" ), symbol3d1.clone(), true ) );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::Symbol3DEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::Symbol3DEntity, QStringLiteral( "settings_1" ) ) );

  mStyle->addFavorite( QgsStyle::Symbol3DEntity, QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 11 );
  QCOMPARE( favoriteChangedSpy.at( 10 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 10 ).at( 1 ).toString(), QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 10 ).at( 2 ).toBool(), true );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::Symbol3DEntity );
  QCOMPARE( favorites.count(), 1 );
  QVERIFY( favorites.contains( QStringLiteral( "settings_1" ) ) );
  QVERIFY( mStyle->isFavorite( QgsStyle::Symbol3DEntity, QStringLiteral( "settings_1" ) ) );

  mStyle->removeFavorite( QgsStyle::Symbol3DEntity, QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.count(), 12 );
  QCOMPARE( favoriteChangedSpy.at( 11 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( favoriteChangedSpy.at( 11 ).at( 1 ).toString(), QStringLiteral( "settings_1" ) );
  QCOMPARE( favoriteChangedSpy.at( 11 ).at( 2 ).toBool(), false );
  favorites = mStyle->symbolsOfFavorite( QgsStyle::Symbol3DEntity );
  QCOMPARE( favorites.count(), 0 );
  QVERIFY( !mStyle->isFavorite( QgsStyle::Symbol3DEntity, QStringLiteral( "settings_1" ) ) );
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
  const std::unique_ptr< QgsMarkerSymbol> sym1( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol> sym2( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol> sym3( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol> sym4( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  QVERIFY( mStyle->saveSymbol( "symbol1", sym1.get(), false, QStringList() << "red" << "starry" ) );
  mStyle->addSymbol( QStringLiteral( "blue starry" ), sym2.release(), true );
  mStyle->addSymbol( QStringLiteral( "red circle" ), sym3.release(), true );
  mStyle->addSymbol( QStringLiteral( "МЕТЕОР" ), sym4.release(), true );

  const QSignalSpy tagsChangedSpy( mStyle, &QgsStyle::entityTagsChanged );

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
  const QgsTextFormat format1;
  QVERIFY( mStyle->addTextFormat( "format1", format1, true ) );
  const QgsTextFormat format2;
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
  const QgsPalLayerSettings settings1;
  QVERIFY( mStyle->addLabelSettings( "settings1", settings1, true ) );
  const QgsPalLayerSettings settings2;
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


  // legend patch shape

  // tag format
  const QgsLegendPatchShape shape1;
  QVERIFY( mStyle->addLegendPatchShape( "shape1", shape1, true ) );
  const QgsLegendPatchShape shape2;
  QVERIFY( mStyle->addLegendPatchShape( "shape2", shape2, true ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::LegendPatchShapeEntity, "shape1", QStringList() << "blue" << "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 31 );
  QCOMPARE( tagsChangedSpy.at( 30 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( tagsChangedSpy.at( 30 ).at( 1 ).toString(), QStringLiteral( "shape1" ) );
  QCOMPARE( tagsChangedSpy.at( 30 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "blue" ) << QStringLiteral( "starry" ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::LegendPatchShapeEntity, "shape2", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 32 );
  QCOMPARE( tagsChangedSpy.at( 31 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( tagsChangedSpy.at( 31 ).at( 1 ).toString(), QStringLiteral( "shape2" ) );
  QCOMPARE( tagsChangedSpy.at( 31 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) );

  //bad format name
  QVERIFY( !mStyle->tagSymbol( QgsStyle::LegendPatchShapeEntity, "no patch", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 32 );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyle::LegendPatchShapeEntity, "shape2", QStringList() << "red patch" ) );
  QCOMPARE( tagsChangedSpy.count(), 33 );
  QCOMPARE( tagsChangedSpy.at( 32 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( tagsChangedSpy.at( 32 ).at( 1 ).toString(), QStringLiteral( "shape2" ) );
  QCOMPARE( tagsChangedSpy.at( 32 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) << QStringLiteral( "red patch" ) );

  tags = mStyle->tags();
  QVERIFY( tags.contains( QStringLiteral( "red patch" ) ) );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "shape1" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "shape2" ) );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "red patch" ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyle::LegendPatchShapeEntity, "shape1", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "shape1" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 34 );
  QCOMPARE( tagsChangedSpy.at( 33 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( tagsChangedSpy.at( 33 ).at( 1 ).toString(), QStringLiteral( "shape1" ) );
  QCOMPARE( tagsChangedSpy.at( 33 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "starry" ) );

  // completely detag symbol
  QVERIFY( mStyle->detagSymbol( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "shape1" ) ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::LegendPatchShapeEntity, QStringLiteral( "shape1" ) );
  QCOMPARE( tags.count(), 0 );
  QCOMPARE( tagsChangedSpy.count(), 35 );
  QCOMPARE( tagsChangedSpy.at( 34 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::LegendPatchShapeEntity ) );
  QCOMPARE( tagsChangedSpy.at( 34 ).at( 1 ).toString(), QStringLiteral( "shape1" ) );
  QCOMPARE( tagsChangedSpy.at( 34 ).at( 2 ).toStringList(), QStringList() );


  // 3d symbols
  // tag format
  const Dummy3DSymbol symbol3d1;
  QVERIFY( mStyle->addSymbol3D( "3dsymbol1", symbol3d1.clone(), true ) );
  const Dummy3DSymbol symbol3d2;
  QVERIFY( mStyle->addSymbol3D( "3dsymbol2", symbol3d2.clone(), true ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::Symbol3DEntity, "3dsymbol1", QStringList() << "blue" << "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 38 );
  QCOMPARE( tagsChangedSpy.at( 37 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( tagsChangedSpy.at( 37 ).at( 1 ).toString(), QStringLiteral( "3dsymbol1" ) );
  QCOMPARE( tagsChangedSpy.at( 37 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "blue" ) << QStringLiteral( "starry" ) );

  QVERIFY( mStyle->tagSymbol( QgsStyle::Symbol3DEntity, "3dsymbol2", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 39 );
  QCOMPARE( tagsChangedSpy.at( 38 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( tagsChangedSpy.at( 38 ).at( 1 ).toString(), QStringLiteral( "3dsymbol2" ) );
  QCOMPARE( tagsChangedSpy.at( 38 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) );

  //bad format name
  QVERIFY( !mStyle->tagSymbol( QgsStyle::Symbol3DEntity, "no patch", QStringList() << "red" << "circle" ) );
  QCOMPARE( tagsChangedSpy.count(), 39 );
  //tag which hasn't been added yet
  QVERIFY( mStyle->tagSymbol( QgsStyle::Symbol3DEntity, "3dsymbol2", QStringList() << "red patch" ) );
  QCOMPARE( tagsChangedSpy.count(), 40 );
  QCOMPARE( tagsChangedSpy.at( 39 ).at( 0 ).toInt(), static_cast< int>( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( tagsChangedSpy.at( 39 ).at( 1 ).toString(), QStringLiteral( "3dsymbol2" ) );
  QCOMPARE( tagsChangedSpy.at( 39 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "red" ) << QStringLiteral( "circle" ) << QStringLiteral( "red patch" ) );

  tags = mStyle->tags();
  QVERIFY( tags.contains( QStringLiteral( "red patch" ) ) );

  //check that tags have been applied
  tags = mStyle->tagsOfSymbol( QgsStyle::Symbol3DEntity, QStringLiteral( "3dsymbol1" ) );
  QCOMPARE( tags.count(), 2 );
  QVERIFY( tags.contains( "blue" ) );
  QVERIFY( tags.contains( "starry" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::Symbol3DEntity, QStringLiteral( "3dsymbol2" ) );
  QCOMPARE( tags.count(), 3 );
  QVERIFY( tags.contains( "red" ) );
  QVERIFY( tags.contains( "circle" ) );
  QVERIFY( tags.contains( "red patch" ) );

  //remove a tag, including a non-present tag
  QVERIFY( mStyle->detagSymbol( QgsStyle::Symbol3DEntity, "3dsymbol1", QStringList() << "bad" << "blue" ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::Symbol3DEntity, QStringLiteral( "3dsymbol1" ) );
  QCOMPARE( tags.count(), 1 );
  QVERIFY( tags.contains( "starry" ) );
  QCOMPARE( tagsChangedSpy.count(), 41 );
  QCOMPARE( tagsChangedSpy.at( 40 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( tagsChangedSpy.at( 40 ).at( 1 ).toString(), QStringLiteral( "3dsymbol1" ) );
  QCOMPARE( tagsChangedSpy.at( 40 ).at( 2 ).toStringList(), QStringList() << QStringLiteral( "starry" ) );

  // completely detag symbol
  QVERIFY( mStyle->detagSymbol( QgsStyle::Symbol3DEntity, QStringLiteral( "3dsymbol1" ) ) );
  tags = mStyle->tagsOfSymbol( QgsStyle::Symbol3DEntity, QStringLiteral( "3dsymbol1" ) );
  QCOMPARE( tags.count(), 0 );
  QCOMPARE( tagsChangedSpy.count(), 42 );
  QCOMPARE( tagsChangedSpy.at( 41 ).at( 0 ).toInt(), static_cast< int >( QgsStyle::Symbol3DEntity ) );
  QCOMPARE( tagsChangedSpy.at( 41 ).at( 1 ).toString(), QStringLiteral( "3dsymbol1" ) );
  QCOMPARE( tagsChangedSpy.at( 41 ).at( 2 ).toStringList(), QStringList() );
}

void TestStyle::testSmartGroup()
{
  QgsStyle style;
  style.createMemoryDatabase();

  const QSignalSpy groupModifiedSpy( &style, &QgsStyle::groupsModified );

  std::unique_ptr< QgsMarkerSymbol > sym1( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym2( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  std::unique_ptr< QgsMarkerSymbol > sym3( QgsMarkerSymbol::createSimple( QVariantMap() ) );
  style.addSymbol( QStringLiteral( "symbolA" ), sym1->clone(), true );
  style.addSymbol( QStringLiteral( "symbolB" ), sym2->clone(), true );
  style.addSymbol( QStringLiteral( "symbolC" ), sym3->clone(), true );
  QgsLimitedRandomColorRamp *randomRamp = new QgsLimitedRandomColorRamp();
  QVERIFY( style.addColorRamp( "ramp a", randomRamp, true ) );
  randomRamp = new QgsLimitedRandomColorRamp();
  QVERIFY( style.addColorRamp( "different bbb", randomRamp, true ) );

  const QgsTextFormat format1;
  QVERIFY( style.addTextFormat( "format a", format1, true ) );
  const QgsTextFormat format2;
  QVERIFY( style.addTextFormat( "different text bbb", format2, true ) );

  const QgsPalLayerSettings settings1;
  QVERIFY( style.addLabelSettings( "settings a", settings1, true ) );
  const QgsPalLayerSettings settings2;
  QVERIFY( style.addLabelSettings( "different l bbb", settings2, true ) );

  const QgsLegendPatchShape shape1;
  QVERIFY( style.addLegendPatchShape( "shp a", shape1, true ) );
  const QgsLegendPatchShape shape2;
  QVERIFY( style.addLegendPatchShape( "different shp bbb", shape2, true ) );

  const Dummy3DSymbol symbol3d1;
  QVERIFY( style.addSymbol3D( "symbol3D a", symbol3d1.clone(), true ) );
  const Dummy3DSymbol symbol3d2;
  QVERIFY( style.addSymbol3D( "different symbol3D bbb", symbol3d2.clone(), true ) );

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
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LegendPatchShapeEntity, 1 ), QStringList() << QStringLiteral( "shp a" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::Symbol3DEntity, 1 ), QStringList() << QStringLiteral( "symbol3D a" ) );

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
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LegendPatchShapeEntity, 2 ), QStringList() << QStringLiteral( "different shp bbb" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::Symbol3DEntity, 2 ), QStringList() << QStringLiteral( "different symbol3D bbb" ) );

  // tag some symbols
  style.tagSymbol( QgsStyle::SymbolEntity, "symbolA", QStringList() << "red" << "blue" );
  style.tagSymbol( QgsStyle::SymbolEntity, "symbolB", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::ColorrampEntity, "ramp a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::ColorrampEntity, "different bbb", QStringList() << "blue" << "red" );
  style.tagSymbol( QgsStyle::TextFormatEntity, "format a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::TextFormatEntity, "different text bbb", QStringList() << "blue" << "red" );
  style.tagSymbol( QgsStyle::LabelSettingsEntity, "settings a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::LabelSettingsEntity, "different l bbb", QStringList() << "blue" << "red" );
  style.tagSymbol( QgsStyle::LegendPatchShapeEntity, "shp a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::LegendPatchShapeEntity, "different shp bbb", QStringList() << "blue" << "red" );
  style.tagSymbol( QgsStyle::Symbol3DEntity, "symbol3D a", QStringList() << "blue" );
  style.tagSymbol( QgsStyle::Symbol3DEntity, "different symbol3D bbb", QStringList() << "blue" << "red" );

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
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LegendPatchShapeEntity, 3 ), QStringList() << QStringLiteral( "shp a" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::Symbol3DEntity, 3 ), QStringList() << QStringLiteral( "symbol3D a" ) );

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
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::LegendPatchShapeEntity, 4 ), QStringList() << QStringLiteral( "different shp bbb" ) );
  QCOMPARE( style.symbolsOfSmartgroup( QgsStyle::Symbol3DEntity, 4 ), QStringList() << QStringLiteral( "different symbol3D bbb" ) );

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

        case QgsStyle::LegendPatchShapeEntity:
          mFound << QStringLiteral( "patch: %1 %2 %3" ).arg( entity.description, entity.identifier, static_cast< const QgsStyleLegendPatchShapeEntity * >( entity.entity )->shape().geometry().asWkt() );
          break;

        case QgsStyle::Symbol3DEntity:
          mFound << QStringLiteral( "symbol 3d: %1 %2 %3" ).arg( entity.description, entity.identifier, static_cast< const QgsStyleSymbol3DEntity * >( entity.entity )->symbol()->type() );
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

  // labeling
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  found.clear();
  QVERIFY( p.accept( &visitor ) );

  QCOMPARE( found, QStringList() << QStringLiteral( "enter: %1 vl2" ).arg( vl2->id() )
            << QStringLiteral( "enter: %1 " ).arg( rule2->ruleKey() )
            << QStringLiteral( "symbol:   #00ff00" )
            << QStringLiteral( "enter: %1 " ).arg( rule3->ruleKey() )
            << QStringLiteral( "symbol:   #00ffff" )
            << QStringLiteral( "exit: %1 " ).arg( rule3->ruleKey() )
            << QStringLiteral( "exit: %1 " ).arg( rule2->ruleKey() )
            << QStringLiteral( "exit: %1 vl2" ).arg( vl2->id() )
            << QStringLiteral( "enter: %1 vl" ).arg( vl->id() )
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

  QgsLayoutItemLegend *legend = new QgsLayoutItemLegend( l );
  l->addLayoutItem( legend );
  const QgsLegendPatchShape shape( Qgis::SymbolType::Marker, QgsGeometry::fromWkt( QStringLiteral( "Point( 3 4)" ) ) );
  qobject_cast< QgsLayerTreeLayer * >( legend->model()->index2node( legend->model()->index( 0, 0 ) ) )->setPatchShape( shape );
  const QList<QgsLayerTreeModelLegendNode *> layerLegendNodes = legend->model()->layerLegendNodes( qobject_cast< QgsLayerTreeLayer * >( legend->model()->index2node( legend->model()->index( 1, 0 ) ) ), false );
  const QgsLegendPatchShape shape2( Qgis::SymbolType::Marker, QgsGeometry::fromWkt( QStringLiteral( "Point( 13 14)" ) ) );
  QCOMPARE( qobject_cast< QgsLayerTreeLayer * >( legend->model()->index2node( legend->model()->index( 1, 0 ) ) )->layer()->name(), QStringLiteral( "vl2" ) );
  QgsMapLayerLegendUtils::setLegendNodePatchShape( qobject_cast< QgsLayerTreeLayer * >( legend->model()->index2node( legend->model()->index( 1, 0 ) ) ), 1, shape2 );
  legend->model()->refreshLayerLegend( qobject_cast< QgsLayerTreeLayer * >( legend->model()->index2node( legend->model()->index( 1, 0 ) ) ) );

  p.layoutManager()->addLayout( l );

  found.clear();
  QVERIFY( p.accept( &visitor ) );

  QCOMPARE( found, QStringList()
            << QStringLiteral( "enter: %1 rl" ).arg( rl->id() )
            << QStringLiteral( "ramp:   #ffff00" )
            << QStringLiteral( "exit: %1 rl" ).arg( rl->id() )
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
            << QStringLiteral( "labels:   Class" )
            << QStringLiteral( "exit: %1 vl" ).arg( vl->id() )
            << QStringLiteral( "enter: layouts Layouts" )
            << QStringLiteral( "enter: layout test layout" )
            << QStringLiteral( "patch: <Legend> %1 Point (3 4)" ).arg( legend->uuid() )
            << QStringLiteral( "patch: <Legend> %1 Point (13 14)" ).arg( legend->uuid() )
            << QStringLiteral( "text format: <Scalebar> %1 QGIS Vera Sans" ).arg( scalebar->uuid() )
            << QStringLiteral( "symbol: Page page #ffffff" )
            << QStringLiteral( "exit: layout test layout" )
            << QStringLiteral( "exit: layouts Layouts" )
          );

  p.removeMapLayer( vl2 );

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
            << QStringLiteral( "patch: <Legend> %1 Point (3 4)" ).arg( legend->uuid() )
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

void TestStyle::testColorRampShaderClassificationEqualInterval()
{
  // Test Type::Interpolated and ClassificationMode::EqualInterval
  {
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 255.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), QgsColorRampShader::Type::Interpolated, QgsColorRampShader::ClassificationMode::EqualInterval ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 0, QColor( 0,  255,  0 ),  "0" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 63.75, QColor( 0,  191,  64 ),  "63.8" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 127.5, QColor( 0,  128,  128 ),  "128" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 191.25, QColor( 0,  64,  191 ),  "191" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 255, QColor( 0,  0,  255 ),  "255" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

  // Test Type::Exact and ClassificationMode::EqualInterval
  {
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 255.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), QgsColorRampShader::Type::Exact, QgsColorRampShader::ClassificationMode::EqualInterval ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 0, QColor( 0,  255,  0 ),  "0" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 63.75, QColor( 0,  191,  64 ),  "63.8" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 127.5, QColor( 0,  128,  128 ),  "128" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 191.25, QColor( 0,  64,  191 ),  "191" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 255, QColor( 0,  0,  255 ),  "255" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

  // Test Type::Discrete and ClassificationMode::EqualInterval
  {
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 255.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), QgsColorRampShader::Type::Discrete, QgsColorRampShader::ClassificationMode::EqualInterval ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 51, QColor( 0,  255,  0 ),  "51" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 102, QColor( 0,  191,  64 ),  "102" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 153, QColor( 0,  128,  128 ),  "153" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 204, QColor( 0,  64,  191 ),  "204" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( qInf(), QColor( 0,  0,  255 ),  "inf" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

  // Test when min == max for EqualInterval mode
  for ( int i = 0; i < 3; ++i )
  {
    const QgsColorRampShader::Type type = static_cast<QgsColorRampShader::Type>( i );
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 0.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), type, QgsColorRampShader::ClassificationMode::EqualInterval ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 0, QColor( 0,  255,  0 ),  "0" ) );
    if ( type == QgsColorRampShader::Type::Discrete )
      itemsList2.append( QgsColorRampShader::ColorRampItem( qInf(), QColor( 0,  0,  255 ),  "inf" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

}

void TestStyle::testColorRampShaderClassificationContinius()
{
  // Test Type::Interpolated and ClassificationMode::Continuous
  {
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 255.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), QgsColorRampShader::Type::Interpolated, QgsColorRampShader::ClassificationMode::Continuous ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 0, QColor( 0,  255,  0 ),  "0" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 255, QColor( 0,  0,  255 ),  "255" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

  // Test Type::Exact and ClassificationMode::Continuous
  {
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 255.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), QgsColorRampShader::Type::Exact, QgsColorRampShader::ClassificationMode::Continuous ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 0, QColor( 0,  255,  0 ),  "0" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( 255, QColor( 0,  0,  255 ),  "255" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

  // Test Type::Discrete and ClassificationMode::Continuous
  {
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 255.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), QgsColorRampShader::Type::Discrete, QgsColorRampShader::ClassificationMode::Continuous ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 127.5, QColor( 0,  255,  0 ),  "128" ) );
    itemsList2.append( QgsColorRampShader::ColorRampItem( qInf(), QColor( 0,  0,  255 ),  "inf" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }

  // Test when min == max for Continuous mode
  for ( int i = 0; i < 3; ++i )
  {
    const QgsColorRampShader::Type type = static_cast<QgsColorRampShader::Type>( i );
    std::unique_ptr<QgsColorRampShader> shader( new QgsColorRampShader( 0.0, 0.0, new QgsGradientColorRamp( Qt::green, Qt::blue ), type, QgsColorRampShader::ClassificationMode::Continuous ) );
    shader->classifyColorRamp( 5, -1 );

    QList<QgsColorRampShader::ColorRampItem> itemsList = shader->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem> itemsList2;
    itemsList2.append( QgsColorRampShader::ColorRampItem( 0, QColor( 0,  255,  0 ),  "0" ) );
    if ( type == QgsColorRampShader::Type::Discrete )
      itemsList2.append( QgsColorRampShader::ColorRampItem( qInf(), QColor( 0,  0,  255 ),  "inf" ) );

    QVERIFY( compareItemLists( itemsList, itemsList2 ) );
  }
}

void TestStyle::testDefaultLabelTextFormat()
{
  // no "Default" text format yet
  QVERIFY( !QgsStyle::defaultStyle()->textFormat( QStringLiteral( "Default" ) ).isValid() );

  const QgsPalLayerSettings settings;
  // should be app-wide default font (gross!)
  QCOMPARE( settings.format().font().family(), QFont().family() );

  // now add a default text format
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont() );
  format.buffer().setEnabled( true );
  QVERIFY( QgsStyle::defaultStyle()->addTextFormat( QStringLiteral( "Default" ), format ) );

  // re-create default label settings
  const QgsPalLayerSettings settings2;
  // should be default text format now, not app default font
  QCOMPARE( settings2.format().font().family(),  QgsFontUtils::getStandardTestFont().family() );
  QVERIFY( settings2.format().buffer().enabled() );
}

QGSTEST_MAIN( TestStyle )
#include "testqgsstyle.moc"
