/***************************************************************************
    qgsmultirenderchecker.cpp
     --------------------------------------
    Date                 : 6.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmultirenderchecker.h"
#include "qgscomposition.h"
#include "qgslayout.h"
#include <QDebug>

void QgsMultiRenderChecker::setControlName( const QString &name )
{
  mControlName = name;
}

void QgsMultiRenderChecker::setControlPathPrefix( const QString &prefix )
{
  mControlPathPrefix = prefix;
}

void QgsMultiRenderChecker::setMapSettings( const QgsMapSettings &mapSettings )
{
  mMapSettings = mapSettings;
}

bool QgsMultiRenderChecker::runTest( const QString &testName, unsigned int mismatchCount )
{
  bool successful = false;

  const QString baseDir = controlImagePath();

  QStringList subDirs = QDir( baseDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot );

  if ( subDirs.isEmpty() )
  {
    subDirs << QLatin1String( "" );
  }

  QVector<QgsDartMeasurement> dartMeasurements;

  Q_FOREACH ( const QString &suffix, subDirs )
  {
    qDebug() << "Checking subdir " << suffix;
    bool result;
    QgsRenderChecker checker;
    checker.enableDashBuffering( true );
    checker.setColorTolerance( mColorTolerance );
    checker.setControlPathPrefix( mControlPathPrefix );
    checker.setControlPathSuffix( suffix );
    checker.setControlName( mControlName );
    checker.setMapSettings( mMapSettings );

    if ( !mRenderedImage.isNull() )
    {
      checker.setRenderedImage( mRenderedImage );
      result = checker.compareImages( testName, mismatchCount, mRenderedImage );
    }
    else
    {
      result = checker.runTest( testName, mismatchCount );
      mRenderedImage = checker.renderedImage();
    }

    successful |= result;

    dartMeasurements << checker.dartMeasurements();

    mReport += checker.report();
  }

  if ( !successful )
  {
    Q_FOREACH ( const QgsDartMeasurement &measurement, dartMeasurements )
      measurement.send();

    QgsDartMeasurement msg( QStringLiteral( "Image not accepted by test" ), QgsDartMeasurement::Text, "This may be caused because the test is supposed to fail or rendering inconsistencies."
                            "If this is a rendering inconsistency, please add another control image folder, add an anomaly image or increase the color tolerance." );
    msg.send();
  }

  return successful;
}

QString QgsMultiRenderChecker::controlImagePath() const
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myControlImageDir = myDataDir + QDir::separator() + "control_images" +
                              QDir::separator() + mControlPathPrefix + QDir::separator() + mControlName + QDir::separator();
  return myControlImageDir;
}

#ifdef ENABLE_TESTS

//
// QgsCompositionChecker
//

///@cond PRIVATE

QgsCompositionChecker::QgsCompositionChecker( const QString &testName, QgsComposition *composition )
  : mTestName( testName )
  , mComposition( composition )
  , mSize( 1122, 794 )
  , mDotsPerMeter( 96 / 25.4 * 1000 )
{
  // The composer has some slight render inconsistencies on the whole image sometimes
  setColorTolerance( 5 );
}

QgsCompositionChecker::QgsCompositionChecker()
  : mDotsPerMeter( 96 / 25.4 * 1000 )
{
}

bool QgsCompositionChecker::testComposition( QString &checkedReport, int page, int pixelDiff )
{
  if ( !mComposition )
  {
    return false;
  }

  setControlName( "expected_" + mTestName );

#if 0
  //fake mode to generate expected image
  //assume 96 dpi and size of the control image 1122 * 794
  QImage newImage( QSize( 1122, 794 ), QImage::Format_RGB32 );
  mComposition->setPlotStyle( QgsComposition::Print );
  newImage.setDotsPerMeterX( 96 / 25.4 * 1000 );
  newImage.setDotsPerMeterY( 96 / 25.4 * 1000 );
  drawBackground( &newImage );
  QPainter expectedPainter( &newImage );
  //QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
  //QRectF targetArea( 0, 0, 3507, 2480 );
  mComposition->renderPage( &expectedPainter, page );
  expectedPainter.end();
  newImage.save( controlImagePath() + QDir::separator() + "expected_" + mTestName + ".png", "PNG" );
  return true;
#endif //0

  QImage outputImage( mSize, QImage::Format_RGB32 );

  mComposition->setPlotStyle( QgsComposition::Print );
  outputImage.setDotsPerMeterX( mDotsPerMeter );
  outputImage.setDotsPerMeterY( mDotsPerMeter );
  drawBackground( &outputImage );
  QPainter p( &outputImage );
  mComposition->renderPage( &p, page );
  p.end();

  QString renderedFilePath = QDir::tempPath() + '/' + QFileInfo( mTestName ).baseName() + "_rendered.png";
  outputImage.save( renderedFilePath, "PNG" );

  setRenderedImage( renderedFilePath );

  bool testResult = runTest( mTestName, pixelDiff );

  checkedReport += report();

  return testResult;
}



//
// QgsLayoutChecker
//

QgsLayoutChecker::QgsLayoutChecker( const QString &testName, QgsLayout *layout )
  : mTestName( testName )
  , mLayout( layout )
  , mSize( 1122, 794 )
  , mDotsPerMeter( 96 / 25.4 * 1000 )
{
  // Qt has some slight render inconsistencies on the whole image sometimes
  setColorTolerance( 5 );
}

bool QgsLayoutChecker::testLayout( QString &checkedReport, int page, int pixelDiff )
{
  if ( !mLayout )
  {
    return false;
  }

  setControlName( "expected_" + mTestName );

#if 0
  //fake mode to generate expected image
  //assume 96 dpi and size of the control image 1122 * 794
  QImage newImage( QSize( 1122, 794 ), QImage::Format_RGB32 );
  mComposition->setPlotStyle( QgsComposition::Print );
  newImage.setDotsPerMeterX( 96 / 25.4 * 1000 );
  newImage.setDotsPerMeterY( 96 / 25.4 * 1000 );
  drawBackground( &newImage );
  QPainter expectedPainter( &newImage );
  //QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
  //QRectF targetArea( 0, 0, 3507, 2480 );
  mComposition->renderPage( &expectedPainter, page );
  expectedPainter.end();
  newImage.save( controlImagePath() + QDir::separator() + "expected_" + mTestName + ".png", "PNG" );
  return true;
#endif //0

  QImage outputImage( mSize, QImage::Format_RGB32 );
  outputImage.setDotsPerMeterX( mDotsPerMeter );
  outputImage.setDotsPerMeterY( mDotsPerMeter );
  drawBackground( &outputImage );
  QPainter p( &outputImage );
  mLayout->exporter().renderPage( &p, page );
  p.end();

  QString renderedFilePath = QDir::tempPath() + '/' + QFileInfo( mTestName ).baseName() + "_rendered.png";
  outputImage.save( renderedFilePath, "PNG" );

  setRenderedImage( renderedFilePath );

  bool testResult = runTest( mTestName, pixelDiff );

  checkedReport += report();

  return testResult;
}


///@endcond

#endif
