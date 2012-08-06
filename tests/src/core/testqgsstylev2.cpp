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

#include "qgsstylev2.h"

/** \ingroup UnitTests
 * This is a unit test to verify that styles are working correctly
 */
class TestStyleV2: public QObject
{
    Q_OBJECT;

  private:

    QgsStyleV2 *mStyle;

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
};


// slots
void TestStyleV2::initTestCase()
{
  // initialize with test directory so we don't mess with user's stuff
  QgsApplication::init( QDir::homePath() + QString( "/.qgis_test" ) );
  QgsApplication::initQgis();

  // output test environment
  QgsApplication::showSettings();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QuantumGIS" );
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
  QgsVectorGradientColorRampV2::StopsMap stops;
  stops[ 0.5 ] = QColor( Qt::white );
  gradientRamp->setStops( stops );
  QVERIFY( mStyle->addColorRamp( "test_gradient", gradientRamp ) == true );

  // random ramp
  QgsVectorRandomColorRampV2* randomRamp = new QgsVectorRandomColorRampV2();
  QVERIFY( mStyle->addColorRamp( "test_random", randomRamp ) == true );

  // color brewer ramp
  QgsVectorColorBrewerColorRampV2* cb1Ramp = new QgsVectorColorBrewerColorRampV2();
  QVERIFY( mStyle->addColorRamp( "test_cb1", cb1Ramp ) == true );
  QgsVectorColorBrewerColorRampV2* cb2Ramp = new QgsVectorColorBrewerColorRampV2( "RdYlGn", 6 );
  QVERIFY( mStyle->addColorRamp( "test_cb2", cb2Ramp ) == true );

  // cpt-city ramp - use gradients that are free to distribute
  // set base dir because we are using a test home path - change this if we distribute a minimal set with qgis
  QgsCptCityColorRampV2::setBaseDir( QDir::homePath() + QString( "/.qgis/cpt-city" ) );
  QgsCptCityColorRampV2::loadSchemes( "" );
  if ( QgsCptCityColorRampV2::hasBasicSchemes() )
  {
    QgsCptCityColorRampV2* cc1Ramp = new QgsCptCityColorRampV2( "jjg/misc/temperature", "" );
    QVERIFY( mStyle->addColorRamp( "test_cc1", cc1Ramp ) == true );
    QgsCptCityColorRampV2* cc2Ramp = new QgsCptCityColorRampV2( "cb/div/PiYG", "_10" );
    QVERIFY( mStyle->addColorRamp( "test_cc2", cc2Ramp ) == true );
  }
  else
  {
    QWARN( "cpt-city support files not found - skipping cpt-city color ramp tests" );
  }
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
  if ( QgsCptCityColorRampV2::hasAllSchemes() )
  {
    colorRampsTest << "test_cc1";
    colorTests.insert( "test_cc1", qMakePair( 0.25, QColor( "#466fcf" ) ) );
    colorTests.insert( "test_cc1", qMakePair( 0.66, QColor( "#dbc85b" ) ) );
    colorRampsTest << "test_cc2";
    colorTests.insert( "test_cc2", qMakePair( 0.25, QColor( "#de77ae" ) ) );
    colorTests.insert( "test_cc2", qMakePair( 0.66, QColor( "#b8e186" ) ) );
  }

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
  mStyle->save();
  mStyle->clear();
  mStyle->load( QgsApplication::userStyleV2Path() );

  QStringList colorRamps = mStyle->colorRampNames();
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
}


QTEST_MAIN( TestStyleV2 )
#include "moc_testqgsstylev2.cxx"
