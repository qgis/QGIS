class QgsVector
{
%TypeHeaderCode
#include <qgsvector.h>
%End

  public:
    QgsVector();

    QgsVector( double x, double y );

    QgsVector operator-() const;

    QgsVector operator*( double scalar ) const;

    QgsVector operator/( double scalar ) const;

    double operator*( QgsVector v ) const;

    QgsVector operator+( QgsVector other ) const;

    QgsVector& operator+=( QgsVector other );

    QgsVector operator-( QgsVector other ) const;

    QgsVector& operator-=( QgsVector other );

    double length() const;

    double x() const;

    double y() const;

    QgsVector perpVector() const;

    double angle() const;

    double angle( QgsVector v ) const;

    QgsVector rotateBy( double rot ) const;

    QgsVector normalized() const;

    bool operator==( QgsVector other ) const;

    bool operator!=( QgsVector other ) const;
};
