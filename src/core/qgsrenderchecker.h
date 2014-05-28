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

#include <qgis.h>
#include <QDir>
#include <QString>
#include <QRegExp>
#include <QList>

#include <qgsmaprenderer.h>
#include <qgslogger.h>
#include <qgsmapsettings.h>

class QImage;

/** \ingroup UnitTests
 * This is a helper class for unit tests that need to
 * write an image and compare it to an expected result
 * or render time.
 */
class CORE_EXPORT QgsRenderChecker
{
  public:

    QgsRenderChecker();

    //! Destructor
    ~QgsRenderChecker() {};

    QString controlImagePath() const;

    QString report() { return mReport; };
    float matchPercent()
    {
      return static_cast<float>( mMismatchCount ) /
             static_cast<float>( mMatchTarget ) * 100;
    }
    unsigned int mismatchCount() { return mMismatchCount; }
    unsigned int matchTarget() { return mMatchTarget; }
    //only records time for actual render part
    int elapsedTime() { return mElapsedTime; }
    void setElapsedTimeTarget( int theTarget ) { mElapsedTimeTarget = theTarget; };
    /** Base directory name for the control image (with control image path
      * suffixed) the path to the image will be constructed like this:
      * controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
      */
    void setControlName( const QString theName );
    /** Prefix where the control images are kept.
     * This will be appended to controlImagePath
      */
    void setControlPathPrefix( const QString theName ) { mControlPathPrefix = theName + QDir::separator(); }
    /** Get an md5 hash that uniquely identifies an image */
    QString imageToHash( QString theImageFile );

    void setRenderedImage( QString theImageFileName ) { mRenderedImageFile = theImageFileName; }
    //! @deprecated since 2.4 - use setMapSettings()
    Q_DECL_DEPRECATED void setMapRenderer( QgsMapRenderer *  thepMapRenderer );

    //! @note added in 2.4
    void setMapSettings( const QgsMapSettings& mapSettings );

    /** Set tolerance for color components used by runTest() and compareImages().
     * Default value is 0.
     * @param theColorTolerance is maximum difference for each color component
     * including alpha to be considered correct.
     * @note added in 2.1
     */
    void setColorTolerance( unsigned int theColorTolerance ) { mColorTolerance = theColorTolerance; }
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
    bool runTest( QString theTestName, unsigned int theMismatchCount = 0 );

    /**
     * Test using two arbitary images (map renderer will not be used)
     * @param theTestName - to be used as the basis for writing a file to
     * e.g. /tmp/theTestName.png
     * @param theMismatchCount - defaults to 0 - the number of pixels that
     * are allowed to be different from the control image. In some cases
     * rendering may be non-deterministic. This parameter allows you to account
     * for that by providing a tolerance.
     * @param theRenderedImageFile to optionally override the output filename
     * @note: make sure to call setExpectedImage and setRenderedImage first.
     */
    bool compareImages( QString theTestName, unsigned int theMismatchCount = 0, QString theRenderedImageFile = "" );
    /** Get a list of all the anomalies. An anomaly is a rendered difference
      * file where there is some red pixel content (indicating a render check
      * mismatch), but where the output was still acceptible. If the render
      * diff matches one of these anomalies we will still consider it to be
      * acceptible.
      * @return a bool indicating if the diff matched one of the anomaly files
    */
    bool isKnownAnomaly( QString theDiffImageFile );

    QString expectedImageFile() { return mExpectedImageFile; };

  protected:
    QString mReport;
    unsigned int mMatchTarget;
    int mElapsedTime;
    QString mRenderedImageFile;
    QString mExpectedImageFile;

  private:
    QString mControlName;
    unsigned int mMismatchCount;
    unsigned int mColorTolerance;
    int mElapsedTimeTarget;
    QgsMapSettings mMapSettings;
    QString mControlPathPrefix;

}; // class QgsRenderChecker


/** Compare two WKT strings with some tolerance
 * @param a first WKT string
 * @param b second WKT string
 * @param tolerance tolerance to use (optional, defaults to 0.000001)
 * @return bool indicating if the WKT are sufficiently equal
 */

inline bool compareWkt( QString a, QString b, double tolerance = 0.000001 )
{
  QgsDebugMsg( QString( "a:%1 b:%2 tol:%3" ).arg( a ).arg( b ).arg( tolerance ) );
  QRegExp re( "-?\\d+(?:\\.\\d+)?(?:[eE]\\d+)?" );

  QString a0( a ), b0( b );
  a0.replace( re, "#" );
  b0.replace( re, "#" );

  QgsDebugMsg( QString( "a0:%1 b0:%2" ).arg( a0 ).arg( b0 ) );

  if ( a0 != b0 )
    return false;

  QList<double> al, bl;

  int pos;
  for ( pos = 0; ( pos = re.indexIn( a, pos ) ) != -1; pos += re.matchedLength() )
  {
    al << re.cap( 0 ).toDouble();
  }
  for ( pos = 0; ( pos = re.indexIn( b, pos ) ) != -1; pos += re.matchedLength() )
  {
    bl << re.cap( 0 ).toDouble();
  }

  if ( al.size() != bl.size() )
    return false;

  for ( int i = 0; i < al.size(); i++ )
  {
    if ( !qgsDoubleNear( al[i], bl[i], tolerance ) )
      return false;
  }

  return true;
}

#endif
