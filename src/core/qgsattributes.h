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
#include "qgsunsetattributevalue.h"
#include "qgsvariantutils.h"


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
 * \brief A vector of attributes. Mostly equal to QVector<QVariant>.
 * \note QgsAttributes is implemented as a Python list of Python objects.
 */
#ifndef SIP_RUN
class QgsAttributes : public QVector<QVariant>
{
  public:

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

      // note that for non-null values, we need to check that the type is equal too!
      // QVariant == comparisons do some weird things, like reporting that a QDateTime(2021, 2, 10, 0, 0) variant is equal
      // to a QString "2021-02-10 00:00" variant!
      while ( i != b )
        if ( !( QgsVariantUtils::isNull( *( --i ) ) == QgsVariantUtils::isNull( *( --j ) ) && ( QgsVariantUtils::isNull( *i ) || i->userType() == j->userType() ) && *i == *j ) )
          return false;
      return true;
    }

    /**
     * Returns a QgsAttributeMap of the attribute values. Null values are
     * excluded from the map.
     * \note not available in Python bindings
     */
    CORE_EXPORT QgsAttributeMap toMap() const SIP_SKIP;

    /**
     * Returns TRUE if the attribute at the specified index is an unset value.
     *
     * \see QgsUnsetAttributeValue
     */
    bool isUnsetValue( int index ) const
    {
      if ( index < 0 || index >= size() )
        return false;

      return at( index ).userType() == qMetaTypeId<QgsUnsetAttributeValue>();
    }

    inline bool operator!=( const QgsAttributes &v ) const { return !( *this == v ); }
};

//! Hash for QgsAttributes
CORE_EXPORT uint qHash( const QgsAttributes &attributes );

#endif

#ifdef SIP_PYQT5_RUN
#ifdef SIP_RUN
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
    const QVariant v = sipCpp->at( i );
    PyObject *tobj = NULL;
    if ( !v.isValid() )
    {
      Py_INCREF( Py_None );
      tobj = Py_None;
    }
    // QByteArray null handling is "special"! See null_from_qvariant_converter in conversions.sip
    else if ( QgsVariantUtils::isNull( v, true ) && v.userType() != QMetaType::Type::QByteArray )
    {
      PyObject *vartype = sipConvertFromEnum( v.type(), sipType_QVariant_Type );
      PyObject *args = PyTuple_Pack( 1, vartype );
      PyTypeObject *typeObj = sipTypeAsPyTypeObject( sipType_QVariant );
      tobj = PyObject_Call( ( PyObject * )typeObj, args, nullptr );
      Py_DECREF( args );
      Py_DECREF( vartype );
    }
    else
    {
      switch ( v.userType() )
      {
        case QMetaType::Type::Int:
          tobj = PyLong_FromLong( v.toInt() );
          break;

        case QMetaType::Type::UInt:
          tobj = PyLong_FromUnsignedLong( v.toUInt() );
          break;

        case QMetaType::Type::Long:
        case QMetaType::Type::LongLong:
          tobj = PyLong_FromLongLong( v.toLongLong() );
          break;

        case QMetaType::Type::ULong:
        case QMetaType::Type::ULongLong:
          tobj = PyLong_FromUnsignedLongLong( v.toULongLong() );
          break;

        case QMetaType::Type::Bool:
          tobj = PyBool_FromLong( v.toBool() ? 1 : 0 );
          break;

        case QMetaType::Type::Float:
        case QMetaType::Type::Double:
          tobj = PyFloat_FromDouble( v.toDouble() );
          break;

        case QMetaType::Type::QString:
          tobj = PyUnicode_FromString( v.toString().toUtf8().constData() );
          break;

        default:
        {
          QVariant *newV = new QVariant( v );
          tobj = sipConvertFromNewType( newV, sipType_QVariant, sipTransferObj );
          break;
        }
      }
    }
    if ( tobj == NULL )
    {
      Py_DECREF( l );
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

  SIP_SSIZE_T listSize = PyList_GET_SIZE( sipPy );
  // Initialize attributes to null. This has two motivations:
  // 1. It speeds up the QVector construction, as otherwise we are creating n default QVariant objects (default QVariant constructor is not free!)
  // 2. It lets us shortcut in the loop below when a Py_None is encountered in the list
  const QVariant nullVariant( QVariant::Int );
  QgsAttributes *qv = new QgsAttributes( listSize, nullVariant );
  QVariant *outData = qv->data();

  for ( SIP_SSIZE_T i = 0; i < listSize; ++i )
  {
    PyObject *obj = PyList_GET_ITEM( sipPy, i );
    if ( obj == Py_None )
    {
      // outData was already initialized to null values
      *outData++;
    }
    else if ( PyBool_Check( obj ) )
    {
      *outData++ = QVariant( PyObject_IsTrue( obj ) == 1 );
    }
    else if ( PyLong_Check( obj ) )
    {
      *outData++ = QVariant( PyLong_AsLongLong( obj ) );
    }
    else if ( PyFloat_Check( obj ) )
    {
      *outData++ = QVariant( PyFloat_AsDouble( obj ) );
    }
    else if ( PyUnicode_Check( obj ) )
    {
      *outData++ = QVariant( QString::fromUtf8( PyUnicode_AsUTF8( obj ) ) );
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

      *outData++ = *t;
      sipReleaseType( t, sipType_QVariant, state );
    }
  }

  *sipCppPtr = qv;

  return sipGetState( sipTransferObj );
  % End
};
#endif
#endif

#ifdef SIP_PYQT6_RUN
#ifdef SIP_RUN
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
    const QVariant v = sipCpp->at( i );
    PyObject *tobj = NULL;
    // QByteArray null handling is "special"! See null_from_qvariant_converter in conversions.sip
    if ( QgsVariantUtils::isNull( v, true ) && v.userType() != QMetaType::Type::QByteArray )
    {
      Py_INCREF( Py_None );
      tobj = Py_None;
    }
    else
    {
      switch ( v.userType() )
      {
        case QMetaType::Type::Int:
          tobj = PyLong_FromLong( v.toInt() );
          break;

        case QMetaType::Type::UInt:
          tobj = PyLong_FromUnsignedLong( v.toUInt() );
          break;

        case QMetaType::Type::Long:
        case QMetaType::Type::LongLong:
          tobj = PyLong_FromLongLong( v.toLongLong() );
          break;

        case QMetaType::Type::ULong:
        case QMetaType::Type::ULongLong:
          tobj = PyLong_FromUnsignedLongLong( v.toULongLong() );
          break;

        case QMetaType::Type::Bool:
          tobj = PyBool_FromLong( v.toBool() ? 1 : 0 );
          break;

        case QMetaType::Type::Float:
        case QMetaType::Type::Double:
          tobj = PyFloat_FromDouble( v.toDouble() );
          break;

        case QMetaType::Type::QString:
          tobj = PyUnicode_FromString( v.toString().toUtf8().constData() );
          break;

        default:
        {
          QVariant *newV = new QVariant( v );
          tobj = sipConvertFromNewType( newV, sipType_QVariant, sipTransferObj );
          break;
        }
      }
    }
    if ( tobj == NULL )
    {
      Py_DECREF( l );
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

  SIP_SSIZE_T listSize = PyList_GET_SIZE( sipPy );
  // Initialize attributes to null. This has two motivations:
  // 1. It speeds up the QVector construction, as otherwise we are creating n default QVariant objects (default QVariant constructor is not free!)
  // 2. It lets us shortcut in the loop below when a Py_None is encountered in the list
  const QVariant nullVariant( QVariant::Int );
  QgsAttributes *qv = new QgsAttributes( listSize, nullVariant );
  QVariant *outData = qv->data();

  for ( SIP_SSIZE_T i = 0; i < listSize; ++i )
  {
    PyObject *obj = PyList_GET_ITEM( sipPy, i );
    if ( obj == Py_None )
    {
      // outData was already initialized to null values
      *outData++;
    }
    else if ( PyBool_Check( obj ) )
    {
      *outData++ = QVariant( PyObject_IsTrue( obj ) == 1 );
    }
    else if ( PyLong_Check( obj ) )
    {
      *outData++ = QVariant( PyLong_AsLongLong( obj ) );
    }
    else if ( PyFloat_Check( obj ) )
    {
      *outData++ = QVariant( PyFloat_AsDouble( obj ) );
    }
    else if ( PyUnicode_Check( obj ) )
    {
      *outData++ = QVariant( QString::fromUtf8( PyUnicode_AsUTF8( obj ) ) );
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

      *outData++ = *t;
      sipReleaseType( t, sipType_QVariant, state );
    }
  }

  *sipCppPtr = qv;

  return sipGetState( sipTransferObj );
  % End
};
#endif
#endif
#endif // QGSATTRIBUTES_H
