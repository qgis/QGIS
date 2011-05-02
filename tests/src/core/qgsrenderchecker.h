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
/* $Id: qgsfield.h 6833 2007-03-24 22:40:10Z wonder $ */

#ifndef QGSRENDERCHECKER_H
#define QGSRENDERCHECKER_H

#include <QString>
#include <qgsmaprenderer.h> 


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

  QString report() { return mReport; };
  float matchPercent() { return static_cast<float>(mMismatchCount) / 
                                static_cast<float>(mMatchTarget) * 100; };
  unsigned int mismatchCount() { return mMismatchCount; };
  unsigned int matchTarget() { return mMatchTarget; };
  //only records time for actual render part
  int elapsedTime() { return mElapsedTime; };
  void setElapsedTimeTarget(int theTarget) { mElapsedTimeTarget = theTarget; };
  void setExpectedImage (QString theImageFileName) { mExpectedImageFile = theImageFileName; };
  void setRenderedImage (QString theImageFileName) { mRenderedImageFile = theImageFileName; };
  void setMapRenderer ( QgsMapRenderer *  thepMapRenderer) { mpMapRenderer = thepMapRenderer; };
  /**
   * Test using renderer to generate the image to be compared.
   * @param theTestName - to be used as the basis for writing a file to 
   * e.g. /tmp/theTestName.png
   * @note make sure to call setExpectedImage and setMapRenderer first
   */
  bool runTest( QString theTestName );

  /**
   * Test using two arbitary images (map renderer will not be used)
   * @param theTestName - to be used as the basis for writing a file to 
   * e.g. /tmp/theTestName.png
   * @note: make sure to call setExpectedImage and setRenderedImage first.
   */
  bool compareImages( QString theTestName );
private:
  QString mReport;
  QString mExpectedImageFile;
  QString mRenderedImageFile; 
  unsigned int mMismatchCount;
  unsigned int mMatchTarget;
  int mElapsedTime;
  int mElapsedTimeTarget;
  QgsMapRenderer * mpMapRenderer;

}; // class QgsRenderChecker

#endif
