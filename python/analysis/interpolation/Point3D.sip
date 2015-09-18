class Point3D
{
%TypeHeaderCode
#include <Point3D.h>
%End
  public:
    Point3D();
    /** Constructor with the x-, y- and z-coordinate as arguments*/
    Point3D( double x, double y, double z );
    Point3D( const Point3D& p );
    ~Point3D();
    // Point3D& operator=( const Point3D& p );
    bool operator==( const Point3D& p );
    bool operator!=( const Point3D& p );
    /** Calculates the three-dimensional distance to another point*/
    double dist3D( Point3D* p ) const;
    /** Returns the x-coordinate of the point*/
    double getX() const;
    /** Returns the y-coordinate of the point*/
    double getY() const;
    /** Returns the z-coordinate of the point*/
    double getZ() const;
    /** Sets the x-coordinate of the point*/
    void setX( double x );
    /** Sets the y-coordinate of the point*/
    void setY( double y );
    /** Sets the z-coordinate of the point*/
    void setZ( double z );
};
