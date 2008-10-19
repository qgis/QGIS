
/*! \class QgsRect
 * \brief A rectangle specified with double values.
 *
 * QgsRect is used to store a rectangle when double values are required. 
 * Examples are storing a layer extent or the current view extent of a map
 */
class QgsRect
{
%TypeHeaderCode
#include <qgsrect.h>
%End

 public:
    //! Constructor
    QgsRect(double xmin=0, double ymin=0, double xmax=0, double ymax=0);
    //! Construct a rectangle from two points. The rectangle is normalized after construction.
    QgsRect(const QgsPoint & p1, const QgsPoint & p2);
    //! Copy constructor
    QgsRect(const QgsRect &other);
    //! Destructor
    ~QgsRect();
    //! Set the rectangle from two QgsPoints. The rectangle is
    //normalised after construction. 
    void set(const QgsPoint& p1, const QgsPoint& p2);
    //! Set the rectangle from four points. The rectangle is
    //  normalised after construction. 
    void set(double xmin, double ymin, double xmax, double ymax);
    //! Set the minimum x value
    void setXMinimum(double x);
    //! Set the maximum x value
    void setXMaximum(double x);
    //! Set the maximum y value
    void setYMinimum(double y);
    //! Set the maximum y value
    void setYMaximum(double y);
    //! Set a rectangle so that min corner is at max
    // and max corner is at min. It is NOT normalized.
    void setMinimal();
    //! Get the x maximum value (right side of rectangle)
    double xMax() const;
    //! Get the x maximum value (right side of rectangle)
    double xMin() const;
    //! Get the x minimum value (left side of rectangle)
    double yMax() const;
    //! Get the y maximum value (top side of rectangle)
    double yMin() const;
    //! Normalize the rectangle so it has non-negative width/height
    void normalize();
    //! Width of the rectangle
    double width() const;
    //! Height of the rectangle
    double height() const;
    //! Center point of the rectangle
    QgsPoint center() const;
    //! Scale the rectangle around its center point
    void scale(double, const QgsPoint *c =0);
    //! Expand the rectangle to support zoom out scaling
    void expand(double, const QgsPoint *c = 0);
    //! return the intersection with the given rectangle
    QgsRect intersect(QgsRect *rect);
    //! returns true when rectangle intersects with other rectangle
    bool intersects(const QgsRect& rect) const;
    //! expand the rectangle so that covers both the original rectangle and the given rectangle
    void combineExtentWith(QgsRect *rect);
    //! expand the rectangle so that covers both the original rectangle and the given point
    void combineExtentWith(double x, double y);
    //! test if rectangle is empty
    bool isEmpty() const;
    //! returns string representation in Wkt form
    QString asWktCoordinates() const;
    //! returns string representation of form xmin,ymin xmax,ymax
    QString toString(bool automaticPrecision = false) const;
    //! overloaded toString that allows precision of numbers to be set
    QString toString(int thePrecision) const;
    //! returns rectangle s a polygon 
    QString asPolygon() const;
    /*! Comparison operator
      @return True if rectangles are equal
    */
    bool operator==(const QgsRect &r1) const;
    /*! Comparison operator
    @return False if rectangles are equal
     */
    bool operator!=(const QgsRect &r1) const;
   
    /** updates rectangle to include passed argument */
    void unionRect(const QgsRect& rect);
 
  
};
 
