

class QgsPoint 
{

%TypeHeaderCode
#include <qgspoint.h>
#include <QString>
%End

    
public:
    /// Default constructor
  QgsPoint();
    
  /*! Create a point from another point */
  QgsPoint(const QgsPoint& p);

  /*! Create a point from x,y coordinates
   * @param x x coordinate
   * @param y y coordinate
   */
  QgsPoint(double x, double y);

  ~QgsPoint();

  /*! Sets the x value of the point
   * @param x x coordinate
   */
  void setX(double x);

  /*! Sets the y value of the point
   * @param y y coordinate
   */
  void setY(double y);
    
  /*! Sets the x and y value of the point */
  void set(double x, double y);
    
  /*! Get the x value of the point
   * @return x coordinate
   */
  double x() const;

  /*! Get the y value of the point
   * @return y coordinate 
   */
  double y() const;

  //! String representation of the point (x,y)
  QString stringRep() const;

  //! As above but with precision for string representaiton of a point
  QString stringRep(int thePrecision) const;
  
  /*! Return the well known text representation for the point.
   * The wkt is created without an SRID.
   * @return Well known text in the form POINT(x y)
   */
  QString wellKnownText();

  /**Returns the squared distance between this point and x,y*/
  double sqrDist(double x, double y) const;

  //! equality operator
  bool operator==(const QgsPoint &other);
    
  //! Inequality operator
  bool operator!=(const QgsPoint &other);
    
  //! Multiply x and y by the given value
  void multiply(const double& scalar);
  
  
  SIP_PYOBJECT __repr__();
%MethodCode
    QString str = "(" + QString::number(sipCpp->x()) + "," + QString::number(sipCpp->y()) + ")";
    //QString str("(%f,%f)").arg(sipCpp->x()).arg(sipCpp->y());
    sipRes = PyString_FromString(str.toLocal8Bit().data());
%End

  
}; // class QgsPOint

