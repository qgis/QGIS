/***************************************************************************
     qgsrenderchecker.cpp
     --------------------------------------
    Date                 : 18 Jan 2008
    Copyright            : (C) 2008 by Tim Sutton
    Email                : tim @ linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrenderchecker.h"

#include "qgis.h"
#include "qgsmaprenderersequentialjob.h"

#include <QColor>
#include <QPainter>
#include <QImage>
#include <QImageReader>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QBuffer>
#include <QUuid>

#ifndef CMAKE_SOURCE_DIR
#error CMAKE_SOURCE_DIR undefined
#endif // CMAKE_SOURCE_DIR

QString QgsRenderChecker::sourcePath()
{
  static QString sSourcePathPrefix;
  static std::once_flag initialized;
  std::call_once( initialized, []
  {
    sSourcePathPrefix = QString( CMAKE_SOURCE_DIR );
    if ( sSourcePathPrefix.endsWith( '/' ) )
      sSourcePathPrefix.chop( 1 );
  } );
  return sSourcePathPrefix;
}

QgsRenderChecker::QgsRenderChecker()
  : mBasePath( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/control_images/" ) ) //defined in CmakeLists.txt
{
  if ( qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" ) )
    mIsCiRun = true;
}

QDir QgsRenderChecker::testReportDir()
{
  if ( qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" ) )
    return QDir( QDir( "/root/QGIS" ).filePath( QStringLiteral( "qgis_test_report" ) ) );
  else
    return QDir( QDir::temp().filePath( QStringLiteral( "qgis_test_report" ) ) );
}

bool QgsRenderChecker::shouldGenerateReport()
{
  return true;
}

QString QgsRenderChecker::controlImagePath() const
{
  return mBasePath + ( mBasePath.endsWith( '/' ) ? QString() : QStringLiteral( "/" ) ) + mControlPathPrefix;
}

void QgsRenderChecker::setControlImagePath( const QString &path )
{
  mBasePath = path;
}

QString QgsRenderChecker::report( bool ignoreSuccess ) const
{
  return ( ( ignoreSuccess && mResult ) || ( mExpectFail && !mResult ) ) ? QString() : mReport;
}

QString QgsRenderChecker::markdownReport( bool ignoreSuccess ) const
{
  return ( ( ignoreSuccess && mResult ) || ( mExpectFail && !mResult ) ) ? QString() : mMarkdownReport;
}

void QgsRenderChecker::setControlName( const QString &name )
{
  mControlName = name;
  mExpectedImageFile = controlImagePath() + name + '/' + mControlPathSuffix + name + "." + mControlExtension;
}

void QgsRenderChecker::setControlPathSuffix( const QString &name )
{
  if ( !name.isEmpty() )
    mControlPathSuffix = name + '/';
  else
    mControlPathSuffix.clear();
}

QString QgsRenderChecker::imageToHash( const QString &imageFile )
{
  QImage myImage;
  myImage.load( imageFile );
  QByteArray myByteArray;
  QBuffer myBuffer( &myByteArray );
  myImage.save( &myBuffer, "PNG" );
  const QString myImageString = QString::fromUtf8( myByteArray.toBase64().data() );
  QCryptographicHash myHash( QCryptographicHash::Md5 );
  myHash.addData( myImageString.toUtf8() );
  return myHash.result().toHex().constData();
}

void QgsRenderChecker::setMapSettings( const QgsMapSettings &mapSettings )
{
  mMapSettings = mapSettings;
}

void QgsRenderChecker::drawBackground( QImage *image )
{
  // create a 2x2 checker-board image
  uchar pixDataRGB[] = { 255, 255, 255, 255,
                         127, 127, 127, 255,
                         127, 127, 127, 255,
                         255, 255, 255, 255
                       };

  const QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
  const QPixmap pix = QPixmap::fromImage( img.scaled( 20, 20 ) );

  // fill image with texture
  QBrush brush;
  brush.setTexture( pix );
  QPainter p( image );
  p.setRenderHint( QPainter::Antialiasing, false );
  p.fillRect( QRect( 0, 0, image->width(), image->height() ), brush );
  p.end();
}


bool QgsRenderChecker::isKnownAnomaly( const QString &diffImageFile )
{
  const QString myControlImageDir = controlImagePath() + mControlName + '/';
  const QDir myDirectory = QDir( myControlImageDir );
  QStringList myList;
  const QString myFilename = QStringLiteral( "*" );
  myList = myDirectory.entryList( QStringList( myFilename ),
                                  QDir::Files | QDir::NoSymLinks );
  //remove the control file from the list as the anomalies are
  //all files except the control file
  myList.removeAll( QFileInfo( mExpectedImageFile ).fileName() );

  const QString myImageHash = imageToHash( diffImageFile );


  for ( int i = 0; i < myList.size(); ++i )
  {
    const QString myFile = myList.at( i );
    mReport += "<tr><td colspan=3>"
               "Checking if " + myFile + " is a known anomaly.";
    mReport += QLatin1String( "</td></tr>" );
    const QString myAnomalyHash = imageToHash( controlImagePath() + mControlName + '/' + myFile );
    QString myHashMessage = QStringLiteral(
                              "Checking if anomaly %1 (hash %2)<br>" )
                            .arg( myFile,
                                  myAnomalyHash );
    myHashMessage += QStringLiteral( "&nbsp; matches %1 (hash %2)" )
                     .arg( diffImageFile,
                           myImageHash );
    //foo CDash
    emitDashMessage( QStringLiteral( "Anomaly check" ), QgsDartMeasurement::Text, myHashMessage );

    mReport += "<tr><td colspan=3>" + myHashMessage + "</td></tr>";
    if ( myImageHash == myAnomalyHash )
    {
      mReport += "<tr><td colspan=3>"
                 "Anomaly found! " + myFile;
      mReport += QLatin1String( "</td></tr>" );
      return true;
    }
  }
  mReport += "<tr><td colspan=3>"
             "No anomaly found! ";
  mReport += QLatin1String( "</td></tr>" );
  return false;
}

void QgsRenderChecker::emitDashMessage( const QgsDartMeasurement &dashMessage )
{
  if ( !mIsCiRun )
    return;

  if ( mBufferDashMessages )
    mDashMessages << dashMessage;
  else
    dashMessage.send();
}

void QgsRenderChecker::emitDashMessage( const QString &name, QgsDartMeasurement::Type type, const QString &value )
{
  emitDashMessage( QgsDartMeasurement( name, type, value ) );
}

#if DUMP_BASE64_IMAGES
void QgsRenderChecker::dumpRenderedImageAsBase64()
{
  QFile fileSource( mRenderedImageFile );
  if ( !fileSource.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  const QByteArray blob = fileSource.readAll();
  const QByteArray encoded = blob.toBase64();
  qDebug() << "Dumping rendered image " << mRenderedImageFile << " as base64\n";
  qDebug() << "################################################################";
  qDebug() << encoded;
  qDebug() << "################################################################";
  qDebug() << "End dump";
}
#endif

void QgsRenderChecker::performPostTestActions( Flags flags )
{
  if ( mResult || mExpectFail )
    return;

#if DUMP_BASE64_IMAGES
  if ( mIsCiRun && QFile::exists( mRenderedImageFile ) && !( flags & Flag::AvoidExportingRenderedImage ) )
    dumpRenderedImageAsBase64();
#endif

  if ( shouldGenerateReport() )
  {
    const QDir reportDir = QgsRenderChecker::testReportDir();
    if ( !reportDir.exists() )
    {
      if ( !QDir().mkpath( reportDir.path() ) )
      {
        qDebug() << "!!!!! cannot create " << reportDir.path();
      }
    }

    if ( QFile::exists( mRenderedImageFile ) && !( flags & Flag::AvoidExportingRenderedImage ) )
    {
      QFileInfo fi( mRenderedImageFile );
      const QString destPath = reportDir.filePath( fi.fileName() );
      if ( QFile::exists( destPath ) )
        QFile::remove( destPath );
      if ( !QFile::copy( mRenderedImageFile, destPath ) )
      {
        qDebug() << "!!!!! could not copy " << mRenderedImageFile << " to " << destPath;
      }
    }
    if ( QFile::exists( mDiffImageFile ) && !( flags & Flag::AvoidExportingRenderedImage ) )
    {
      QFileInfo fi( mDiffImageFile );
      const QString destPath = reportDir.filePath( fi.fileName() );
      if ( QFile::exists( destPath ) )
        QFile::remove( destPath );
      QFile::copy( mDiffImageFile, destPath );
    }
  }
}

bool QgsRenderChecker::runTest( const QString &testName,
                                unsigned int mismatchCount,
                                QgsRenderChecker::Flags flags )
{
  mResult = false;
  if ( mExpectedImageFile.isEmpty() )
  {
    qDebug( "QgsRenderChecker::runTest failed - Expected Image File not set." );
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
              "Image File not set.</td></tr></table>\n";
    mMarkdownReport = QStringLiteral( "Failed because expected image file not set\n" );
    performPostTestActions( flags );
    return mResult;
  }
  //
  // Load the expected result pixmap
  //
  const QImageReader expectedImageReader( mExpectedImageFile );
  if ( !expectedImageReader.canRead() )
  {
    qDebug() << "QgsRenderChecker::runTest failed - Could not load expected image from " << mExpectedImageFile;
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
              "Image File could not be loaded.</td></tr></table>\n";
    mMarkdownReport = QStringLiteral( "Failed because expected image file (%1) could not be loaded\n" ).arg( mExpectedImageFile );
    performPostTestActions( flags );
    return mResult;
  }

  const QSize expectedSize = expectedImageReader.size();
  mMatchTarget = expectedSize.width() * expectedSize.height();
  //
  // Now render our layers onto a pixmap
  //
  mMapSettings.setBackgroundColor( qRgb( 152, 219, 249 ) );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing );
  mMapSettings.setOutputSize( expectedSize / mMapSettings.devicePixelRatio() );

  QElapsedTimer myTime;
  myTime.start();

  QgsMapRendererSequentialJob job( mMapSettings );
  job.start();
  job.waitForFinished();

  mElapsedTime = myTime.elapsed();

  mRenderedImageFile = QDir::tempPath() + '/' + testName + "_result.png";

  mRenderedImage = job.renderedImage();
  Q_ASSERT( mRenderedImage.devicePixelRatioF() == mMapSettings.devicePixelRatio() );
  const bool res = compareImages( testName, mismatchCount, QString(), flags );

  if ( ! res )
  {
    // If test failed, save the pixmap to disk so the user can make a
    // visual assessment
    if ( ! mRenderedImage.save( mRenderedImageFile, "PNG", 100 ) )
    {
      qDebug() << "QgsRenderChecker::runTest failed - Could not save rendered image to " << mRenderedImageFile;
      mReport = "<table>"
                "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
                "<tr><td>Nothing rendered</td>\n<td>Failed because Rendered "
                "Image File could not be saved.</td></tr></table>\n";
      mMarkdownReport = QStringLiteral( "Failed because rendered image file could not be saved to %1\n" ).arg( mRenderedImageFile );

      performPostTestActions( flags );
      return mResult;
    }

    //create a world file to go with the image...
    QFile wldFile( QDir::tempPath() + '/' + testName + "_result.wld" );
    if ( wldFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      const QgsRectangle r = mMapSettings.extent();

      QTextStream stream( &wldFile );
      stream << QStringLiteral( "%1\r\n0 \r\n0 \r\n%2\r\n%3\r\n%4\r\n" )
             .arg( qgsDoubleToString( mMapSettings.mapUnitsPerPixel() ),
                   qgsDoubleToString( -mMapSettings.mapUnitsPerPixel() ),
                   qgsDoubleToString( r.xMinimum() + mMapSettings.mapUnitsPerPixel() / 2.0 ),
                   qgsDoubleToString( r.yMaximum() - mMapSettings.mapUnitsPerPixel() / 2.0 ) );
    }
  }

  return res;
}


bool QgsRenderChecker::compareImages( const QString &testName,
                                      unsigned int mismatchCount,
                                      const QString &renderedImageFile,
                                      QgsRenderChecker::Flags flags )
{
  mResult = false;
  if ( mExpectedImageFile.isEmpty() )
  {
    qDebug( "QgsRenderChecker::runTest failed - Expected Image (control) File not set." );
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
              "Image File not set.</td></tr></table>\n";
    mMarkdownReport = QStringLiteral( "Failed because expected image file was not set\n" );

    performPostTestActions( flags );
    return mResult;
  }

  return compareImages( testName, mExpectedImageFile, renderedImageFile, mismatchCount, flags );
}

bool QgsRenderChecker::compareImages( const QString &testName, const QString &referenceImageFile, const QString &renderedImageFile, unsigned int mismatchCount, QgsRenderChecker::Flags flags )
{
  mResult = false;
  if ( ! renderedImageFile.isEmpty() )
  {
    mRenderedImageFile = renderedImageFile;
#ifdef Q_OS_WIN
    mRenderedImageFile = mRenderedImageFile.replace( '\\', '/' );
#endif
  }

  if ( mRenderedImageFile.isEmpty() )
  {
    qDebug( "QgsRenderChecker::runTest failed - Rendered Image File not set." );
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Rendered "
              "Image File not set.</td></tr></table>\n";
    mMarkdownReport = QStringLiteral( "Failed because rendered image file was not set\n" );
    performPostTestActions( flags );
    return mResult;
  }

  //
  // Load /create the images
  //
  QImage expectedImage( referenceImageFile );
  if ( expectedImage.isNull() )
  {
    qDebug() << "QgsRenderChecker::runTest failed - Could not load control image from " << referenceImageFile;
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because control "
              "image file could not be loaded.</td></tr></table>\n";
    mMarkdownReport = QStringLiteral( "Failed because expected image file (%1) could not be loaded\n" ).arg( referenceImageFile );
    performPostTestActions( flags );
    return mResult;
  }

  const QString expectedImageString = QStringLiteral( "<a href=\"%1\" style=\"color: inherit\" target=\"_blank\">expected</a> image" ).arg( QUrl::fromLocalFile( referenceImageFile ).toString() );
  const QString renderedImageString = QStringLiteral( "<a href=\"%2\" style=\"color: inherit\" target=\"_blank\">rendered</a> image" ).arg( QUrl::fromLocalFile( renderedImageFile ).toString() );
  auto upperFirst = []( const QString & string ) -> QString
  {
    const int firstNonTagIndex = string.indexOf( '>' ) + 1;
    return string.left( firstNonTagIndex ) + string.at( firstNonTagIndex ).toUpper() + string.mid( firstNonTagIndex + 1 );
  };

  QImage myResultImage = mRenderedImage.isNull() ? QImage( mRenderedImageFile ) : mRenderedImage;
  if ( myResultImage.isNull() )
  {
    qDebug() << "QgsRenderChecker::runTest failed - Could not load rendered image from " << mRenderedImageFile;
    mReport = QStringLiteral( "<table>"
                              "<tr><td>Test Result:</td><td>%1:</td></tr>\n"
                              "<tr><td>Nothing rendered</td>\n<td>Failed because Rendered "
                              "Image File could not be loaded.</td></tr></table>\n" ).arg( upperFirst( expectedImageString ) );
    mMarkdownReport = QStringLiteral( "Failed because rendered image (%1) could not be loaded\n" ).arg( mRenderedImageFile );
    performPostTestActions( flags );
    return mResult;
  }
  QImage myDifferenceImage( expectedImage.width(),
                            expectedImage.height(),
                            QImage::Format_RGB32 );
  mDiffImageFile = QDir::tempPath() + '/' + testName + "_result_diff.png";
  myDifferenceImage.fill( qRgb( 152, 219, 249 ) );

  //check for mask
  QString maskImagePath = referenceImageFile;
  maskImagePath.chop( 4 ); //remove .png extension
  maskImagePath += QLatin1String( "_mask.png" );
  const QImage maskImage( maskImagePath );
  const bool hasMask = !maskImage.isNull();

  //
  // Set pixel count score and target
  //
  mMatchTarget = expectedImage.width() * expectedImage.height();
  const unsigned int myPixelCount = myResultImage.width() * myResultImage.height();
  //
  // Set the report with the result
  //
  mReport += QLatin1String( "<table>" );
  mReport += QLatin1String( "<tr><td colspan=2>" );
  mReport += QStringLiteral( "<tr><td colspan=2>"
                             "%8 and %9 for %1<br>"
                             "Expected size: %2 w x %3 h (%4 pixels)<br>"
                             "Rendered size: %5 w x %6 h (%7 pixels)"
                             "</td></tr>" )
             .arg( testName )
             .arg( expectedImage.width() ).arg( expectedImage.height() ).arg( mMatchTarget )
             .arg( myResultImage.width() ).arg( myResultImage.height() ).arg( myPixelCount )
             .arg( upperFirst( expectedImageString ), renderedImageString );
  mReport += QString( "<tr><td colspan=2>\n"
                      "Expected Duration : <= %1 (0 indicates not specified)<br>"
                      "Actual Duration : %2 ms<br></td></tr>" )
             .arg( mElapsedTimeTarget )
             .arg( mElapsedTime );

  // limit image size in page to something reasonable
  int imgWidth = 420;
  int imgHeight = 280;
  if ( ! expectedImage.isNull() )
  {
    imgWidth = std::min( expectedImage.width(), imgWidth );
    imgHeight = expectedImage.height() * imgWidth / expectedImage.width();
  }

  const QString renderedImageFileName = QFileInfo( mRenderedImageFile ).fileName();
  const QString diffImageFileName = QFileInfo( mDiffImageFile ).fileName();
  const QString myImagesString = QString(
                                   "<tr>"
                                   "<td colspan=2>Compare %10 and %11</td>"
                                   "<td>Difference (all blue is good, any red is bad)</td>"
                                   "</tr>\n<tr>"
                                   "<td colspan=2 id=\"td-%1-%7\"></td>\n"
                                   "<td align=center><img width=%5 height=%6 src=\"%2\"></td>\n"
                                   "</tr>"
                                   "</table>\n"
                                   "<script>\naddComparison(\"td-%1-%7\",\"%3\",\"file://%4\",%5,%6);\n</script>\n"
                                   "<p>If the new image looks good, create or update a test mask with<br>"
                                   "<code>scripts/generate_test_mask_image.py \"%8\" \"%9\"</code>" )
                                 .arg( testName,
                                       diffImageFileName,
                                       renderedImageFileName,
                                       referenceImageFile )
                                 .arg( imgWidth ).arg( imgHeight )
                                 .arg( QUuid::createUuid().toString().mid( 1, 6 ),
                                       referenceImageFile,
                                       mRenderedImageFile,
                                       expectedImageString,
                                       renderedImageString
                                     );

  QString prefix;
  if ( !mControlPathPrefix.isNull() )
  {
    prefix = QStringLiteral( " (prefix %1)" ).arg( mControlPathPrefix );
  }

  //
  // Put the same info to debug too
  //

  if ( expectedImage.width() != myResultImage.width() || expectedImage.height() != myResultImage.height() )
  {
    qDebug( "Expected size: %dw x %dh", expectedImage.width(), expectedImage.height() );
    qDebug( "Actual   size: %dw x %dh", myResultImage.width(), myResultImage.height() );
    if ( hasMask )
      qDebug( "Mask size: %dw x %dh", maskImage.width(), maskImage.height() );
  }

  if ( mMatchTarget != myPixelCount )
  {
    qDebug( "Expected image and rendered image for %s are different dimensions", testName.toLocal8Bit().constData() );

    if ( std::abs( expectedImage.width() - myResultImage.width() ) > mMaxSizeDifferenceX ||
         std::abs( expectedImage.height() - myResultImage.height() ) > mMaxSizeDifferenceY )
    {
      emitDashMessage( "Rendered Image " + testName + prefix, QgsDartMeasurement::ImagePng, mRenderedImageFile );
      emitDashMessage( "Expected Image " + testName + prefix, QgsDartMeasurement::ImagePng, referenceImageFile );

      mReport += QLatin1String( "<tr><td colspan=3>" );
      mReport += QStringLiteral( "<font color=red>%1 and %2 for " ).arg( upperFirst( expectedImageString ), renderedImageString ) + testName + " are different dimensions - FAILING!</font>";
      mReport += QLatin1String( "</td></tr>" );
      mMarkdownReport += QStringLiteral( "Failed because rendered image and expected image are different dimensions (%1x%2 v2 %3x%4)\n" )
                         .arg( myResultImage.width() )
                         .arg( myResultImage.height() )
                         .arg( expectedImage.width() )
                         .arg( expectedImage.height() );

      const QString diffSizeImagesString = QString(
                                             "<tr>"
                                             "<td colspan=3>Compare %5 and %6</td>"
                                             "</tr>\n<tr>"
                                             "<td align=center><img width=%3 height=%4 src=\"%2\"></td>\n"
                                             "<td align=center><img src=\"%1\"></td>\n"
                                             "</tr>"
                                             "</table>\n" )
                                           .arg(
                                             renderedImageFileName,
                                             referenceImageFile )
                                           .arg( imgWidth ).arg( imgHeight )
                                           .arg( expectedImageString, renderedImageString );

      mReport += diffSizeImagesString;
      performPostTestActions( flags );
      return mResult;
    }
    else
    {
      mReport += QLatin1String( "<tr><td colspan=3>" );
      mReport += QStringLiteral( "%1 and %2 for " ).arg( upperFirst( expectedImageString ), renderedImageString ) + testName + " are different dimensions, but within tolerance";
      mReport += QLatin1String( "</td></tr>" );
    }
  }

  if ( expectedImage.format() == QImage::Format_Indexed8 )
  {
    if ( myResultImage.format() != QImage::Format_Indexed8 )
    {
      emitDashMessage( "Rendered Image " + testName + prefix, QgsDartMeasurement::ImagePng, mRenderedImageFile );
      emitDashMessage( "Expected Image " + testName + prefix, QgsDartMeasurement::ImagePng, referenceImageFile );

      qDebug() << "Expected image and rendered image for " << testName << " have different formats (8bit format is expected) - FAILING!";

      mReport += QLatin1String( "<tr><td colspan=3>" );
      mReport += "<font color=red>Expected image and rendered image for " + testName + " have different formats (8bit format is expected) - FAILING!</font>";
      mReport += QLatin1String( "</td></tr>" );
      mReport += myImagesString;

      mMarkdownReport += QLatin1String( "Failed because rendered image and expected image have different formats (8bit format is expected)\n" );
      performPostTestActions( flags );
      return mResult;
    }

    // When we compute the diff between the 2 images, we use constScanLine expecting a QRgb color
    // but this method returns color table index for 8 bit image, not color.
    // So we convert the 2 images in 32 bits so the diff works correctly
    myResultImage = myResultImage.convertToFormat( QImage::Format_ARGB32 );
    expectedImage = expectedImage.convertToFormat( QImage::Format_ARGB32 );
  }
  if ( expectedImage.format() != QImage::Format_RGB32
       && expectedImage.format() != QImage::Format_ARGB32
       && expectedImage.format() != QImage::Format_ARGB32_Premultiplied )
  {
    mReport += QLatin1String( "<tr><td colspan=3>" );
    mReport += QStringLiteral( "<font color=red>Expected image for %1 is not a compatible format (%2). Must be 32 bit RGB format.</font>" ).arg( testName, qgsEnumValueToKey( expectedImage.format() ) );
    mReport += QLatin1String( "</td></tr>" );
    mReport += myImagesString;

    mMarkdownReport += QStringLiteral( "Failed because expected image has an incompatible format - %1 (32 bit format is expected)\n" ).arg( qgsEnumValueToKey( expectedImage.format() ) );
    performPostTestActions( flags );
    return mResult;
  }
  if ( myResultImage.format() != QImage::Format_RGB32
       && myResultImage.format() != QImage::Format_ARGB32
       && myResultImage.format() != QImage::Format_ARGB32_Premultiplied )
  {
    mReport += QLatin1String( "<tr><td colspan=3>" );
    mReport += QStringLiteral( "<font color=red>Rendered image for %1 is not a compatible format (%2). Must be 32 bit RGB format.</font>" ).arg( testName, qgsEnumValueToKey( myResultImage.format() ) );
    mReport += QLatin1String( "</td></tr>" );
    mReport += myImagesString;

    mMarkdownReport += QStringLiteral( "Failed because rendered image has an incompatible format - %1 (32 bit format is expected)\n" ).arg( qgsEnumValueToKey( myResultImage.format() ) );
    performPostTestActions( flags );
    return mResult;
  }

  //
  // Now iterate through them counting how many
  // dissimilar pixel values there are
  //

  const int maxHeight = std::min( expectedImage.height(), myResultImage.height() );
  const int maxWidth = std::min( expectedImage.width(), myResultImage.width() );

  const int maskWidth = maskImage.width();

  mMismatchCount = 0;
  const int colorTolerance = static_cast< int >( mColorTolerance );
  for ( int y = 0; y < maxHeight; ++y )
  {
    const QRgb *expectedScanline = reinterpret_cast< const QRgb * >( expectedImage.constScanLine( y ) );
    const QRgb *resultScanline = reinterpret_cast< const QRgb * >( myResultImage.constScanLine( y ) );
    const QRgb *maskScanline = ( hasMask && maskImage.height() > y ) ? reinterpret_cast< const QRgb * >( maskImage.constScanLine( y ) ) : nullptr;
    QRgb *diffScanline = reinterpret_cast< QRgb * >( myDifferenceImage.scanLine( y ) );

    for ( int x = 0; x < maxWidth; ++x )
    {
      const int pixelTolerance = maskScanline
                                 ? std::max( colorTolerance, ( maskWidth > x ) ? qRed( maskScanline[ x ] ) : 0 )
                                 : colorTolerance;
      if ( pixelTolerance == 255 )
      {
        //skip pixel
        continue;
      }

      const QRgb myExpectedPixel = expectedScanline[x];
      const QRgb myActualPixel = resultScanline[x];
      if ( pixelTolerance == 0 )
      {
        if ( myExpectedPixel != myActualPixel )
        {
          ++mMismatchCount;
          diffScanline[ x ] = qRgb( 255, 0, 0 );
        }
      }
      else
      {
        if ( std::abs( qRed( myExpectedPixel ) - qRed( myActualPixel ) ) > pixelTolerance ||
             std::abs( qGreen( myExpectedPixel ) - qGreen( myActualPixel ) ) > pixelTolerance ||
             std::abs( qBlue( myExpectedPixel ) - qBlue( myActualPixel ) ) > pixelTolerance ||
             std::abs( qAlpha( myExpectedPixel ) - qAlpha( myActualPixel ) ) > pixelTolerance )
        {
          ++mMismatchCount;
          diffScanline[ x ] = qRgb( 255, 0, 0 );
        }
      }
    }
  }

  //
  // Send match result to debug
  //
  if ( mMismatchCount > mismatchCount )
  {
    emitDashMessage( "Rendered Image " + testName + prefix, QgsDartMeasurement::ImagePng, mRenderedImageFile );
    emitDashMessage( "Expected Image " + testName + prefix, QgsDartMeasurement::ImagePng, referenceImageFile );

    qDebug( "%d/%d pixels mismatched (%d allowed)", mMismatchCount, mMatchTarget, mismatchCount );

    //
    //save the diff image to disk
    //
    myDifferenceImage.save( mDiffImageFile );
    emitDashMessage( "Difference Image " + testName + prefix, QgsDartMeasurement::ImagePng, mDiffImageFile );
  }

  //
  // Send match result to report
  //
  mReport += QStringLiteral( "<tr><td colspan=3>%1/%2 pixels mismatched (allowed threshold: %3, allowed color component tolerance: %4)</td></tr>" )
             .arg( mMismatchCount ).arg( mMatchTarget ).arg( mismatchCount ).arg( mColorTolerance );

  //
  // And send it to CDash
  //
  if ( mMismatchCount > 0 )
  {
    emitDashMessage( QStringLiteral( "Mismatch Count" ), QgsDartMeasurement::Integer, QStringLiteral( "%1/%2" ).arg( mMismatchCount ).arg( mMatchTarget ) );
  }

  if ( mMismatchCount <= mismatchCount )
  {
    mReport += QLatin1String( "<tr><td colspan = 3>\n" );
    mReport += QStringLiteral( "%1 and %2 for " ).arg( upperFirst( expectedImageString ), renderedImageString ) + testName + " are matched<br>";
    mReport += QLatin1String( "</td></tr>" );
    if ( mElapsedTimeTarget != 0 && mElapsedTimeTarget < mElapsedTime )
    {
      //test failed because it took too long...
      qDebug( "Test failed because render step took too long" );
      mReport += QLatin1String( "<tr><td colspan = 3>\n" );
      mReport += QLatin1String( "<font color=red>Test failed because render step took too long</font>" );
      mReport += QLatin1String( "</td></tr>" );
      mReport += myImagesString;

      mMarkdownReport += QLatin1String( "Test failed because render step took too long\n" );

      performPostTestActions( flags );
      return mResult;
    }
    else
    {
      mReport += myImagesString;
      mResult = true;
      performPostTestActions( flags );
      return mResult;
    }
  }

  mReport += QLatin1String( "<tr><td colspan=3></td></tr>" );
  emitDashMessage( QStringLiteral( "Image mismatch" ), QgsDartMeasurement::Text, "Difference image did not match any known anomaly or mask."
                   " If you feel the difference image should be considered an anomaly "
                   "you can do something like this\n"
                   "cp '" + mDiffImageFile + "' " + controlImagePath() + mControlName +
                   "/\nIf it should be included in the mask run\n"
                   "scripts/generate_test_mask_image.py '" + referenceImageFile + "' '" + mRenderedImageFile + "'\n" );

  mReport += QLatin1String( "<tr><td colspan = 3>\n" );
  mReport += QStringLiteral( "<font color=red>%1 and %2 for " ).arg( upperFirst( expectedImageString ), renderedImageString ) + testName + " are mismatched</font><br>";
  mReport += QLatin1String( "</td></tr>" );
  mReport += myImagesString;

  const QString githubSha = qgetenv( "GITHUB_SHA" );
  if ( !githubSha.isEmpty() )
  {
    const QString githubBlobUrl = QStringLiteral( "https://github.com/qgis/QGIS/blob/%1/%2" ).arg(
                                    githubSha, QDir( sourcePath() ).relativeFilePath( referenceImageFile ) );
    mMarkdownReport += QStringLiteral( "Rendered image did not match [%1](%2) (found %3 pixels different)\n" ).arg(
                         QDir( sourcePath() ).relativeFilePath( referenceImageFile ), githubBlobUrl ).arg( mMismatchCount );
  }
  else
  {
    mMarkdownReport += QStringLiteral( "Rendered image did not match [%1](%2) (found %3 pixels different)\n" ).arg(
                         QDir( sourcePath() ).relativeFilePath( referenceImageFile ),
                         QUrl::fromLocalFile( referenceImageFile ).toString() ).arg( mMismatchCount );
  }

  performPostTestActions( flags );
  return mResult;
}
