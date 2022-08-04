
/***************************************************************************
                         testqgslayoutpolyline.cpp
                         ---------------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsapplication.h"
#include "qgslayoutitempolyline.h"
#include "qgsrenderchecker.h"

#include <QLocale>
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutPolyline : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutPolyline() : QgsTest( QStringLiteral( "Layout Polyline Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void drawArrowHead();

  private:
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );

};

void TestQgsLayoutPolyline::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayoutPolyline::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutPolyline::drawArrowHead()
{
  //test drawing with no painter
  QgsLayoutItemPolyline::drawArrowHead( nullptr, 100, 100, 90, 30 );

  //test painting on to image
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsLayoutItemPolyline::drawArrowHead( &testPainter, 100, 100, 45, 30 );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawarrowhead", testImage, 40 ) );
}


bool TestQgsLayoutPolyline::renderCheck( const QString &testName, QImage &image, int mismatchCount )
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

QGSTEST_MAIN( TestQgsLayoutPolyline )
#include "testqgslayoutpolyline.moc"
