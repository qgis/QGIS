class Vector3D
{
%TypeHeaderCode
#include <Vector3D.h>
%End

  public:
    /** Constructor taking the three components as arguments*/
    Vector3D( double x, double y, double z );
    /** Default constructor*/
    Vector3D();
    /** Copy constructor*/
    Vector3D( const Vector3D& v );
    /** Destructor*/
    ~Vector3D();
    // Vector3D& operator=( const Vector3D& v );
    bool operator==( const Vector3D& v ) const;
    bool operator!=( const Vector3D& v ) const;
    /** Returns the x-component of the vector*/
    double getX() const;
    /** Returns the y-component of the vector*/
    double getY() const;
    /** Returns the z-component of the vector*/
    double getZ() const;
    /** Returns the length of the vector*/
    double getLength() const;
    /** Sets the x-component of the vector*/
    void setX( double x );
    /** Sets the y-component of the vector*/
    void setY( double y );
    /** Sets the z-component of the vector*/
    void setZ( double z );
    /** Standardises the vector*/
    void standardise();
};
