/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmargins.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





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

.. seealso:: :py:func:`setLeft()`

.. seealso:: :py:func:`setRight()`

.. seealso:: :py:func:`setTop()`

.. seealso:: :py:func:`setBottom()`
%End

    bool isNull() const;
%Docstring
Returns \c true if all margins are is 0; otherwise returns false.

:rtype: bool
%End

    double left() const;
%Docstring
Returns the left margin.

:rtype: float

.. seealso:: :py:func:`setLeft()`
%End

    double top() const;
%Docstring
Returns the top margin.

:rtype: float

.. seealso:: :py:func:`setTop()`
%End

    double right() const;
%Docstring
Returns the right margin.

:rtype: float

.. seealso:: :py:func:`setRight()`
%End

    double bottom() const;
%Docstring
Returns the bottom margin.

:rtype: float

.. seealso:: :py:func:`setBottom()`
%End

    void setLeft( double left );
%Docstring
Sets the left margin to ``left``.

.. seealso:: :py:func:`left()`
%End

    void setTop( double top );
%Docstring
Sets the top margin to ``top``.

.. seealso:: :py:func:`top()`
%End

    void setRight( double right );
%Docstring
Sets the right margin to ``right``.

.. seealso:: :py:func:`right()`
%End

    void setBottom( double bottom );
%Docstring
Sets the bottom margin to ``bottom``.

.. seealso:: :py:func:`bottom()`
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

:rtype: str

.. seealso:: :py:func:`fromString()`
%End

    static QgsMargins fromString( const QString &string );
%Docstring
Returns a QgsMargins object decoded from a string, or a null QgsMargins
if the string could not be interpreted as margins.

:rtype: QgsMargins

.. seealso:: :py:func:`toString()`
%End

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


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmargins.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
