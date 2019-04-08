/***************************************************************************
  qgsattributes.h - QgsAttributes

 ---------------------
 begin                : 29.3.2017
 copyright            : (C) 2017 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSATTRIBUTES_H
#define QGSATTRIBUTES_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QVector>
#include <QSet>
#include <QExplicitlySharedDataPointer>


#include "qgsfields.h"


class QgsRectangle;
class QgsFeature;
class QgsFeaturePrivate;

// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

#ifdef SIP_RUN
typedef QMap<int, QgsField> QgsFieldMap;
#endif


/**
 * \ingroup core
 * A vector of attributes. Mostly equal to QVector<QVariant>.
 \note QgsAttributes is implemented as a Python list of Python objects.
 */
#ifndef SIP_RUN
class CORE_EXPORT QgsAttributes : public QVector<QVariant>
{
  public:

    //! Constructor for QgsAttributes
    QgsAttributes() = default;

    /**
     * Create a new vector of attributes with the given size
     *
     * \param size Number of attributes
     */
    QgsAttributes( int size )
      : QVector<QVariant>( size )
    {}

    /**
     * Constructs a vector with an initial size of size elements. Each element is initialized with value.
     * \param size Number of elements
     * \param v    Initial value
     */
    QgsAttributes( int size, const QVariant &v )
      : QVector<QVariant>( size, v )
    {}

    /**
     * Copies another vector of attributes
     * \param v Attributes to copy
     */
    QgsAttributes( const QVector<QVariant> &v )
      : QVector<QVariant>( v )
    {}

    /**
     * \brief Compares two vectors of attributes.
     * They are considered equal if all their members contain the same value and NULL flag.
     * This was introduced because the default Qt implementation of QVariant comparison does not
     * handle NULL values for certain types (like int).
     *
     * \param v The attributes to compare
     * \returns TRUE if v is equal
     */
    bool operator==( const QgsAttributes &v ) const
    {
      if ( size() != v.size() )
        return false;
      const QVariant *b = constData();
      const QVariant *i = b + size();
      const QVariant *j = v.constData() + size();
      while ( i != b )
        if ( !( *--i == *--j && i->isNull() == j->isNull() ) )
          return false;
      return true;
    }

    /**
     * Returns a QgsAttributeMap of the attribute values. Null values are
     * excluded from the map.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    QgsAttributeMap toMap() const SIP_SKIP;

    inline bool operator!=( const QgsAttributes &v ) const { return !( *this == v ); }
};

//! Hash for QgsAttributes
CORE_EXPORT uint qHash( const QgsAttributes &attributes );

#else
typedef QVector<QVariant> QgsAttributes;

% MappedType QgsAttributes
{
  % TypeHeaderCode
#include "qgsfeature.h"
  % End

  % ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ( ( l = PyList_New( sipCpp->size() ) ) == NULL )
    return NULL;

  // Set the list elements.
  for ( int i = 0; i < sipCpp->size(); ++i )
  {
    QVariant *v = new QVariant( sipCpp->at( i ) );
    PyObject *tobj;

    if ( ( tobj = sipConvertFromNewType( v, sipType_QVariant, Py_None ) ) == NULL )
    {
      Py_DECREF( l );
      delete v;

      return NULL;
    }

    PyList_SET_ITEM( l, i, tobj );
  }

  return l;
  % End

  % ConvertToTypeCode
  // Check the type if that is all that is required.
  if ( sipIsErr == NULL )
  {
    if ( !PyList_Check( sipPy ) )
      return 0;

    for ( SIP_SSIZE_T i = 0; i < PyList_GET_SIZE( sipPy ); ++i )
      if ( !sipCanConvertToType( PyList_GET_ITEM( sipPy, i ), sipType_QVariant, SIP_NOT_NONE ) )
        return 0;

    return 1;
  }

  QgsAttributes *qv = new QgsAttributes;
  SIP_SSIZE_T listSize = PyList_GET_SIZE( sipPy );
  qv->reserve( listSize );

  for ( SIP_SSIZE_T i = 0; i < listSize; ++i )
  {
    PyObject *obj = PyList_GET_ITEM( sipPy, i );
    if ( obj == Py_None )
    {
      qv->append( QVariant( QVariant::Int ) );
    }
    else
    {
      int state;
      QVariant *t = reinterpret_cast<QVariant *>( sipConvertToType( obj, sipType_QVariant, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr ) );

      if ( *sipIsErr )
      {
        sipReleaseType( t, sipType_QVariant, state );

        delete qv;
        return 0;
      }

      qv->append( *t );
      sipReleaseType( t, sipType_QVariant, state );
    }
  }

  *sipCppPtr = qv;

  return sipGetState( sipTransferObj );
  % End
};
#endif


#endif // QGSATTRIBUTES_H
