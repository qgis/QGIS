/***************************************************************************
     qgsrenderchecker.h - check maprender output against an expected image 
                     --------------------------------------
               Date                 : 18 Jan 2008
               Copyright            : (C) 2008 by Tim Sutton
               email                : tim  @ linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERCHECKER_H
#define QGSRENDERCHECKER_H

#include <QString>
#include <qgsmaprenderer.h> 
class QImage;

/** \ingroup UnitTests
 * This is a helper class for unit tests that need to 
 * write an image and compare it to an expected result
 * or render time.
 */
class QgsRenderChecker
{
public:
  
  QgsRenderChecker();

  //! Destructor
   ~QgsRenderChecker(){};

  QString controlImagePath() const;

  QString report() { return mReport; };
  float matchPercent() { return static_cast<float>(mMismatchCount) / 
                                static_cast<float>(mMatchTarget) * 100; };
  unsigned int mismatchCount() { return mMismatchCount; };
  unsigned int matchTarget() { return mMatchTarget; };
  //only records time for actual render part
  int elapsedTime() { return mElapsedTime; };
  void setElapsedTimeTarget(int theTarget) { mElapsedTimeTarget = theTarget; };
  /** Base directory name for the control image (with control image path
    * suffixed) the path to the image will be constructed like this:
    * controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
    */
  void setControlName(const QString theName);
  /** Get an md5 hash that uniquely identifies an image */
  QString imageToHash( QString theImageFile );

  void setRenderedImage (QString theImageFileName) { mRenderedImageFile = theImageFileName; };
  void setMapRenderer ( QgsMapRenderer *  thepMapRenderer) { mpMapRenderer = thepMapRenderer; };
  /**
   * Test using renderer to generate the image to be compared.
   * @param theTestName - to be used as the basis for writing a file to 
   * e.g. /tmp/theTestName.png
   * @param theMismatchCount - defaults to 0 - the number of pixels that
   * are allowed to be different from the control image. In some cases
   * rendering may be non-deterministic. This parameter allows you to account
   * for that by providing a tolerance.
   * @note make sure to call setExpectedImage and setMapRenderer first
   */
  bool runTest( QString theTestName, unsigned int theMismatchCount=0 );

  /**
   * Test using two arbitary images (map renderer will not be used)
   * @param theTestName - to be used as the basis for writing a file to 
   * e.g. /tmp/theTestName.png
   * @param theMismatchCount - defaults to 0 - the number of pixels that
   * are allowed to be different from the control image. In some cases
   * rendering may be non-deterministic. This parameter allows you to account
   * for that by providing a tolerance.
   * @note: make sure to call setExpectedImage and setRenderedImage first.
   */
  bool compareImages( QString theTestName, unsigned int theMismatchCount=0 );
  /** Get a list of all teh anomalies. An anomaly is a rendered difference
    * file where there is some red pixel content (indicating a render check
    * mismatch), but where the output was still acceptible. If the render
    * diff matches one of these anomalies we will still consider it to be
    * acceptible.
    * @return a bool indicating if the diff matched one of the anomaly files
  */
  bool isKnownAnomaly( QString theDiffImageFile );

private:

  QString mReport;
  QString mExpectedImageFile;
  QString mControlName;
  QString mRenderedImageFile; 
  unsigned int mMismatchCount;
  unsigned int mMatchTarget;
  int mElapsedTime;
  int mElapsedTimeTarget;
  QgsMapRenderer * mpMapRenderer;

}; // class QgsRenderChecker

#endif
