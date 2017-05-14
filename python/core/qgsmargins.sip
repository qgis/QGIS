class QgsMargins
{

%TypeHeaderCode
#include "qgsmargins.h"
%End
  public:

    QgsMargins();
%Docstring
 Constructs a margins object with all margins set to 0.
%End

    QgsMargins( double left, double top, double right, double bottom );
%Docstring
 Constructs margins with the given ``left``, ``top``, ``right``, ``bottom``
.. seealso:: setLeft()
.. seealso:: setRight()
.. seealso:: setTop()
.. seealso:: setBottom()
%End

    bool isNull() const;
%Docstring
 Returns \c true if all margins are is 0; otherwise returns false.
 :rtype: bool
%End

    double left() const;
%Docstring
 Returns the left margin.
.. seealso:: setLeft()
 :rtype: float
%End

    double top() const;
%Docstring
 Returns the top margin.
.. seealso:: setTop()
 :rtype: float
%End

    double right() const;
%Docstring
 Returns the right margin.
.. seealso:: setRight()
 :rtype: float
%End

    double bottom() const;
%Docstring
 Returns the bottom margin.
.. seealso:: setBottom()
 :rtype: float
%End

    void setLeft( double left );
%Docstring
 Sets the left margin to ``left``.
.. seealso:: left()
%End

    void setTop( double top );
%Docstring
 Sets the top margin to ``top``.
.. seealso:: top()
%End

    void setRight( double right );
%Docstring
 Sets the right margin to ``right``.
.. seealso:: right()
%End

    void setBottom( double bottom );
%Docstring
 Sets the bottom margin to ``bottom``.
.. seealso:: bottom()
%End

    QgsMargins &operator+=( const QgsMargins &margins );
%Docstring
 Add each component of ``margins`` to the respective component of this object
 and returns a reference to it.
 :rtype: QgsMargins
%End

    QgsMargins &operator-=( const QgsMargins &margins );
%Docstring
 Subtract each component of ``margins`` from the respective component of this object
 and returns a reference to it.
 :rtype: QgsMargins
%End

    QgsMargins &operator+=( double addend );
%Docstring
 Adds the ``addend`` to each component of this object and returns a reference to it.
 :rtype: QgsMargins
%End

    QgsMargins &operator-=( double subtrahend );
%Docstring
 Subtracts the ``subtrahend`` from each component of this object
 and returns a reference to it.
 :rtype: QgsMargins
%End

    QgsMargins &operator*=( double factor );
%Docstring
 Multiplies each component of this object by ``factor``
 and returns a reference to it.
 :rtype: QgsMargins
%End

    QgsMargins &operator/=( double divisor );
%Docstring
 Multiplies each component of this object by ``factor``
 and returns a reference to it.
 :rtype: QgsMargins
%End

    QString toString() const;
%Docstring
 Returns the margins encoded to a string.
.. seealso:: fromString()
 :rtype: str
%End

    static QgsMargins fromString( const QString &string );
%Docstring
 Returns a QgsMargins object decoded from a string, or a null QgsMargins
 if the string could not be interpreted as margins.
.. seealso:: toString()
 :rtype: QgsMargins
%End

};


bool operator==( const QgsMargins &lhs, const QgsMargins &rhs );
%Docstring
 Returns \c true if ``lhs`` and ``rhs`` are equal; otherwise returns \c false.
 :rtype: bool
%End

bool operator!=( const QgsMargins &lhs, const QgsMargins &rhs );
%Docstring
 Returns \c true if ``lhs`` and ``rhs`` are different; otherwise returns \c false.
 :rtype: bool
%End

QgsMargins operator+( const QgsMargins &m1, const QgsMargins &m2 );
%Docstring
 Returns a QgsMargins object that is the sum of the given margins, ``m1``
 and ``m2``; each component is added separately.
 :rtype: QgsMargins
%End

QgsMargins operator-( const QgsMargins &m1, const QgsMargins &m2 );
%Docstring
 Returns a QgsMargins object that is formed by subtracting ``m2`` from
 ``m1``; each component is subtracted separately.
 :rtype: QgsMargins
%End

QgsMargins operator+( const QgsMargins &lhs, double rhs );
%Docstring
 Returns a QgsMargins object that is formed by adding ``rhs`` to ``lhs``.
 :rtype: QgsMargins
%End

QgsMargins operator+( double lhs, const QgsMargins &rhs );
%Docstring
 Returns a QgsMargins object that is formed by adding ``lhs`` to ``rhs``.
 :rtype: QgsMargins
%End

QgsMargins operator-( const QgsMargins &lhs, double rhs );
%Docstring
 Returns a QgsMargins object that is formed by subtracting ``rhs`` from ``lhs``.
 :rtype: QgsMargins
%End

QgsMargins operator*( const QgsMargins &margins, double factor );
%Docstring
 Returns a QgsMargins object that is formed by multiplying each component
 of the given ``margins`` by ``factor``.
 :rtype: QgsMargins
%End

QgsMargins operator*( double factor, const QgsMargins &margins );
%Docstring
 Returns a QgsMargins object that is formed by multiplying each component
 of the given ``margins`` by ``factor``.
 :rtype: QgsMargins
%End

QgsMargins operator/( const QgsMargins &margins, double divisor );
%Docstring
 Returns a QgsMargins object that is formed by dividing the components of
 the given ``margins`` by the given ``divisor``.
 :rtype: QgsMargins
%End

QgsMargins &QgsMargins::operator+=( const QgsMargins &margins );

QgsMargins &QgsMargins::operator-=( const QgsMargins &margins );

QgsMargins &QgsMargins::operator+=( double addend );

QgsMargins &QgsMargins::operator-=( double subtrahend );

QgsMargins &QgsMargins::operator*=( double factor );

QgsMargins &QgsMargins::operator/=( double divisor );

QgsMargins operator+( const QgsMargins &margins );
%Docstring
 Returns a QgsMargins object that is formed from all components of ``margins``.
 :rtype: QgsMargins
%End

QgsMargins operator-( const QgsMargins &margins );
%Docstring
 Returns a QgsMargins object that is formed by negating all components of ``margins``.
 :rtype: QgsMargins
%End
