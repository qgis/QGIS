/***************************************************************************
                         qgsprofilepoint.h
                         ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSPROFILEPOINT_H
#define QGSPROFILEPOINT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

/**
 * \brief Encapsulates a point on a distance-elevation profile.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfilePoint
{
  public:

    /**
     * Constructor for an empty point.
     */
    QgsProfilePoint() = default;

    /**
     * Create a point at the specified distance and elevation coordinates
     */
    QgsProfilePoint( double distance, double elevation ) SIP_HOLDGIL
  : mDistance( distance )
    , mElevation( elevation )
    , mIsEmpty( false )
    {}

    /**
     * Sets the \a distance of the point.
     *
     * \see distance()
     */
    void setDistance( double distance ) SIP_HOLDGIL
    {
      mDistance = distance;
      mIsEmpty = false;
    }

    /**
     * Sets the \a elevation of the point.
     *
     * \see elevation()
     */
    void setElevation( double elevation ) SIP_HOLDGIL
    {
      mElevation = elevation;
      mIsEmpty = false;
    }

    /**
     * Returns the distance of the point.
     *
     * \see setDistance()
     */
    double distance() const SIP_HOLDGIL
    {
      return mDistance;
    }

    /**
     * Returns the elevation of the point.
     *
     * \see setElevation()
     */
    double elevation() const SIP_HOLDGIL
    {
      return mElevation;
    }

    /**
     * Returns TRUE if the point is empty.
     *
     * A QgsProfilePoint is considered empty when the coordinates have not been explicitly filled in.
     */
    bool isEmpty() const SIP_HOLDGIL { return mIsEmpty; }

    bool operator==( const QgsProfilePoint &other ) SIP_HOLDGIL
    {
      if ( isEmpty() && other.isEmpty() )
        return true;
      if ( isEmpty() && !other.isEmpty() )
        return false;
      if ( ! isEmpty() && other.isEmpty() )
        return false;

      bool equal = true;
      equal &= qgsDoubleNear( other.mDistance, mDistance, 1E-8 );
      equal &= qgsDoubleNear( other.mElevation, mElevation, 1E-8 );

      return equal;
    }

    bool operator!=( const QgsProfilePoint &other ) const SIP_HOLDGIL
    {
      if ( isEmpty() && other.isEmpty() )
        return false;
      if ( isEmpty() && !other.isEmpty() )
        return true;
      if ( ! isEmpty() && other.isEmpty() )
        return true;

      bool equal = true;
      equal &= qgsDoubleNear( other.mDistance, mDistance, 1E-8 );
      equal &= qgsDoubleNear( other.mElevation, mElevation, 1E-8 );

      return !equal;
    }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = sipCpp->isEmpty()
                        ? QStringLiteral( "<QgsProfilePoint: EMPTY>" )
                        : QStringLiteral( "<QgsProfilePoint: %1, %2>" ).arg( sipCpp->distance() ).arg( sipCpp->elevation() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End

    int __len__();
    % MethodCode
    sipRes = 2;
    % End

    SIP_PYOBJECT __getitem__( int );
    % MethodCode
    if ( a0 == 0 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->distance() );
    }
    else if ( a0 == 1 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->elevation() );
    }
    else
    {
      QString msg = QString( "Bad index: %1" ).arg( a0 );
      PyErr_SetString( PyExc_IndexError, msg.toLatin1().constData() );
    }
    % End

#endif

  private:

    double mDistance = 0;
    double mElevation = 0;
    bool mIsEmpty = true;
};

#endif // QGSPROFILEPOINT_H
