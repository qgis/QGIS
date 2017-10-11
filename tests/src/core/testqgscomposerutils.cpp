/***************************************************************************
                         testqgscomposerutils.cpp
                         -----------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgsapplication.h" //for standard test font
#include "qgscomposerutils.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"
#include "qgsproject.h"
#include "qgsproperty.h"
#include <QObject>
#include "qgstest.h"
#include <QMap>

class TestQgsComposerUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsComposerUtils();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void drawArrowHead(); //test drawing an arrow head
    void angle(); //test angle utility function
    void rotate(); //test rotation helper function
    void normalizedAngle(); //test normalised angle function
    void snappedAngle(); //test snapped angle function
    void largestRotatedRect(); //test largest rotated rect helper function
    void pointsToMM(); //test conversion of point size to mm
    void mmToPoints(); //test conversion of mm to point size
    void relativePosition(); //test relative position function
    void relativeResizeRect(); //test relative resize of rectangle function
    void decodePaperOrientation(); //test decoding paper orientation
    void decodePaperSize(); //test decoding paper size
    void readOldDataDefinedProperty(); //test reading a data defined property
    void readOldDataDefinedPropertyMap(); //test reading a whole data defined property map
    void scaledFontPixelSize(); //test creating a scaled font
    void fontAscentMM(); //test calculating font ascent in mm
    void fontDescentMM(); //test calculating font descent in mm
    void fontHeightMM(); //test calculating font height in mm
    void fontHeightCharacterMM(); //test calculating font character height in mm
    void textWidthMM(); //test calculating text width in mm
    void textHeightMM(); //test calculating text height in mm
    void drawTextPos(); //test drawing text at a pos
    void drawTextRect(); //test drawing text in a rect
    void createRenderContextFromComposition();
    void createRenderContextFromMap();

  private:
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );
    QgsComposition *mComposition = nullptr;
    QString mReport;
    QFont mTestFont;

};

TestQgsComposerUtils::TestQgsComposerUtils() = default;


void TestQgsComposerUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis(); //for access to test font

  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = QStringLiteral( "<h1>Composer Utils Tests</h1>\n" );

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Oblique" ) );
  mTestFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Oblique " ) );
  mTestFont.setItalic( true );

}

void TestQgsComposerUtils::cleanupTestCase()
{
  delete mComposition;

  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerUtils::init()
{

}

void TestQgsComposerUtils::cleanup()
{

}

void TestQgsComposerUtils::drawArrowHead()
{
  //test drawing with no painter
  QgsComposerUtils::drawArrowHead( 0, 100, 100, 90, 30 );

  //test painting on to image
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsComposerUtils::drawArrowHead( &testPainter, 100, 100, 45, 30 );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawarrowhead", testImage, 40 ) );
}

void TestQgsComposerUtils::angle()
{
  //test angle with zero length line
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 1, 1 ) ), 0.0 );

  //test angles to different quadrants
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 1, 2 ) ), 180.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 2, 2 ) ), 135.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 2, 1 ) ), 90.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 2, 0 ) ), 45.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 1, 0 ) ), 0.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 0, 0 ) ), 315.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 0, 1 ) ), 270.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 0, 2 ) ), 225.0 );
}

void TestQgsComposerUtils::rotate()
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
    QgsComposerUtils::rotate( ( *it ).second, x, y );
    QGSCOMPARENEAR( x, ( *it ).first.x2(), 4 * DBL_EPSILON );
    QGSCOMPARENEAR( y, ( *it ).first.y2(), 4 * DBL_EPSILON );
  }
}

void TestQgsComposerUtils::normalizedAngle()
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
    QGSCOMPARENEAR( QgsComposerUtils::normalizedAngle( ( *it ).first ), ( *it ).second, 4 * DBL_EPSILON );
  }
}

void TestQgsComposerUtils::snappedAngle()
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
    QGSCOMPARENEAR( QgsComposerUtils::snappedAngle( ( *it ).first ), ( *it ).second, 4 * DBL_EPSILON );
  }
}

void TestQgsComposerUtils::largestRotatedRect()
{
  QRectF wideRect = QRectF( 0, 0, 2, 1 );
  QRectF highRect = QRectF( 0, 0, 1, 2 );
  QRectF bounds = QRectF( 0, 0, 4, 2 );

  //simple cases
  //0 rotation
  QRectF result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 0 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 0 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );
  // 90 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 90 );
  QCOMPARE( result, QRectF( 1.5, 0, 2, 1 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 90 );
  QCOMPARE( result, QRectF( 0, 0, 2, 4 ) );
  // 180 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 180 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 0 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );
  // 270 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 270 );
  QCOMPARE( result, QRectF( 1.5, 0, 2, 1 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 270 );
  QCOMPARE( result, QRectF( 0, 0, 2, 4 ) );
  //360 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 360 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 360 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );

  //full test, run through a circle in 10 degree increments
  for ( double rotation = 10; rotation < 360; rotation += 10 )
  {
    result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, rotation );
    QTransform t;
    t.rotate( rotation );
    QRectF rotatedRectBounds = t.mapRect( result );
    //one of the rotated rects dimensions must equal the bounding rectangles dimensions (ie, it has been constrained by one dimension)
    //and the other dimension must be less than or equal to bounds dimension
    QVERIFY( ( qgsDoubleNear( rotatedRectBounds.width(), bounds.width(), 0.001 ) && ( rotatedRectBounds.height() <= bounds.height() ) )
             || ( qgsDoubleNear( rotatedRectBounds.height(), bounds.height(), 0.001 ) && ( rotatedRectBounds.width() <= bounds.width() ) ) );

    //also verify that aspect ratio of rectangle has not changed
    QGSCOMPARENEAR( result.width() / result.height(), wideRect.width() / wideRect.height(), 4 * DBL_EPSILON );
  }
  //and again for the high rectangle
  for ( double rotation = 10; rotation < 360; rotation += 10 )
  {
    result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, rotation );
    QTransform t;
    t.rotate( rotation );
    QRectF rotatedRectBounds = t.mapRect( result );
    //one of the rotated rects dimensions must equal the bounding rectangles dimensions (ie, it has been constrained by one dimension)
    //and the other dimension must be less than or equal to bounds dimension
    QVERIFY( ( qgsDoubleNear( rotatedRectBounds.width(), bounds.width(), 0.001 ) && ( rotatedRectBounds.height() <= bounds.height() ) )
             || ( qgsDoubleNear( rotatedRectBounds.height(), bounds.height(), 0.001 ) && ( rotatedRectBounds.width() <= bounds.width() ) ) );

    //also verify that aspect ratio of rectangle has not changed
    QGSCOMPARENEAR( result.width() / result.height(), highRect.width() / highRect.height(), 4 * DBL_EPSILON );
  }
}

void TestQgsComposerUtils::pointsToMM()
{
  //test conversion of points to mm, based on 1 point = 1 / 72 of an inch
  QGSCOMPARENEAR( QgsComposerUtils::pointsToMM( 72 / 25.4 ), 1, 0.001 );
}

void TestQgsComposerUtils::mmToPoints()
{
  //test conversion of mm to points, based on 1 point = 1 / 72 of an inch
  QGSCOMPARENEAR( QgsComposerUtils::mmToPoints( 25.4 / 72 ), 1, 0.001 );
}

void TestQgsComposerUtils::relativePosition()
{
  //+ve gradient
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 1, 0, 2, 0, 4 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 0, 0, 2, 0, 4 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 2, 0, 2, 0, 4 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 4, 0, 2, 0, 4 ), 8, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( -2, 0, 2, 0, 4 ), -4, 0.001 );
  //-ve gradient
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 1, 0, 2, 4, 0 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 0, 0, 2, 4, 0 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 2, 0, 2, 4, 0 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 4, 0, 2, 4, 0 ), -4, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( -2, 0, 2, 4, 0 ), 8, 0.001 );
  //-ve domain
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 1, 2, 0, 0, 4 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 0, 2, 0, 0, 4 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 2, 2, 0, 0, 4 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 4, 2, 0, 0, 4 ), -4, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( -2, 2, 0, 0, 4 ), 8, 0.001 );
  //-ve domain and gradient
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 1, 2, 0, 4, 0 ), 2, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 0, 2, 0, 4, 0 ), 0, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 2, 2, 0, 4, 0 ), 4, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( 4, 2, 0, 4, 0 ), 8, 0.001 );
  QGSCOMPARENEAR( QgsComposerUtils::relativePosition( -2, 2, 0, 4, 0 ), -4, 0.001 );
}

void TestQgsComposerUtils::relativeResizeRect()
{
  //test rectangle which fills bounds
  QRectF testRect = QRectF( 0, 0, 1, 1 );
  QRectF boundsBefore = QRectF( 0, 0, 1, 1 );
  QRectF boundsAfter = QRectF( 0, 0, 1, 1 );
  QgsComposerUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 1, 1 ) );
  testRect = QRectF( 0, 0, 1, 1 );
  boundsAfter = QRectF( 0, 0, 2, 2 );
  QgsComposerUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 2, 2 ) );
  testRect = QRectF( 0, 0, 1, 1 );
  boundsAfter = QRectF( 0, 0, 0.5, 4 );
  QgsComposerUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0, 0, 0.5, 4 ) );

  //test rectangle which doesn't fill bounds
  testRect = QRectF( 1, 2, 1, 2 );
  boundsBefore = QRectF( 0, 0, 4, 8 );
  boundsAfter = QRectF( 0, 0, 2, 4 );
  QgsComposerUtils::relativeResizeRect( testRect, boundsBefore, boundsAfter );
  QCOMPARE( testRect, QRectF( 0.5, 1, 0.5, 1 ) );
}

void TestQgsComposerUtils::decodePaperOrientation()
{
  QgsComposition::PaperOrientation orientation;
  bool ok = false;
  orientation = QgsComposerUtils::decodePaperOrientation( QStringLiteral( "bad string" ), ok );
  QVERIFY( !ok );
  QCOMPARE( orientation, QgsComposition::Landscape ); //should default to landscape
  ok = false;
  orientation = QgsComposerUtils::decodePaperOrientation( QStringLiteral( "portrait" ), ok );
  QVERIFY( ok );
  QCOMPARE( orientation, QgsComposition::Portrait );
  ok = false;
  orientation = QgsComposerUtils::decodePaperOrientation( QStringLiteral( "LANDSCAPE" ), ok );
  QVERIFY( ok );
  QCOMPARE( orientation, QgsComposition::Landscape );
}

void TestQgsComposerUtils::decodePaperSize()
{
  double width = 0;
  double height = 0;
  QVERIFY( ! QgsComposerUtils::decodePresetPaperSize( "bad string", width, height ) );

  //good strings
  QVERIFY( QgsComposerUtils::decodePresetPaperSize( "a4", width, height ) );
  QCOMPARE( width, 210.0 );
  QCOMPARE( height, 297.0 );
  QVERIFY( QgsComposerUtils::decodePresetPaperSize( "B0", width, height ) );
  QCOMPARE( width, 1000.0 );
  QCOMPARE( height, 1414.0 );
  QVERIFY( QgsComposerUtils::decodePresetPaperSize( "letter", width, height ) );
  QCOMPARE( width, 215.9 );
  QCOMPARE( height, 279.4 );
  QVERIFY( QgsComposerUtils::decodePresetPaperSize( "LEGAL", width, height ) );
  QCOMPARE( width, 215.9 );
  QCOMPARE( height, 355.6 );
}

void TestQgsComposerUtils::readOldDataDefinedProperty()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement itemElem = doc.createElement( QStringLiteral( "item" ) );

  //dd element
  QDomElement ddElem = doc.createElement( QStringLiteral( "dataDefinedTestProperty" ) );
  ddElem.setAttribute( QStringLiteral( "active" ), QStringLiteral( "true" ) );
  ddElem.setAttribute( QStringLiteral( "useExpr" ), QStringLiteral( "true" ) );
  ddElem.setAttribute( QStringLiteral( "expr" ), QStringLiteral( "test expression" ) );
  ddElem.setAttribute( QStringLiteral( "field" ), QStringLiteral( "test field" ) );
  itemElem.appendChild( ddElem );
  rootNode.appendChild( itemElem );

  //try reading dd elements

  //bad data defined properties - should not be read into dataDefinedProperties map
  QVERIFY( !QgsComposerUtils::readOldDataDefinedProperty( QgsComposerObject::NoProperty, ddElem ) );
  QVERIFY( !QgsComposerUtils::readOldDataDefinedProperty( QgsComposerObject::AllProperties, ddElem ) );

  //read into valid property
  QgsProperty p( QgsComposerUtils::readOldDataDefinedProperty( QgsComposerObject::TestProperty, ddElem ) );
  QVERIFY( p );
  QVERIFY( p.isActive() );
  QCOMPARE( p.propertyType(), QgsProperty::ExpressionBasedProperty );
  QCOMPARE( p.expressionString(), QString( "test expression" ) );

  ddElem.setAttribute( QStringLiteral( "useExpr" ), QStringLiteral( "false" ) );
  p = QgsComposerUtils::readOldDataDefinedProperty( QgsComposerObject::TestProperty, ddElem );
  QVERIFY( p );
  QVERIFY( p.isActive() );
  QCOMPARE( p.propertyType(), QgsProperty::FieldBasedProperty );
  QCOMPARE( p.field(), QString( "test field" ) );

  //reading false parameters
  QDomElement ddElem2 = doc.createElement( QStringLiteral( "dataDefinedProperty2" ) );
  ddElem2.setAttribute( QStringLiteral( "active" ), QStringLiteral( "false" ) );
  itemElem.appendChild( ddElem2 );
  p = QgsComposerUtils::readOldDataDefinedProperty( QgsComposerObject::TestProperty, ddElem2 );
  QVERIFY( p );
  QVERIFY( !p.isActive() );
}

void TestQgsComposerUtils::readOldDataDefinedPropertyMap()
{
  //create a test dom element
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement itemElem = doc.createElement( QStringLiteral( "item" ) );

  //dd elements
  QDomElement ddElem = doc.createElement( QStringLiteral( "dataDefinedBlendMode" ) );
  ddElem.setAttribute( QStringLiteral( "active" ), QStringLiteral( "true" ) );
  ddElem.setAttribute( QStringLiteral( "useExpr" ), QStringLiteral( "true" ) );
  ddElem.setAttribute( QStringLiteral( "expr" ), QStringLiteral( "test expression" ) );
  ddElem.setAttribute( QStringLiteral( "field" ), QStringLiteral( "test field" ) );
  itemElem.appendChild( ddElem );
  QDomElement ddElem2 = doc.createElement( QStringLiteral( "dataDefinedTransparency" ) );
  ddElem2.setAttribute( QStringLiteral( "active" ), QStringLiteral( "false" ) );
  ddElem2.setAttribute( QStringLiteral( "useExpr" ), QStringLiteral( "false" ) );
  ddElem2.setAttribute( QStringLiteral( "expr" ), QStringLiteral( "test expression 2" ) );
  ddElem2.setAttribute( QStringLiteral( "field" ), QStringLiteral( "test field 2" ) );
  itemElem.appendChild( ddElem2 );
  QDomElement ddElem3 = doc.createElement( QStringLiteral( "dataDefinedProperty" ) );
  ddElem3.setAttribute( QStringLiteral( "active" ), QStringLiteral( "true" ) );
  ddElem3.setAttribute( QStringLiteral( "useExpr" ), QStringLiteral( "false" ) );
  ddElem3.setAttribute( QStringLiteral( "expr" ), QStringLiteral( "test expression 3" ) );
  ddElem3.setAttribute( QStringLiteral( "field" ), QStringLiteral( "test field 3" ) );
  itemElem.appendChild( ddElem3 );
  rootNode.appendChild( itemElem );

  //try reading dd elements
  QgsPropertyCollection dataDefinedProperties;

  QgsComposerUtils::readOldDataDefinedPropertyMap( itemElem, dataDefinedProperties );
  //check returned values
  QCOMPARE( dataDefinedProperties.count(), 3 );
  QVERIFY( ( dataDefinedProperties.property( QgsComposerObject::BlendMode ) ).isActive() );
  QCOMPARE( ( dataDefinedProperties.property( QgsComposerObject::BlendMode ) ).propertyType(), QgsProperty::ExpressionBasedProperty );
  QCOMPARE( dataDefinedProperties.property( QgsComposerObject::BlendMode ).expressionString(), QString( "test expression" ) );
  QVERIFY( !( dataDefinedProperties.property( QgsComposerObject::Opacity ) ).isActive() );
  QCOMPARE( ( dataDefinedProperties.property( QgsComposerObject::Opacity ) ).propertyType(), QgsProperty::FieldBasedProperty );
  QCOMPARE( dataDefinedProperties.property( QgsComposerObject::Opacity ).field(), QString( "test field 2" ) );
  QVERIFY( ( dataDefinedProperties.property( QgsComposerObject::TestProperty ) ).isActive() );
  QCOMPARE( ( dataDefinedProperties.property( QgsComposerObject::TestProperty ) ).propertyType(), QgsProperty::FieldBasedProperty );
  QCOMPARE( dataDefinedProperties.property( QgsComposerObject::TestProperty ).field(), QString( "test field 3" ) );
}

void TestQgsComposerUtils::scaledFontPixelSize()
{
  //create a 12 point test font
  mTestFont.setPointSize( 12 );

  //test scaling of font for painting
  QFont scaledFont = QgsComposerUtils::scaledFontPixelSize( mTestFont );
  QCOMPARE( scaledFont.pixelSize(), 42 );
  QCOMPARE( scaledFont.family(), mTestFont.family() );
}

void TestQgsComposerUtils::fontAscentMM()
{
  mTestFont.setPointSize( 12 );
  //platform specific font rendering differences mean these tests need to be very leniant
  QGSCOMPARENEAR( QgsComposerUtils::fontAscentMM( mTestFont ), 3.9, 0.5 );
}

void TestQgsComposerUtils::fontDescentMM()
{
  mTestFont.setPointSize( 12 );
  QGSCOMPARENEAR( QgsComposerUtils::fontDescentMM( mTestFont ), 0.9, 0.05 );
}

void TestQgsComposerUtils::fontHeightMM()
{
  mTestFont.setPointSize( 12 );
  //platform specific font rendering differences mean these tests need to be very leniant
  QGSCOMPARENEAR( QgsComposerUtils::fontHeightMM( mTestFont ), 4.9, 0.5 );
}

void TestQgsComposerUtils::fontHeightCharacterMM()
{
  mTestFont.setPointSize( 12 );
  //platform specific font rendering differences mean these tests need to be very leniant
  QGSCOMPARENEAR( QgsComposerUtils::fontHeightCharacterMM( mTestFont, QChar( 'a' ) ), 2.4, 0.15 );
  QGSCOMPARENEAR( QgsComposerUtils::fontHeightCharacterMM( mTestFont, QChar( 'l' ) ), 3.15, 0.16 );
  QGSCOMPARENEAR( QgsComposerUtils::fontHeightCharacterMM( mTestFont, QChar( 'g' ) ), 3.2, 0.11 );
}

void TestQgsComposerUtils::textWidthMM()
{
  //platform specific font rendering differences mean this test needs to be very leniant
  mTestFont.setPointSize( 12 );
  QGSCOMPARENEAR( QgsComposerUtils::textWidthMM( mTestFont, QString( "test string" ) ), 20, 2 );
}

void TestQgsComposerUtils::textHeightMM()
{
  //platform specific font rendering differences mean this test needs to be very leniant
  mTestFont.setPointSize( 12 );
  QgsDebugMsg( QString( "height: %1" ).arg( QgsComposerUtils::textHeightMM( mTestFont, QString( "test string" ) ) ) );
  QGSCOMPARENEAR( QgsComposerUtils::textHeightMM( mTestFont, QString( "test string" ) ), 3.9, 0.2 );
  QgsDebugMsg( QString( "height: %1" ).arg( QgsComposerUtils::textHeightMM( mTestFont, QString( "test\nstring" ) ) ) );
  QGSCOMPARENEAR( QgsComposerUtils::textHeightMM( mTestFont, QString( "test\nstring" ) ), 8.7, 0.2 );
  QgsDebugMsg( QString( "height: %1" ).arg( QgsComposerUtils::textHeightMM( mTestFont, QString( "test\nstring" ), 2 ) ) );
  QGSCOMPARENEAR( QgsComposerUtils::textHeightMM( mTestFont, QString( "test\nstring" ), 2 ), 13.5, 0.2 );
  QgsDebugMsg( QString( "height: %1" ).arg( QgsComposerUtils::textHeightMM( mTestFont, QString( "test\nstring\nstring" ) ) ) );
  QGSCOMPARENEAR( QgsComposerUtils::textHeightMM( mTestFont, QString( "test\nstring\nstring" ) ), 13.5, 0.2 );
}

void TestQgsComposerUtils::drawTextPos()
{
  //test drawing with no painter
  QgsComposerUtils::drawText( 0, QPointF( 5, 15 ), QStringLiteral( "Abc123" ), mTestFont );

  //test drawing text on to image
  mTestFont.setPointSize( 48 );
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsComposerUtils::drawText( &testPainter, QPointF( 5, 15 ), QStringLiteral( "Abc123" ), mTestFont, Qt::white );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_pos", testImage, 100 ) );

  //test drawing with pen color set on painter and no specified color
  //text should be drawn using painter pen color
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  testPainter.setPen( QPen( Qt::green ) );
  QgsComposerUtils::drawText( &testPainter, QPointF( 5, 15 ), QStringLiteral( "Abc123" ), mTestFont );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_posnocolor", testImage, 100 ) );
}

void TestQgsComposerUtils::drawTextRect()
{
  //test drawing with no painter
  QgsComposerUtils::drawText( 0, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont );

  //test drawing text on to image
  mTestFont.setPointSize( 48 );
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsComposerUtils::drawText( &testPainter, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont, Qt::white );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rect", testImage, 100 ) );

  //test drawing with pen color set on painter and no specified color
  //text should be drawn using painter pen color
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  testPainter.setPen( QPen( Qt::green ) );
  QgsComposerUtils::drawText( &testPainter, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rectnocolor", testImage, 100 ) );

  //test alignment settings
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  QgsComposerUtils::drawText( &testPainter, QRectF( 5, 15, 200, 50 ), QStringLiteral( "Abc123" ), mTestFont, Qt::black, Qt::AlignRight, Qt::AlignBottom );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rectalign", testImage, 100 ) );

  //test extra flags - render without clipping
  testImage.fill( qRgb( 152, 219, 249 ) );
  testPainter.begin( &testImage );
  QgsComposerUtils::drawText( &testPainter, QRectF( 5, 15, 20, 50 ), QStringLiteral( "Abc123" ), mTestFont, Qt::white, Qt::AlignLeft, Qt::AlignTop, Qt::TextDontClip );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawtext_rectflag", testImage, 100 ) );
}

void TestQgsComposerUtils::createRenderContextFromComposition()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no composition
  QgsRenderContext rc = QgsComposerUtils::createRenderContextForComposition( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no composition, no painter
  rc = QgsComposerUtils::createRenderContextForComposition( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create composition with no reference map
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );
  rc = QgsComposerUtils::createRenderContextForComposition( composition, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // composition, no map, no painter
  rc = QgsComposerUtils::createRenderContextForComposition( composition, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  // add a reference map
  QgsComposerMap *map = new QgsComposerMap( composition );
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  composition->addComposerMap( map );
  composition->setReferenceMap( map );

  rc = QgsComposerUtils::createRenderContextForComposition( composition, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QCOMPARE( rc.painter(), &p );

  // composition, reference map, no painter
  rc = QgsComposerUtils::createRenderContextForComposition( composition, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QVERIFY( !rc.painter() );

  p.end();
}

void TestQgsComposerUtils::createRenderContextFromMap()
{
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.setDotsPerMeterX( 150 / 25.4 * 1000 );
  testImage.setDotsPerMeterY( 150 / 25.4 * 1000 );
  QPainter p( &testImage );

  // no map
  QgsRenderContext rc = QgsComposerUtils::createRenderContextForMap( nullptr, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QCOMPARE( rc.painter(), &p );

  // no map, no painter
  rc = QgsComposerUtils::createRenderContextForMap( nullptr, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QVERIFY( !rc.painter() );

  //create composition with no reference map
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );

  // add a map
  QgsComposerMap *map = new QgsComposerMap( composition );
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  composition->addComposerMap( map );

  rc = QgsComposerUtils::createRenderContextForMap( map, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QCOMPARE( rc.painter(), &p );

  // map, no painter
  rc = QgsComposerUtils::createRenderContextForMap( map, nullptr );
  QGSCOMPARENEAR( rc.scaleFactor(), 88 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map->scale(), 1000000 );
  QVERIFY( !rc.painter() );

  // secondary map
  QgsComposerMap *map2 = new QgsComposerMap( composition );
  map2->setNewExtent( extent );
  map2->setSceneRect( QRectF( 30, 60, 100, 50 ) );
  composition->addComposerMap( map2 );
  rc = QgsComposerUtils::createRenderContextForMap( map2, &p );
  QGSCOMPARENEAR( rc.scaleFactor(), 150 / 25.4, 0.001 );
  QGSCOMPARENEAR( rc.rendererScale(), map2->scale(), 1000000 );
  QVERIFY( rc.painter() );

  p.end();
}

bool TestQgsComposerUtils::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "composer_utils" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsComposerUtils )
#include "testqgscomposerutils.moc"
