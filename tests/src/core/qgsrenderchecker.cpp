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

#include <QDir>
#include <QColor>
#include <QPainter>
#include <QImage>
#include <QTime>


QgsRenderChecker::QgsRenderChecker( ) :
    mReport( "" ),
    mExpectedImageFile( "" ),
    mRenderedImageFile( "" ),
    mMismatchCount( 0 ),
    mMatchTarget( 0 ),
    mElapsedTime( 0 ),
    mElapsedTimeTarget( 0 ),
    mpMapRenderer( NULL )
{

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
  QImage myImage( myExpectedImage.width(),
                  myExpectedImage.height(),
                  QImage::Format_RGB32 );
  myImage.fill( qRgb( 152, 219, 249 ) );
  QPainter myPainter( &myImage );
  myPainter.setRenderHint( QPainter::Antialiasing );
  mpMapRenderer->setOutputSize( QSize(
                                  myExpectedImage.width(),
                                  myExpectedImage.height() ),
                                  myExpectedImage.logicalDpiX());
  QTime myTime;
  myTime.start();
  mpMapRenderer->render( &myPainter );
  mElapsedTime = myTime.elapsed();
  myPainter.end();
  //
  // Save the pixmap to disk so the user can make a
  // visual assessment if needed
  //
  mRenderedImageFile = QDir::tempPath() + QDir::separator() +
      theTestName + "_result.png";
  myImage.save( mRenderedImageFile, "PNG", 100 );
  return compareImages( theTestName, theMismatchCount );

}


bool QgsRenderChecker::compareImages( QString theTestName,
                                      unsigned int theMismatchCount )
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
  if ( mRenderedImageFile.isEmpty() )
  {
    qDebug( "QgsRenderChecker::runTest failed - Rendered Image File not set." );
    mReport = "<table>"
              "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
              "<tr><td>Nothing rendered</td>\n<td>Failed because Expected "
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
  QString myResultDiffImage = QDir::tempPath() + QDir::separator() +
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
             QString::number( myExpectedImage.width() ).toLocal8Bit() + "h (" +
             QString::number( mMatchTarget ).toLocal8Bit() + " pixels)<br>"
             "Actual   size: " + QString::number( myResultImage.width() ).toLocal8Bit() + "w x " +
             QString::number( myResultImage.width() ).toLocal8Bit() + "h (" +
             QString::number( myPixelCount ).toLocal8Bit() + " pixels)";
  mReport += "</td></tr>";
  mReport += "<tr><td colspan = 2>\n";
  mReport += "Expected Duration : <= " + QString::number( mElapsedTimeTarget ) +
             "ms (0 indicates not specified)<br>";
  mReport += "Actual Duration :  " + QString::number( mElapsedTime ) + "ms<br>";
  QString myImagesString = "</td></tr>"
                           "<tr><td>Test Result:</td><td>Expected Result:</td><td>Difference (all blue is good, any red is bad)</td></tr>\n"
                           "<tr><td><img src=\"file://" +
                           mRenderedImageFile +
                           "\"></td>\n<td><img src=\"file://" +
                           mExpectedImageFile +
                           "\"></td><td><img src=\"file://" +
                           myResultDiffImage  +
                           "\"></td>\n</tr>\n</table>";
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
  for ( int x = 0; x < myExpectedImage.width(); ++x )
  {
    for ( int y = 0; y < myExpectedImage.height(); ++y )
    {
      QRgb myExpectedPixel = myExpectedImage.pixel( x, y );
      QRgb myActualPixel = myResultImage.pixel( x, y );
      if ( myExpectedPixel != myActualPixel )
      {
        ++mMismatchCount;
        myDifferenceImage.setPixel( x, y, qRgb( 255, 0, 0 ) );
      }
    }
  }
  //
  //save the diff image to disk
  //
  myDifferenceImage.save( myResultDiffImage );

  //
  // Send match result to debug
  //
  qDebug( "%d/%d pixels mismatched", mMismatchCount, mMatchTarget );

  //
  // Send match result to report
  //
  mReport += "<tr><td colspan=3>" +
             QString::number( mMismatchCount ) + "/" +
             QString::number( mMatchTarget ) +
             " pixels mismatched";
  mReport += "</td></tr>";


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
