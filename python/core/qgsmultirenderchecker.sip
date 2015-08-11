/***************************************************************************
    qgsmultirenderchecker.sip
     --------------------------------------
    Date                 : 6.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 *
 * @note added in 2.8
 */

class QgsMultiRenderChecker
{
%TypeHeaderCode
#include <qgsmultirenderchecker.h>
%End
  public:
    QgsMultiRenderChecker();

    /**
     * Base directory name for the control image (with control image path
     * suffixed) the path to the image will be constructed like this:
     * controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
     */
    void setControlName( const QString& theName );

    void setControlPathPrefix( const QString& prefix );

    /**
     * Set the path to the rendered image. If this is not set or set to QString::Null, an image
     * will be rendered based on the provided mapsettings
     *
     * @param renderedImagePath A path to the rendered image with which control images will be compared
     */
    void setRenderedImage( const QString& renderedImagePath );

    /**
     * Set the map settings to use to render the image
     *
     * @param mapSettings The map settings
     */
    void setMapSettings( const QgsMapSettings& mapSettings );

    /**
     * Set tolerance for color components used by runTest()
     * Default value is 0.
     *
     * @param theColorTolerance The maximum difference for each color component
     *                          including alpha to be considered correct.
     */
    void setColorTolerance( unsigned int theColorTolerance );

    /**
     * Test using renderer to generate the image to be compared.
     *
     * @param theTestName - to be used as the basis for writing a file to
     * e.g. /tmp/theTestName.png
     *
     * @param theMismatchCount - defaults to 0 - the number of pixels that
     * are allowed to be different from the control image. In some cases
     * rendering may be non-deterministic. This parameter allows you to account
     * for that by providing a tolerance.
     *
     * @note make sure to call setExpectedImage and setMapSettings first
     */
    bool runTest( const QString& theTestName, unsigned int theMismatchCount = 0 );

    /**
     * Returns a report for this test
     *
     * @return A report
     */
    const QString& report() const;

    /**
     * @brief controlImagePath
     * @return
     */
    const QString controlImagePath() const;

    /** Draws a checkboard pattern for image backgrounds, so that transparency is visible
     * without requiring a transparent background for the image
     */
    static void drawBackground( QImage* image );
};

