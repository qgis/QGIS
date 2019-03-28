/***************************************************************************
    qgsmultirenderchecker.h
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

#ifndef QGSMULTIRENDERCHECKER_H
#define QGSMULTIRENDERCHECKER_H

#include "qgis_core.h"
#include "qgsrenderchecker.h"

class QgsLayout;

/**
 * \ingroup core
 * This class allows checking rendered images against comparison images.
 * Its main purpose is for the unit testing framework.
 *
 * It will either
 * <ul>
 *   <li>take an externally rendered image (setRenderedImage())</li>
 *   <li>render the image based on provided mapSettings (setMapSettings())</li>
 * </ul>
 *
 * This image will then be compared against one or several images in a folder inside
 * the control directory (tests/testdata/control_images/{controlName}).
 *
 * There are modes for single and for multiple reference images.
 * <ul>
 *   <li>If there are no subfolders in the control directory, it will assume an image
 *       with the name {controlImage}.png in the control directory itself.</li>
 *
 *   <li>If there are subfolders inside the control directory, it will search for images
 *       with the name {controlImage}.png in every subfolder.</li>
 * </ul>
 *
 * For every control image there may be one or several randomly named anomaly images defining
 * allowed anomalies.
 * For every control image, the allowed mismatch and color tolerance values will be calculated
 * individually.
 *
 * \since QGIS 2.8
 */

class CORE_EXPORT QgsMultiRenderChecker
{
  public:

    /**
     * Constructor for QgsMultiRenderChecker.
     */
    QgsMultiRenderChecker() = default;

    virtual ~QgsMultiRenderChecker() = default;

    /**
     * Base directory name for the control image (with control image path
     * suffixed) the path to the image will be constructed like this:
     * controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
     */
    void setControlName( const QString &name );

    void setControlPathPrefix( const QString &prefix );

    /**
     * Set the path to the rendered image. If this is not set or set to null QString, an image
     * will be rendered based on the provided mapsettings
     *
     * \param renderedImagePath A path to the rendered image with which control images will be compared
     */
    void setRenderedImage( const QString &renderedImagePath ) { mRenderedImage = renderedImagePath; }

    /**
     * Set the map settings to use to render the image
     *
     * \param mapSettings The map settings
     */
    void setMapSettings( const QgsMapSettings &mapSettings );

    /**
     * Set tolerance for color components used by runTest()
     * Default value is 0.
     *
     * \param colorTolerance The maximum difference for each color component
     *                          including alpha to be considered correct.
     */
    void setColorTolerance( unsigned int colorTolerance ) { mColorTolerance = colorTolerance; }

    /**
     * Sets the largest allowable difference in size between the rendered and the expected image.
     * \param xTolerance x tolerance in pixels
     * \param yTolerance y tolerance in pixels
     * \since QGIS 3.0
     */
    void setSizeTolerance( int xTolerance, int yTolerance ) { mMaxSizeDifferenceX = xTolerance; mMaxSizeDifferenceY = yTolerance; }

    /**
     * Test using renderer to generate the image to be compared.
     *
     * \param testName - to be used as the basis for writing a file to
     * e.g. /tmp/theTestName.png
     *
     * \param mismatchCount - defaults to 0 - the number of pixels that
     * are allowed to be different from the control image. In some cases
     * rendering may be non-deterministic. This parameter allows you to account
     * for that by providing a tolerance.
     *
     * \note make sure to call setExpectedImage and setMapSettings first
     */
    bool runTest( const QString &testName, unsigned int mismatchCount = 0 );

    /**
     * Returns a report for this test
     */
    QString report() const { return mReport; }

    /**
     * Returns the path to the control images.
     */
    QString controlImagePath() const;

    /**
     * Draws a checkboard pattern for image backgrounds, so that opacity is visible
     * without requiring a transparent background for the image
     */
    static void drawBackground( QImage *image ) { QgsRenderChecker::drawBackground( image ); }

  private:
    QString mReport;
    QString mRenderedImage;
    QString mControlName;
    QString mControlPathPrefix;
    unsigned int mColorTolerance = 0;
    int mMaxSizeDifferenceX = 0;
    int mMaxSizeDifferenceY = 0;
    QgsMapSettings mMapSettings;
};

SIP_FEATURE( TESTS )
SIP_IF_FEATURE( TESTS )

///@cond PRIVATE

/**
 * \ingroup core
 * \class QgsLayoutChecker
 * Renders a layout to an image and compares with an expected output
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutChecker : public QgsMultiRenderChecker
{
  public:

    /**
     * Constructor for QgsLayoutChecker.
     */
    QgsLayoutChecker( const QString &testName, QgsLayout *layout );

    /**
     * Sets the output (reference) image \a size.
     */
    void setSize( QSize size ) { mSize = size; }

    /**
     * Runs a render check on the layout, adding results to the specified \a report.
     *
     * The maximum number of allowable pixels differing from the reference image is
     * specified via the \a pixelDiff argument.
     *
     * A reference image can be created by setting \a createReferenceImage to TRUE
     * in this case the test will always return TRUE.
     *
     * The page number is specified via \a page, where 0 corresponds to the first
     * page in the layout.
     *
     * Returns FALSE if the rendered layout differs from the expected reference image.
     */
    bool testLayout( QString &report, int page = 0, int pixelDiff = 0, bool createReferenceImage = false );

  private:
    QgsLayoutChecker() = delete;

    QString mTestName;
    QgsLayout *mLayout = nullptr;
    QSize mSize;
    int mDotsPerMeter;
};
///@endcond

SIP_END


#endif // QGSMULTIRENDERCHECKER_H
