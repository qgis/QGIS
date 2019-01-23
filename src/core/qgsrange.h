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

/**
 * \class QgsRange
 * \ingroup core
 * A template based class for storing ranges (lower to upper values).
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
 * \since QGIS 3.0
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
     * Returns true if the lower bound is inclusive, or false if the lower
     * bound is exclusive.
     * \see lower()
     * \see includeUpper()
     */
    bool includeLower() const { return mIncludeLower; }

    /**
     * Returns true if the upper bound is inclusive, or false if the upper
     * bound is exclusive.
     * \see upper()
     * \see includeLower()
     */
    bool includeUpper() const { return mIncludeUpper; }

    /**
     * Returns true if the range is empty, ie the lower bound equals (or exceeds) the upper bound
     * and either the bounds are exclusive.
     * \see isSingleton()
     */
    bool isEmpty() const { return mLower > mUpper || ( mUpper == mLower && !( mIncludeLower || mIncludeUpper ) ); }

    /**
     * Returns true if the range consists only of a single value or instant.
     * \see isEmpty()
     */
    bool isSingleton() const { return mLower == mUpper && ( mIncludeLower || mIncludeUpper ); }

    /**
     * Returns true if this range contains another range.
     * \see overlaps()
     */
    bool contains( const QgsRange<T> &other ) const
    {
      bool lowerOk = ( mIncludeLower && mLower <= other.mLower )
                     || ( !mIncludeLower && mLower < other.mLower )
                     || ( !mIncludeLower && !other.mIncludeLower && mLower <= other.mLower );
      if ( !lowerOk )
        return false;

      bool upperOk = ( mIncludeUpper && mUpper >= other.mUpper )
                     || ( !mIncludeUpper && mUpper > other.mUpper )
                     || ( !mIncludeUpper && !other.mIncludeUpper && mUpper >= other.mUpper );
      if ( !upperOk )
        return false;

      return true;
    }

    /**
     * Returns true if this range contains a specified \a element.
     */
    bool contains( T element ) const
    {
      bool lowerOk = ( mIncludeLower && mLower <= element )
                     || ( !mIncludeLower && mLower < element );
      if ( !lowerOk )
        return false;

      bool upperOk = ( mIncludeUpper && mUpper >= element )
                     || ( !mIncludeUpper && mUpper > element );
      if ( !upperOk )
        return false;

      return true;
    }

    /**
     * Returns true if this range overlaps another range.
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


  private:

    T mLower;
    T mUpper;
    bool mIncludeLower = true;
    bool mIncludeUpper = true;

};


/**
 * QgsRange which stores a range of double values.
 * \see QgsIntRange
 * \see QgsDateRange
 * \see QgsDateTimeRange
 * \since QGIS 3.0
 */
typedef QgsRange< double > QgsDoubleRange;




/**
 * QgsRange which stores a range of integer values.
 * \see QgsDoubleRange
 * \see QgsDateRange
 * \see QgsDateTimeRange
 * \since QGIS 3.0
 */
typedef QgsRange< int > QgsIntRange;


/**
 * \class QgsTemporalRange
 * \ingroup core
 * A template based class for storing temporal ranges (beginning to end values).
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
 * \since QGIS 3.0
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
     * Returns true if the beginning is inclusive, or false if the beginning
     * is exclusive.
     * \see begin()
     * \see includeEnd()
     */
    bool includeBeginning() const { return mIncludeLower; }

    /**
     * Returns true if the end is inclusive, or false if the end is exclusive.
     * \see end()
     * \see includeBeginning()
     */
    bool includeEnd() const { return mIncludeUpper; }

    /**
     * Returns true if the range consists only of a single instant.
     * \see isEmpty()
     * \see isInfinite()
     */
    bool isInstant() const { return mLower.isValid() && mUpper.isValid() && mLower == mUpper && ( mIncludeLower || mIncludeUpper ); }

    /**
     * Returns true if the range consists of all possible values.
     * \see isEmpty()
     * \see isInstant()
     */
    bool isInfinite() const
    {
      return !mLower.isValid() && !mUpper.isValid();
    }

    /**
     * Returns true if the range is empty, ie the beginning equals (or exceeds) the end
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
     * Returns true if this range contains another range.
     */
    bool contains( const QgsTemporalRange<T> &other ) const
    {
      if ( !other.mLower.isValid() && mLower.isValid() )
        return false;

      if ( mLower.isValid() )
      {
        bool lowerOk = ( mIncludeLower && mLower <= other.mLower )
                       || ( !mIncludeLower && mLower < other.mLower )
                       || ( !mIncludeLower && !other.mIncludeLower && mLower <= other.mLower );
        if ( !lowerOk )
          return false;
      }

      if ( !other.mUpper.isValid() && mUpper.isValid() )
        return false;

      if ( mUpper.isValid() )
      {
        bool upperOk = ( mIncludeUpper && mUpper >= other.mUpper )
                       || ( !mIncludeUpper && mUpper > other.mUpper )
                       || ( !mIncludeUpper && !other.mIncludeUpper && mUpper >= other.mUpper );
        if ( !upperOk )
          return false;
      }

      return true;
    }

    /**
     * Returns true if this range contains a specified \a element.
     */
    bool contains( const T &element ) const
    {
      if ( !element.isValid() )
        return false;

      if ( mLower.isValid() )
      {
        bool lowerOk = ( mIncludeLower && mLower <= element )
                       || ( !mIncludeLower && mLower < element );
        if ( !lowerOk )
          return false;
      }

      if ( mUpper.isValid() )
      {
        bool upperOk = ( mIncludeUpper && mUpper >= element )
                       || ( !mIncludeUpper && mUpper > element );
        if ( !upperOk )
          return false;
      }

      return true;
    }

    /**
     * Returns true if this range overlaps another range.
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

    bool operator==( const QgsTemporalRange<T> &other ) const
    {
      return mLower == other.mLower &&
             mUpper == other.mUpper &&
             mIncludeLower == other.mIncludeLower &&
             mIncludeUpper == other.mIncludeUpper;
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
 * \since QGIS 3.0
 */
typedef QgsTemporalRange< QDate > QgsDateRange SIP_DOC_TEMPLATE;

/**
 * QgsRange which stores a range of date times.
 *
 * Invalid QDateTimes as the beginning or end are permitted. In this case,
 * the bound is considered to be infinite. E.g. QgsDateTimeRange(QDateTime(),QDateTime(2017,1,1))
 * is treated as a range containing all dates before 2017-1-1.
 * QgsDateTimeRange(QDateTime(2017,1,1),QDateTime()) is treated as a range containing all dates after 2017-1-1.
 * \see QgsDateRange
 * \since QGIS 3.0
 */
typedef QgsTemporalRange< QDateTime > QgsDateTimeRange SIP_DOC_TEMPLATE;

#endif // QGSRANGE_H
