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
#include <QtTest>
#include <QObject>
#include <QStringList>
#include <QObject>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include <qgsapplication.h>
#include "qgsconfig.h"
#include "qgslogger.h"
#include "qgsvectorcolorrampv2.h"
#include "qgscptcityarchive.h"

#include "qgsstylev2.h"

/** \ingroup UnitTests
 * This is a unit test to verify that styles are working correctly
 */
class TestStyleV2: public QObject
{
    Q_OBJECT;

  private:

    QgsStyleV2 *mStyle;
    QString mTestDataDir;

    bool testValidColor( QgsVectorColorRampV2 *ramp, double value, QColor expected );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.
    // void initStyles();

    void testCreateColorRamps();
    void testLoadColorRamps();
    void testSaveLoad();
    void testParseColor();
    void testParseColorList();
};


// slots
void TestStyleV2::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDB();
  mTestDataDir = QString( TEST_DATA_DIR ) + QDir::separator(); //defined in CmakeLists.txt

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
}

void TestStyleV2::cleanupTestCase()
{
  // don't save
  // mStyle->save();
  delete mStyle;
}

bool TestStyleV2::testValidColor( QgsVectorColorRampV2 *ramp, double value, QColor expected )
{
  QColor result = ramp->color( value );
  if ( result != expected )
  {
    QWARN( QString( "value = %1 result = %2 expected = %3" ).arg( value ).arg(
             result.name() ).arg( expected.name() ).toLocal8Bit().data() );
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
  colorTests.insert( "test_gradient", qMakePair( 0.25, QColor( "#ff7f7f" ) ) );
  colorTests.insert( "test_gradient", qMakePair( 0.66, QColor( "#adadff" ) ) );
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
  colorTests.insert( "test_cc3", qMakePair( 0.25, QColor( "#7f7f7f" ) ) );
  colorTests.insert( "test_cc3", qMakePair( 0.66, QColor( "#ffad00" ) ) );

  QgsDebugMsg( "loaded colorRamps: " + colorRamps.join( " " ) );

  foreach ( QString name, colorRampsTest )
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

  foreach ( QString name, colorRampsTest )
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

void TestStyleV2::testParseColor()
{
  // values for color tests
  QMap< QString, QPair< QColor, bool> > colorTests;
  colorTests.insert( "bad color", qMakePair( QColor(), false ) );
  colorTests.insert( "red", qMakePair( QColor( 255, 0, 0 ), false ) );
  colorTests.insert( "#ff00ff", qMakePair( QColor( 255, 0, 255 ), false ) );
  colorTests.insert( "#99AA00", qMakePair( QColor( 153, 170, 0 ), false ) );
  colorTests.insert( "#GG0000", qMakePair( QColor(), false ) );
  colorTests.insert( "000000", qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( "00ff00", qMakePair( QColor( 0, 255, 0 ), false ) );
  colorTests.insert( "00gg00", qMakePair( QColor(), false ) );
  colorTests.insert( "00ff000", qMakePair( QColor(), false ) );
  colorTests.insert( "fff", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "fff0", qMakePair( QColor(), false ) );

  // hex rrggbbaa colors
  colorTests.insert( "#ff00ffaa", qMakePair( QColor( 255, 0, 255, 170 ), true ) );
  colorTests.insert( "#99AA0099", qMakePair( QColor( 153, 170, 0, 153 ), true ) );
  colorTests.insert( "#GG0000aa", qMakePair( QColor(), false ) );
  colorTests.insert( "00000000", qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( "00ff0011", qMakePair( QColor( 0, 255, 0, 17 ), true ) );
  colorTests.insert( "00gg0011", qMakePair( QColor(), false ) );
  colorTests.insert( "00ff00000", qMakePair( QColor(), false ) );

  colorTests.insert( "0,0,0", qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( "127,60,0", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "255,255,255", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "256,60,0", qMakePair( QColor(), false ) );
  colorTests.insert( "rgb(127,60,0)", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "rgb(255,255,255)", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "rgb(256,60,0)", qMakePair( QColor(), false ) );
  colorTests.insert( " rgb(  127, 60 ,  0 ) ", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "rgb(127,60,0);", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "(127,60,0);", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "(127,60,0)", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "127,060,000", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "0,0,0,0", qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( "127,60,0,0.5", qMakePair( QColor( 127, 60, 0, 128 ), true ) );
  colorTests.insert( "255,255,255,0.1", qMakePair( QColor( 255, 255, 255, 26 ), true ) );
  colorTests.insert( "rgba(127,60,0,1.0)", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "rgba(255,255,255,0.0)", qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( " rgba(  127, 60 ,  0  , 0.2 ) ", qMakePair( QColor( 127, 60, 0, 51 ), true ) );
  colorTests.insert( "rgba(127,60,0,0.1);", qMakePair( QColor( 127, 60, 0, 26 ), true ) );
  colorTests.insert( "(127,60,0,1);", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "(127,60,0,1.0)", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "127,060,000,1", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "0%,0%,0%", qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( "50 %,60 %,0 %", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "100%, 100%, 100%", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "rgb(50%,60%,0%)", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "rgb(100%, 100%, 100%)", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( " rgb(  50 % , 60 % ,  0  % ) ", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "rgb(50%,60%,0%);", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "(50%,60%,0%);", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "(50%,60%,0%)", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "050%,060%,000%", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "0%,0%,0%,0", qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( "50 %,60 %,0 %,0.5", qMakePair( QColor( 127, 153, 0, 128 ), true ) );
  colorTests.insert( "100%, 100%, 100%, 1.0", qMakePair( QColor( 255, 255, 255, 255 ), true ) );
  colorTests.insert( "rgba(50%,60%,0%, 1.0)", qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( "rgba(100%, 100%, 100%, 0.0)", qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( " rgba(  50 % , 60 % ,  0  %, 0.5 ) ", qMakePair( QColor( 127, 153, 0, 128 ), true ) );
  colorTests.insert( "rgba(50%,60%,0%,0);", qMakePair( QColor( 127, 153, 0, 0 ), true ) );
  colorTests.insert( "(50%,60%,0%,1);", qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( "(50%,60%,0%,1.0)", qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( "050%,060%,000%,0", qMakePair( QColor( 127, 153, 0, 0 ), true ) );

  QMap<QString, QPair< QColor, bool> >::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsg( "color string: " +  i.key() );
    bool hasAlpha = false;
    QColor result = QgsSymbolLayerV2Utils::parseColorWithAlpha( i.key(), hasAlpha );
    QVERIFY( result == i.value().first );
    QVERIFY( hasAlpha == i.value().second );
    ++i;
  }
}

void TestStyleV2::testParseColorList()
{
  //ensure that majority of single parseColor tests work for lists
  //note that some are not possible, as the colors may be ambiguous when treated as a list
  QMap< QString, QColor > colorTests;
  colorTests.insert( "bad color", QColor() );
  colorTests.insert( "red", QColor( 255, 0, 0 ) );
  colorTests.insert( "#ff00ff", QColor( 255, 0, 255 ) );
  colorTests.insert( "#99AA00", QColor( 153, 170, 0 ) );
  colorTests.insert( "#GG0000", QColor() );
  //colorTests.insert( "000000", QColor( 0, 0, 0 ) );
  //colorTests.insert( "00ff00", QColor( 0, 255, 0 ) );
  //colorTests.insert( "00gg00", QColor() );
  colorTests.insert( "00ff000", QColor() );
  //colorTests.insert( "fff", QColor( 255, 255, 255 ) );
  colorTests.insert( "fff0", QColor() );

  // hex rrggbbaa colors
  colorTests.insert( "#ff00ffaa", QColor( 255, 0, 255, 170 ) );
  colorTests.insert( "#99AA0099", QColor( 153, 170, 0, 153 ) );
  colorTests.insert( "#GG0000aa", QColor() );
  colorTests.insert( "00000000", QColor( 0, 0, 0, 0 ) );
  colorTests.insert( "00ff0011", QColor( 0, 255, 0, 17 ) );
  colorTests.insert( "00gg0011", QColor() );
  colorTests.insert( "00ff00000",  QColor() );

  colorTests.insert( "0,0,0", QColor( 0, 0, 0 ) );
  colorTests.insert( "127,60,0", QColor( 127, 60, 0 ) );
  colorTests.insert( "255,255,255", QColor( 255, 255, 255 ) );
  //colorTests.insert( "256,60,0", QColor() );
  colorTests.insert( "rgb(127,60,0)", QColor( 127, 60, 0 ) );
  colorTests.insert( "rgb(255,255,255)", QColor( 255, 255, 255 ) );
  colorTests.insert( "rgb(256,60,0)", QColor() );
  colorTests.insert( " rgb(  127, 60 ,  0 ) ", QColor( 127, 60, 0 ) );
  colorTests.insert( "rgb(127,60,0);", QColor( 127, 60, 0 ) );
  colorTests.insert( "(127,60,0);", QColor( 127, 60, 0 ) );
  colorTests.insert( "(127,60,0)", QColor( 127, 60, 0 ) );
  colorTests.insert( "127,060,000", QColor( 127, 60, 0 ) );
  colorTests.insert( "0,0,0,0", QColor( 0, 0, 0, 0 ) );
  colorTests.insert( "127,60,0,0.5", QColor( 127, 60, 0, 128 ) );
  colorTests.insert( "255,255,255,0.1", QColor( 255, 255, 255, 26 ) );
  colorTests.insert( "rgba(127,60,0,1.0)", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "rgba(255,255,255,0.0)", QColor( 255, 255, 255, 0 ) );
  colorTests.insert( " rgba(  127, 60 ,  0  , 0.2 ) ", QColor( 127, 60, 0, 51 ) );
  colorTests.insert( "rgba(127,60,0,0.1);", QColor( 127, 60, 0, 26 ) );
  colorTests.insert( "(127,60,0,1);", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "(127,60,0,1.0)", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "127,060,000,1", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "0%,0%,0%", QColor( 0, 0, 0 ) );
  colorTests.insert( "50 %,60 %,0 %", QColor( 127, 153, 0 ) );
  colorTests.insert( "100%, 100%, 100%", QColor( 255, 255, 255 ) );
  colorTests.insert( "rgb(50%,60%,0%)", QColor( 127, 153, 0 ) );
  colorTests.insert( "rgb(100%, 100%, 100%)", QColor( 255, 255, 255 ) );
  colorTests.insert( " rgb(  50 % , 60 % ,  0  % ) ", QColor( 127, 153, 0 ) );
  colorTests.insert( "rgb(50%,60%,0%);", QColor( 127, 153, 0 ) );
  colorTests.insert( "(50%,60%,0%);", QColor( 127, 153, 0 ) );
  colorTests.insert( "(50%,60%,0%)", QColor( 127, 153, 0 ) );
  colorTests.insert( "050%,060%,000%", QColor( 127, 153, 0 ) );
  colorTests.insert( "0%,0%,0%,0", QColor( 0, 0, 0, 0 ) );
  colorTests.insert( "50 %,60 %,0 %,0.5", QColor( 127, 153, 0, 128 ) );
  colorTests.insert( "100%, 100%, 100%, 1.0", QColor( 255, 255, 255, 255 ) );
  colorTests.insert( "rgba(50%,60%,0%, 1.0)", QColor( 127, 153, 0, 255 ) );
  colorTests.insert( "rgba(100%, 100%, 100%, 0.0)", QColor( 255, 255, 255, 0 ) );
  colorTests.insert( " rgba(  50 % , 60 % ,  0  %, 0.5 ) ", QColor( 127, 153, 0, 128 ) );
  colorTests.insert( "rgba(50%,60%,0%,0);", QColor( 127, 153, 0, 0 ) );
  colorTests.insert( "(50%,60%,0%,1);", QColor( 127, 153, 0, 255 ) );
  colorTests.insert( "(50%,60%,0%,1.0)", QColor( 127, 153, 0, 255 ) );
  colorTests.insert( "050%,060%,000%,0", QColor( 127, 153, 0, 0 ) );

  QMap<QString, QColor >::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsg( "color list string: " +  i.key() );
    QList< QColor > result = QgsSymbolLayerV2Utils::parseColorList( i.key() );
    if ( i.value().isValid() )
    {
      QCOMPARE( result.length(), 1 );
      QVERIFY( result.at( 0 ) == i.value() );
    }
    else
    {
      QCOMPARE( result.length(), 0 );
    }
    ++i;
  }

  QList< QPair< QString, QList<QColor> > > colorListTests;
  QList<QColor> list1;
  list1 << QColor( QString( "blue" ) ) << QColor( QString( "red" ) ) << QColor( QString( "green" ) );
  colorListTests.append( qMakePair( QString( "blue red green" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue,red,green" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue\nred\ngreen" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue\nred green" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue\nred,green" ), list1 ) );
  QList<QColor> list2;
  list2 << QColor( QString( "#ff0000" ) ) << QColor( QString( "#00ff00" ) ) << QColor( QString( "#0000ff" ) );
  colorListTests.append( qMakePair( QString( "#ff0000 #00ff00 #0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000,#00ff00,#0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000\n#00ff00\n#0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000\n#00ff00 #0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000\n#00ff00,#0000ff" ), list2 ) );
  QList<QColor> list3;
  list3 << QColor( QString( "#ff0000" ) ) << QColor( QString( "#00ff00" ) ) << QColor( QString( "#0000ff" ) );
  colorListTests.append( qMakePair( QString( "rgb(255,0,0) rgb(0,255,0) rgb(0,0,255)" ), list3 ) );
  colorListTests.append( qMakePair( QString( "rgb(255,0,0)\nrgb(0,255,0)\nrgb(0,0,255)" ), list3 ) );
  colorListTests.append( qMakePair( QString( "rgb(255,0,0)\nrgb(0,255,0) rgb(0,0,255)" ), list3 ) );

  QList< QPair< QString, QList<QColor> > >::const_iterator it = colorListTests.constBegin();
  while ( it != colorListTests.constEnd() )
  {
    QgsDebugMsg( "color list string: " + ( *it ).first );
    QList< QColor > result = QgsSymbolLayerV2Utils::parseColorList(( *it ).first );
    if (( *it ).second.length() > 0 )
    {
      QCOMPARE( result.length(), ( *it ).second.length() );
      int index = 0;
      for ( QList<QColor>::const_iterator colorIt = ( *it ).second.constBegin();  colorIt != ( *it ).second.constEnd(); ++colorIt )
      {
        QVERIFY( result.at( index ) == ( *colorIt ) );
        index++;
      }
    }
    else
    {
      QCOMPARE( result.length(), 0 );
    }
    ++it;
  }

}


QTEST_MAIN( TestStyleV2 )
#include "testqgsstylev2.moc"
