class QgsMargins
{
%TypeHeaderCode
#include <qgsmargins.h>
%End

  public:

    QgsMargins();
    QgsMargins( double left, double top, double right, double bottom );

    bool isNull() const;

    double left() const;
    double top() const;
    double right() const;
    double bottom() const;

    void setLeft( double left );
    void setTop( double top );
    void setRight( double right );
    void setBottom( double bottom );

    QgsMargins &operator+=( const QgsMargins &margins );
    QgsMargins &operator-=( const QgsMargins &margins );
    QgsMargins &operator+=( double addend );
    QgsMargins &operator-=( double subtrahend );
    QgsMargins &operator*=( double factor );
    QgsMargins &operator/=( double divisor );

    QString toString() const;
    static QgsMargins fromString( const QString& string );
};

bool operator==( const QgsMargins &lhs, const QgsMargins &rhs );
bool operator!=( const QgsMargins &lhs, const QgsMargins &rhs );
QgsMargins operator+( const QgsMargins &m1, const QgsMargins &m2 );
QgsMargins operator-( const QgsMargins &m1, const QgsMargins &m2 );
QgsMargins operator+( const QgsMargins &lhs, double rhs );
QgsMargins operator+( double lhs, const QgsMargins &rhs );
QgsMargins operator-( const QgsMargins &lhs, double rhs );
QgsMargins operator*( const QgsMargins &margins, double factor );
QgsMargins operator*( double factor, const QgsMargins &margins );
QgsMargins operator/( const QgsMargins &margins, double divisor );
QgsMargins operator+( const QgsMargins &margins );
QgsMargins operator-( const QgsMargins &margins );

