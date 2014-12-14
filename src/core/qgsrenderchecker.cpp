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

QgsRenderChecker::QgsRenderChecker() :
    mReport( "" ),
    mMatchTarget( 0 ),
    mElapsedTime( 0 ),
    mRenderedImageFile( "" ),
    mExpectedImageFile( "" ),
    mMismatchCount( 0 ),
    mColorTolerance( 0 ),
    mElapsedTimeTarget( 0 ),
    mBufferDashMessages( false )
{
}

QString QgsRenderChecker::controlImagePath() const
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myControlImageDir = myDataDir + QDir::separator() + "control_images" +
                              QDir::separator() + mControlPathPrefix;
  return myControlImageDir;
}

void QgsRenderChecker::setControlName( const QString &theName )
{
  mControlName = theName;
  mExpectedImageFile = controlImagePath() + theName + QDir::separator() + mControlPathSuffix
                       + theName + ".png";
}

QString QgsRenderChecker::imageToHash( QString theImageFile )
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

bool QgsRenderChecker::isKnownAnomaly( QString theDiffImageFile )
{
  QString myControlImageDir = controlImagePath() + mControlName
                              + QDir::separator();
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
    QString myAnomalyHash = imageToHash( controlImagePath() + mControlName
                                         + QDir::separator() + myFile );
    QString myHashMessage = QString(
                              "Checking if anomaly %1 (hash %2)<br>" )
                            .arg( myFile )
                            .arg( myAnomalyHash );
    myHashMessage += QString( "&nbsp; matches %1 (hash %2)" )
                     .arg( theDiffImageFile )
                     .arg( myImageHash );
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

bool QgsRenderChecker::runTest( QString theTestName,
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
  mRenderedImageFile = QDir::tempPath() + QDir::separator() +
                       theTestName + "_result.png";

  myImage.setDotsPerMeterX( myExpectedImage.dotsPerMeterX() );
  myImage.setDotsPerMeterY( myExpectedImage.dotsPerMeterY() );
  myImage.save( mRenderedImageFile, "PNG", 100 );

  //create a world file to go with the image...

  QFile wldFile( QDir::tempPath() + QDir::separator() + theTestName + "_result.wld" );
  if ( wldFile.open( QIODevice::WriteOnly ) )
  {
    QgsRectangle r = mMapSettings.extent();

    QTextStream stream( &wldFile );
    stream << QString( "%1\r\n0 \r\n0 \r\n%2\r\n%3\r\n%4\r\n" )
    .arg( qgsDoubleToString( mMapSettings.mapUnitsPerPixel() ) )
    .arg( qgsDoubleToString( -mMapSettings.mapUnitsPerPixel() ) )
    .arg( qgsDoubleToString( r.xMinimum() + mMapSettings.mapUnitsPerPixel() / 2.0 ) )
    .arg( qgsDoubleToString( r.yMaximum() - mMapSettings.mapUnitsPerPixel() / 2.0 ) );
  }

  return compareImages( theTestName, theMismatchCount );
}


bool QgsRenderChecker::compareImages( QString theTestName,
                                      unsigned int theMismatchCount,
                                      QString theRenderedImageFile )
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
  QImage myDifferenceImage( myExpectedImage.width(),
                            myExpectedImage.height(),
                            QImage::Format_RGB32 );
  QString myDiffImageFile = QDir::tempPath() + QDir::separator() +
                            QDir::separator()  +
                            theTestName + "_result_diff.png";
  myDifferenceImage.fill( qRgb( 152, 219, 249 ) );

  //
  // Set pixel count score and target
  //
  mMatchTarget = myExpectedImage.width() * myExpectedImage.height();
  unsigned int myPixelCount = myResultImage.width() * myResultImage.height();
  //
  // Set the report with the result
  //
  mReport = "<table>";
  mReport += "<tr><td colspan=2>";
  mReport += "Test image and result image for " + theTestName + "<br>"
             "Expected size: " + QString::number( myExpectedImage.width() ).toLocal8Bit() + "w x " +
             QString::number( myExpectedImage.height() ).toLocal8Bit() + "h (" +
             QString::number( mMatchTarget ).toLocal8Bit() + " pixels)<br>"
             "Actual   size: " + QString::number( myResultImage.width() ).toLocal8Bit() + "w x " +
             QString::number( myResultImage.height() ).toLocal8Bit() + "h (" +
             QString::number( myPixelCount ).toLocal8Bit() + " pixels)";
  mReport += "</td></tr>";
  mReport += "<tr><td colspan = 2>\n";
  mReport += "Expected Duration : <= " + QString::number( mElapsedTimeTarget ) +
             "ms (0 indicates not specified)<br>";
  mReport += "Actual Duration :  " + QString::number( mElapsedTime ) + "ms<br>";

  // limit image size in page to something reasonable
  int imgWidth = 420;
  int imgHeight = 280;
  if ( ! myExpectedImage.isNull() )
  {
    imgWidth = qMin( myExpectedImage.width(), imgWidth );
    imgHeight = myExpectedImage.height() * imgWidth / myExpectedImage.width();
  }
  QString myImagesString = "</td></tr>"
                           "<tr><td>Test Result:</td><td>Expected Result:</td><td>Difference (all blue is good, any red is bad)</td></tr>\n"
                           "<tr><td><img width=" + QString::number( imgWidth ) +
                           " height=" + QString::number( imgHeight ) +
                           " src=\"file://" +
                           mRenderedImageFile +
                           "\"></td>\n<td><img width=" + QString::number( imgWidth ) +
                           " height=" + QString::number( imgHeight ) +
                           " src=\"file://" +
                           mExpectedImageFile +
                           "\"></td>\n<td><img width=" + QString::number( imgWidth ) +
                           " height=" + QString::number( imgHeight ) +
                           " src=\"file://" +
                           myDiffImageFile  +
                           "\"></td>\n</tr>\n</table>";

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
  emitDashMessage( "Difference Image " + theTestName + prefix, QgsDartMeasurement::ImagePng, myDiffImageFile );

  //
  // Put the same info to debug too
  //

  qDebug( "Expected size: %dw x %dh", myExpectedImage.width(), myExpectedImage.height() );
  qDebug( "Actual   size: %dw x %dh", myResultImage.width(), myResultImage.height() );

  if ( mMatchTarget != myPixelCount )
  {
    qDebug( "Test image and result image for %s are different - FAILING!", theTestName.toLocal8Bit().constData() );
    mReport += "<tr><td colspan=3>";
    mReport += "<font color=red>Expected image and result image for " + theTestName + " are different dimensions - FAILING!</font>";
    mReport += "</td></tr>";
    mReport += myImagesString;
    return false;
  }

  //
  // Now iterate through them counting how many
  // dissimilar pixel values there are
  //

  mMismatchCount = 0;
  int colorTolerance = ( int ) mColorTolerance;
  for ( int x = 0; x < myExpectedImage.width(); ++x )
  {
    for ( int y = 0; y < myExpectedImage.height(); ++y )
    {
      QRgb myExpectedPixel = myExpectedImage.pixel( x, y );
      QRgb myActualPixel = myResultImage.pixel( x, y );
      if ( mColorTolerance == 0 )
      {
        if ( myExpectedPixel != myActualPixel )
        {
          ++mMismatchCount;
          myDifferenceImage.setPixel( x, y, qRgb( 255, 0, 0 ) );
        }
      }
      else
      {
        if ( qAbs( qRed( myExpectedPixel ) - qRed( myActualPixel ) ) > colorTolerance ||
             qAbs( qGreen( myExpectedPixel ) - qGreen( myActualPixel ) ) > colorTolerance ||
             qAbs( qBlue( myExpectedPixel ) - qBlue( myActualPixel ) ) > colorTolerance ||
             qAbs( qAlpha( myExpectedPixel ) - qAlpha( myActualPixel ) ) > colorTolerance )
        {
          ++mMismatchCount;
          myDifferenceImage.setPixel( x, y, qRgb( 255, 0, 0 ) );
        }
      }
    }
  }
  //
  //save the diff image to disk
  //
  myDifferenceImage.save( myDiffImageFile );

  //
  // Send match result to debug
  //
  qDebug( "%d/%d pixels mismatched (%d allowed)", mMismatchCount, mMatchTarget, theMismatchCount );

  //
  // Send match result to report
  //
  mReport += "<tr><td colspan=3>" +
             QString::number( mMismatchCount ) + "/" +
             QString::number( mMatchTarget ) +
             " pixels mismatched (allowed threshold: " +
             QString::number( theMismatchCount ) +
             ", allowed color component tolerance: " +
             QString::number( mColorTolerance ) + ")";
  mReport += "</td></tr>";

  //
  // And send it to CDash
  //
  emitDashMessage( "Mismatch Count", QgsDartMeasurement::Integer, QString( "%1/%2" ).arg( mMismatchCount ).arg( mMatchTarget ) );

  bool myAnomalyMatchFlag = isKnownAnomaly( myDiffImageFile );

  if ( myAnomalyMatchFlag )
  {
    mReport += "<tr><td colspan=3>"
               "Difference image matched a known anomaly - passing test! "
               "</td></tr>";
    return true;
  }
  else
  {
    mReport += "<tr><td colspan=3>"
               "</td></tr>";
    emitDashMessage( "No Anomalies Match", QgsDartMeasurement::Text, "Difference image did not match any known anomaly."
                     " If you feel the difference image should be considered an anomaly "
                     "you can do something like this\n"
                     "cp " + myDiffImageFile  + " ../tests/testdata/control_images/" + theTestName +
                     "/<imagename>.{wld,png}" );
  }

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
  else
  {
    mReport += "<tr><td colspan = 3>\n";
    mReport += "<font color=red>Test image and result image for " + theTestName + " are mismatched</font><br>";
    mReport += "</td></tr>";
    mReport += myImagesString;
    return false;
  }
}
