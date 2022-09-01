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
#include "qgslayout.h"
#include <QDebug>

QgsMultiRenderChecker::QgsMultiRenderChecker()
{
  if ( qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" ) )
    mIsCiRun = true;
}

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
  mResult = false;

  mReport += "<h2>" + testName + "</h2>\n";

  const QString baseDir = controlImagePath();
  if ( !QFile::exists( baseDir ) )
  {
    qDebug() << "Control image path " << baseDir << " does not exist!";
    return mResult;
  }

  QStringList subDirs = QDir( baseDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot );

  if ( subDirs.isEmpty() )
  {
    subDirs << QString();
  }

  QVector<QgsDartMeasurement> dartMeasurements;

  // we can only report one diff image, so just use the first
  QString diffImageFile;

  for ( const QString &suffix : std::as_const( subDirs ) )
  {
    if ( subDirs.count() > 1 )
    {
      qDebug() << "Checking subdir " << suffix;
    }
    bool result;
    QgsRenderChecker checker;
    checker.enableDashBuffering( true );
    checker.setColorTolerance( mColorTolerance );
    checker.setSizeTolerance( mMaxSizeDifferenceX, mMaxSizeDifferenceY );
    checker.setControlPathPrefix( mControlPathPrefix );
    checker.setControlPathSuffix( suffix );
    checker.setControlName( mControlName );
    checker.setMapSettings( mMapSettings );

    if ( !mRenderedImage.isNull() )
    {
      checker.setRenderedImage( mRenderedImage );
      result = checker.compareImages( testName, mismatchCount, mRenderedImage, QgsRenderChecker::Flag::AvoidExportingRenderedImage );
    }
    else
    {
      result = checker.runTest( testName, mismatchCount, QgsRenderChecker::Flag::AvoidExportingRenderedImage );
      mRenderedImage = checker.renderedImage();
    }

    mResult |= result;

    dartMeasurements << checker.dartMeasurements();

    mReport += checker.report( false );

    if ( !mResult && diffImageFile.isEmpty() )
    {
      diffImageFile = checker.mDiffImageFile;
    }
  }

  if ( !mResult && !mExpectFail && mIsCiRun )
  {
    const auto constDartMeasurements = dartMeasurements;
    for ( const QgsDartMeasurement &measurement : constDartMeasurements )
      measurement.send();

    QgsDartMeasurement msg( QStringLiteral( "Image not accepted by test" ), QgsDartMeasurement::Text, "This may be caused because the test is supposed to fail or rendering inconsistencies."
                            "If this is a rendering inconsistency, please add another control image folder, add an anomaly image or increase the color tolerance." );
    msg.send();

#if DUMP_BASE64_IMAGES
    QFile fileSource( mRenderedImage );
    fileSource.open( QIODevice::ReadOnly );

    const QByteArray blob = fileSource.readAll();
    const QByteArray encoded = blob.toBase64();
    qDebug() << "Dumping rendered image " << mRenderedImage << " as base64\n";
    qDebug() << "################################################################";
    qDebug() << encoded;
    qDebug() << "################################################################";
    qDebug() << "End dump";
#endif
  }

  if ( !mResult && !mExpectFail )
  {
    const QDir reportDir = QgsRenderChecker::testReportDir();
    if ( !reportDir.exists() )
    {
      if ( !QDir().mkpath( reportDir.path() ) )
      {
        qDebug() << "!!!!! cannot create " << reportDir.path();
      }
    }
    if ( QFile::exists( mRenderedImage ) )
    {
      QFileInfo fi( mRenderedImage );
      const QString destPath = reportDir.filePath( fi.fileName() );
      if ( QFile::exists( destPath ) )
        QFile::remove( destPath );

      if ( !QFile::copy( mRenderedImage, destPath ) )
      {
        qDebug() << "!!!!! could not copy " << mRenderedImage << " to " << destPath;
      }
    }

    if ( !diffImageFile.isEmpty() && QFile::exists( diffImageFile ) )
    {
      QFileInfo fi( diffImageFile );
      const QString destPath = reportDir.filePath( fi.fileName() );
      if ( QFile::exists( destPath ) )
        QFile::remove( destPath );

      if ( !QFile::copy( diffImageFile, destPath ) )
      {
        qDebug() << "!!!!! could not copy " << diffImageFile << " to " << destPath;
      }
    }
  }

  return mResult;
}

QString QgsMultiRenderChecker::report() const
{
  return !mResult ? mReport : QString();
}

QString QgsMultiRenderChecker::controlImagePath() const
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myControlImageDir = myDataDir + QDir::separator() + "control_images" +
                              QDir::separator() + mControlPathPrefix + QDir::separator() + mControlName + QDir::separator();
  return myControlImageDir;
}

//
// QgsLayoutChecker
//

///@cond PRIVATE

QgsLayoutChecker::QgsLayoutChecker( const QString &testName, QgsLayout *layout )
  : mTestName( testName )
  , mLayout( layout )
  , mSize( 1122, 794 )
  , mDotsPerMeter( 96 / 25.4 * 1000 )
{
  // Qt has some slight render inconsistencies on the whole image sometimes
  setColorTolerance( 5 );
}

bool QgsLayoutChecker::testLayout( QString &checkedReport, int page, int pixelDiff, bool createReferenceImage )
{
#ifdef QT_NO_PRINTER
  return false;
#else
  if ( !mLayout )
  {
    return false;
  }

  setControlName( "expected_" + mTestName );


  if ( createReferenceImage )
  {
    //fake mode to generate expected image
    //assume 96 dpi


    QImage _outputImage( mSize, QImage::Format_RGB32 );
    _outputImage.setDotsPerMeterX( 96 / 25.4 * 1000 );
    _outputImage.setDotsPerMeterY( 96 / 25.4 * 1000 );
    QPainter _p( &_outputImage );
    QgsLayoutExporter _exporter( mLayout );
    _exporter.renderPage( &_p, page );
    _p.end();

    if ( ! QDir( controlImagePath() ).exists() )
    {
      QDir().mkdir( controlImagePath() );
    }
    _outputImage.save( controlImagePath() + QDir::separator() + "expected_" + mTestName + ".png", "PNG" );
    qDebug( ) << "Reference image saved to : " + controlImagePath() + QDir::separator() + "expected_" + mTestName + ".png";

  }

  QImage outputImage( mSize, QImage::Format_RGB32 );
  outputImage.setDotsPerMeterX( mDotsPerMeter );
  outputImage.setDotsPerMeterY( mDotsPerMeter );
  drawBackground( &outputImage );
  QPainter p( &outputImage );
  QgsLayoutExporter exporter( mLayout );
  exporter.renderPage( &p, page );
  p.end();

  QString renderedFilePath = QDir::tempPath() + '/' + QFileInfo( mTestName ).baseName() + "_rendered.png";
  outputImage.save( renderedFilePath, "PNG" );

  setRenderedImage( renderedFilePath );

  bool testResult = runTest( mTestName, pixelDiff );

  checkedReport += report();

  return testResult;
#endif // QT_NO_PRINTER
}



///@endcond
