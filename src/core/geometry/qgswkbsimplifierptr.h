/***************************************************************************
    qgswkbsimplifierptr.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWKBSIMPLIFIERPTR_H
#define QGSWKBSIMPLIFIERPTR_H

#include "qgswkbptr.h"
#include "qgsvectorsimplifymethod.h"

class QgsGeometry;

/** \ingroup core
 * \class QgsConstWkbSimplifierPtr
 * \note not available in Python bindings
 */

class CORE_EXPORT QgsConstWkbSimplifierPtr : public QgsConstWkbPtr
{
    /** Simplification object which holds the information about how to simplify the features for fast rendering */
    const QgsVectorSimplifyMethod& mSimplifyMethod;

  public:
    //! Constructor
    QgsConstWkbSimplifierPtr( const unsigned char *p, int size, const QgsVectorSimplifyMethod &simplifyMethod );

    //! Read a double
    inline const QgsConstWkbPtr &operator>>( double &v ) const { return QgsConstWkbPtr::operator>>( v ); }
    //! Read a float
    inline const QgsConstWkbPtr &operator>>( float &r ) const { return QgsConstWkbPtr::operator>>( r ); }
    //! Read a int
    inline const QgsConstWkbPtr &operator>>( int &v ) const { return QgsConstWkbPtr::operator>>( v ); }
    //! Read a uint
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const { return QgsConstWkbPtr::operator>>( v ); }
    //! Read a char
    inline const QgsConstWkbPtr &operator>>( char &v ) const { return QgsConstWkbPtr::operator>>( v ); }

    //! Read a point
    virtual const QgsConstWkbPtr &operator>>( QPointF &point ) const override;
    //! Read a point array
    virtual const QgsConstWkbPtr &operator>>( QPolygonF &points ) const override;

    //! Move position forward
    inline void operator+=( int n ) { QgsConstWkbPtr::operator+=( n ); }
    //! Move position backward
    inline void operator-=( int n ) { QgsConstWkbPtr::operator-=( n ); }

    //! unsigned char * operator
    inline operator const unsigned char *() const { return mP; }
};

#endif // QGSWKBSIMPLIFIERPTR_H
