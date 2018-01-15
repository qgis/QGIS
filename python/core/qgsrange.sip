/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrange.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




template <T>
class QgsRange
{
%Docstring
A template based class for storing ranges (lower to upper values).

QgsRange classes represent a range of values of some element type. For instance,
ranges of int might be used to represent integer ranges.

Ranges can indicate whether the upper and lower values are inclusive or exclusive.
The inclusivity or exclusivity of bounds is considered when determining things like
whether ranges overlap or during calculation of range intersections.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsDoubleRange`

.. seealso:: :py:class:`QgsIntRange`

.. note::

   not available in Python bindings (but class provided for template-based inheritance)
%End

%TypeHeaderCode
#include "qgsrange.h"
%End
  public:

    QgsRange( T lower, T upper, bool includeLower = true, bool includeUpper = true );
%Docstring
Constructor for QgsRange. The ``lower`` and ``upper`` bounds are specified,
and optionally whether or not these bounds are included in the range.
%End

    T lower() const;
%Docstring
Returns the lower bound of the range.

.. seealso:: :py:func:`upper`

.. seealso:: :py:func:`includeLower`
%End

    T upper() const;
%Docstring
Returns the upper bound of the range.

.. seealso:: :py:func:`lower`

.. seealso:: :py:func:`includeUpper`
%End

    bool includeLower() const;
%Docstring
Returns true if the lower bound is inclusive, or false if the lower
bound is exclusive.

.. seealso:: :py:func:`lower`

.. seealso:: :py:func:`includeUpper`
%End

    bool includeUpper() const;
%Docstring
Returns true if the upper bound is inclusive, or false if the upper
bound is exclusive.

.. seealso:: :py:func:`upper`

.. seealso:: :py:func:`includeLower`
%End

    bool isEmpty() const;
%Docstring
Returns true if the range is empty, ie the lower bound equals (or exceeds) the upper bound
and either the bounds are exclusive.

.. seealso:: :py:func:`isSingleton`
%End

    bool isSingleton() const;
%Docstring
Returns true if the range consists only of a single value or instant.

.. seealso:: :py:func:`isEmpty`
%End

    bool contains( const QgsRange<T> &other ) const;
%Docstring
Returns true if this range contains another range.

.. seealso:: :py:func:`overlaps`
%End

    bool contains( T element ) const;
%Docstring
Returns true if this range contains a specified ``element``.
%End

    bool overlaps( const QgsRange<T> &other ) const;
%Docstring
Returns true if this range overlaps another range.

.. seealso:: :py:func:`contains`
%End


};


typedef QgsRange< double > QgsDoubleRange;




typedef QgsRange< int > QgsIntRange;


template <T>
class QgsTemporalRange
{
%Docstring
A template based class for storing temporal ranges (beginning to end values).

QgsTemporalRange classes represent a range of values of some temporal type. For instance,
ranges of QDateTime might be used to represent datetime ranges.

Ranges can indicate whether the upper and lower values are inclusive or exclusive.
The inclusivity or exclusivity of bounds is considered when determining things like
whether ranges overlap or during calculation of range intersections.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsDateRange`

.. note::

   not available in Python bindings (but class provided for template-based inheritance)
%End

%TypeHeaderCode
#include "qgsrange.h"
%End
  public:

    QgsTemporalRange( const T &begin, const T &end, bool includeBeginning = true, bool includeEnd = true );
%Docstring
Constructor for QgsTemporalRange. The ``begin`` and ``end`` are specified,
and optionally whether or not these bounds are included in the range.

.. note::

   in Python ``begin`` and ``end`` must be provided.
%End
    // default constructor as default value for templates is not handled in SIP

    T begin() const;
%Docstring
Returns the beginning of the range.

.. seealso:: :py:func:`end`

.. seealso:: :py:func:`includeBeginning`
%End

    T end() const;
%Docstring
Returns the upper bound of the range.

.. seealso:: :py:func:`begin`

.. seealso:: :py:func:`includeEnd`
%End

    bool includeBeginning() const;
%Docstring
Returns true if the beginning is inclusive, or false if the beginning
is exclusive.

.. seealso:: :py:func:`begin`

.. seealso:: :py:func:`includeEnd`
%End

    bool includeEnd() const;
%Docstring
Returns true if the end is inclusive, or false if the end is exclusive.

.. seealso:: :py:func:`end`

.. seealso:: :py:func:`includeBeginning`
%End

    bool isInstant() const;
%Docstring
Returns true if the range consists only of a single instant.

.. seealso:: :py:func:`isEmpty`

.. seealso:: :py:func:`isInfinite`
%End

    bool isInfinite() const;
%Docstring
Returns true if the range consists of all possible values.

.. seealso:: :py:func:`isEmpty`

.. seealso:: :py:func:`isInstant`
%End

    bool isEmpty() const;
%Docstring
Returns true if the range is empty, ie the beginning equals (or exceeds) the end
and either of the bounds are exclusive.
A range with both invalid beginning and end is considered infinite and not empty.
%End

    bool contains( const QgsTemporalRange<T> &other ) const;
%Docstring
Returns true if this range contains another range.
%End

    bool contains( const T &element ) const;
%Docstring
Returns true if this range contains a specified ``element``.
%End

    bool overlaps( const QgsTemporalRange<T> &other ) const;
%Docstring
Returns true if this range overlaps another range.
%End

};


typedef QgsTemporalRange< QDate > QgsDateRange;

typedef QgsTemporalRange< QDateTime > QgsDateTimeRange;

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrange.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
