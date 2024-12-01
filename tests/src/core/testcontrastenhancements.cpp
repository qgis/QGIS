/***************************************************************************
     testcontrastenhancements.cpp
     --------------------------------------
    Date                 : Frida  Nov 23  2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim@linfiniti.com
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
#include <QApplication>
#include <QDesktopServices>


//qgis includes...
#include <qgsrasterlayer.h>
#include <qgscliptominmaxenhancement.h>
#include <qgscontrastenhancement.h>
#include <qgslinearminmaxenhancement.h>
#include <qgslinearminmaxenhancementwithclip.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the ContrastEnhancements contrast enhancement classes.
 */
class TestContrastEnhancements : public QgsTest
{
    Q_OBJECT
  public:
    TestContrastEnhancements()
      : QgsTest( QStringLiteral( "Raster Contrast Enhancement Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void clipMinMaxEnhancementTest();
    void linearMinMaxEnhancementWithClipTest();
    void linearMinMaxEnhancementTest();
};


void TestContrastEnhancements::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestContrastEnhancements::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestContrastEnhancements::clipMinMaxEnhancementTest()
{
  //Clips 0 < x < 10, 240 < X < 256
  //Stretch no stretch is applied
  QgsClipToMinMaxEnhancement myEnhancement( Qgis::DataType::Byte, 10.0, 240.0 );
  // Original pixel value 0.0 Should be out of range thus clipped
  QVERIFY( !myEnhancement.isValueInDisplayableRange( 0.0 ) );
  //Original pixel value of 10.0 should be scaled to 10.0
  QVERIFY( 10.0 == myEnhancement.enhance( 10.0 ) );
  //Original pixel value of 240 should be scaled to 240
  QVERIFY( 240.0 == myEnhancement.enhance( 240.0 ) );
}

void TestContrastEnhancements::linearMinMaxEnhancementWithClipTest()
{
  //First clips 0 < x < 10, 240 < X < 256
  //Then stretch 10 = 0, 240 = 255 linearly distribute values 10 -> 240 between 0 -> 255
  QgsLinearMinMaxEnhancementWithClip myEnhancement( Qgis::DataType::Byte, 10.0, 240.0 );
  // Original pixel value 0.0 Should be out of range thus clipped
  QVERIFY( !myEnhancement.isValueInDisplayableRange( 0.0 ) );
  //Original pixel value of 10.0 should be scaled to 0.0
  QVERIFY( 0.0 == myEnhancement.enhance( 10.0 ) );
  //Original pixel value of 240 should be scaled to 255
  QVERIFY( 255.0 == myEnhancement.enhance( 240.0 ) );
}

void TestContrastEnhancements::linearMinMaxEnhancementTest()
{
  //Stretch 10 = 0, 240 = 255 linearly distribute values 10 -> 240 between 0 -> 255
  QgsLinearMinMaxEnhancement myEnhancement( Qgis::DataType::Byte, 10.0, 240.0 );
  //0 should be scaled to 10 and not clipped
  QVERIFY( myEnhancement.isValueInDisplayableRange( 0.0 ) );
  //Original pixel value of 10.0 should be scaled to 0.0
  QVERIFY( 0.0 == myEnhancement.enhance( 10.0 ) );
  //Original pixel value of 240 should be scaled to 255
  QVERIFY( 255.0 == myEnhancement.enhance( 240.0 ) );
}
QGSTEST_MAIN( TestContrastEnhancements )
#include "testcontrastenhancements.moc"
