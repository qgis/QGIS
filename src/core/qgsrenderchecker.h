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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QDir>
#include <QString>
#include <QRegExp>
#include <QList>

#include "qgslogger.h"
#include "qgsmapsettings.h"
#include "qgsdartmeasurement.h"

class QImage;

/**
 * \ingroup core
 * This is a helper class for unit tests that need to
 * write an image and compare it to an expected result
 * or render time.
 */
class CORE_EXPORT QgsRenderChecker
{
  public:

    /**
     * Constructor for QgsRenderChecker.
     */
    QgsRenderChecker() = default;

    QString controlImagePath() const;

    QString report() { return mReport; }

    float matchPercent()
    {
      return static_cast<float>( mMismatchCount ) /
             static_cast<float>( mMatchTarget ) * 100;
    }
    unsigned int mismatchCount() { return mMismatchCount; }
    unsigned int matchTarget() { return mMatchTarget; }
    //only records time for actual render part
    int elapsedTime() { return mElapsedTime; }
    void setElapsedTimeTarget( int target ) { mElapsedTimeTarget = target; }

    /**
     * Base directory name for the control image (with control image path
     * suffixed) the path to the image will be constructed like this:
     * controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
     */
    void setControlName( const QString &name );

    /**
     * Prefix where the control images are kept.
     * This will be appended to controlImagePath
     */
    void setControlPathPrefix( const QString &name ) { mControlPathPrefix = name + '/'; }

    void setControlPathSuffix( const QString &name );

    //! Gets an md5 hash that uniquely identifies an image
    QString imageToHash( const QString &imageFile );

    void setRenderedImage( const QString &imageFileName ) { mRenderedImageFile = imageFileName; }

    /**
     * The path of the rendered image can be retrieved through that method.
     * Will return the path set with setRenderedImage() or generated in runTest()
     *
     * \returns The path to the rendered image
     */
    QString renderedImage() { return mRenderedImageFile; }

    //! \since QGIS 2.4
    void setMapSettings( const QgsMapSettings &mapSettings );

    /**
     * Set tolerance for color components used by runTest() and compareImages().
     * Default value is 0.
     * \param colorTolerance is maximum difference for each color component
     * including alpha to be considered correct.
     * \since QGIS 2.1
     */
    void setColorTolerance( unsigned int colorTolerance ) { mColorTolerance = colorTolerance; }

    /**
     * Sets the largest allowable difference in size between the rendered and the expected image.
     * \param xTolerance x tolerance in pixels
     * \param yTolerance y tolerance in pixels
     * \since QGIS 2.12
     */
    void setSizeTolerance( int xTolerance, int yTolerance ) { mMaxSizeDifferenceX = xTolerance; mMaxSizeDifferenceY = yTolerance; }

    /**
     * Test using renderer to generate the image to be compared.
     * \param testName - to be used as the basis for writing a file to
     * e.g. /tmp/theTestName.png
     * \param mismatchCount - defaults to 0 - the number of pixels that
     * are allowed to be different from the control image. In some cases
     * rendering may be non-deterministic. This parameter allows you to account
     * for that by providing a tolerance.
     * \note make sure to call setExpectedImage and setMapRenderer first
     */
    bool runTest( const QString &testName, unsigned int mismatchCount = 0 );

    /**
     * Test using two arbitrary images (map renderer will not be used)
     * \param testName - to be used as the basis for writing a file to
     * e.g. /tmp/theTestName.png
     * \param mismatchCount - defaults to 0 - the number of pixels that
     * are allowed to be different from the control image. In some cases
     * rendering may be non-deterministic. This parameter allows you to account
     * for that by providing a tolerance.
     * \param renderedImageFile to optionally override the output filename
     * \note: make sure to call setExpectedImage and setRenderedImage first.
     */
    bool compareImages( const QString &testName, unsigned int mismatchCount = 0, const QString &renderedImageFile = QString() );

    /**
     * Gets a list of all the anomalies. An anomaly is a rendered difference
     * file where there is some red pixel content (indicating a render check
     * mismatch), but where the output was still acceptable. If the render
     * diff matches one of these anomalies we will still consider it to be
     * acceptable.
     * \returns a bool indicating if the diff matched one of the anomaly files
     */
    bool isKnownAnomaly( const QString &diffImageFile );

    /**
     * Draws a checkboard pattern for image backgrounds, so that opacity is visible
     * without requiring a transparent background for the image
     */
    static void drawBackground( QImage *image );

    /**
     * Returns the path to the expected image file
     *
     * \returns Path to the expected image file
     */
    QString expectedImageFile() const { return mExpectedImageFile; }

    /**
     * Call this to enable internal buffering of dash messages. You may later call
     * dashMessages() to get access to the buffered messages. If disabled (default)
     * dash messages will be sent immediately.
     *
     * \param enable Enable or disable buffering
     */
    void enableDashBuffering( bool enable ) { mBufferDashMessages = enable; }

    /**
     * Gets access to buffered dash messages.
     * Only will return something if you call enableDashBuffering( TRUE ); before.
     *
     * \returns buffered dash messages
     */
    QVector<QgsDartMeasurement> dartMeasurements() const { return mDashMessages; }

  protected:
    QString mReport;
    unsigned int mMatchTarget = 0;
    int mElapsedTime = 0;
    QString mRenderedImageFile;
    QString mExpectedImageFile;

  private:
    void emitDashMessage( const QgsDartMeasurement &dashMessage );
    void emitDashMessage( const QString &name, QgsDartMeasurement::Type type, const QString &value );

    QString mControlName;
    unsigned int mMismatchCount = 0;
    unsigned int mColorTolerance = 0;
    int mMaxSizeDifferenceX = 0;
    int mMaxSizeDifferenceY = 0;
    int mElapsedTimeTarget = 0;
    QgsMapSettings mMapSettings;
    QString mControlPathPrefix;
    QString mControlPathSuffix;
    QVector<QgsDartMeasurement> mDashMessages;
    bool mBufferDashMessages = false;
}; // class QgsRenderChecker


/**
 * Compare two WKT strings with some tolerance
 * \param a first WKT string
 * \param b second WKT string
 * \param tolerance tolerance to use (optional, defaults to 0.000001)
 * \returns bool indicating if the WKT are sufficiently equal
 */

inline bool compareWkt( const QString &a, const QString &b, double tolerance = 0.000001 )
{
  QgsDebugMsg( QStringLiteral( "a:%1 b:%2 tol:%3" ).arg( a, b ).arg( tolerance ) );
  QRegExp re( "-?\\d+(?:\\.\\d+)?(?:[eE]\\d+)?" );

  QString a0( a ), b0( b );
  a0.replace( re, QStringLiteral( "#" ) );
  b0.replace( re, QStringLiteral( "#" ) );

  QgsDebugMsg( QStringLiteral( "a0:%1 b0:%2" ).arg( a0, b0 ) );

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
