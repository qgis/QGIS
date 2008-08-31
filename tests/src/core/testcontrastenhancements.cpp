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
#include <QtTest>
#include <QObject>
#include <QApplication>
#include <QDesktopServices>


//qgis includes...
#include <qgsrasterlayer.h>
#include <qgscliptominmaxenhancement.h>
#include <qgscontrastenhancement.h>
#include <qgslinearminmaxenhancement.h>

/** \ingroup UnitTests
 * This is a unit test for the ContrastEnhancements contrast enhancement classes.
 */
class TestContrastEnhancements: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void minMaxEnhancementTest();
    void linearMinMaxEnhancementTest();
  private:
    QString mReport;
};

//runs before all tests
void TestContrastEnhancements::initTestCase()
{
  mReport += "<h1>Raster Contrast Enhancement Tests</h1>\n";
}
//runs after all tests
void TestContrastEnhancements::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "contrastenhancementest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    QDesktopServices::openUrl( "file://" + myReportFile );
  }

}


void TestContrastEnhancements::minMaxEnhancementTest()
{
  QgsClipToMinMaxEnhancement myEnhancement(QgsContrastEnhancement::QGS_Byte, 10.0, 240.0);
  QVERIFY(!myEnhancement.isValueInDisplayableRange(0.0));
  QVERIFY(10.0 == myEnhancement.enhanceValue(0.0)) ; 
  QVERIFY(250.0 == myEnhancement.enhanceValue(240.0)) ; 
}
void TestContrastEnhancements::linearMinMaxEnhancementTest()
{
  QgsLinearMinMaxEnhancement myEnhancement(QgsContrastEnhancement::QGS_Byte, 10.0, 240.0);
  //0 should be scaled to 10 and not clipped
  QVERIFY(myEnhancement.isValueInDisplayableRange(0.0));
  QVERIFY(10.0 == myEnhancement.enhanceValue(0.0)) ; 
  QVERIFY(250.0 == myEnhancement.enhanceValue(240.0)) ; 
}
QTEST_MAIN( TestContrastEnhancements )
#include "moc_testcontrastenhancements.cxx"

