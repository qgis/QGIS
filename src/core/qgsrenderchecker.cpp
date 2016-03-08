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
#include <QTime>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QBuffer>

static int renderCounter = 0;

QgsRenderChecker::QgsRenderChecker()
    : mReport( "" )
    , mMatchTarget( 0 )
    , mElapsedTime( 0 )
    , mRenderedImageFile( "" )
    , mExpectedImageFile( "" )
    , mMismatchCount( 0 )
    , mColorTolerance( 0 )
    , mMaxSizeDifferenceX( 0 )
    , mMaxSizeDifferenceY( 0 )
    , mElapsedTimeTarget( 0 )
    , mBufferDashMessages( false )
{
}

QString QgsRenderChecker::controlImagePath() const
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myControlImageDir = myDataDir + "/control_images/" + mControlPathPrefix;
  return myControlImageDir;
}

void QgsRenderChecker::setControlName( const QString &theName )
{
  mControlName = theName;
  mExpectedImageFile = controlImagePath() + theName + '/' + mControlPathSuffix + theName + ".png";
}

void QgsRenderChecker::setControlPathSuffix( const QString& theName )
{
  if ( !theName.isEmpty() )
    mControlPathSuffix = theName + '/';
  else
    mControlPathSuffix.clear();
}

QString QgsRenderChecker::imageToHash( const QString& theImageFile )
{
  QImage myImage;
  myImage.load( theImageFile );
  QByteArray myByteArray;
  QBuffer myBuffer( &myByteArray );
  myImage.save( &myBuffer, "PNG" );
  QString myImageString = QString::fromUtf8( myByteArray.toBase64().data() );
  QCryptographicHash myHash( QCryptographicHash::Md5 );
  myHash.addData( myImageString.toUtf8() );
  return myHash.result().toHex().constData();
}

void QgsRenderChecker::setMapRenderer( QgsMapRenderer* thepMapRenderer )
{
  mMapSettings = thepMapRenderer->mapSettings();
}

void QgsRenderChecker::setMapSettings( const QgsMapSettings& mapSettings )
{
  mMapSettings = mapSettings;
}

void QgsRenderChecker::drawBackground( QImage* image )
{
  // create a 2x2 checker-board image
  uchar pixDataRGB[] = { 255, 255, 255, 255,
                         127, 127, 127, 255,
                         127, 127, 127, 255,
                         255, 255, 255, 255
                       };

  QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
  QPixmap pix = QPixmap::fromImage( img.scaled( 20, 20 ) );

  // fill image with texture
  QBrush brush;
  brush.setTexture( pix );
  QPainter p( image );
  p.setRenderHint( QPainter::Antialiasing, false );
  p.fillRect( QRect( 0, 0, image->width(), image->height() ), brush );
  p.end();
}

bool QgsRenderChecker::isKnownAnomaly( const QString& theDiffImageFile )
{
  QString myControlImageDir = controlImagePath() + mControlName + '/';
  QDir myDirectory = QDir( myControlImageDir );
  QStringList myList;
  QString myFilename = "*";
  myList = myDirectory.entryList( QStringList( myFilename ),
                                  QDir::Files | QDir::NoSymLinks );
  //remove the control file from the list as the anomalies are
  //all files except the control file
  myList.removeAt( myList.indexOf( QFileInfo( mExpectedImageFile ).fileName() ) );

  QString myImageHash = imageToHash( theDiffImageFile );


  for ( int i = 0; i < myList.size(); ++i )
  {
    QString myFile = myList.at( i );
    mReport += "<tr><td colspan=3>"
               "Checking if " + myFile + " is a known anomaly.";
    mReport += "</td></tr>";
    QString myAnomalyHash = imageToHash( controlImagePath() + mControlName + '/' + myFile );
    QString myHashMessage = QString(
                              "Checking if anomaly %1 (hash %2)<br>" )
                            .arg( myFile,
                                  myAnomalyHash );
    myHashMessage += QString( "&nbsp; matches %1 (hash %2)" )
                     .arg( theDiffImageFile,
                           myImageHash );
    //foo CDash
    emitDashMessage( "Anomaly check", QgsDartMeasurement::Text, myHashMessage );

    mReport += "<tr><td colspan=3>" + myHashMessage + "</td></tr>";
    if ( myImageHash == myAnomalyHash )
    {
      mReport += "<tr><td colspan=3>"
                 "Anomaly found! " + myFile;
      mReport += "</td></tr>";
      return true;
    }
  }
  mReport += "<tr><td colspan=3>"
             "No anomaly found! ";
  mReport += "</td></tr>";
  return false;
}

void QgsRenderChecker::emitDashMessage( const QgsDartMeasurement& dashMessage )
{
  if ( mBufferDashMessages )
    mDashMessages << dashMessage;
  else
    dashMessage.send();
}

void QgsRenderChecker::emitDashMessage( const QString& name, QgsDartMeasurement::Type type, const QString& value )
{
  emitDashMessage( QgsDartMeasurement( name, type, value ) );
}

bool QgsRenderChecker::runTest( const QString& theTestName,
                                unsigned int theMismatchCount )
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
  QImage myExpectedImage( mExpectedImageFile );
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
  mMapSettings.setFlag( QgsMapSettings::Antialiasing );
  mMapSettings.setOutputSize( QSize( myExpectedImage.width(), myExpectedImage.height() ) );

  QTime myTime;
  myTime.start();

  QgsMapRendererSequentialJob job( mMapSettings );
  job.start();
  job.waitForFinished();

  mElapsedTime = myTime.elapsed();

  QImage myImage = job.renderedImage();

  //
  // Save the pixmap to disk so the user can make a
  // visual assessment if needed
  //
  mRenderedImageFile = QDir::tempPath() + '/' + theTestName + "_result.png";

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

  QFile wldFile( QDir::tempPath() + '/' + theTestName + "_result.wld" );
  if ( wldFile.open( QIODevice::WriteOnly ) )
  {
    QgsRectangle r = mMapSettings.extent();

    QTextStream stream( &wldFile );
    stream << QString( "%1\r\n0 \r\n0 \r\n%2\r\n%3\r\n%4\r\n" )
    .arg( qgsDoubleToString( mMapSettings.mapUnitsPerPixel() ),
          qgsDoubleToString( -mMapSettings.mapUnitsPerPixel() ),
          qgsDoubleToString( r.xMinimum() + mMapSettings.mapUnitsPerPixel() / 2.0 ),
          qgsDoubleToString( r.yMaximum() - mMapSettings.mapUnitsPerPixel() / 2.0 ) );
  }

  return compareImages( theTestName, theMismatchCount );
}


bool QgsRenderChecker::compareImages( const QString& theTestName,
                                      unsigned int theMismatchCount,
                                      const QString& theRenderedImageFile )
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
  if ( ! theRenderedImageFile.isEmpty() )
  {
    mRenderedImageFile = theRenderedImageFile;
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
  QImage myExpectedImage( mExpectedImageFile );
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
  QString myDiffImageFile = QDir::tempPath() + '/' + theTestName + "_result_diff.png";
  myDifferenceImage.fill( qRgb( 152, 219, 249 ) );

  //check for mask
  QString maskImagePath = mExpectedImageFile;
  maskImagePath.chop( 4 ); //remove .png extension
  maskImagePath += "_mask.png";
  QImage* maskImage = new QImage( maskImagePath );
  bool hasMask = !maskImage->isNull();
  if ( hasMask )
  {
    qDebug( "QgsRenderChecker using mask image" );
  }

  //
  // Set pixel count score and target
  //
  mMatchTarget = myExpectedImage.width() * myExpectedImage.height();
  unsigned int myPixelCount = myResultImage.width() * myResultImage.height();
  //
  // Set the report with the result
  //
  mReport = QString( "<script src=\"file://%1/../renderchecker.js\"></script>\n" ).arg( TEST_DATA_DIR );
  mReport += "<table>";
  mReport += "<tr><td colspan=2>";
  mReport += QString( "<tr><td colspan=2>"
                      "Test image and result image for %1<br>"
                      "Expected size: %2 w x %3 h (%4 pixels)<br>"
                      "Actual   size: %5 w x %6 h (%7 pixels)"
                      "</td></tr>" )
             .arg( theTestName )
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
    imgWidth = qMin( myExpectedImage.width(), imgWidth );
    imgHeight = myExpectedImage.height() * imgWidth / myExpectedImage.width();
  }

  QString myImagesString = QString(
                             "<tr>"
                             "<td colspan=2>Compare actual and expected result</td>"
                             "<td>Difference (all blue is good, any red is bad)</td>"
                             "</tr>\n<tr>"
                             "<td colspan=2 id=\"td-%1-%7\"></td>\n"
                             "<td align=center><img width=%5 height=%6 src=\"file://%2\"></td>\n"
                             "</tr>"
                             "</table>\n"
                             "<script>\naddComparison(\"td-%1-%7\",\"file://%3\",\"file://%4\",%5,%6);\n</script>\n" )
                           .arg( theTestName,
                                 myDiffImageFile,
                                 mRenderedImageFile,
                                 mExpectedImageFile )
                           .arg( imgWidth ).arg( imgHeight )
                           .arg( renderCounter++ );

  QString prefix;
  if ( !mControlPathPrefix.isNull() )
  {
    prefix = QString( " (prefix %1)" ).arg( mControlPathPrefix );
  }
  //
  // To get the images into CDash
  //
  emitDashMessage( "Rendered Image " + theTestName + prefix, QgsDartMeasurement::ImagePng, mRenderedImageFile );
  emitDashMessage( "Expected Image " + theTestName + prefix, QgsDartMeasurement::ImagePng, mExpectedImageFile );

  //
  // Put the same info to debug too
  //

  qDebug( "Expected size: %dw x %dh", myExpectedImage.width(), myExpectedImage.height() );
  qDebug( "Actual   size: %dw x %dh", myResultImage.width(), myResultImage.height() );

  if ( mMatchTarget != myPixelCount )
  {
    qDebug( "Test image and result image for %s are different dimensions", theTestName.toLocal8Bit().constData() );

    if ( qAbs( myExpectedImage.width() - myResultImage.width() ) > mMaxSizeDifferenceX ||
         qAbs( myExpectedImage.height() - myResultImage.height() ) > mMaxSizeDifferenceY )
    {
      mReport += "<tr><td colspan=3>";
      mReport += "<font color=red>Expected image and result image for " + theTestName + " are different dimensions - FAILING!</font>";
      mReport += "</td></tr>";
      mReport += myImagesString;
      delete maskImage;
      return false;
    }
    else
    {
      mReport += "<tr><td colspan=3>";
      mReport += "Expected image and result image for " + theTestName + " are different dimensions, but within tolerance";
      mReport += "</td></tr>";
    }
  }

  //
  // Now iterate through them counting how many
  // dissimilar pixel values there are
  //

  int maxHeight = qMin( myExpectedImage.height(), myResultImage.height() );
  int maxWidth = qMin( myExpectedImage.width(), myResultImage.width() );

  mMismatchCount = 0;
  int colorTolerance = static_cast< int >( mColorTolerance );
  for ( int y = 0; y < maxHeight; ++y )
  {
    const QRgb* expectedScanline = reinterpret_cast< const QRgb* >( myExpectedImage.constScanLine( y ) );
    const QRgb* resultScanline = reinterpret_cast< const QRgb* >( myResultImage.constScanLine( y ) );
    const QRgb* maskScanline = hasMask ? reinterpret_cast< const QRgb* >( maskImage->constScanLine( y ) ) : nullptr;
    QRgb* diffScanline = reinterpret_cast< QRgb* >( myDifferenceImage.scanLine( y ) );

    for ( int x = 0; x < maxWidth; ++x )
    {
      int maskTolerance = hasMask ? qRed( maskScanline[ x ] ) : 0;
      int pixelTolerance = qMax( colorTolerance, maskTolerance );
      if ( pixelTolerance == 255 )
      {
        //skip pixel
        continue;
      }

      QRgb myExpectedPixel = expectedScanline[x];
      QRgb myActualPixel = resultScanline[x];
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
        if ( qAbs( qRed( myExpectedPixel ) - qRed( myActualPixel ) ) > pixelTolerance ||
             qAbs( qGreen( myExpectedPixel ) - qGreen( myActualPixel ) ) > pixelTolerance ||
             qAbs( qBlue( myExpectedPixel ) - qBlue( myActualPixel ) ) > pixelTolerance ||
             qAbs( qAlpha( myExpectedPixel ) - qAlpha( myActualPixel ) ) > pixelTolerance )
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
  emitDashMessage( "Difference Image " + theTestName + prefix, QgsDartMeasurement::ImagePng, myDiffImageFile );
  delete maskImage;

  //
  // Send match result to debug
  //
  qDebug( "%d/%d pixels mismatched (%d allowed)", mMismatchCount, mMatchTarget, theMismatchCount );

  //
  // Send match result to report
  //
  mReport += QString( "<tr><td colspan=3>%1/%2 pixels mismatched (allowed threshold: %3, allowed color component tolerance: %4)</td></tr>" )
             .arg( mMismatchCount ).arg( mMatchTarget ).arg( theMismatchCount ).arg( mColorTolerance );

  //
  // And send it to CDash
  //
  emitDashMessage( "Mismatch Count", QgsDartMeasurement::Integer, QString( "%1/%2" ).arg( mMismatchCount ).arg( mMatchTarget ) );

  if ( mMismatchCount <= theMismatchCount )
  {
    mReport += "<tr><td colspan = 3>\n";
    mReport += "Test image and result image for " + theTestName + " are matched<br>";
    mReport += "</td></tr>";
    if ( mElapsedTimeTarget != 0 && mElapsedTimeTarget < mElapsedTime )
    {
      //test failed because it took too long...
      qDebug( "Test failed because render step took too long" );
      mReport += "<tr><td colspan = 3>\n";
      mReport += "<font color=red>Test failed because render step took too long</font>";
      mReport += "</td></tr>";
      mReport += myImagesString;
      return false;
    }
    else
    {
      mReport += myImagesString;
      return true;
    }
  }

  bool myAnomalyMatchFlag = isKnownAnomaly( myDiffImageFile );
  if ( myAnomalyMatchFlag )
  {
    mReport += "<tr><td colspan=3>"
               "Difference image matched a known anomaly - passing test! "
               "</td></tr>";
    return true;
  }

  mReport += "<tr><td colspan=3></td></tr>";
  emitDashMessage( "Image mismatch", QgsDartMeasurement::Text, "Difference image did not match any known anomaly or mask."
                   " If you feel the difference image should be considered an anomaly "
                   "you can do something like this\n"
                   "cp '" + myDiffImageFile + "' " + controlImagePath() + mControlName +
                   "/\nIf it should be included in the mask run\n"
                   "scripts/generate_test_mask_image.py '" + mExpectedImageFile + "' '" + mRenderedImageFile + "'\n" );

  mReport += "<tr><td colspan = 3>\n";
  mReport += "<font color=red>Test image and result image for " + theTestName + " are mismatched</font><br>";
  mReport += "</td></tr>";
  mReport += myImagesString;
  return false;
}
