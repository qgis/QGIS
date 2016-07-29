%Feature ARM

class QgsClipper
{
%TypeHeaderCode
#include <qgsclipper.h>
%End

  public:

    // Coordinates of the rectangular box that we trim to.
    //
    // These are the limits for X11 screen coordinates. The actual
    // values are +/-32767, but we allow a little bit of space for
    // rounding errors.

    // You may wonder why the clipping is done to these coordindates
    // rather than the boundaries of the qgis canvas. Reasons include:
    // - making the boundaries static const allows the compiler to
    //   optimise the code that uses these values more than if they changed
    //   for every call to the trim code.
    // - clipping takes quite a bit of CPU effort, and the less that this is
    //   done the better. More stuff would have to be clipped if the
    //   boundaries were the qgis canvas (but this may be offset by
    //   having less to draw).
    //
    // The limit is set to 30,000 instead of 32768 because that things
    // still go wrong.

    static const double MAX_X;
    static const double MIN_X;
    static const double MAX_Y;
    static const double MIN_Y;


    //! A handy way to refer to the four boundaries
    enum Boundary {XMax, XMin, YMax, YMin};

%If (!ARM)
    /**
     * Trims the given feature to a rectangular box. Returns the trimmed
     * feature in x and y. The shapeOpen parameter determines whether
     * the function treats the points as a closed shape (polygon), or as
     * an open shape (linestring).
     *
     * @note not available in python bindings on android
     */
    static void trimFeature( QVector<double>& x,
                             QVector<double>& y,
                             bool shapeOpen );
%End

    static void trimPolygon( QPolygonF& pts, const QgsRectangle& clipRect );

    /** Reads a polyline from WKB and clips it to clipExtent
      @param wkb pointer to the start of the line wkb
      @param clipExtent clipping bounds
      @param line out: clipped line coordinates*/
    static QgsConstWkbPtr clippedLineWKB( QgsConstWkbPtr& wkb, const QgsRectangle& clipExtent, QPolygonF& line );
};
