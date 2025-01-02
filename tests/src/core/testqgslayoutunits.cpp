/***************************************************************************
                         testqgslayoutunits.cpp
                         -----------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgstest.h"

#include "qgsunittypes.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutpoint.h"
#include "qgslayoutsize.h"
#include "qgslayoutmeasurementconverter.h"
#include "qgis.h"

class TestQgsLayoutUnits : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsLayoutUnits()
      : QgsTest( QStringLiteral( "Layout Units Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    //QgsLayoutUnits
    void encodeDecode(); //test encoding and decoding layout units

    //QgsLayoutMeasurement
    void create();                  //test creating new measurement
    void gettersSetters();          //test getting/setting properties
    void copyConstructor();         //test copy constructor
    void equality();                //test measurement equality
    void assignment();              //test measurement assignment
    void operators();               //test measurement operators
    void unitTypes();               //test unit types
    void measurementEncodeDecode(); //test encoding and decoding measurement

    //QgsLayoutSize
    void createSize();          //test creating new layout size
    void sizeGettersSetters();  //test getting/setting properties
    void sizeCopyConstructor(); //test copy constructor
    void sizeEquality();        //test size equality
    void sizeAssignment();      //test size assignment
    void sizeOperators();       //test size operators
    void isEmpty();             //test isEmpty method
    void toQSizeF();            //test conversion to QSizeF
    void sizeEncodeDecode();    //test encoding and decoding size

    //QgsLayoutPoint
    void createPoint();          //test creating new layout point
    void pointGettersSetters();  //test getting/setting properties
    void pointCopyConstructor(); //test copy constructor
    void pointEquality();        //test point equality
    void pointAssignment();      //test point assignment
    void pointOperators();       //test point operators
    void isNull();               //test isEmpty method
    void toQPointF();            //test conversion to QPointF
    void pointEncodeDecode();    //test encoding and decoding point

    void converterCreate();         //test creating new converter
    void converterCopy();           //test creating new converter using copy constructor
    void converterGettersSetters(); //test getting/setting converter properties
    void conversionToMM();          //test conversion to mm
    void conversionToCM();          //test conversion to cm
    void conversionToM();           //test conversion to m
    void conversionToInches();      //test conversion to inches
    void conversionToFeet();        //test conversion to feet
    void conversionToPoints();      //test conversion to points
    void conversionToPicas();       //test conversion to picas
    void conversionFromPixels();    //test conversion from pixels and handling of dpi
    void conversionToPixels();      //test conversion to pixels and handling of dpi
    void sizeConversion();          //test conversion of QgsLayoutSizes
    void pointConversion();         //test conversion of QgsLayoutPoint

  private:
};

void TestQgsLayoutUnits::initTestCase()
{
}

void TestQgsLayoutUnits::cleanupTestCase()
{
}

void TestQgsLayoutUnits::init()
{
}

void TestQgsLayoutUnits::cleanup()
{
}

void TestQgsLayoutUnits::encodeDecode()
{
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Millimeters ) ), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Centimeters ) ), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Meters ) ), Qgis::LayoutUnit::Meters );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Inches ) ), Qgis::LayoutUnit::Inches );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Feet ) ), Qgis::LayoutUnit::Feet );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Points ) ), Qgis::LayoutUnit::Points );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Picas ) ), Qgis::LayoutUnit::Picas );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( Qgis::LayoutUnit::Pixels ) ), Qgis::LayoutUnit::Pixels );
}

void TestQgsLayoutUnits::create()
{
  const QgsLayoutMeasurement defaultUnits( 5.0 );
  //default units should be millimeters
  QCOMPARE( defaultUnits.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( defaultUnits.length(), 5.0 );

  //test overriding default unit
  const QgsLayoutMeasurement inches( 7.0, Qgis::LayoutUnit::Inches );
  QCOMPARE( inches.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( inches.length(), 7.0 );
}

void TestQgsLayoutUnits::gettersSetters()
{
  QgsLayoutMeasurement measurement( 5.0 );

  measurement.setLength( 9.0 );
  QCOMPARE( measurement.length(), 9.0 );
  measurement.setUnits( Qgis::LayoutUnit::Inches );
  QCOMPARE( measurement.units(), Qgis::LayoutUnit::Inches );
}

void TestQgsLayoutUnits::copyConstructor()
{
  const QgsLayoutMeasurement source( 6.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement dest( source );
  QCOMPARE( source, dest );
}

void TestQgsLayoutUnits::equality()
{
  const QgsLayoutMeasurement m1( 6.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement m2( 6.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement m3( 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement m4( 6.0, Qgis::LayoutUnit::Points );

  QVERIFY( m1 == m2 );
  QVERIFY( !( m1 == m3 ) );
  QVERIFY( !( m1 == m4 ) );
  QVERIFY( !( m3 == m4 ) );
  QVERIFY( !( m1 != m2 ) );
  QVERIFY( m1 != m3 );
  QVERIFY( m1 != m4 );
  QVERIFY( m3 != m4 );
}

void TestQgsLayoutUnits::assignment()
{
  const QgsLayoutMeasurement m1( 6.0, Qgis::LayoutUnit::Inches );
  QgsLayoutMeasurement m2( 9.0, Qgis::LayoutUnit::Points );
  m2 = m1;
  QCOMPARE( m2, m1 );
}

void TestQgsLayoutUnits::operators()
{
  // +
  const QgsLayoutMeasurement m1( 6.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement m2 = m1 + 3.0;
  QCOMPARE( m2.units(), m1.units() );
  QCOMPARE( m2.length(), 9.0 );

  // +=
  QgsLayoutMeasurement m4( 6.0, Qgis::LayoutUnit::Inches );
  m4 += 2.0;
  QCOMPARE( m4.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( m4.length(), 8.0 );

  // -
  const QgsLayoutMeasurement m3 = m1 - 3.0;
  QCOMPARE( m3.units(), m1.units() );
  QCOMPARE( m3.length(), 3.0 );

  // -=
  QgsLayoutMeasurement m5( 6.0, Qgis::LayoutUnit::Inches );
  m5 -= 2.0;
  QCOMPARE( m5.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( m5.length(), 4.0 );

  // *
  const QgsLayoutMeasurement m6 = m1 * 3.0;
  QCOMPARE( m6.units(), m1.units() );
  QCOMPARE( m6.length(), 18.0 );

  // *=
  QgsLayoutMeasurement m7( 6.0, Qgis::LayoutUnit::Inches );
  m7 *= 2.0;
  QCOMPARE( m7.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( m7.length(), 12.0 );

  // /
  const QgsLayoutMeasurement m8 = m1 / 3.0;
  QCOMPARE( m8.units(), m1.units() );
  QCOMPARE( m8.length(), 2.0 );

  // /=
  QgsLayoutMeasurement m9( 6.0, Qgis::LayoutUnit::Inches );
  m9 /= 2.0;
  QCOMPARE( m9.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( m9.length(), 3.0 );
}

void TestQgsLayoutUnits::unitTypes()
{
  QCOMPARE( QgsUnitTypes::unitType( Qgis::LayoutUnit::Centimeters ), Qgis::LayoutUnitType::PaperUnits );
  QCOMPARE( QgsUnitTypes::unitType( Qgis::LayoutUnit::Pixels ), Qgis::LayoutUnitType::ScreenUnits );
}

void TestQgsLayoutUnits::measurementEncodeDecode()
{
  const QgsLayoutMeasurement original( 6.0, Qgis::LayoutUnit::Pixels );
  QgsLayoutMeasurement result = QgsLayoutMeasurement::decodeMeasurement( original.encodeMeasurement() );
  QCOMPARE( original, result );

  //test with bad string
  result = QgsLayoutMeasurement::decodeMeasurement( QStringLiteral( "1" ) );
  QCOMPARE( result, QgsLayoutMeasurement( 0 ) );
}

void TestQgsLayoutUnits::createSize()
{
  const QgsLayoutSize defaultUnits( 5.0, 6.0 );
  //default units should be millimeters
  QCOMPARE( defaultUnits.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( defaultUnits.width(), 5.0 );
  QCOMPARE( defaultUnits.height(), 6.0 );

  //test overriding default unit
  const QgsLayoutSize inches( 7.0, 8.0, Qgis::LayoutUnit::Inches );
  QCOMPARE( inches.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( inches.width(), 7.0 );
  QCOMPARE( inches.height(), 8.0 );

  //test empty constructor
  const QgsLayoutSize empty( Qgis::LayoutUnit::Pixels );
  QCOMPARE( empty.units(), Qgis::LayoutUnit::Pixels );
  QCOMPARE( empty.width(), 0.0 );
  QCOMPARE( empty.height(), 0.0 );

  //test constructing from QSizeF
  const QgsLayoutSize fromQSizeF( QSizeF( 17.0, 18.0 ), Qgis::LayoutUnit::Inches );
  QCOMPARE( fromQSizeF.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( fromQSizeF.width(), 17.0 );
  QCOMPARE( fromQSizeF.height(), 18.0 );
}

void TestQgsLayoutUnits::sizeGettersSetters()
{
  QgsLayoutSize size( 5.0, 6.0, Qgis::LayoutUnit::Picas );

  size.setWidth( 9.0 );
  size.setHeight( 10.0 );
  QCOMPARE( size.width(), 9.0 );
  QCOMPARE( size.height(), 10.0 );
  QCOMPARE( size.units(), Qgis::LayoutUnit::Picas );

  size.setUnits( Qgis::LayoutUnit::Inches );
  QCOMPARE( size.width(), 9.0 );
  QCOMPARE( size.height(), 10.0 );
  QCOMPARE( size.units(), Qgis::LayoutUnit::Inches );

  size.setSize( 11.0, 12.0 );
  QCOMPARE( size.width(), 11.0 );
  QCOMPARE( size.height(), 12.0 );
  QCOMPARE( size.units(), Qgis::LayoutUnit::Inches );
}

void TestQgsLayoutUnits::sizeCopyConstructor()
{
  const QgsLayoutSize source( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutSize dest( source );
  QCOMPARE( source, dest );
}

void TestQgsLayoutUnits::sizeEquality()
{
  const QgsLayoutSize s1( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutSize s2( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutSize s3( 7.0, 8.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutSize s4( 6.0, 7.0, Qgis::LayoutUnit::Points );

  QVERIFY( s1 == s2 );
  QVERIFY( !( s1 == s3 ) );
  QVERIFY( !( s1 == s4 ) );
  QVERIFY( !( s3 == s4 ) );
  QVERIFY( !( s1 != s2 ) );
  QVERIFY( s1 != s3 );
  QVERIFY( s1 != s4 );
  QVERIFY( s3 != s4 );
}

void TestQgsLayoutUnits::sizeAssignment()
{
  const QgsLayoutSize s1( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  QgsLayoutSize s2( 9.0, 10.0, Qgis::LayoutUnit::Points );
  s2 = s1;
  QCOMPARE( s2, s1 );
}

void TestQgsLayoutUnits::sizeOperators()
{
  const QgsLayoutSize s1( 6.0, 12.0, Qgis::LayoutUnit::Inches );

  // *
  const QgsLayoutSize s2 = s1 * 3.0;
  QCOMPARE( s2.units(), s1.units() );
  QCOMPARE( s2.width(), 18.0 );
  QCOMPARE( s2.height(), 36.0 );

  // *=
  QgsLayoutSize s3( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  s3 *= 2.0;
  QCOMPARE( s3.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( s3.width(), 12.0 );
  QCOMPARE( s3.height(), 24.0 );

  // /
  const QgsLayoutSize s4 = s1 / 3.0;
  QCOMPARE( s4.units(), s1.units() );
  QCOMPARE( s4.width(), 2.0 );
  QCOMPARE( s4.height(), 4.0 );

  // /=
  QgsLayoutSize s5( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  s5 /= 2.0;
  QCOMPARE( s5.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( s5.width(), 3.0 );
  QCOMPARE( s5.height(), 6.0 );
}

void TestQgsLayoutUnits::isEmpty()
{
  QgsLayoutSize size( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  QVERIFY( !size.isEmpty() );
  size.setSize( 0, 0 );
  QVERIFY( size.isEmpty() );
  const QgsLayoutSize size2( Qgis::LayoutUnit::Millimeters ); //test empty constructor
  QVERIFY( size2.isEmpty() );
}

void TestQgsLayoutUnits::toQSizeF()
{
  const QgsLayoutSize size( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  const QSizeF converted = size.toQSizeF();
  QCOMPARE( converted.width(), size.width() );
  QCOMPARE( converted.height(), size.height() );
}

void TestQgsLayoutUnits::sizeEncodeDecode()
{
  const QgsLayoutSize original( 6.0, 12.0, Qgis::LayoutUnit::Points );
  QgsLayoutSize result = QgsLayoutSize::decodeSize( original.encodeSize() );
  QCOMPARE( original, result );

  //test with bad string
  result = QgsLayoutSize::decodeSize( QStringLiteral( "1,2" ) );
  QCOMPARE( result, QgsLayoutSize() );

  //test with nan values
  result = QgsLayoutSize::decodeSize( QStringLiteral( "nan,2,mm" ) );
  QCOMPARE( result, QgsLayoutSize() );
  result = QgsLayoutSize::decodeSize( QStringLiteral( "2,nan,mm" ) );
  QCOMPARE( result, QgsLayoutSize() );
}

void TestQgsLayoutUnits::createPoint()
{
  const QgsLayoutPoint defaultUnits( 5.0, 6.0 );
  //default units should be millimeters
  QCOMPARE( defaultUnits.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( defaultUnits.x(), 5.0 );
  QCOMPARE( defaultUnits.y(), 6.0 );

  //test overriding default unit
  const QgsLayoutPoint inches( 7.0, 8.0, Qgis::LayoutUnit::Inches );
  QCOMPARE( inches.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( inches.x(), 7.0 );
  QCOMPARE( inches.y(), 8.0 );

  //test empty constructor
  const QgsLayoutPoint empty( Qgis::LayoutUnit::Pixels );
  QCOMPARE( empty.units(), Qgis::LayoutUnit::Pixels );
  QCOMPARE( empty.x(), 0.0 );
  QCOMPARE( empty.y(), 0.0 );

  //test constructing from QPointF
  const QgsLayoutPoint fromQPointF( QPointF( 17.0, 18.0 ), Qgis::LayoutUnit::Inches );
  QCOMPARE( fromQPointF.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( fromQPointF.x(), 17.0 );
  QCOMPARE( fromQPointF.y(), 18.0 );
}

void TestQgsLayoutUnits::pointGettersSetters()
{
  QgsLayoutPoint point( 5.0, 6.0, Qgis::LayoutUnit::Picas );

  point.setX( 9.0 );
  point.setY( 10.0 );
  QCOMPARE( point.x(), 9.0 );
  QCOMPARE( point.y(), 10.0 );
  QCOMPARE( point.units(), Qgis::LayoutUnit::Picas );

  point.setUnits( Qgis::LayoutUnit::Inches );
  QCOMPARE( point.x(), 9.0 );
  QCOMPARE( point.y(), 10.0 );
  QCOMPARE( point.units(), Qgis::LayoutUnit::Inches );

  point.setPoint( 11.0, 12.0 );
  QCOMPARE( point.x(), 11.0 );
  QCOMPARE( point.y(), 12.0 );
  QCOMPARE( point.units(), Qgis::LayoutUnit::Inches );
}

void TestQgsLayoutUnits::pointCopyConstructor()
{
  const QgsLayoutPoint source( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutPoint dest( source );
  QCOMPARE( source, dest );
}

void TestQgsLayoutUnits::pointEquality()
{
  const QgsLayoutPoint p1( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutPoint p2( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutPoint p3( 7.0, 8.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutPoint p4( 6.0, 7.0, Qgis::LayoutUnit::Points );

  QVERIFY( p1 == p2 );
  QVERIFY( !( p1 == p3 ) );
  QVERIFY( !( p1 == p4 ) );
  QVERIFY( !( p3 == p4 ) );
  QVERIFY( !( p1 != p2 ) );
  QVERIFY( p1 != p3 );
  QVERIFY( p1 != p4 );
  QVERIFY( p3 != p4 );
}

void TestQgsLayoutUnits::pointAssignment()
{
  const QgsLayoutPoint p1( 6.0, 7.0, Qgis::LayoutUnit::Inches );
  QgsLayoutPoint p2( 9.0, 10.0, Qgis::LayoutUnit::Points );
  p2 = p1;
  QCOMPARE( p2, p1 );
}

void TestQgsLayoutUnits::pointOperators()
{
  const QgsLayoutPoint p1( 6.0, 12.0, Qgis::LayoutUnit::Inches );

  // *
  const QgsLayoutPoint p2 = p1 * 3.0;
  QCOMPARE( p2.units(), p1.units() );
  QCOMPARE( p2.x(), 18.0 );
  QCOMPARE( p2.y(), 36.0 );

  // *=
  QgsLayoutPoint p3( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  p3 *= 2.0;
  QCOMPARE( p3.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( p3.x(), 12.0 );
  QCOMPARE( p3.y(), 24.0 );

  // /
  const QgsLayoutPoint p4 = p1 / 3.0;
  QCOMPARE( p4.units(), p1.units() );
  QCOMPARE( p4.x(), 2.0 );
  QCOMPARE( p4.y(), 4.0 );

  // /=
  QgsLayoutPoint p5( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  p5 /= 2.0;
  QCOMPARE( p5.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( p5.x(), 3.0 );
  QCOMPARE( p5.y(), 6.0 );
}

void TestQgsLayoutUnits::isNull()
{
  QgsLayoutPoint point( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  QVERIFY( !point.isNull() );
  point.setPoint( 0, 0 );
  QVERIFY( point.isNull() );
  const QgsLayoutPoint point2( Qgis::LayoutUnit::Millimeters ); //test empty constructor
  QVERIFY( point2.isNull() );
}

void TestQgsLayoutUnits::toQPointF()
{
  const QgsLayoutPoint point( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  const QPointF converted = point.toQPointF();
  QCOMPARE( converted.x(), point.x() );
  QCOMPARE( converted.y(), point.y() );
}

void TestQgsLayoutUnits::pointEncodeDecode()
{
  const QgsLayoutPoint original( 6.0, 12.0, Qgis::LayoutUnit::Inches );
  QgsLayoutPoint result = QgsLayoutPoint::decodePoint( original.encodePoint() );
  QCOMPARE( original, result );

  //test with bad string
  result = QgsLayoutPoint::decodePoint( QStringLiteral( "1,2" ) );
  QCOMPARE( result, QgsLayoutPoint() );

  //test with nan values
  result = QgsLayoutPoint::decodePoint( QStringLiteral( "nan,2,mm" ) );
  QCOMPARE( result, QgsLayoutPoint() );
  result = QgsLayoutPoint::decodePoint( QStringLiteral( "2,nan,mm" ) );
  QCOMPARE( result, QgsLayoutPoint() );
  result = QgsLayoutPoint::decodePoint( QStringLiteral( "9.99999e+06,1e+07,mm" ) );
  QCOMPARE( result, QgsLayoutPoint() );
}

void TestQgsLayoutUnits::converterCreate()
{
  QgsLayoutMeasurementConverter *converter = new QgsLayoutMeasurementConverter();
  QVERIFY( converter );
  delete converter;
}

void TestQgsLayoutUnits::converterCopy()
{
  QgsLayoutMeasurementConverter source;
  source.setDpi( 96.0 );
  const QgsLayoutMeasurementConverter copy( source );
  QCOMPARE( copy.dpi(), source.dpi() );
}

void TestQgsLayoutUnits::conversionToMM()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Millimeters );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Millimeters );

  QCOMPARE( convertedFromMillimeters.length(), 1.0 );
  QCOMPARE( convertedFromCentimeters.length(), 10.0 );
  QCOMPARE( convertedFromMeters.length(), 1000.0 );
  QCOMPARE( convertedFromInches.length(), 25.4 );
  QCOMPARE( convertedFromFeet.length(), 304.8 );
  QCOMPARE( convertedFromPoints.length(), 0.352777778 );
  QCOMPARE( convertedFromPicas.length(), 4.23333333 );
}

void TestQgsLayoutUnits::conversionToCM()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Centimeters );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Centimeters );

  QCOMPARE( convertedFromMillimeters.length(), 0.1 );
  QCOMPARE( convertedFromCentimeters.length(), 1.0 );
  QCOMPARE( convertedFromMeters.length(), 100.0 );
  QCOMPARE( convertedFromInches.length(), 2.54 );
  QCOMPARE( convertedFromFeet.length(), 30.48 );
  QCOMPARE( convertedFromPoints.length(), 0.0352777778 );
  QCOMPARE( convertedFromPicas.length(), 0.423333333 );
}

void TestQgsLayoutUnits::conversionToM()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Meters );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Meters );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Meters );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Meters );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Meters );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Meters );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Meters );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Meters );

  QCOMPARE( convertedFromMillimeters.length(), 0.001 );
  QCOMPARE( convertedFromCentimeters.length(), 0.01 );
  QCOMPARE( convertedFromMeters.length(), 1.0 );
  QCOMPARE( convertedFromInches.length(), 0.0254 );
  QCOMPARE( convertedFromFeet.length(), 0.3048 );
  QCOMPARE( convertedFromPoints.length(), 0.000352777778 );
  QCOMPARE( convertedFromPicas.length(), 0.00423333333 );
}

void TestQgsLayoutUnits::conversionToInches()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Inches );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Inches );

  QGSCOMPARENEAR( convertedFromMillimeters.length(), 0.0393701, 0.000001 );
  QGSCOMPARENEAR( convertedFromCentimeters.length(), 0.3937008, 0.000001 );
  QGSCOMPARENEAR( convertedFromMeters.length(), 39.3700787, 0.000001 );
  QCOMPARE( convertedFromInches.length(), 1.0 );
  QCOMPARE( convertedFromFeet.length(), 12.0 );
  QGSCOMPARENEAR( convertedFromPoints.length(), 0.0138888889, 0.000001 );
  QGSCOMPARENEAR( convertedFromPicas.length(), 0.166666667, 0.000001 );
}

void TestQgsLayoutUnits::conversionToFeet()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Feet );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Feet );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Feet );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Feet );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Feet );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Feet );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Feet );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Feet );

  QGSCOMPARENEAR( convertedFromMillimeters.length(), 0.0032808399, 0.000001 );
  QGSCOMPARENEAR( convertedFromCentimeters.length(), 0.032808399, 0.000001 );
  QGSCOMPARENEAR( convertedFromMeters.length(), 3.2808399, 0.000001 );
  QGSCOMPARENEAR( convertedFromInches.length(), 0.0833333, 0.000001 );
  QCOMPARE( convertedFromFeet.length(), 1.0 );
  QGSCOMPARENEAR( convertedFromPoints.length(), 0.00115740741, 0.000001 );
  QGSCOMPARENEAR( convertedFromPicas.length(), 0.0138888889, 0.000001 );
}

void TestQgsLayoutUnits::conversionToPoints()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Points );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Points );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Points );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Points );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Points );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Points );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Points );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Points );

  QGSCOMPARENEAR( convertedFromMillimeters.length(), 2.83464567, 0.000001 );
  QGSCOMPARENEAR( convertedFromCentimeters.length(), 28.3464567, 0.000001 );
  QGSCOMPARENEAR( convertedFromMeters.length(), 2834.64567, 0.000001 );
  QGSCOMPARENEAR( convertedFromInches.length(), 72.0, 0.000001 );
  QGSCOMPARENEAR( convertedFromFeet.length(), 864.0, 0.000001 );
  QCOMPARE( convertedFromPoints.length(), 1.0 );
  QGSCOMPARENEAR( convertedFromPicas.length(), 12.0, 0.000001 );
}

void TestQgsLayoutUnits::conversionToPicas()
{
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, Qgis::LayoutUnit::Millimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, Qgis::LayoutUnit::Meters );
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, Qgis::LayoutUnit::Feet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, Qgis::LayoutUnit::Points );
  const QgsLayoutMeasurement measurementInPicas( 1.0, Qgis::LayoutUnit::Picas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, Qgis::LayoutUnit::Picas );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, Qgis::LayoutUnit::Picas );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, Qgis::LayoutUnit::Picas );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, Qgis::LayoutUnit::Picas );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, Qgis::LayoutUnit::Picas );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, Qgis::LayoutUnit::Picas );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, Qgis::LayoutUnit::Picas );

  QCOMPARE( convertedFromMillimeters.units(), Qgis::LayoutUnit::Picas );
  QCOMPARE( convertedFromCentimeters.units(), Qgis::LayoutUnit::Picas );
  QCOMPARE( convertedFromMeters.units(), Qgis::LayoutUnit::Picas );
  QCOMPARE( convertedFromInches.units(), Qgis::LayoutUnit::Picas );
  QCOMPARE( convertedFromFeet.units(), Qgis::LayoutUnit::Picas );
  QCOMPARE( convertedFromPoints.units(), Qgis::LayoutUnit::Picas );
  QCOMPARE( convertedFromPicas.units(), Qgis::LayoutUnit::Picas );

  QGSCOMPARENEAR( convertedFromMillimeters.length(), 0.236220472, 0.000001 );
  QGSCOMPARENEAR( convertedFromCentimeters.length(), 2.36220472, 0.000001 );
  QGSCOMPARENEAR( convertedFromMeters.length(), 236.220472, 0.000001 );
  QGSCOMPARENEAR( convertedFromInches.length(), 6.0, 0.000001 );
  QGSCOMPARENEAR( convertedFromFeet.length(), 72.0, 0.000001 );
  QGSCOMPARENEAR( convertedFromPoints.length(), 0.0833333333, 0.000001 );
  QCOMPARE( convertedFromPicas.length(), 1.0 );
}

void TestQgsLayoutUnits::converterGettersSetters()
{
  QgsLayoutMeasurementConverter converter;
  converter.setDpi( 96.0 );
  QCOMPARE( converter.dpi(), 96.0 );
}

void TestQgsLayoutUnits::conversionFromPixels()
{
  const QgsLayoutMeasurement measurementInPixels( 300.0, Qgis::LayoutUnit::Pixels );

  QgsLayoutMeasurementConverter converter;
  //try initially with 300 dpi
  converter.setDpi( 300.0 );
  QgsLayoutMeasurement convertedToInches = converter.convert( measurementInPixels, Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedToInches.units(), Qgis::LayoutUnit::Inches );
  QGSCOMPARENEAR( convertedToInches.length(), 1.0, 0.000001 );
  QgsLayoutMeasurement convertedToMM = converter.convert( measurementInPixels, Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedToMM.units(), Qgis::LayoutUnit::Millimeters );
  QGSCOMPARENEAR( convertedToMM.length(), 25.4, 0.000001 );

  //try with 96 dpi
  converter.setDpi( 96.0 );
  convertedToInches = converter.convert( measurementInPixels, Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedToInches.units(), Qgis::LayoutUnit::Inches );
  QGSCOMPARENEAR( convertedToInches.length(), 3.125, 0.000001 );
  convertedToMM = converter.convert( measurementInPixels, Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedToMM.units(), Qgis::LayoutUnit::Millimeters );
  QGSCOMPARENEAR( convertedToMM.length(), 79.375, 0.000001 );
}

void TestQgsLayoutUnits::conversionToPixels()
{
  const QgsLayoutMeasurement measurementInInches( 1.0, Qgis::LayoutUnit::Inches );
  const QgsLayoutMeasurement measurementInMM( 1.0, Qgis::LayoutUnit::Millimeters );

  QgsLayoutMeasurementConverter converter;
  //try initially with 300 dpi
  converter.setDpi( 300.0 );
  QgsLayoutMeasurement convertedToPixels = converter.convert( measurementInInches, Qgis::LayoutUnit::Pixels );
  QCOMPARE( convertedToPixels.units(), Qgis::LayoutUnit::Pixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 300.0, 0.000001 );
  convertedToPixels = converter.convert( measurementInMM, Qgis::LayoutUnit::Pixels );
  QCOMPARE( convertedToPixels.units(), Qgis::LayoutUnit::Pixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 11.811023622, 0.000001 );

  //try with 96 dpi
  converter.setDpi( 96.0 );
  convertedToPixels = converter.convert( measurementInInches, Qgis::LayoutUnit::Pixels );
  QCOMPARE( convertedToPixels.units(), Qgis::LayoutUnit::Pixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 96.0, 0.000001 );
  convertedToPixels = converter.convert( measurementInMM, Qgis::LayoutUnit::Pixels );
  QCOMPARE( convertedToPixels.units(), Qgis::LayoutUnit::Pixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 3.77952755906, 0.000001 );
}

void TestQgsLayoutUnits::sizeConversion()
{
  const QgsLayoutSize sizeInMM( 1.0, 2.0, Qgis::LayoutUnit::Millimeters );
  QgsLayoutMeasurementConverter converter;

  //test conversion to same units
  QgsLayoutSize convertedSize = converter.convert( sizeInMM, Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedSize, sizeInMM );

  //convert to other units
  convertedSize = converter.convert( sizeInMM, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutSize expectedSize( 0.1, 0.2, Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedSize, expectedSize );

  //pixel conversion
  converter.setDpi( 96.0 );
  const QgsLayoutSize convertedToInches = converter.convert( QgsLayoutSize( 96, 192, Qgis::LayoutUnit::Pixels ), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedToInches.width(), 1.0 );
  QCOMPARE( convertedToInches.height(), 2.0 );
  QCOMPARE( convertedToInches.units(), Qgis::LayoutUnit::Inches );
}

void TestQgsLayoutUnits::pointConversion()
{
  const QgsLayoutPoint pointInMM( 1.0, 2.0, Qgis::LayoutUnit::Millimeters );
  QgsLayoutMeasurementConverter converter;

  //test conversion to same units
  QgsLayoutPoint convertedPoint = converter.convert( pointInMM, Qgis::LayoutUnit::Millimeters );
  QCOMPARE( convertedPoint, pointInMM );

  //convert to other units
  convertedPoint = converter.convert( pointInMM, Qgis::LayoutUnit::Centimeters );
  const QgsLayoutPoint expectedPoint( 0.1, 0.2, Qgis::LayoutUnit::Centimeters );
  QCOMPARE( convertedPoint, expectedPoint );

  //pixel conversion
  converter.setDpi( 96.0 );
  const QgsLayoutPoint convertedToInches = converter.convert( QgsLayoutPoint( 96, 192, Qgis::LayoutUnit::Pixels ), Qgis::LayoutUnit::Inches );
  QCOMPARE( convertedToInches.x(), 1.0 );
  QCOMPARE( convertedToInches.y(), 2.0 );
  QCOMPARE( convertedToInches.units(), Qgis::LayoutUnit::Inches );
}

QTEST_MAIN( TestQgsLayoutUnits )
#include "testqgslayoutunits.moc"
