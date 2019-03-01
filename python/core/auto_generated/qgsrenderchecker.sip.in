/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrenderchecker.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRenderChecker
{
%Docstring
This is a helper class for unit tests that need to
write an image and compare it to an expected result
or render time.
%End

%TypeHeaderCode
#include "qgsrenderchecker.h"
%End
  public:

    QgsRenderChecker();
%Docstring
Constructor for QgsRenderChecker.
%End

    QString controlImagePath() const;

    QString report();

    float matchPercent();
    unsigned int mismatchCount();
    unsigned int matchTarget();
    int elapsedTime();
    void setElapsedTimeTarget( int target );

    void setControlName( const QString &name );
%Docstring
Base directory name for the control image (with control image path
suffixed) the path to the image will be constructed like this:
controlImagePath + '/' + mControlName + '/' + mControlName + '.png'
%End

    void setControlPathPrefix( const QString &name );
%Docstring
Prefix where the control images are kept.
This will be appended to controlImagePath
%End

    void setControlPathSuffix( const QString &name );

    QString imageToHash( const QString &imageFile );
%Docstring
Gets an md5 hash that uniquely identifies an image
%End

    void setRenderedImage( const QString &imageFileName );

    QString renderedImage();
%Docstring
The path of the rendered image can be retrieved through that method.
Will return the path set with setRenderedImage() or generated in runTest()

:return: The path to the rendered image
%End

    void setMapSettings( const QgsMapSettings &mapSettings );
%Docstring

.. versionadded:: 2.4
%End

    void setColorTolerance( unsigned int colorTolerance );
%Docstring
Set tolerance for color components used by runTest() and compareImages().
Default value is 0.

:param colorTolerance: is maximum difference for each color component
                       including alpha to be considered correct.

.. versionadded:: 2.1
%End

    void setSizeTolerance( int xTolerance, int yTolerance );
%Docstring
Sets the largest allowable difference in size between the rendered and the expected image.

:param xTolerance: x tolerance in pixels
:param yTolerance: y tolerance in pixels

.. versionadded:: 2.12
%End

    bool runTest( const QString &testName, unsigned int mismatchCount = 0 );
%Docstring
Test using renderer to generate the image to be compared.

:param testName: - to be used as the basis for writing a file to
                 e.g. /tmp/theTestName.png
:param mismatchCount: - defaults to 0 - the number of pixels that
                      are allowed to be different from the control image. In some cases
                      rendering may be non-deterministic. This parameter allows you to account
                      for that by providing a tolerance.

.. note::

   make sure to call setExpectedImage and setMapRenderer first
%End

    bool compareImages( const QString &testName, unsigned int mismatchCount = 0, const QString &renderedImageFile = QString() );
%Docstring
Test using two arbitrary images (map renderer will not be used)

:param testName: - to be used as the basis for writing a file to
                 e.g. /tmp/theTestName.png
:param mismatchCount: - defaults to 0 - the number of pixels that
                      are allowed to be different from the control image. In some cases
                      rendering may be non-deterministic. This parameter allows you to account
                      for that by providing a tolerance.
:param renderedImageFile: to optionally override the output filename
\note: make sure to call setExpectedImage and setRenderedImage first.
%End

    bool isKnownAnomaly( const QString &diffImageFile );
%Docstring
Gets a list of all the anomalies. An anomaly is a rendered difference
file where there is some red pixel content (indicating a render check
mismatch), but where the output was still acceptable. If the render
diff matches one of these anomalies we will still consider it to be
acceptable.

:return: a bool indicating if the diff matched one of the anomaly files
%End

    static void drawBackground( QImage *image );
%Docstring
Draws a checkboard pattern for image backgrounds, so that opacity is visible
without requiring a transparent background for the image
%End

    QString expectedImageFile() const;
%Docstring
Returns the path to the expected image file

:return: Path to the expected image file
%End

    void enableDashBuffering( bool enable );
%Docstring
Call this to enable internal buffering of dash messages. You may later call
dashMessages() to get access to the buffered messages. If disabled (default)
dash messages will be sent immediately.

:param enable: Enable or disable buffering
%End

    QVector<QgsDartMeasurement> dartMeasurements() const;
%Docstring
Gets access to buffered dash messages.
Only will return something if you call enableDashBuffering( ``True`` ); before.

:return: buffered dash messages
%End

  protected:

}; // class QgsRenderChecker



bool compareWkt( const QString &a, const QString &b, double tolerance = 0.000001 );
%Docstring
Compare two WKT strings with some tolerance

:param a: first WKT string
:param b: second WKT string
:param tolerance: tolerance to use (optional, defaults to 0.000001)

:return: bool indicating if the WKT are sufficiently equal
%End

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrenderchecker.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
