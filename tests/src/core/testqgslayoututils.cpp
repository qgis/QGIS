/***************************************************************************
                         testqgslayoututils.cpp
                         ---------------------
    begin                : July 2017
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
#include "qgslayoututils.h"
#include "qgsproject.h"
#include "qgslayoutitemmap.h"
#include "qgsfontutils.h"
#include "qgsrenderchecker.h"
#include "qgsvectorlayer.h"
#include "qgslayout.h"

#include <QStyleOptionGraphicsItem>

class TestQgsLayoutUtils: public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutUtils() : QgsTest( QStringLiteral( "Layout Utils Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void rotate();
    void normalizedAngle(); //test normalised angle function
    void snappedAngle();
    void createRenderContextFromLayout();
    void createRenderContextFromMap();
    void relativePosition();
    void relativeResizeRect();
    void pointsToMM(); //test conversion of point size to mm
    void mmToPoints(); //test conversion of mm to point size
    void scaledFontPixelSize(); //test creating a scaled font
    void fontAscentMM(); //test calculating font ascent in mm
    void fontDescentMM(); //test calculating font descent in mm
    void fontHeightMM(); //test calculating font height in mm
    void fontHeightCharacterMM(); //test calculating font character height in mm
    void textWidthMM(); //test calculating text width in mm
    void textHeightMM(); //test calculating text height in mm
    void drawTextPos(); //test drawing text at a pos
    void drawTextRect(); //test drawing text in a rect
    void largestRotatedRect(); //test largest rotated rect helper function
    void decodePaperOrientation();
    void mapLayerFromString();

  private:

    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );

    QFont mTestFont;
};

void TestQgsLayoutUtils::initTestCase()
{
  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Oblique" ) );
  mTestFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Oblique " ) );
  mTestFont.setItalic( true );
}

void TestQgsLayoutUtils::rotate()
{
  // pairs of lines from before -> expected after position and angle to rotate
  QList< QPair< QLineF, double > > testVals;
  testVals << qMakePair( QLineF( 0, 1, 0, 1 ), 0.0 );
  testVals << qMakePair( QLineF( 0, 1, -1, 0 ), 90.0 );
  testVals << qMakePair( QLineF( 0, 1, 0, -1 ), 180.0 );
  testVals << qMakePair( QLineF( 0, 1, 1, 0 ), 270.0 );
  testVals << qMakePair( QLineF( 0, 1, 0, 1 ), 360.0 );

  //test rotate helper function
  QList< QPair< QLineF, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )
  {
    double x = ( *it ).first.x1();
    double y = ( *it ).first.y1();
    QgsLayoutUtils::rotate( ( *it ).second, x, y );
    QGSCOMPARENEAR( x, ( *it ).first.x2(), 4 * std::numeric_limits<double>::epsilon() );
    QGSCOMPARENEAR( y, ( *it ).first.y2(), 4 * std::numeric_limits<double>::epsilon() );
  }
}

void TestQgsLayoutUtils::normalizedAngle()
{
  QList< QPair< double, double > > testVals;
  testVals << qMakePair( 0.0, 0.0 );
  testVals << qMakePair( 90.0, 90.0 );
  testVals << qMakePair( 180.0, 180.0 );
  testVals << qMakePair( 270.0, 270.0 );
  testVals << qMakePair( 360.0, 0.0 );
  testVals << qMakePair( 390.0, 30.0 );
  testVals << qMakePair( 720.0, 0.0 );
  testVals << qMakePair( 730.0, 10.0 );
  testVals << qMakePair( -10.0, 350.0 );
  testVals << qMakePair( -360.0, 0.0 );
  testVals << qMakePair( -370.0, 350.0 );
  testVals << qMakePair( -760.0, 320.0 );

  //test normalized angle helper function
  QList< QPair< double, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )

  {
    const double result = QgsLayoutUtils::normalizedAngle( ( *it ).first );
    qDebug() << QStringLiteral( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QGSCOMPARENEAR( result, ( *it ).second, 4 * std::numeric_limits<double>::epsilon() );

  }

  //test with allowing negative angles
  QList< QPair< double, double > > negativeTestVals;
  negativeTestVals << qMakePair( 0.0, 0.0 );
  negativeTestVals << qMakePair( 90.0, 90.0 );
  negativeTestVals << qMakePair( 360.0, 0.0 );
  negativeTestVals << qMakePair( -10.0, -10.0 );
  negativeTestVals << qMakePair( -359.0, -359.0 );
  negativeTestVals << qMakePair( -360.0, 0.0 );
  negativeTestVals << qMakePair( -361.0, -1.0 );
  negativeTestVals << qMakePair( -370.0, -10.0 );
  negativeTestVals << qMakePair( -760.0, -40.0 );
  it = negativeTestVals.constBegin();
  for ( ; it != negativeTestVals.constEnd(); ++it )

  {
    const double result = QgsLayoutUtils::normalizedAngle( ( *it ).first, true );
    qDebug() << QStringLiteral( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QGSCOMPARENEAR( result, ( *it ).second, 4 * std::numeric_limits<double>::epsilon() );

  }
}

void TestQgsLayoutUtils::snappedAngle()
{
  QList< QPair< double, double > > testVals;
  testVals << qMakePair( 0.0, 0.0 );
  testVals << qMakePair( 10.0, 0.0 );
  testVals << qMakePair( 20.0, 0.0 );
  testVals << qMakePair( 30.0, 45.0 );
  testVals << qMakePair( 40.0, 45.0 );
  testVals << qMakePair( 50.0, 45.0 );
  testVals << qMakePair( 60.0, 45.0 );
  testVals << qMakePair( 70.0, 90.0 );
  testVals << qMakePair( 80.0, 90.0 );
  testVals << qMakePair( 90.0, 90.0 );
  testVals << qMakePair( 100.0, 90.0 );
  testVals << qMakePair( 110.0, 90.0 );
  testVals << qMakePair( 120.0, 135.0 );
  testVals << qMakePair( 130.0, 135.0 );
  testVals << qMakePair( 140.0, 135.0 );
  testVals << qMakePair( 150.0, 135.0 );
  testVals << qMakePair( 160.0, 180.0 );
  testVals << qMakePair( 170.0, 180.0 );
  testVals << qMakePair( 180.0, 180.0 );
  testVals << qMakePair( 190.0, 180.0 );
  testVals << qMakePair( 200.0, 180.0 );
  testVals << qMakePair( 210.0, 225.0 );
  testVals << qMakePair( 220.0, 225.0 );
  testVals << qMakePair( 230.0, 225.0 );
  testVals << qMakePair( 240.0, 225.0 );
  testVals << qMakePair( 250.0, 270.0 );
  testVals << qMakePair( 260.0, 270.0 );
  testVals << qMakePair( 270.0, 270.0 );
  testVals << qMakePair( 280.0, 270.0 );
  testVals << qMakePair( 290.0, 270.0 );
  testVals << qMakePair( 300.0, 315.0 );
  testVals << qMakePair( 310.0, 315.0 );
  testVals << qMakePair( 320.0, 315.0 );
  testVals << qMakePair( 330.0, 315.0 );
  testVals << qMakePair( 340.0, 0.0 );
  testVals << qMakePair( 350.0, 0.0 );
  testVals << qMakePair( 360.0, 0.0 );

  //test snapped angle helper function
  QList< QPair< double, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )
  {
    QGSCOMPARENEAR( QgsLayoutUtils::snappedAngle( ( *it ).first ), ( *it ).second, 4 * std::numeric_limits<double>::epsilon() );
  }
}


void TestQgsLayoutUtils::createRenderContextFromLayout()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no layout
  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForLayout( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no layout, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create layout with no reference map
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsProject project;
  QgsLayout l( &project );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // layout, no map, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  // add a reference map
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );
  l.setReferenceMap( map );

  rc = QgsLayoutUtils::createRenderContextForLayout( &l, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QCOMPARE( rc.painter(), &p );

  // layout, reference map, no painter
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QVERIFY( !rc.painter() );

  // check render context flags are correctly set
  l.renderContext().setFlags( QgsLayoutRenderContext::Flags() );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( !( rc.flags() & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( rc.flags() & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::ForceVectorOutput ) );

  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( rc.flags() & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::ForceVectorOutput ) );

  l.renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::ForceVectorOutput ) );

  // check text format is correctly set
  l.renderContext().setTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QCOMPARE( rc.textRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );
  l.renderContext().setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  rc = QgsLayoutUtils::createRenderContextForLayout( &l, nullptr );
  QCOMPARE( rc.textRenderFormat(), Qgis::TextRenderFormat::AlwaysText );

  p.end();
}

void TestQgsLayoutUtils::createRenderContextFromMap()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no map
  QgsRenderContext rc = QgsLayoutUtils::createRenderContextForMap( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no map, no painter
  rc = QgsLayoutUtils::createRenderContextForMap( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create composition with no reference map
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsProject project;
  QgsLayout l( &project );

  // add a map
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );

  rc = QgsLayoutUtils::createRenderContextForMap( map, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QCOMPARE( rc.painter(), &p );

  // map, no painter
  rc = QgsLayoutUtils::createRenderContextForMap( map, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QVERIFY( !rc.painter() );

  // secondary map
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 30, 60, 100, 50 ) );
  map2->setExtent( extent );
  l.addLayoutItem( map2 );

  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map2->scale(), 1000000 );
  QVERIFY( rc.painter() );

  // check render context flags are correctly set
  l.renderContext().setFlags( QgsLayoutRenderContext::Flags() );
  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QVERIFY( !( rc.flags() & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( rc.flags() & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::ForceVectorOutput ) );

  l.renderContext().setFlag( QgsLayoutRenderContext::FlagAntialiasing );
  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( rc.flags() & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::ForceVectorOutput ) );

  l.renderContext().setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects );
  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( rc.flags() & Qgis::RenderContextFlag::ForceVectorOutput ) );

  // check text format is correctly set
  l.renderContext().setTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QCOMPARE( rc.textRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );
  l.renderContext().setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  rc = QgsLayoutUtils::createRenderContextForMap( map2, &p );
  QCOMPARE( rc.textRenderFormat(), Qgis::TextRenderFormat::AlwaysText );

  p.end();
}


void TestQgsLayoutUtils::relativePosition()
{
  //+ve gradient
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 0, 2, 0, 4 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 0, 2, 0, 4 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 0, 2, 0, 4 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 0, 2, 0, 4 ), 8, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 0, 2, 0, 4 ), -4, 0.001 );
  //-ve gradient
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 0, 2, 4, 0 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 0, 2, 4, 0 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 0, 2, 4, 0 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 0, 2, 4, 0 ), -4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 0, 2, 4, 0 ), 8, 0.001 );
  //-ve domain
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 2, 0, 0, 4 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 2, 0, 0, 4 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 2, 0, 0, 4 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 2, 0, 0, 4 ), -4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 2, 0, 0, 4 ), 8, 0.001 );
  //-ve domain and gradient
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 1, 2, 0, 4, 0 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 0, 2, 0, 4, 0 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 2, 2, 0, 4, 0 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( 4, 2, 0, 4, 0 ), 8, 0.001 );
  QGSCOMPARENEAR( QgsLayoutUtils::relativePosition( -2, 2, 0, 4, 0 ), -4, 0.001 );
}

void TestQgsLayoutUtils::relativeResizeRect()
{
  //test rectangle which fills bounds
  QRectF testRect = QRectF( 0, 0, 1, 1 );
  QRectF boundsBefore = QRectF( 0, 0, 1, 1 );
  QRectF boundsAfter = QRectF( 0, 0, 1, 1 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 1, 1 ) );
  testRect = QRectF( 0, 0, 1, 1 );
  boundsAfter = QRectF( 0, 0, 2, 2 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 2, 2 ) );
  testRect = QRectF( 0, 0, 1, 1 );
  boundsAfter = QRectF( 0, 0, 0.5, 4 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 0.5, 4 ) );

  //test rectangle which doesn't fill bounds
  testRect = QRectF( 1, 2, 1, 2 );
  boundsBefore = QRectF( 0, 0, 4, 8 );
  boundsAfter = QRectF( 0, 0, 2, 4 );
  QgsLayoutUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0.5, 1, 0.5, 1 ) );
}

void TestQgsLayoutUtils::pointsToMM()
{
  //test conversion of points to mm, based on 1 point = 1 / 72 of an inch
  QGSCOMPARENEAR( QgsLayoutUtils::pointsToMM( 72 / 25.4 ), 1, 0.001 );
}

void TestQgsLayoutUtils::mmToPoints()
{
  //test conversion of mm to points, based on 1 point = 1 / 72 of an inch
  QGSCOMPARENEAR( QgsLayoutUtils::mmToPoints( 25.4 / 72 ), 1, 0.001 );
}

void TestQgsLayoutUtils::scaledFontPixelSize()
{
  //create a 12 point test font
  mTestFont.setPointSize( 12 );

  //test scaling of font for painting
  const QFont scaledFont = QgsLayoutUtils::scaledFontPixelSize( mTestFont );
  QCOMPARE( scaledFont.pixelSize(), 42 );
  QCOMPARE( scaledFont.family(), mTestFont.family() );
}

void TestQgsLayoutUtils::fontAscentMM()
{
  mTestFont.setPointSize( 12 );
  //platform specific font rendering differences mean these tests need to be very lenient
  QGSCOMPARENEAR( QgsLayoutUtils::fontAscentMM( mTestFont ), 3.9, 0.5 );
}

void TestQgsLayoutUtils::fontDescentMM()
{
  mTestFont.setPointSize( 12 );
  QGSCOMPARENEAR( QgsLayoutUtils::fontDescentMM( mTestFont ), 0.9, 0.15 );
}

void TestQgsLayoutUtils::fontHeightMM()
{
  mTestFont.setPointSize( 12 );
  //platform specific font rendering differences mean these tests need to be very lenient
  QGSCOMPARENEAR( QgsLayoutUtils::fontHeightMM( mTestFont ), 4.9, 0.5 );
}

void TestQgsLayoutUtils::fontHeightCharacterMM()
{
  mTestFont.setPointSize( 12 );
  //platform specific font rendering differences mean these tests need to be very lenient
  QGSCOMPARENEAR( QgsLayoutUtils::fontHeightCharacterMM( mTestFont, QChar( 'a' ) ), 2.4, 0.15 );
  QGSCOMPARENEAR( QgsLayoutUtils::fontHeightCharacterMM( mTestFont, QChar( 'l' ) ), 3.15, 0.16 );
  QGSCOMPARENEAR( QgsLayoutUtils::fontHeightCharacterMM( mTestFont, QChar( 'g' ) ), 3.2, 0.11 );

}

void TestQgsLayoutUtils::textWidthMM()
{
  //platform specific font rendering differences mean this test needs to be very lenient
  mTestFont.setPointSize( 12 );
  QGSCOMPARENEAR( QgsLayoutUtils::textWidthMM( mTestFont, QString( "test string" ) ), 20, 2 );

}

void TestQgsLayoutUtils::textHeightMM()
{
  //platform specific font rendering differences mean this test needs to be very lenient
  mTestFont.setPointSize( 12 );
  QGSCOMPARENEAR( QgsLayoutUtils::textHeightMM( mTestFont, QString( "test string" ) ), 3.9, 0.2 );
  QGSCOMPARENEAR( QgsLayoutUtils::textHeightMM( mTestFont, QString( "test\nstring" ) ), 8.7, 0.2 );
  QGSCOMPARENEAR( QgsLayoutUtils::textHeightMM( mTestFont, QString( "test\nstring" ), 2 ), 13.5, 0.2 );
  QGSCOMPARENEAR( QgsLayoutUtils::textHeightMM( mTestFont, QString( "test\nstring\nstring" ) ), 13.5, 0.2 );

}

void TestQgsLayoutUtils::drawTextPos()
{
  //test drawing with no painter
  QgsLayoutUtils::drawText( nullptr, QPointF( 5, 15 ), QStringLiteral( "Abc123" ), mTestFont );

  //test drawing text on to image
  mTestFont.setPointSize( 48 );
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsLayoutUtils::drawText( &testPainter, QPointF( 5, 15 ), QStringLiteral( "Abc123" ), mTestFont, Qt::white );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_pos", testImage, 100 ) );

  //test drawing with pen color set on painter and no specified color
  //text should be drawn using painter pen color
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  testPainter.setPen( QPen( Qt::green ) );
  QgsLayoutUtils::drawText( &testPainter, QPointF( 5, 15 ), QStringLiteral( "Abc123" ), mTestFont );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_posnocolor", testImage, 100 ) );
}

void TestQgsLayoutUtils::drawTextRect()
{
  //test drawing with no painter
  QgsLayoutUtils::drawText( nullptr, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont );

  //test drawing text on to image
  mTestFont.setPointSize( 48 );
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsLayoutUtils::drawText( &testPainter, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont, Qt::white );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rect", testImage, 100 ) );

  //test drawing with pen color set on painter and no specified color
  //text should be drawn using painter pen color
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  testPainter.setPen( QPen( Qt::green ) );
  QgsLayoutUtils::drawText( &testPainter, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rectnocolor", testImage, 100 ) );

  //test alignment settings
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  QgsLayoutUtils::drawText( &testPainter, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont, Qt::black, Qt::AlignRight, Qt::AlignBottom );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rectalign", testImage, 100 ) );

  //test extra flags - render without clipping
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  QgsLayoutUtils::drawText( &testPainter, QRectF( 5, 15, 20, 50 ), QStringLiteral( "Abc123" ), mTestFont, Qt::white, Qt::AlignLeft, Qt::AlignTop, Qt::TextDontClip );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rectflag", testImage, 100 ) );
}

void TestQgsLayoutUtils::largestRotatedRect()
{
  const QRectF wideRect = QRectF( 0, 0, 2, 1 );
  const QRectF highRect = QRectF( 0, 0, 1, 2 );
  const QRectF bounds = QRectF( 0, 0, 4, 2 );

  //simple cases
  //0 rotation
  QRectF result = QgsLayoutUtils::largestRotatedRectWithinBounds( wideRect, bounds, 0 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( highRect, bounds, 0 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );
  // 90 rotation
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( wideRect, bounds, 90 );
  QCOMPARE( result, QRectF( 1.5, 0, 2, 1 ) );
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( highRect, bounds, 90 );
  QCOMPARE( result, QRectF( 0, 0, 2, 4 ) );
  // 180 rotation
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( wideRect, bounds, 180 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( highRect, bounds, 0 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );
  // 270 rotation
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( wideRect, bounds, 270 );
  QCOMPARE( result, QRectF( 1.5, 0, 2, 1 ) );
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( highRect, bounds, 270 );
  QCOMPARE( result, QRectF( 0, 0, 2, 4 ) );
  //360 rotation
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( wideRect, bounds, 360 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsLayoutUtils::largestRotatedRectWithinBounds( highRect, bounds, 360 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );

  //full test, run through a circle in 10 degree increments
  for ( double rotation = 10; rotation < 360; rotation += 10 )
  {
    result = QgsLayoutUtils::largestRotatedRectWithinBounds( wideRect, bounds, rotation );
    QTransform t;
    t.rotate( rotation );
    const QRectF rotatedRectBounds = t.mapRect( result );
    //one of the rotated rects dimensions must equal the bounding rectangles dimensions (ie, it has been constrained by one dimension)
    //and the other dimension must be less than or equal to bounds dimension
    QVERIFY( ( qgsDoubleNear( rotatedRectBounds.width(), bounds.width(), 0.001 ) && ( rotatedRectBounds.height() <= bounds.height() ) )
             || ( qgsDoubleNear( rotatedRectBounds.height(), bounds.height(), 0.001 ) && ( rotatedRectBounds.width() <= bounds.width() ) ) );

    //also verify that aspect ratio of rectangle has not changed
    QGSCOMPARENEAR( result.width() / result.height(), wideRect.width() / wideRect.height(), 4 * std::numeric_limits<double>::epsilon() );
  }
  //and again for the high rectangle
  for ( double rotation = 10; rotation < 360; rotation += 10 )
  {
    result = QgsLayoutUtils::largestRotatedRectWithinBounds( highRect, bounds, rotation );
    QTransform t;
    t.rotate( rotation );
    const QRectF rotatedRectBounds = t.mapRect( result );
    //one of the rotated rects dimensions must equal the bounding rectangles dimensions (ie, it has been constrained by one dimension)
    //and the other dimension must be less than or equal to bounds dimension
    QVERIFY( ( qgsDoubleNear( rotatedRectBounds.width(), bounds.width(), 0.001 ) && ( rotatedRectBounds.height() <= bounds.height() ) )
             || ( qgsDoubleNear( rotatedRectBounds.height(), bounds.height(), 0.001 ) && ( rotatedRectBounds.width() <= bounds.width() ) ) );

    //also verify that aspect ratio of rectangle has not changed
    QGSCOMPARENEAR( result.width() / result.height(), highRect.width() / highRect.height(), 4 * std::numeric_limits<double>::epsilon() );
  }
}

void TestQgsLayoutUtils::decodePaperOrientation()
{
  QgsLayoutItemPage::Orientation orientation;
  bool ok = false;
  orientation = QgsLayoutUtils::decodePaperOrientation( QStringLiteral( "bad string" ), ok );
  QVERIFY( !ok );
  QCOMPARE( orientation, QgsLayoutItemPage::Landscape ); //should default to landscape
  ok = false;
  orientation = QgsLayoutUtils::decodePaperOrientation( QStringLiteral( "portrait" ), ok );
  QVERIFY( ok );
  QCOMPARE( orientation, QgsLayoutItemPage::Portrait );
  ok = false;
  orientation = QgsLayoutUtils::decodePaperOrientation( QStringLiteral( " LANDSCAPE  " ), ok );
  QVERIFY( ok );
  QCOMPARE( orientation, QgsLayoutItemPage::Landscape );
}

void TestQgsLayoutUtils::mapLayerFromString()
{
  // add some layers to a project
  QgsVectorLayer *l1 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "layer 1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *l2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "layer 2" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *l2a = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer&field=col2:integer&field=col3:integer" ), QStringLiteral( "LAYER 2" ), QStringLiteral( "memory" ) );
  QgsProject p;
  p.addMapLayer( l1 );
  p.addMapLayer( l2 );
  p.addMapLayer( l2a );

  QCOMPARE( QgsLayoutUtils::mapLayerFromString( "layer 1", &p ), l1 );
  QCOMPARE( QgsLayoutUtils::mapLayerFromString( "LAYER 1", &p ), l1 );
  QCOMPARE( QgsLayoutUtils::mapLayerFromString( "layer 2", &p ), l2 );
  QCOMPARE( QgsLayoutUtils::mapLayerFromString( "LAYER 2", &p ), l2a );
  QCOMPARE( QgsLayoutUtils::mapLayerFromString( l1->id(), &p ), l1 );
  QCOMPARE( QgsLayoutUtils::mapLayerFromString( l2->id(), &p ), l2 );
  QCOMPARE( QgsLayoutUtils::mapLayerFromString( l2a->id(), &p ), l2a );
  QVERIFY( !QgsLayoutUtils::mapLayerFromString( "none", &p ) );

}

bool TestQgsLayoutUtils::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  const QString myTmpDir = QDir::tempPath() + '/';
  const QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "composer_utils" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  const bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsLayoutUtils )
#include "testqgslayoututils.moc"
