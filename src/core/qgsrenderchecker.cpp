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
#include "qgsgeometry.h"

#include <QColor>
#include <QPainter>
#include <QImage>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QBuffer>
#include <QUuid>

QgsRenderChecker::QgsRenderChecker()
  : mBasePath( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/control_images/" ) ) //defined in CmakeLists.txt
{
  if ( qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" ) )
    mIsCiRun = true;
}

QString QgsRenderChecker::controlImagePath() const
{
  return mBasePath + ( mBasePath.endsWith( '/' ) ? QString() : QStringLiteral( "/" ) ) + mControlPathPrefix;
}

void QgsRenderChecker::setControlImagePath( const QString &path )
{
  mBasePath = path;
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

void QgsRenderChecker::dumpRenderedImageAsBase64()
{
  QFile fileSource( mRenderedImageFile );
  if ( !fileSource.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  const QByteArray blob = fileSource.readAll();
  const QByteArray encoded = blob.toBase64();
  qDebug() << encoded;
}

bool QgsRenderChecker::runTest( const QString &testName,
                                unsigned int mismatchCount )
{
  if ( mExpectedImageFile.isEmpty() )
  {
    qDebug( "QgsRenderChecker::runTest failed - Expected Image File not set." );
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
              "Image File not set.</td></tr></table>\n";
    return false;
  }
  //
  // Load the expected result pixmap
  //
  const QImage myExpectedImage( mExpectedImageFile );
  if ( myExpectedImage.isNull() )
  {
    qDebug() << "QgsRenderChecker::runTest failed - Could not load expected image from " << mExpectedImageFile;
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
              "Image File could not be loaded.</td></tr></table>\n";
    return false;
  }
  mMatchTarget = myExpectedImage.width() * myExpectedImage.height();
  //
  // Now render our layers onto a pixmap
  //
  mMapSettings.setBackgroundColor( qRgb( 152, 219, 249 ) );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::Antialiasing );
  mMapSettings.setOutputSize( QSize( myExpectedImage.width(), myExpectedImage.height() ) / mMapSettings.devicePixelRatio() );

  QElapsedTimer myTime;
  myTime.start();

  QgsMapRendererSequentialJob job( mMapSettings );
  job.start();
  job.waitForFinished();

  mElapsedTime = myTime.elapsed();

  QImage myImage = job.renderedImage();
  Q_ASSERT( myImage.devicePixelRatioF() == mMapSettings.devicePixelRatio() );

  //
  // Save the pixmap to disk so the user can make a
  // visual assessment if needed
  //
  mRenderedImageFile = QDir::tempPath() + '/' + testName + "_result.png";

  myImage.setDotsPerMeterX( myExpectedImage.dotsPerMeterX() );
  myImage.setDotsPerMeterY( myExpectedImage.dotsPerMeterY() );
  if ( ! myImage.save( mRenderedImageFile, "PNG", 100 ) )
  {
    qDebug() << "QgsRenderChecker::runTest failed - Could not save rendered image to " << mRenderedImageFile;
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Rendered "
              "Image File could not be saved.</td></tr></table>\n";
    return false;
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

  return compareImages( testName, mismatchCount );
}


bool QgsRenderChecker::compareImages( const QString &testName,
                                      unsigned int mismatchCount,
                                      const QString &renderedImageFile )
{
  if ( mExpectedImageFile.isEmpty() )
  {
    qDebug( "QgsRenderChecker::runTest failed - Expected Image (control) File not set." );
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
              "Image File not set.</td></tr></table>\n";
    return false;
  }

  return compareImages( testName, mExpectedImageFile, renderedImageFile, mismatchCount );
}

bool QgsRenderChecker::compareImages( const QString &testName, const QString &referenceImageFile, const QString &renderedImageFile, unsigned int mismatchCount )
{
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
    return false;
  }

  //
  // Load /create the images
  //
  QImage myExpectedImage( referenceImageFile );
  QImage myResultImage( mRenderedImageFile );
  if ( myResultImage.isNull() )
  {
    qDebug() << "QgsRenderChecker::runTest failed - Could not load rendered image from " << mRenderedImageFile;
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Rendered "
              "Image File could not be loaded.</td></tr></table>\n";
    return false;
  }
  QImage myDifferenceImage( myExpectedImage.width(),
                            myExpectedImage.height(),
                            QImage::Format_RGB32 );
  const QString myDiffImageFile = QDir::tempPath() + '/' + testName + "_result_diff.png";
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
  mMatchTarget = myExpectedImage.width() * myExpectedImage.height();
  const unsigned int myPixelCount = myResultImage.width() * myResultImage.height();
  //
  // Set the report with the result
  //
  mReport = QStringLiteral( "<script src=\"file://%1/../renderchecker.js\"></script>\n" ).arg( TEST_DATA_DIR );
  mReport += QLatin1String( "<table>" );
  mReport += QLatin1String( "<tr><td colspan=2>" );
  mReport += QString( "<tr><td colspan=2>"
                      "Test image and result image for %1<br>"
                      "Expected size: %2 w x %3 h (%4 pixels)<br>"
                      "Actual   size: %5 w x %6 h (%7 pixels)"
                      "</td></tr>" )
             .arg( testName )
             .arg( myExpectedImage.width() ).arg( myExpectedImage.height() ).arg( mMatchTarget )
             .arg( myResultImage.width() ).arg( myResultImage.height() ).arg( myPixelCount );
  mReport += QString( "<tr><td colspan=2>\n"
                      "Expected Duration : <= %1 (0 indicates not specified)<br>"
                      "Actual Duration : %2 ms<br></td></tr>" )
             .arg( mElapsedTimeTarget )
             .arg( mElapsedTime );

  // limit image size in page to something reasonable
  int imgWidth = 420;
  int imgHeight = 280;
  if ( ! myExpectedImage.isNull() )
  {
    imgWidth = std::min( myExpectedImage.width(), imgWidth );
    imgHeight = myExpectedImage.height() * imgWidth / myExpectedImage.width();
  }

  const QString myImagesString = QString(
                                   "<tr>"
                                   "<td colspan=2>Compare actual and expected result</td>"
                                   "<td>Difference (all blue is good, any red is bad)</td>"
                                   "</tr>\n<tr>"
                                   "<td colspan=2 id=\"td-%1-%7\"></td>\n"
                                   "<td align=center><img width=%5 height=%6 src=\"file://%2\"></td>\n"
                                   "</tr>"
                                   "</table>\n"
                                   "<script>\naddComparison(\"td-%1-%7\",\"file://%3\",\"file://%4\",%5,%6);\n</script>\n" )
                                 .arg( testName,
                                       myDiffImageFile,
                                       mRenderedImageFile,
                                       referenceImageFile )
                                 .arg( imgWidth ).arg( imgHeight )
                                 .arg( QUuid::createUuid().toString().mid( 1, 6 ) );

  QString prefix;
  if ( !mControlPathPrefix.isNull() )
  {
    prefix = QStringLiteral( " (prefix %1)" ).arg( mControlPathPrefix );
  }
  //
  // To get the images into CDash
  //
  emitDashMessage( "Rendered Image " + testName + prefix, QgsDartMeasurement::ImagePng, mRenderedImageFile );
  emitDashMessage( "Expected Image " + testName + prefix, QgsDartMeasurement::ImagePng, referenceImageFile );

  //
  // Put the same info to debug too
  //

  if ( myExpectedImage.width() != myResultImage.width() || myExpectedImage.height() != myResultImage.height() )
  {
    qDebug( "Expected size: %dw x %dh", myExpectedImage.width(), myExpectedImage.height() );
    qDebug( "Actual   size: %dw x %dh", myResultImage.width(), myResultImage.height() );
    if ( hasMask )
      qDebug( "Mask size: %dw x %dh", maskImage.width(), maskImage.height() );
  }

  if ( mMatchTarget != myPixelCount )
  {
    qDebug( "Test image and result image for %s are different dimensions", testName.toLocal8Bit().constData() );

    if ( std::abs( myExpectedImage.width() - myResultImage.width() ) > mMaxSizeDifferenceX ||
         std::abs( myExpectedImage.height() - myResultImage.height() ) > mMaxSizeDifferenceY )
    {
      mReport += QLatin1String( "<tr><td colspan=3>" );
      mReport += "<font color=red>Expected image and result image for " + testName + " are different dimensions - FAILING!</font>";
      mReport += QLatin1String( "</td></tr>" );
      mReport += myImagesString;
      if ( mIsCiRun )
        dumpRenderedImageAsBase64();
      return false;
    }
    else
    {
      mReport += QLatin1String( "<tr><td colspan=3>" );
      mReport += "Expected image and result image for " + testName + " are different dimensions, but within tolerance";
      mReport += QLatin1String( "</td></tr>" );
    }
  }

  if ( myExpectedImage.format() == QImage::Format_Indexed8 )
  {
    if ( myResultImage.format() != QImage::Format_Indexed8 )
    {
      qDebug() << "Expected image and result image for " << testName << " have different formats (8bit format is expected) - FAILING!";

      mReport += QLatin1String( "<tr><td colspan=3>" );
      mReport += "<font color=red>Expected image and result image for " + testName + " have different formats (8bit format is expected) - FAILING!</font>";
      mReport += QLatin1String( "</td></tr>" );
      mReport += myImagesString;
      if ( mIsCiRun )
        dumpRenderedImageAsBase64();
      return false;
    }

    // When we compute the diff between the 2 images, we use constScanLine expecting a QRgb color
    // but this method returns color table index for 8 bit image, not color.
    // So we convert the 2 images in 32 bits so the diff works correctly
    myResultImage = myResultImage.convertToFormat( QImage::Format_ARGB32 );
    myExpectedImage = myExpectedImage.convertToFormat( QImage::Format_ARGB32 );
  }


  //
  // Now iterate through them counting how many
  // dissimilar pixel values there are
  //

  const int maxHeight = std::min( myExpectedImage.height(), myResultImage.height() );
  const int maxWidth = std::min( myExpectedImage.width(), myResultImage.width() );

  mMismatchCount = 0;
  const int colorTolerance = static_cast< int >( mColorTolerance );
  for ( int y = 0; y < maxHeight; ++y )
  {
    const QRgb *expectedScanline = reinterpret_cast< const QRgb * >( myExpectedImage.constScanLine( y ) );
    const QRgb *resultScanline = reinterpret_cast< const QRgb * >( myResultImage.constScanLine( y ) );
    const QRgb *maskScanline = ( hasMask && maskImage.height() > y ) ? reinterpret_cast< const QRgb * >( maskImage.constScanLine( y ) ) : nullptr;
    QRgb *diffScanline = reinterpret_cast< QRgb * >( myDifferenceImage.scanLine( y ) );

    for ( int x = 0; x < maxWidth; ++x )
    {
      const int maskTolerance = ( maskScanline && maskImage.width() > x ) ? qRed( maskScanline[ x ] ) : 0;
      const int pixelTolerance = std::max( colorTolerance, maskTolerance );
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
  //save the diff image to disk
  //
  myDifferenceImage.save( myDiffImageFile );
  emitDashMessage( "Difference Image " + testName + prefix, QgsDartMeasurement::ImagePng, myDiffImageFile );

  //
  // Send match result to debug
  //
  if ( mMismatchCount > mismatchCount )
  {
    qDebug( "%d/%d pixels mismatched (%d allowed)", mMismatchCount, mMatchTarget, mismatchCount );
  }

  //
  // Send match result to report
  //
  mReport += QStringLiteral( "<tr><td colspan=3>%1/%2 pixels mismatched (allowed threshold: %3, allowed color component tolerance: %4)</td></tr>" )
             .arg( mMismatchCount ).arg( mMatchTarget ).arg( mismatchCount ).arg( mColorTolerance );

  //
  // And send it to CDash
  //
  emitDashMessage( QStringLiteral( "Mismatch Count" ), QgsDartMeasurement::Integer, QStringLiteral( "%1/%2" ).arg( mMismatchCount ).arg( mMatchTarget ) );

  if ( mMismatchCount <= mismatchCount )
  {
    mReport += QLatin1String( "<tr><td colspan = 3>\n" );
    mReport += "Test image and result image for " + testName + " are matched<br>";
    mReport += QLatin1String( "</td></tr>" );
    if ( mElapsedTimeTarget != 0 && mElapsedTimeTarget < mElapsedTime )
    {
      //test failed because it took too long...
      qDebug( "Test failed because render step took too long" );
      mReport += QLatin1String( "<tr><td colspan = 3>\n" );
      mReport += QLatin1String( "<font color=red>Test failed because render step took too long</font>" );
      mReport += QLatin1String( "</td></tr>" );
      mReport += myImagesString;
      if ( mIsCiRun )
        dumpRenderedImageAsBase64();
      return false;
    }
    else
    {
      mReport += myImagesString;
      return true;
    }
  }

  const bool myAnomalyMatchFlag = isKnownAnomaly( myDiffImageFile );
  if ( myAnomalyMatchFlag )
  {
    mReport += "<tr><td colspan=3>"
               "Difference image matched a known anomaly - passing test! "
               "</td></tr>";
    return true;
  }

  mReport += QLatin1String( "<tr><td colspan=3></td></tr>" );
  emitDashMessage( QStringLiteral( "Image mismatch" ), QgsDartMeasurement::Text, "Difference image did not match any known anomaly or mask."
                   " If you feel the difference image should be considered an anomaly "
                   "you can do something like this\n"
                   "cp '" + myDiffImageFile + "' " + controlImagePath() + mControlName +
                   "/\nIf it should be included in the mask run\n"
                   "scripts/generate_test_mask_image.py '" + referenceImageFile + "' '" + mRenderedImageFile + "'\n" );

  mReport += QLatin1String( "<tr><td colspan = 3>\n" );
  mReport += "<font color=red>Test image and result image for " + testName + " are mismatched</font><br>";
  mReport += QLatin1String( "</td></tr>" );
  mReport += myImagesString;
  if ( mIsCiRun )
    dumpRenderedImageAsBase64();
  return false;
}
