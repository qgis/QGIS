/***************************************************************************
                             qgsrange.h
                             ----------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRANGE_H
#define QGSRANGE_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

#include <QDate>
#include <QDateTime>

/**
 * \class QgsRange
 * \ingroup core
 * \brief A template based class for storing ranges (lower to upper values).
 *
 * QgsRange classes represent a range of values of some element type. For instance,
 * ranges of int might be used to represent integer ranges.
 *
 * Ranges can indicate whether the upper and lower values are inclusive or exclusive.
 * The inclusivity or exclusivity of bounds is considered when determining things like
 * whether ranges overlap or during calculation of range intersections.
 *
 * \see QgsDoubleRange
 * \see QgsIntRange
 * \note not available in Python bindings (but class provided for template-based inheritance)
 */
template <typename T>
class QgsRange
{
  public:

    /**
     * Constructor for QgsRange. The \a lower and \a upper bounds are specified,
     * and optionally whether or not these bounds are included in the range.
     */
    QgsRange( T lower, T upper, bool includeLower = true, bool includeUpper = true )
      : mLower( lower )
      , mUpper( upper )
      , mIncludeLower( includeLower )
      , mIncludeUpper( includeUpper )
    {}

    /**
     * Constructor for QgsRange. The \a lower and \a upper bounds are specified,
     * and whether or not these bounds are included in the range.
     *
     * \since QGIS 3.38
     */
    QgsRange( T lower, T upper, Qgis::RangeLimits limits )
      : mLower( lower )
      , mUpper( upper )
      , mIncludeLower( limits == Qgis::RangeLimits::IncludeLowerExcludeUpper || limits == Qgis::RangeLimits::IncludeBoth )
      , mIncludeUpper( limits == Qgis::RangeLimits::ExcludeLowerIncludeUpper || limits == Qgis::RangeLimits::IncludeBoth )
    {}

    /**
     * Returns the lower bound of the range.
     * \see upper()
     * \see includeLower()
     */
    T lower() const { return mLower; }

    /**
     * Returns the upper bound of the range.
     * \see lower()
     * \see includeUpper()
     */
    T upper() const { return mUpper; }

    /**
     * Returns TRUE if the lower bound is inclusive, or FALSE if the lower
     * bound is exclusive.
     * \see lower()
     * \see includeUpper()
     */
    bool includeLower() const { return mIncludeLower; }

    /**
     * Returns TRUE if the upper bound is inclusive, or FALSE if the upper
     * bound is exclusive.
     * \see upper()
     * \see includeLower()
     */
    bool includeUpper() const { return mIncludeUpper; }

    /**
     * Returns the limit handling of the range.
     *
     * \since QGIS 3.38
     */
    Qgis::RangeLimits rangeLimits() const
    {
      if ( mIncludeLower && mIncludeUpper )
        return Qgis::RangeLimits::IncludeBoth;
      else if ( mIncludeLower && !mIncludeUpper )
        return Qgis::RangeLimits::IncludeLowerExcludeUpper;
      else if ( !mIncludeLower && mIncludeUpper )
        return Qgis::RangeLimits::ExcludeLowerIncludeUpper;
      else
        return Qgis::RangeLimits::ExcludeBoth;
    }

    /**
     * Returns TRUE if the range is empty, ie the lower bound equals (or exceeds) the upper bound
     * and either the bounds are exclusive.
     * \see isSingleton()
     */
    bool isEmpty() const { return mLower > mUpper || ( mUpper == mLower && !( mIncludeLower || mIncludeUpper ) ); }

    /**
     * Returns TRUE if the range consists only of a single value or instant.
     * \see isEmpty()
     */
    bool isSingleton() const { return mLower == mUpper && ( mIncludeLower || mIncludeUpper ); }

    /**
     * Returns TRUE if this range contains another range.
     * \see overlaps()
     */
    bool contains( const QgsRange<T> &other ) const
    {
      const bool lowerOk = ( mIncludeLower && mLower <= other.mLower )
                           || ( !mIncludeLower && mLower < other.mLower )
                           || ( !mIncludeLower && !other.mIncludeLower && mLower <= other.mLower );
      if ( !lowerOk )
        return false;

      const bool upperOk = ( mIncludeUpper && mUpper >= other.mUpper )
                           || ( !mIncludeUpper && mUpper > other.mUpper )
                           || ( !mIncludeUpper && !other.mIncludeUpper && mUpper >= other.mUpper );
      if ( !upperOk )
        return false;

      return true;
    }

    /**
     * Returns TRUE if this range contains a specified \a element.
     */
    bool contains( T element ) const
    {
      const bool lowerOk = ( mIncludeLower && mLower <= element )
                           || ( !mIncludeLower && mLower < element );
      if ( !lowerOk )
        return false;

      const bool upperOk = ( mIncludeUpper && mUpper >= element )
                           || ( !mIncludeUpper && mUpper > element );
      if ( !upperOk )
        return false;

      return true;
    }

    /**
     * Returns TRUE if this range overlaps another range.
     * \see contains()
     */
    bool overlaps( const QgsRange<T> &other ) const
    {
      if ( ( ( mIncludeLower && mLower <= other.mLower ) || ( !mIncludeLower && mLower < other.mLower ) )
           && ( ( mIncludeUpper  && mUpper >= other.mUpper ) || ( !mIncludeUpper && mUpper > other.mUpper ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower <= other.mLower ) || ( !mIncludeLower && mLower < other.mLower ) )
           && ( ( mIncludeUpper  && mUpper >= other.mLower ) || ( !mIncludeUpper && mUpper > other.mLower ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower <= other.mUpper ) || ( !mIncludeLower && mLower < other.mUpper ) )
           && ( ( mIncludeUpper && mUpper >= other.mUpper ) || ( !mIncludeUpper && mUpper > other.mUpper ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower >= other.mLower ) || ( !mIncludeLower && mLower > other.mLower ) )
           && ( ( mIncludeLower && mLower <= other.mUpper ) || ( !mIncludeLower && mLower < other.mUpper ) ) )
        return true;

      if ( mLower == other.mLower && mUpper == other.mUpper )
        return true;

      return false;
    }

    bool operator==( const QgsRange<T> &other ) const
    {
      return mLower == other.mLower &&
             mUpper == other.mUpper &&
             mIncludeLower == other.includeLower() &&
             mIncludeUpper == other.includeUpper();
    }

    bool operator!=( const QgsRange<T> &other ) const
    {
      return ( ! operator==( other ) );
    }

  protected:

    T mLower;
    T mUpper;
    bool mIncludeLower = true;
    bool mIncludeUpper = true;

};


/**
 * \brief QgsRange which stores a range of double values.
 * \ingroup core
 * \see QgsIntRange
 * \see QgsDateRange
 * \see QgsDateTimeRange
 */
class CORE_EXPORT QgsDoubleRange : public QgsRange< double >
{
  public:

    /**
     * Constructor for QgsDoubleRange. The \a lower and \a upper bounds are specified,
     * and whether or not these bounds are included in the range.
     *
     * \since QGIS 3.38
     */
    QgsDoubleRange( double lower, double upper, Qgis::RangeLimits limits )
      : QgsRange( lower, upper, limits )
    {}

#ifndef SIP_RUN

    /**
     * Constructor for QgsDoubleRange. The \a lower and \a upper bounds are specified,
     * and optionally whether or not these bounds are included in the range.
     *
     * The default values for \a lower and \a upper construct an infinite range (see isInfinite()).
     *
     * \since QGIS 3.18
     */
    QgsDoubleRange( double lower = std::numeric_limits< double >::lowest(),
                    double upper = std::numeric_limits< double >::max(),
                    bool includeLower = true, bool includeUpper = true )
      : QgsRange( lower, upper, includeLower, includeUpper )
    {}
#else

    /**
     * Constructor for QgsDoubleRange. The \a lower and \a upper bounds are specified,
     * and optionally whether or not these bounds are included in the range.
     */
    QgsDoubleRange( double lower,
                    double upper,
                    bool includeLower = true, bool includeUpper = true )
      : QgsRange( lower, upper, includeLower, includeUpper )
    {}

    /**
     * Constructor for QgsDoubleRange containing an infinite range (see isInfinite()).
     *
     * \since QGIS 3.18
     */
    QgsDoubleRange()
      : QgsRange( std::numeric_limits< double >::lowest(), std::numeric_limits< double >::max(), true, true )
    {}
#endif

    /**
     * Returns TRUE if the range consists of all possible values.
     * \since QGIS 3.18
     */
    bool isInfinite() const
    {
      return lower() == std::numeric_limits< double >::lowest() && upper() == std::numeric_limits< double >::max();
    }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsDoubleRange: %1%2, %3%4>" ).arg( sipCpp->includeLower() ? QStringLiteral( "[" ) : QStringLiteral( "(" ) )
                  .arg( sipCpp->lower() )
                  .arg( sipCpp->upper() )
                  .arg( sipCpp->includeUpper() ? QStringLiteral( "]" ) : QStringLiteral( ")" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    bool operator==( const QgsDoubleRange &other ) const
    {
      return qgsDoubleNear( mLower, other.mLower ) &&
             qgsDoubleNear( mUpper, other.mUpper ) &&
             mIncludeLower == other.includeLower() &&
             mIncludeUpper == other.includeUpper();
    }

    bool operator!=( const QgsDoubleRange &other ) const
    {
      return ( ! operator==( other ) );
    }

};

Q_DECLARE_METATYPE( QgsDoubleRange )


/**
 * \brief QgsRange which stores a range of integer values.
 * \ingroup core
 * \see QgsDoubleRange
 * \see QgsDateRange
 * \see QgsDateTimeRange
 */
class CORE_EXPORT QgsIntRange : public QgsRange< int >
{
  public:

    /**
     * Constructor for QgsIntRange. The \a lower and \a upper bounds are specified,
     * and whether or not these bounds are included in the range.
     *
     * \since QGIS 3.38
     */
    QgsIntRange( int lower, int upper, Qgis::RangeLimits limits )
      : QgsRange( lower, upper, limits )
    {}

#ifndef SIP_RUN

    /**
     * Constructor for QgsIntRange. The \a lower and \a upper bounds are specified,
     * and optionally whether or not these bounds are included in the range.
     *
     * The default values for \a lower and \a upper construct an infinite range (see isInfinite()).
     *
     * \since QGIS 3.18
     */
    QgsIntRange( int lower = std::numeric_limits< int >::lowest(),
                 int upper = std::numeric_limits< int >::max(),
                 bool includeLower = true, bool includeUpper = true )
      : QgsRange( lower, upper, includeLower, includeUpper )
    {}
#else

    /**
     * Constructor for QgsIntRange. The \a lower and \a upper bounds are specified,
     * and optionally whether or not these bounds are included in the range.
     */
    QgsIntRange( int lower,
                 int upper,
                 bool includeLower = true, bool includeUpper = true )
      : QgsRange( lower, upper, includeLower, includeUpper )
    {}

    /**
     * Constructor for QgsIntRange containing an infinite range (see isInfinite()).
     *
     * \since QGIS 3.18
     */
    QgsIntRange()
      : QgsRange( std::numeric_limits< int >::lowest(), std::numeric_limits< int >::max(), true, true )
    {}
#endif

    /**
     * Returns TRUE if the range consists of all possible values.
     * \since QGIS 3.18
     */
    bool isInfinite() const
    {
      return lower() == std::numeric_limits< int >::lowest() && upper() == std::numeric_limits< int >::max();
    }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsIntRange: %1%2, %3%4>" ).arg( sipCpp->includeLower() ? QStringLiteral( "[" ) : QStringLiteral( "(" ) )
                  .arg( sipCpp->lower() )
                  .arg( sipCpp->upper() )
                  .arg( sipCpp->includeUpper() ? QStringLiteral( "]" ) : QStringLiteral( ")" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

Q_DECLARE_METATYPE( QgsIntRange )


/**
 * \class QgsTemporalRange
 * \ingroup core
 * \brief A template based class for storing temporal ranges (beginning to end values).
 *
 * QgsTemporalRange classes represent a range of values of some temporal type. For instance,
 * ranges of QDateTime might be used to represent datetime ranges.
 *
 * Ranges can indicate whether the upper and lower values are inclusive or exclusive.
 * The inclusivity or exclusivity of bounds is considered when determining things like
 * whether ranges overlap or during calculation of range intersections.
 *
 * \see QgsDateRange
 * \note not available in Python bindings (but class provided for template-based inheritance)
 */
template <typename T>
class QgsTemporalRange
{
  public:

    /**
     * Constructor for QgsTemporalRange. The \a begin and \a end are specified,
     * and optionally whether or not these bounds are included in the range.
     * \note in Python \a begin and \a end must be provided.
     */
#ifndef SIP_RUN
    QgsTemporalRange( const T &begin = T(), const T &end = T(), bool includeBeginning = true, bool includeEnd = true )
      : mLower( begin )
      , mUpper( end )
      , mIncludeLower( includeBeginning )
      , mIncludeUpper( includeEnd )
    {}
#else
    QgsTemporalRange( const T &begin, const T &end, bool includeBeginning = true, bool includeEnd = true );
    // default constructor as default value for templates is not handled in SIP
#endif

    /**
     * Returns the beginning of the range.
     * \see end()
     * \see includeBeginning()
     */
    T begin() const { return mLower; }

    /**
     * Returns the upper bound of the range.
     * \see begin()
     * \see includeEnd()
     */
    T end() const { return mUpper; }

    /**
     * Returns TRUE if the beginning is inclusive, or FALSE if the beginning
     * is exclusive.
     * \see begin()
     * \see includeEnd()
     */
    bool includeBeginning() const { return mIncludeLower; }

    /**
     * Returns TRUE if the end is inclusive, or FALSE if the end is exclusive.
     * \see end()
     * \see includeBeginning()
     */
    bool includeEnd() const { return mIncludeUpper; }

    /**
     * Returns TRUE if the range consists only of a single instant.
     * \see isEmpty()
     * \see isInfinite()
     */
    bool isInstant() const { return mLower.isValid() && mUpper.isValid() && mLower == mUpper && ( mIncludeLower || mIncludeUpper ); }

    /**
     * Returns TRUE if the range consists of all possible values.
     * \see isEmpty()
     * \see isInstant()
     */
    bool isInfinite() const
    {
      return !mLower.isValid() && !mUpper.isValid();
    }

    /**
     * Returns TRUE if the range is empty, ie the beginning equals (or exceeds) the end
     * and either of the bounds are exclusive.
     * A range with both invalid beginning and end is considered infinite and not empty.
     */
    bool isEmpty() const
    {
      if ( !mLower.isValid() && !mUpper.isValid() )
        return false;

      if ( mLower.isValid() != mUpper.isValid() )
        return false;

      if ( mLower > mUpper )
        return true;

      if ( mLower == mUpper && !( mIncludeLower || mIncludeUpper ) )
        return true;

      return false;
    }

    /**
     * Returns TRUE if this range contains another range.
     */
    bool contains( const QgsTemporalRange<T> &other ) const
    {
      if ( !other.mLower.isValid() && mLower.isValid() )
        return false;

      if ( mLower.isValid() )
      {
        const bool lowerOk = ( mIncludeLower && mLower <= other.mLower )
                             || ( !mIncludeLower && mLower < other.mLower )
                             || ( !mIncludeLower && !other.mIncludeLower && mLower <= other.mLower );
        if ( !lowerOk )
          return false;
      }

      if ( !other.mUpper.isValid() && mUpper.isValid() )
        return false;

      if ( mUpper.isValid() )
      {
        const bool upperOk = ( mIncludeUpper && mUpper >= other.mUpper )
                             || ( !mIncludeUpper && mUpper > other.mUpper )
                             || ( !mIncludeUpper && !other.mIncludeUpper && mUpper >= other.mUpper );
        if ( !upperOk )
          return false;
      }

      return true;
    }

    /**
     * Returns TRUE if this range contains a specified \a element.
     */
    bool contains( const T &element ) const
    {
      if ( !element.isValid() )
        return false;

      if ( mLower.isValid() )
      {
        const bool lowerOk = ( mIncludeLower && mLower <= element )
                             || ( !mIncludeLower && mLower < element );
        if ( !lowerOk )
          return false;
      }

      if ( mUpper.isValid() )
      {
        const bool upperOk = ( mIncludeUpper && mUpper >= element )
                             || ( !mIncludeUpper && mUpper > element );
        if ( !upperOk )
          return false;
      }

      return true;
    }

    /**
     * Returns TRUE if this range overlaps another range.
     */
    bool overlaps( const QgsTemporalRange<T> &other ) const
    {
      if ( !mUpper.isValid() && ( ( mIncludeLower && mLower <= other.mUpper ) || ( !mIncludeLower && mLower < other.mUpper ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower <= other.mLower ) || ( !mIncludeLower && mLower < other.mLower ) )
           && ( ( mIncludeUpper  && mUpper >= other.mUpper ) || ( !mIncludeUpper && mUpper > other.mUpper ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower <= other.mLower ) || ( !mIncludeLower && mLower < other.mLower ) )
           && ( ( mIncludeUpper  && mUpper >= other.mLower ) || ( !mIncludeUpper && mUpper > other.mLower ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower <= other.mUpper ) || ( !mIncludeLower && mLower < other.mUpper ) )
           && ( ( mIncludeUpper && mUpper >= other.mUpper ) || ( !mIncludeUpper && mUpper > other.mUpper ) ) )
        return true;

      if ( ( ( mIncludeLower && mLower >= other.mLower ) || ( !mIncludeLower && mLower > other.mLower ) )
           && ( ( mIncludeLower && mLower <= other.mUpper ) || ( !mIncludeLower && mLower < other.mUpper ) ) )
        return true;

      if ( mLower == other.mLower && mUpper == other.mUpper )
        return true;

      return false;
    }

    /**
     * Extends the range in place by extending this range out to include an \a other range.
     * If \a other is empty the range is not changed.
     * If the range is empty and \a other is not, the range is changed and set to \a other.
     * \see isEmpty()
     * \returns TRUE if the range was extended
     * \since QGIS 3.12
     */
    bool extend( const QgsTemporalRange<T> &other )
    {
      if ( other.isEmpty() )
      {
        return false;
      }
      else if ( isEmpty() )
      {
        mLower = other.begin();
        mUpper = other.end();
        mIncludeLower = other.includeBeginning();
        mIncludeUpper = other.includeEnd();
        return true;
      }

      // Both not empty, do some math
      bool changed { false };

      // Lower
      if ( ! other.begin().isValid()
           || ( begin().isValid() && other.begin() < mLower ) )
      {
        mLower = other.begin();
        mIncludeLower = other.includeBeginning();
        changed = true;
      }
      else if ( other.begin() == mLower && other.includeBeginning() && ! mIncludeLower )
      {
        mIncludeLower = true;
        changed = true;
      }

      // Upper
      if ( ! other.end().isValid()
           || ( end().isValid() && other.end() > mUpper ) )
      {
        mUpper = other.end();
        mIncludeUpper = other.includeEnd();
        changed = true;
      }
      else if ( other.end() == mUpper && other.includeEnd() && ! mIncludeUpper )
      {
        mIncludeUpper = true;
        changed = true;
      }
      return changed;
    }

#ifndef SIP_RUN

    /**
     * Merges a list of temporal ranges.
     *
     * Any overlapping ranges will be converted to a single range which covers the entire
     * range of the input ranges.
     *
     * The returned value will be a list of non-contiguous ranges which completely encompass
     * the input \a ranges, sorted in ascending order.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.20
     */
    static QList< QgsTemporalRange<T> > mergeRanges( const QList< QgsTemporalRange<T> > &ranges )
    {
      if ( ranges.empty() )
        return {};

      QList< QgsTemporalRange<T > > sortedRanges = ranges;
      // cppcheck-suppress mismatchingContainerExpression
      std::sort( sortedRanges.begin(), sortedRanges.end(), []( const QgsTemporalRange< T > &a, const QgsTemporalRange< T > &b ) -> bool { return a.begin() < b.begin(); } );
      QList< QgsTemporalRange<T>> res;
      res.reserve( sortedRanges.size() );

      QgsTemporalRange<T> prevRange;
      auto it = sortedRanges.constBegin();
      prevRange = *it++;
      for ( ; it != sortedRanges.constEnd(); ++it )
      {
        if ( prevRange.overlaps( *it ) )
        {
          prevRange.extend( *it );
        }
        else
        {
          res << prevRange;
          prevRange = *it;
        }
      }
      res << prevRange;
      return res;
    }
#endif

    bool operator==( const QgsTemporalRange<T> &other ) const
    {
      return mLower == other.mLower &&
             mUpper == other.mUpper &&
             mIncludeLower == other.includeBeginning() &&
             mIncludeUpper == other.includeEnd();
    }

    bool operator!=( const QgsTemporalRange<T> &other ) const
    {
      return ( ! operator==( other ) );
    }

  private:

    T mLower;
    T mUpper;
    bool mIncludeLower = true;
    bool mIncludeUpper = true;
};


/**
 * QgsRange which stores a range of dates.
 *
 * Invalid QDates as the beginning or end are permitted. In this case,
 * the bound is considered to be infinite. E.g. QgsDateRange(QDate(),QDate(2017,1,1))
 * is treated as a range containing all dates before 2017-1-1.
 * QgsDateRange(QDate(2017,1,1),QDate()) is treated as a range containing all dates after 2017-1-1.
 * \see QgsDateTimeRange
 */
typedef QgsTemporalRange< QDate > QgsDateRange SIP_DOC_TEMPLATE;

Q_DECLARE_METATYPE( QgsDateRange )

/**
 * QgsRange which stores a range of date times.
 *
 * Invalid QDateTimes as the beginning or end are permitted. In this case,
 * the bound is considered to be infinite. E.g. QgsDateTimeRange(QDateTime(),QDateTime(2017,1,1))
 * is treated as a range containing all dates before 2017-1-1.
 * QgsDateTimeRange(QDateTime(2017,1,1),QDateTime()) is treated as a range containing all dates after 2017-1-1.
 * \see QgsDateRange
 */
typedef QgsTemporalRange< QDateTime > QgsDateTimeRange SIP_DOC_TEMPLATE;

Q_DECLARE_METATYPE( QgsDateTimeRange )

#endif // QGSRANGE_H
