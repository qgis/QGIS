/***************************************************************************
     testziplayer.cpp
     --------------------------------------
    Date                 : Mon Jul 16 15:50:29 BRT 2012
    Copyright            : (C) 2012 Etienne Tourigny
    Email                : etourigny.dev@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QObject>
#include <QApplication>
#include <QFileInfo>

#include <qwt_global.h>

//qgis includes...
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsrasterlayer.h>
#include <qgsconfig.h>
#include "qgssinglebandgrayrendererwidget.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgsrasterhistogramwidget.h"

//qgis unit test includes
#include <qgsrenderchecker.h>


/** \ingroup UnitTests
 * This is a unit test to verify that raster histogram works
 */
class TestRasterHistogram: public QObject
{
    Q_OBJECT

  private:

    QString mDataDir;
    QString mTestPrefix;
    int mWidth, mHeight, mImageQuality;
    QgsRasterLayer* mRasterLayer;
    QgsSingleBandGrayRendererWidget* mGrayRendererWidget;
    QgsMultiBandColorRendererWidget* mRGBRendererWidget;
    QgsSingleBandPseudoColorRendererWidget* mPseudoRendererWidget;
    QgsRasterHistogramWidget* mHistogramWidget;
    QString mReport;

    bool openLayer( const QString& fileName );
    void closeLayer();
    bool saveImage( const QString& fileName );
    int testFile( QString testName,
                  QString rendererName,
                  QgsRasterRendererWidget* rendererWidget,
                  QStringList actionsList = QStringList(),
                  int selectedBand = -1 );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    // tests
    void testGray1();
    void testGray2();
    void testRGB1();
    void testRGB2();
    void testRGB3();
    void testRGB4();
    void testPseudo1();
};


// slots

void TestRasterHistogram::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n", "<br />" );

  // output test environment
  QgsApplication::showSettings();
  qDebug() << "QWT version:   " << QWT_VERSION_STR;
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  mTestPrefix = "histogram_qwt6";
#else
  mTestPrefix = "histogram_qwt5";
#endif

  // save data dir
  mDataDir = QString( TEST_DATA_DIR ) + QDir::separator();
  mWidth = mHeight = 400;
  mImageQuality = -1;
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  // setup objects
  mRasterLayer = 0;
  mGrayRendererWidget = 0;
  mRGBRendererWidget = 0;
  mPseudoRendererWidget = 0;
  mHistogramWidget = 0;

  mReport += "<h1>Raster Histogram Tests</h1>\n";
  mReport += "<p>" + mySettings + "</p>";

  // remove .aux.xml file to make sure histogram computation is fresh
  QFile::remove( mDataDir + QDir::separator() + "landsat.tif.aux.xml" );
  QVERIFY( openLayer( "landsat.tif" ) );
}

void TestRasterHistogram::cleanupTestCase()
{
  closeLayer();
  // remove .aux.xml file to make sure histogram computation is fresh
  QFile::remove( mDataDir + QDir::separator() + "landsat.tif.aux.xml" );
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgishistotest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
}

// grayscale, all bands
void TestRasterHistogram::testGray1()
{
  QStringList actionsList;
  QVERIFY( testFile( "gray1", "singlebandgray", mGrayRendererWidget, actionsList ) >= 0 );
}

// grayscale, gray band
void TestRasterHistogram::testGray2()
{
  QStringList actionsList( "Show RGB" );
  QVERIFY( testFile( "gray2", "singlebandgray", mGrayRendererWidget, actionsList ) >= 0 );
}

// RGB, all bands
void TestRasterHistogram::testRGB1()
{
  QStringList actionsList;
  QVERIFY( testFile( "rgb1", "multibandcolor", mRGBRendererWidget, actionsList ) >= 0 );
}

// RGB, RGB bands
void TestRasterHistogram::testRGB2()
{
  QStringList actionsList( "Show RGB" );
  QVERIFY( testFile( "rgb2", "multibandcolor", mRGBRendererWidget, actionsList ) >= 0 );
}

// RGB, band 5
void TestRasterHistogram::testRGB3()
{
  QStringList actionsList( "Show selected" );
  QVERIFY( testFile( "rgb3", "multibandcolor", mRGBRendererWidget, actionsList, 5 ) >= 0 );
}

// RGB, all bands + markers, load 1 stddev
void TestRasterHistogram::testRGB4()
{
  QStringList actionsList;
  actionsList << "Show selected" << "Show markers" << "Load 1 stddev";
  QVERIFY( testFile( "rgb4", "multibandcolor", mRGBRendererWidget, actionsList ) >= 0 );
}

// pseudocolor, all bands
void TestRasterHistogram::testPseudo1()
{
  QStringList actionsList;
  QVERIFY( testFile( "pseudo1", "singlebandpseudocolor", mPseudoRendererWidget, actionsList ) >= 0 );
}

// helper methods

bool TestRasterHistogram::openLayer( const QString& fileName )
{
  mRasterLayer = new QgsRasterLayer( mDataDir + QDir::separator() + fileName, fileName );
  if ( ! mRasterLayer )
    return false;
  mGrayRendererWidget = new QgsSingleBandGrayRendererWidget( mRasterLayer );
  mRGBRendererWidget = new QgsMultiBandColorRendererWidget( mRasterLayer );
  mPseudoRendererWidget = new QgsSingleBandPseudoColorRendererWidget( mRasterLayer );
  mHistogramWidget = new QgsRasterHistogramWidget( mRasterLayer, 0 );
  mHistogramWidget->computeHistogram( true );
  return true;
}

void TestRasterHistogram::closeLayer()
{
  if ( mHistogramWidget )
  {
    delete mHistogramWidget;
    mHistogramWidget = 0;
  }
  if ( mGrayRendererWidget )
  {
    delete mGrayRendererWidget;
    mGrayRendererWidget = 0;
  }
  if ( mRGBRendererWidget )
  {
    delete mRGBRendererWidget;
    mRGBRendererWidget = 0;
  }
  if ( mPseudoRendererWidget )
  {
    delete mPseudoRendererWidget;
    mPseudoRendererWidget = 0;
  }
  if ( mRasterLayer )
  {
    delete mRasterLayer;
    mRasterLayer = 0;
  }
}

bool TestRasterHistogram::saveImage( const QString& fileName )
{
  return mHistogramWidget->histoSaveAsImage( fileName, mWidth, mHeight, mImageQuality );
}

// test resulting image file - relax this test because there are too many possible outputs depending on machine
// 1 means pass, 0 means warning (different images), -1 means fail (no image output)
int TestRasterHistogram::testFile( QString theTestType,
                                   QString rendererName, QgsRasterRendererWidget* rendererWidget,
                                   QStringList actionsList, int selectedBand )
{
  if ( mRasterLayer == 0 )
  {
    QWARN( QString( "Invalid raster layer" ).toLocal8Bit().data() );
    return false;
  }

  // reset histogram widget to clear previous state
  if ( mHistogramWidget )
    delete mHistogramWidget;
  mHistogramWidget = new QgsRasterHistogramWidget( mRasterLayer, 0 );

  // setup histogram widget
  mHistogramWidget->setRendererWidget( rendererName, rendererWidget );
  foreach ( QString actionName, actionsList )
  {
    mHistogramWidget->histoAction( actionName );
  }
  if ( selectedBand != -1 )
  {
    mHistogramWidget->setSelectedBand( selectedBand );
  }
  QString fileName = QDir::tempPath() + QDir::separator() +
                     theTestType + "_result.png";
  if ( ! saveImage( fileName ) )
  {
    QWARN( QString( "Did not save image file " + fileName ).toLocal8Bit().data() );
    return -1;
  }
  mReport += "<h2>" + theTestType + "</h2>\n";

  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( mTestPrefix );
  myChecker.setControlName( "expected_histo_" + theTestType );
  //  myChecker.setMapRenderer( mpMapRenderer );
  bool myResultFlag = myChecker.compareImages( theTestType, 0, fileName );
  mReport += "\n\n\n" + myChecker.report();

  // return myResultFlag;
  if ( ! myResultFlag )
  {
    QWARN( QString( "Test %1 failed with file %2 " ).arg( theTestType ).arg( fileName ).toLocal8Bit().data() );
    return 0;
  }
  return 1;
}


QTEST_MAIN( TestRasterHistogram )
#include "testqgsrasterhistogram.moc"
