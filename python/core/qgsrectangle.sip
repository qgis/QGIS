
/*! \class QgsRectangle
 * \brief A rectangle specified with double values.
 *
 * QgsRectangle is used to store a rectangle when double values are required.
 * Examples are storing a layer extent or the current view extent of a map
 */
class QgsRectangle
{
%TypeHeaderCode
#include <qgsrectangle.h>
%End

  public:
    //! Constructor
    QgsRectangle( double xmin = 0, double ymin = 0, double xmax = 0, double ymax = 0 );
    //! Construct a rectangle from two points. The rectangle is normalized after construction.
    QgsRectangle( const QgsPoint & p1, const QgsPoint & p2 );
    //! Construct a rectangle from a QRectF. The rectangle is normalized after construction.
    //! @note added in 2.0
    QgsRectangle( const QRectF & qRectF );
    //! Copy constructor
    QgsRectangle( const QgsRectangle &other );
    //! Destructor
    ~QgsRectangle();
    //! Set the rectangle from two QgsPoints. The rectangle is
    //! normalised after construction.
    void set( const QgsPoint& p1, const QgsPoint& p2 );
    //! Set the rectangle from four points. The rectangle is
    //! normalised after construction.
    void set( double xmin, double ymin, double xmax, double ymax );
    //! Set the minimum x value
    void setXMinimum( double x );
    //! Set the maximum x value
    void setXMaximum( double x );
    //! Set the minimum y value
    void setYMinimum( double y );
    //! Set the maximum y value
    void setYMaximum( double y );
    //! Set a rectangle so that min corner is at max
    //! and max corner is at min. It is NOT normalized.
    void setMinimal();
    //! Get the x maximum value (right side of rectangle)
    double xMaximum() const;
    //! Get the x minimum value (left side of rectangle)
    double xMinimum() const;
    //! Get the y maximum value (top side of rectangle)
    double yMaximum() const;
    //! Get the y minimum value (bottom side of rectangle)
    double yMinimum() const;
    //! Normalize the rectangle so it has non-negative width/height
    void normalize();
    //! Width of the rectangle
    double width() const;
    //! Height of the rectangle
    double height() const;
    //! Center point of the rectangle
    QgsPoint center() const;
    //! Scale the rectangle around its center point
    void scale( double scaleFactor, const QgsPoint *c = 0 );
    void scale( double scaleFactor, double centerX, double centerY );
    /** Get rectangle enlarged by buffer.
     * @note added in 2.1 */
    QgsRectangle buffer( double width );
    //! return the intersection with the given rectangle
    QgsRectangle intersect( const QgsRectangle *rect ) const;
    //! returns true when rectangle intersects with other rectangle
    bool intersects( const QgsRectangle& rect ) const;
    //! return true when rectangle contains other rectangle
    //! @note added in version 1.1
    bool contains( const QgsRectangle& rect ) const;
    //! return true when rectangle contains a point
    //! @note added in version 1.3
    bool contains( const QgsPoint &p ) const;
    //! expand the rectangle so that covers both the original rectangle and the given rectangle
    void combineExtentWith( QgsRectangle *rect );
    //! expand the rectangle so that covers both the original rectangle and the given point
    void combineExtentWith( double x, double y );
    //! test if rectangle is empty
    //! Empty rectangle may still be non-null if it contains valid information (e.g. bounding box of a point)
    bool isEmpty() const;
    //! test if the rectangle is null (all coordinates zero or after call to setMinimal()).
    //! Null rectangle is also an empty rectangle.
    //! @note added in 2.4
    bool isNull() const;
    //! returns string representation in Wkt form
    QString asWktCoordinates() const;
    //! returns string representation as WKT Polygon
    //! @note added in 2.0
    QString asWktPolygon() const;
    //! returns a QRectF with same coordinates.
    //! @note added in 2.0
    QRectF toRectF() const;
    //! returns string representation of form xmin,ymin xmax,ymax
    QString toString( bool automaticPrecision = false ) const;
    //! overloaded toString that allows precision of numbers to be set
    QString toString( int thePrecision ) const;
    //! returns rectangle as a polygon
    QString asPolygon() const;
    /*! Comparison operator
      @return True if rectangles are equal
    */
    bool operator==( const QgsRectangle &r1 ) const;
    /*! Comparison operator
    @return False if rectangles are equal
     */
    bool operator!=( const QgsRectangle &r1 ) const;
    /** updates rectangle to include passed argument */
    void unionRect( const QgsRectangle& rect );

    /** Returns true if the rectangle has finite boundaries. Will
        return false if any of the rectangle boundaries are NaN or Inf. */
    bool isFinite() const;

    //! swap x/y
    //! @note added in 1.9
    void invert();
};
