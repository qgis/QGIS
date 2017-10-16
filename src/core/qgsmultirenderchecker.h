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
     *
     * \returns A report
     */
    QString report() const { return mReport; }

    /**
     * \brief controlImagePath
     * \returns
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
    QgsMapSettings mMapSettings;
};

#ifdef ENABLE_TESTS
SIP_FEATURE( TESTS )
SIP_IF_FEATURE( TESTS )

///@cond PRIVATE

/**
 * \ingroup core
 * \class QgsCompositionChecker
 * Renders a composition to an image and compares with an expected output
 */
class CORE_EXPORT QgsCompositionChecker : public QgsMultiRenderChecker
{
  public:
    QgsCompositionChecker( const QString &testName, QgsComposition *composition );

    void setSize( QSize size ) { mSize = size; }

    bool testComposition( QString &checkedReport, int page = 0, int pixelDiff = 0 );

  private:
    QgsCompositionChecker(); //forbidden

    QString mTestName;
    QgsComposition *mComposition = nullptr;
    QSize mSize;
    int mDotsPerMeter;
};

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
     * The page number is specified via \a page, where 0 corresponds to the first
     * page in the layout.
     *
     * Returns false if the rendered layout differs from the expected reference image.
     */
    bool testLayout( QString &report, int page = 0, int pixelDiff = 0 );

  private:
    QgsLayoutChecker() = delete;

    QString mTestName;
    QgsLayout *mLayout = nullptr;
    QSize mSize;
    int mDotsPerMeter;
};
///@endcond

SIP_END
#endif


#endif // QGSMULTIRENDERCHECKER_H
