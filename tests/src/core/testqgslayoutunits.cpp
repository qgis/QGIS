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

class TestQgsLayoutUnits : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    //QgsLayoutUnits
    void encodeDecode(); //test encoding and decoding layout units

    //QgsLayoutMeasurement
    void create(); //test creating new measurement
    void gettersSetters(); //test getting/setting properties
    void copyConstructor(); //test copy constructor
    void equality(); //test measurement equality
    void assignment(); //test measurement assignment
    void operators(); //test measurement operators
    void unitTypes(); //test unit types
    void measurementEncodeDecode(); //test encoding and decoding measurement

    //QgsLayoutSize
    void createSize(); //test creating new layout size
    void sizeGettersSetters(); //test getting/setting properties
    void sizeCopyConstructor(); //test copy constructor
    void sizeEquality(); //test size equality
    void sizeAssignment(); //test size assignment
    void sizeOperators(); //test size operators
    void isEmpty(); //test isEmpty method
    void toQSizeF(); //test conversion to QSizeF
    void sizeEncodeDecode(); //test encoding and decoding size

    //QgsLayoutPoint
    void createPoint(); //test creating new layout point
    void pointGettersSetters(); //test getting/setting properties
    void pointCopyConstructor(); //test copy constructor
    void pointEquality(); //test point equality
    void pointAssignment(); //test point assignment
    void pointOperators(); //test point operators
    void isNull(); //test isEmpty method
    void toQPointF(); //test conversion to QPointF
    void pointEncodeDecode(); //test encoding and decoding point

    void converterCreate(); //test creating new converter
    void converterCopy(); //test creating new converter using copy constructor
    void converterGettersSetters(); //test getting/setting converter properties
    void conversionToMM(); //test conversion to mm
    void conversionToCM(); //test conversion to cm
    void conversionToM(); //test conversion to m
    void conversionToInches(); //test conversion to inches
    void conversionToFeet(); //test conversion to feet
    void conversionToPoints(); //test conversion to points
    void conversionToPicas(); //test conversion to picas
    void conversionFromPixels(); //test conversion from pixels and handling of dpi
    void conversionToPixels(); //test conversion to pixels and handling of dpi
    void sizeConversion(); //test conversion of QgsLayoutSizes
    void pointConversion(); //test conversion of QgsLayoutPoint

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
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutMillimeters ) ), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutCentimeters ) ), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutMeters ) ), QgsUnitTypes::LayoutMeters );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutInches ) ), QgsUnitTypes::LayoutInches );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutFeet ) ), QgsUnitTypes::LayoutFeet );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutPoints ) ), QgsUnitTypes::LayoutPoints );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutPicas ) ), QgsUnitTypes::LayoutPicas );
  QCOMPARE( QgsUnitTypes::decodeLayoutUnit( QgsUnitTypes::encodeUnit( QgsUnitTypes::LayoutPixels ) ), QgsUnitTypes::LayoutPixels );
}

void TestQgsLayoutUnits::create()
{
  const QgsLayoutMeasurement defaultUnits( 5.0 );
  //default units should be millimeters
  QCOMPARE( defaultUnits.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( defaultUnits.length(), 5.0 );

  //test overriding default unit
  const QgsLayoutMeasurement inches( 7.0, QgsUnitTypes::LayoutInches );
  QCOMPARE( inches.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( inches.length(), 7.0 );
}

void TestQgsLayoutUnits::gettersSetters()
{
  QgsLayoutMeasurement measurement( 5.0 );

  measurement.setLength( 9.0 );
  QCOMPARE( measurement.length(), 9.0 );
  measurement.setUnits( QgsUnitTypes::LayoutInches );
  QCOMPARE( measurement.units(), QgsUnitTypes::LayoutInches );
}

void TestQgsLayoutUnits::copyConstructor()
{
  const QgsLayoutMeasurement source( 6.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement dest( source );
  QCOMPARE( source, dest );
}

void TestQgsLayoutUnits::equality()
{
  const QgsLayoutMeasurement m1( 6.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement m2( 6.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement m3( 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement m4( 6.0, QgsUnitTypes::LayoutPoints );

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
  const QgsLayoutMeasurement m1( 6.0, QgsUnitTypes::LayoutInches );
  QgsLayoutMeasurement m2( 9.0, QgsUnitTypes::LayoutPoints );
  m2 = m1;
  QCOMPARE( m2, m1 );
}

void TestQgsLayoutUnits::operators()
{
  // +
  const QgsLayoutMeasurement m1( 6.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement m2 = m1 + 3.0;
  QCOMPARE( m2.units(), m1.units() );
  QCOMPARE( m2.length(), 9.0 );

  // +=
  QgsLayoutMeasurement m4( 6.0, QgsUnitTypes::LayoutInches );
  m4 += 2.0;
  QCOMPARE( m4.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( m4.length(), 8.0 );

  // -
  const QgsLayoutMeasurement m3 = m1 - 3.0;
  QCOMPARE( m3.units(), m1.units() );
  QCOMPARE( m3.length(), 3.0 );

  // -=
  QgsLayoutMeasurement m5( 6.0, QgsUnitTypes::LayoutInches );
  m5 -= 2.0;
  QCOMPARE( m5.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( m5.length(), 4.0 );

  // *
  const QgsLayoutMeasurement m6 = m1 * 3.0;
  QCOMPARE( m6.units(), m1.units() );
  QCOMPARE( m6.length(), 18.0 );

  // *=
  QgsLayoutMeasurement m7( 6.0, QgsUnitTypes::LayoutInches );
  m7 *= 2.0;
  QCOMPARE( m7.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( m7.length(), 12.0 );

  // /
  const QgsLayoutMeasurement m8 = m1 / 3.0;
  QCOMPARE( m8.units(), m1.units() );
  QCOMPARE( m8.length(), 2.0 );

  // /=
  QgsLayoutMeasurement m9( 6.0, QgsUnitTypes::LayoutInches );
  m9 /= 2.0;
  QCOMPARE( m9.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( m9.length(), 3.0 );
}

void TestQgsLayoutUnits::unitTypes()
{
  QCOMPARE( QgsUnitTypes::unitType( QgsUnitTypes::LayoutCentimeters ), QgsUnitTypes::LayoutPaperUnits );
  QCOMPARE( QgsUnitTypes::unitType( QgsUnitTypes::LayoutPixels ), QgsUnitTypes::LayoutScreenUnits );
}

void TestQgsLayoutUnits::measurementEncodeDecode()
{
  const QgsLayoutMeasurement original( 6.0, QgsUnitTypes::LayoutPixels );
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
  QCOMPARE( defaultUnits.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( defaultUnits.width(), 5.0 );
  QCOMPARE( defaultUnits.height(), 6.0 );

  //test overriding default unit
  const QgsLayoutSize inches( 7.0, 8.0, QgsUnitTypes::LayoutInches );
  QCOMPARE( inches.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( inches.width(), 7.0 );
  QCOMPARE( inches.height(), 8.0 );

  //test empty constructor
  const QgsLayoutSize empty( QgsUnitTypes::LayoutPixels );
  QCOMPARE( empty.units(), QgsUnitTypes::LayoutPixels );
  QCOMPARE( empty.width(), 0.0 );
  QCOMPARE( empty.height(), 0.0 );

  //test constructing from QSizeF
  const QgsLayoutSize fromQSizeF( QSizeF( 17.0, 18.0 ), QgsUnitTypes::LayoutInches );
  QCOMPARE( fromQSizeF.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( fromQSizeF.width(), 17.0 );
  QCOMPARE( fromQSizeF.height(), 18.0 );
}

void TestQgsLayoutUnits::sizeGettersSetters()
{
  QgsLayoutSize size( 5.0, 6.0, QgsUnitTypes::LayoutPicas );

  size.setWidth( 9.0 );
  size.setHeight( 10.0 );
  QCOMPARE( size.width(), 9.0 );
  QCOMPARE( size.height(), 10.0 );
  QCOMPARE( size.units(), QgsUnitTypes::LayoutPicas );

  size.setUnits( QgsUnitTypes::LayoutInches );
  QCOMPARE( size.width(), 9.0 );
  QCOMPARE( size.height(), 10.0 );
  QCOMPARE( size.units(), QgsUnitTypes::LayoutInches );

  size.setSize( 11.0, 12.0 );
  QCOMPARE( size.width(), 11.0 );
  QCOMPARE( size.height(), 12.0 );
  QCOMPARE( size.units(), QgsUnitTypes::LayoutInches );
}

void TestQgsLayoutUnits::sizeCopyConstructor()
{
  const QgsLayoutSize source( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutSize dest( source );
  QCOMPARE( source, dest );
}

void TestQgsLayoutUnits::sizeEquality()
{
  const QgsLayoutSize s1( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutSize s2( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutSize s3( 7.0, 8.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutSize s4( 6.0, 7.0, QgsUnitTypes::LayoutPoints );

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
  const QgsLayoutSize s1( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  QgsLayoutSize s2( 9.0, 10.0, QgsUnitTypes::LayoutPoints );
  s2 = s1;
  QCOMPARE( s2, s1 );
}

void TestQgsLayoutUnits::sizeOperators()
{
  const QgsLayoutSize s1( 6.0, 12.0, QgsUnitTypes::LayoutInches );

  // *
  const QgsLayoutSize s2 = s1 * 3.0;
  QCOMPARE( s2.units(), s1.units() );
  QCOMPARE( s2.width(), 18.0 );
  QCOMPARE( s2.height(), 36.0 );

  // *=
  QgsLayoutSize s3( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  s3 *= 2.0;
  QCOMPARE( s3.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( s3.width(), 12.0 );
  QCOMPARE( s3.height(), 24.0 );

  // /
  const QgsLayoutSize s4 = s1 / 3.0;
  QCOMPARE( s4.units(), s1.units() );
  QCOMPARE( s4.width(), 2.0 );
  QCOMPARE( s4.height(), 4.0 );

  // /=
  QgsLayoutSize s5( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  s5 /= 2.0;
  QCOMPARE( s5.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( s5.width(), 3.0 );
  QCOMPARE( s5.height(), 6.0 );
}

void TestQgsLayoutUnits::isEmpty()
{
  QgsLayoutSize size( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  QVERIFY( !size.isEmpty() );
  size.setSize( 0, 0 );
  QVERIFY( size.isEmpty() );
  const QgsLayoutSize size2( QgsUnitTypes::LayoutMillimeters ); //test empty constructor
  QVERIFY( size2.isEmpty() );
}

void TestQgsLayoutUnits::toQSizeF()
{
  const QgsLayoutSize size( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  const QSizeF converted = size.toQSizeF();
  QCOMPARE( converted.width(), size.width() );
  QCOMPARE( converted.height(), size.height() );
}

void TestQgsLayoutUnits::sizeEncodeDecode()
{
  const QgsLayoutSize original( 6.0, 12.0, QgsUnitTypes::LayoutPoints );
  QgsLayoutSize result = QgsLayoutSize::decodeSize( original.encodeSize() );
  QCOMPARE( original, result );

  //test with bad string
  result = QgsLayoutSize::decodeSize( QStringLiteral( "1,2" ) );
  QCOMPARE( result, QgsLayoutSize() );
}

void TestQgsLayoutUnits::createPoint()
{
  const QgsLayoutPoint defaultUnits( 5.0, 6.0 );
  //default units should be millimeters
  QCOMPARE( defaultUnits.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( defaultUnits.x(), 5.0 );
  QCOMPARE( defaultUnits.y(), 6.0 );

  //test overriding default unit
  const QgsLayoutPoint inches( 7.0, 8.0, QgsUnitTypes::LayoutInches );
  QCOMPARE( inches.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( inches.x(), 7.0 );
  QCOMPARE( inches.y(), 8.0 );

  //test empty constructor
  const QgsLayoutPoint empty( QgsUnitTypes::LayoutPixels );
  QCOMPARE( empty.units(), QgsUnitTypes::LayoutPixels );
  QCOMPARE( empty.x(), 0.0 );
  QCOMPARE( empty.y(), 0.0 );

  //test constructing from QPointF
  const QgsLayoutPoint fromQPointF( QPointF( 17.0, 18.0 ), QgsUnitTypes::LayoutInches );
  QCOMPARE( fromQPointF.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( fromQPointF.x(), 17.0 );
  QCOMPARE( fromQPointF.y(), 18.0 );
}

void TestQgsLayoutUnits::pointGettersSetters()
{
  QgsLayoutPoint point( 5.0, 6.0, QgsUnitTypes::LayoutPicas );

  point.setX( 9.0 );
  point.setY( 10.0 );
  QCOMPARE( point.x(), 9.0 );
  QCOMPARE( point.y(), 10.0 );
  QCOMPARE( point.units(), QgsUnitTypes::LayoutPicas );

  point.setUnits( QgsUnitTypes::LayoutInches );
  QCOMPARE( point.x(), 9.0 );
  QCOMPARE( point.y(), 10.0 );
  QCOMPARE( point.units(), QgsUnitTypes::LayoutInches );

  point.setPoint( 11.0, 12.0 );
  QCOMPARE( point.x(), 11.0 );
  QCOMPARE( point.y(), 12.0 );
  QCOMPARE( point.units(), QgsUnitTypes::LayoutInches );
}

void TestQgsLayoutUnits::pointCopyConstructor()
{
  const QgsLayoutPoint source( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutPoint dest( source );
  QCOMPARE( source, dest );
}

void TestQgsLayoutUnits::pointEquality()
{
  const QgsLayoutPoint p1( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutPoint p2( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutPoint p3( 7.0, 8.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutPoint p4( 6.0, 7.0, QgsUnitTypes::LayoutPoints );

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
  const QgsLayoutPoint p1( 6.0, 7.0, QgsUnitTypes::LayoutInches );
  QgsLayoutPoint p2( 9.0, 10.0, QgsUnitTypes::LayoutPoints );
  p2 = p1;
  QCOMPARE( p2, p1 );
}

void TestQgsLayoutUnits::pointOperators()
{
  const QgsLayoutPoint p1( 6.0, 12.0, QgsUnitTypes::LayoutInches );

  // *
  const QgsLayoutPoint p2 = p1 * 3.0;
  QCOMPARE( p2.units(), p1.units() );
  QCOMPARE( p2.x(), 18.0 );
  QCOMPARE( p2.y(), 36.0 );

  // *=
  QgsLayoutPoint p3( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  p3 *= 2.0;
  QCOMPARE( p3.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( p3.x(), 12.0 );
  QCOMPARE( p3.y(), 24.0 );

  // /
  const QgsLayoutPoint p4 = p1 / 3.0;
  QCOMPARE( p4.units(), p1.units() );
  QCOMPARE( p4.x(), 2.0 );
  QCOMPARE( p4.y(), 4.0 );

  // /=
  QgsLayoutPoint p5( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  p5 /= 2.0;
  QCOMPARE( p5.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( p5.x(), 3.0 );
  QCOMPARE( p5.y(), 6.0 );
}

void TestQgsLayoutUnits::isNull()
{
  QgsLayoutPoint point( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  QVERIFY( !point.isNull() );
  point.setPoint( 0, 0 );
  QVERIFY( point.isNull() );
  const QgsLayoutPoint point2( QgsUnitTypes::LayoutMillimeters ); //test empty constructor
  QVERIFY( point2.isNull() );
}

void TestQgsLayoutUnits::toQPointF()
{
  const QgsLayoutPoint point( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  const QPointF converted = point.toQPointF();
  QCOMPARE( converted.x(), point.x() );
  QCOMPARE( converted.y(), point.y() );
}

void TestQgsLayoutUnits::pointEncodeDecode()
{
  const QgsLayoutPoint original( 6.0, 12.0, QgsUnitTypes::LayoutInches );
  QgsLayoutPoint result = QgsLayoutPoint::decodePoint( original.encodePoint() );
  QCOMPARE( original, result );

  //test with bad string
  result = QgsLayoutPoint::decodePoint( QStringLiteral( "1,2" ) );
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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutMillimeters );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutMillimeters );

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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutCentimeters );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutCentimeters );

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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutMeters );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutMeters );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutMeters );

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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutInches );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutInches );

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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutFeet );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutFeet );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutFeet );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutFeet );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutFeet );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutFeet );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutFeet );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutFeet );

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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutPoints );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutPoints );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutPoints );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutPoints );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutPoints );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutPoints );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutPoints );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutPoints );

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
  const QgsLayoutMeasurement measurementInMillimeters( 1.0, QgsUnitTypes::LayoutMillimeters );
  const QgsLayoutMeasurement measurementInCentimeters( 1.0, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutMeasurement measurementInMeters( 1.0, QgsUnitTypes::LayoutMeters );
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInFeet( 1.0, QgsUnitTypes::LayoutFeet );
  const QgsLayoutMeasurement measurementInPoints( 1.0, QgsUnitTypes::LayoutPoints );
  const QgsLayoutMeasurement measurementInPicas( 1.0, QgsUnitTypes::LayoutPicas );

  const QgsLayoutMeasurementConverter converter;
  const QgsLayoutMeasurement convertedFromMillimeters = converter.convert( measurementInMillimeters, QgsUnitTypes::LayoutPicas );
  const QgsLayoutMeasurement convertedFromCentimeters = converter.convert( measurementInCentimeters, QgsUnitTypes::LayoutPicas );
  const QgsLayoutMeasurement convertedFromMeters = converter.convert( measurementInMeters, QgsUnitTypes::LayoutPicas );
  const QgsLayoutMeasurement convertedFromInches = converter.convert( measurementInInches, QgsUnitTypes::LayoutPicas );
  const QgsLayoutMeasurement convertedFromFeet = converter.convert( measurementInFeet, QgsUnitTypes::LayoutPicas );
  const QgsLayoutMeasurement convertedFromPoints = converter.convert( measurementInPoints, QgsUnitTypes::LayoutPicas );
  const QgsLayoutMeasurement convertedFromPicas = converter.convert( measurementInPicas, QgsUnitTypes::LayoutPicas );

  QCOMPARE( convertedFromMillimeters.units(), QgsUnitTypes::LayoutPicas );
  QCOMPARE( convertedFromCentimeters.units(), QgsUnitTypes::LayoutPicas );
  QCOMPARE( convertedFromMeters.units(), QgsUnitTypes::LayoutPicas );
  QCOMPARE( convertedFromInches.units(), QgsUnitTypes::LayoutPicas );
  QCOMPARE( convertedFromFeet.units(), QgsUnitTypes::LayoutPicas );
  QCOMPARE( convertedFromPoints.units(), QgsUnitTypes::LayoutPicas );
  QCOMPARE( convertedFromPicas.units(), QgsUnitTypes::LayoutPicas );

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
  const QgsLayoutMeasurement measurementInPixels( 300.0, QgsUnitTypes::LayoutPixels );

  QgsLayoutMeasurementConverter converter;
  //try initially with 300 dpi
  converter.setDpi( 300.0 );
  QgsLayoutMeasurement convertedToInches = converter.convert( measurementInPixels, QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedToInches.units(), QgsUnitTypes::LayoutInches );
  QGSCOMPARENEAR( convertedToInches.length(), 1.0, 0.000001 );
  QgsLayoutMeasurement convertedToMM = converter.convert( measurementInPixels, QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedToMM.units(), QgsUnitTypes::LayoutMillimeters );
  QGSCOMPARENEAR( convertedToMM.length(), 25.4, 0.000001 );

  //try with 96 dpi
  converter.setDpi( 96.0 );
  convertedToInches = converter.convert( measurementInPixels, QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedToInches.units(), QgsUnitTypes::LayoutInches );
  QGSCOMPARENEAR( convertedToInches.length(), 3.125, 0.000001 );
  convertedToMM = converter.convert( measurementInPixels, QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedToMM.units(), QgsUnitTypes::LayoutMillimeters );
  QGSCOMPARENEAR( convertedToMM.length(), 79.375, 0.000001 );
}

void TestQgsLayoutUnits::conversionToPixels()
{
  const QgsLayoutMeasurement measurementInInches( 1.0, QgsUnitTypes::LayoutInches );
  const QgsLayoutMeasurement measurementInMM( 1.0, QgsUnitTypes::LayoutMillimeters );

  QgsLayoutMeasurementConverter converter;
  //try initially with 300 dpi
  converter.setDpi( 300.0 );
  QgsLayoutMeasurement convertedToPixels = converter.convert( measurementInInches, QgsUnitTypes::LayoutPixels );
  QCOMPARE( convertedToPixels.units(), QgsUnitTypes::LayoutPixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 300.0, 0.000001 );
  convertedToPixels = converter.convert( measurementInMM, QgsUnitTypes::LayoutPixels );
  QCOMPARE( convertedToPixels.units(), QgsUnitTypes::LayoutPixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 11.811023622, 0.000001 );

  //try with 96 dpi
  converter.setDpi( 96.0 );
  convertedToPixels = converter.convert( measurementInInches, QgsUnitTypes::LayoutPixels );
  QCOMPARE( convertedToPixels.units(), QgsUnitTypes::LayoutPixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 96.0, 0.000001 );
  convertedToPixels = converter.convert( measurementInMM, QgsUnitTypes::LayoutPixels );
  QCOMPARE( convertedToPixels.units(), QgsUnitTypes::LayoutPixels );
  QGSCOMPARENEAR( convertedToPixels.length(), 3.77952755906, 0.000001 );
}

void TestQgsLayoutUnits::sizeConversion()
{
  const QgsLayoutSize sizeInMM( 1.0, 2.0, QgsUnitTypes::LayoutMillimeters );
  QgsLayoutMeasurementConverter converter;

  //test conversion to same units
  QgsLayoutSize convertedSize = converter.convert( sizeInMM, QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedSize, sizeInMM );

  //convert to other units
  convertedSize = converter.convert( sizeInMM, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutSize expectedSize( 0.1, 0.2, QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedSize, expectedSize );

  //pixel conversion
  converter.setDpi( 96.0 );
  const QgsLayoutSize convertedToInches = converter.convert( QgsLayoutSize( 96, 192, QgsUnitTypes::LayoutPixels ), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedToInches.width(), 1.0 );
  QCOMPARE( convertedToInches.height(), 2.0 );
  QCOMPARE( convertedToInches.units(), QgsUnitTypes::LayoutInches );
}

void TestQgsLayoutUnits::pointConversion()
{
  const QgsLayoutPoint pointInMM( 1.0, 2.0, QgsUnitTypes::LayoutMillimeters );
  QgsLayoutMeasurementConverter converter;

  //test conversion to same units
  QgsLayoutPoint convertedPoint = converter.convert( pointInMM, QgsUnitTypes::LayoutMillimeters );
  QCOMPARE( convertedPoint, pointInMM );

  //convert to other units
  convertedPoint = converter.convert( pointInMM, QgsUnitTypes::LayoutCentimeters );
  const QgsLayoutPoint expectedPoint( 0.1, 0.2, QgsUnitTypes::LayoutCentimeters );
  QCOMPARE( convertedPoint, expectedPoint );

  //pixel conversion
  converter.setDpi( 96.0 );
  const QgsLayoutPoint convertedToInches = converter.convert( QgsLayoutPoint( 96, 192, QgsUnitTypes::LayoutPixels ), QgsUnitTypes::LayoutInches );
  QCOMPARE( convertedToInches.x(), 1.0 );
  QCOMPARE( convertedToInches.y(), 2.0 );
  QCOMPARE( convertedToInches.units(), QgsUnitTypes::LayoutInches );
}

QTEST_MAIN( TestQgsLayoutUnits )
#include "testqgslayoutunits.moc"
