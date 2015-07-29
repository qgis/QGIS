
/** Render checker for tests in python */

class QgsRenderChecker
{
%TypeHeaderCode
#include <qgsrenderchecker.h>
%End
  public:

    QgsRenderChecker();

    //! Destructor
    ~QgsRenderChecker();

    QString controlImagePath() const;

    QString report();
    float matchPercent();
    unsigned int mismatchCount();
    unsigned int matchTarget();
    //only records time for actual render part
    int elapsedTime();
    void setElapsedTimeTarget( int theTarget );

    /** Base directory name for the control image (with control image path
      * suffixed) the path to the image will be constructed like this:
      * controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
      */
    void setControlName( const QString &theName );

    /** Prefix where the control images are kept.
     * This will be appended to controlImagePath
      */
    void setControlPathPrefix( const QString &theName );

    void setControlPathSuffix( const QString& theName );

    /** Get an md5 hash that uniquely identifies an image */
    QString imageToHash( QString theImageFile );

    void setRenderedImage( QString theImageFileName );

    /**
     * The path of the rendered image can be retrieved through that method.
     * Will return the path set with setRenderedImage() or generated in runTest()
     *
     * @return The path to the rendered image
     */
    const QString& renderedImage();

    //! @deprecated since 2.4 - use setMapSettings()
    void setMapRenderer( QgsMapRenderer *  thepMapRenderer ) /Deprecated/;

    //! @note added in 2.4
    void setMapSettings( const QgsMapSettings& mapSettings );

    /** Set tolerance for color components used by runTest() and compareImages().
     * Default value is 0.
     * @param theColorTolerance is maximum difference for each color component
     * including alpha to be considered correct.
     * @note added in 2.1
     */
    void setColorTolerance( unsigned int theColorTolerance );

    /** Sets the largest allowable difference in size between the rendered and the expected image.
     * @param xTolerance x tolerance in pixels
     * @param yTolerance y tolerance in pixels
     * @note added in QGIS 2.12
     */
    void setSizeTolerance( int xTolerance, int yTolerance );

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

    /** Draws a checkboard pattern for image backgrounds, so that transparency is visible
     * without requiring a transparent background for the image
     */
    static void drawBackground( QImage* image );

    /**
     * Returns the path to the expected image file
     *
     * @return Path to the expected image file
     */
    const QString& expectedImageFile() const;

    /**
     * Call this to enable internal buffering of dash messages. You may later call
     * dashMessages() to get access to the buffered messages. If disabled (default)
     * dash messages will be sent immediately.
     *
     * @param enable Enable or disable buffering
     */
    void enableDashBuffering( bool enable );

    /**
     * Get access to buffered dash messages.
     * Only will return something if you call enableDashBuffering( true ); before.
     *
     * @return buffered dash messages
     * @note not available in python bindings
     */
    // const QVector<QgsDartMeasurement>& dartMeasurements() const;
};
