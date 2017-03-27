/***************************************************************************
                      qgsfeature.h - Spatial Feature Class
                     --------------------------------------
Date                 : 09-Sep-2003
Copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURE_H
#define QGSFEATURE_H

#include "qgis_core.h"
#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QVector>
#include <QSet>
#include <QExplicitlySharedDataPointer>


#include "qgsfields.h"

class QgsGeometry;
class QgsRectangle;
class QgsFeature;
class QgsFeaturePrivate;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

// feature id class (currently 64 bit)

// 64 bit feature ids
typedef qint64 QgsFeatureId;
#define FID_IS_NEW(fid)     (fid<0)
#define FID_TO_NUMBER(fid)  static_cast<qint64>(fid)
#define FID_TO_STRING(fid)  QString::number( fid )
#define STRING_TO_FID(str)  (str).toLongLong()

// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

/** \ingroup core
 * A vector of attributes. Mostly equal to QVector<QVariant>.
 @note QgsAttributes is implemented as a Python list of Python objects.
 */
#ifndef SIP_RUN
class CORE_EXPORT QgsAttributes : public QVector<QVariant>
{
  public:
    QgsAttributes()
      : QVector<QVariant>()
    {}

    /**
     * Create a new vector of attributes with the given size
     *
     * @param size Number of attributes
     */
    QgsAttributes( int size )
      : QVector<QVariant>( size )
    {}

    /**
     * Constructs a vector with an initial size of size elements. Each element is initialized with value.
     * @param size Number of elements
     * @param v    Initial value
     */
    QgsAttributes( int size, const QVariant &v )
      : QVector<QVariant>( size, v )
    {}

    /**
     * Copies another vector of attributes
     * @param v Attributes to copy
     */
    QgsAttributes( const QVector<QVariant> &v )
      : QVector<QVariant>( v )
    {}

    /**
     * @brief Compares two vectors of attributes.
     * They are considered equal if all their members contain the same value and NULL flag.
     * This was introduced because the default Qt implementation of QVariant comparison does not
     * handle NULL values for certain types (like int).
     *
     * @param v The attributes to compare
     * @return True if v is equal
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
     * @note added in QGIS 3.0
     * @note not available in Python bindings
     */
    QgsAttributeMap toMap() const; // SIP_SKIP

    inline bool operator!=( const QgsAttributes &v ) const { return !( *this == v ); }
};
#else
typedef QVector<QVariant> QgsAttributes;

% MappedType QgsAttributes
{
  % TypeHeaderCode
#include <qgsfeature.h>
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

  for ( SIP_SSIZE_T i = 0; i < PyList_GET_SIZE( sipPy ); ++i )
  {
    int state;
    PyObject *obj = PyList_GET_ITEM( sipPy, i );
    QVariant *t;
    if ( obj == Py_None )
    {
      t = new QVariant( QVariant::Int );
    }
    else
    {
      t = reinterpret_cast<QVariant *>( sipConvertToType( obj, sipType_QVariant, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr ) );

      if ( *sipIsErr )
      {
        sipReleaseType( t, sipType_QVariant, state );

        delete qv;
        return 0;
      }
    }

    qv->append( *t );

    sipReleaseType( t, sipType_QVariant, state );
  }

  *sipCppPtr = qv;

  return sipGetState( sipTransferObj );
  % End
};
#endif

class QgsField;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * The feature class encapsulates a single feature including its id,
 * geometry and a list of field/values attributes.
 * \note QgsFeature objects are implicitly shared.
 * @author Gary E.Sherman
 */
class CORE_EXPORT QgsFeature
{
#ifdef SIP_RUN
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipType_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
#endif
    Q_GADGET

    Q_PROPERTY( QgsFeatureId id READ id WRITE setId )
    Q_PROPERTY( QgsAttributes attributes READ attributes WRITE setAttributes )
    Q_PROPERTY( QgsFields fields READ fields WRITE setFields )

  public:

#ifdef SIP_RUN
    SIP_PYOBJECT __iter__();
    % MethodCode
    QgsAttributes attributes = sipCpp->attributes();
    PyObject *attrs = sipConvertFromType( &attributes, sipType_QgsAttributes, Py_None );
    sipRes = PyObject_GetIter( attrs );
    % End

    SIP_PYOBJECT __getitem__( int key );
    % MethodCode
    QgsAttributes attrs = sipCpp->attributes();
    if ( a0 < 0 || a0 >= attrs.count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( attrs.at( a0 ) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
    % End

    SIP_PYOBJECT __getitem__( const QString &name );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( sipCpp->attribute( fieldIdx ) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
    % End

    void __setitem__( int key, QVariant value / GetWrapper / );
    % MethodCode
    bool rv;

    if ( a1Wrapper == Py_None )
    {
      rv = sipCpp->setAttribute( a0, QVariant( QVariant::Int ) );
    }
    else
    {
      rv = sipCpp->setAttribute( a0, *a1 );
    }

    if ( !rv )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End

    void __setitem__( const QString &key, QVariant value / GetWrapper / );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
    {
      if ( a1Wrapper == Py_None )
      {
        sipCpp->setAttribute( *a0, QVariant( QVariant::Int ) );
      }
      else
      {
        sipCpp->setAttribute( fieldIdx, *a1 );
      }
    }
    % End

    void __delitem__( int key );
    % MethodCode
    if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
      sipCpp->deleteAttribute( a0 );
    else
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End

    void __delitem__( const QString &name );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
      sipCpp->deleteAttribute( fieldIdx );
    % End
#endif

    /** Constructor for QgsFeature
     * @param id feature id
     */
#ifndef SIP_RUN
    QgsFeature( QgsFeatureId id = QgsFeatureId() );
#else
    QgsFeature( qint64 id = 0 );
#endif

    /** Constructor for QgsFeature
     * @param fields feature's fields
     * @param id feature id
     */
#ifndef SIP_RUN
    QgsFeature( const QgsFields &fields, QgsFeatureId id = QgsFeatureId() );
#else
    QgsFeature( const QgsFields &fields, qint64 id = 0 );
#endif

    /** Copy constructor
     */
    QgsFeature( const QgsFeature &rhs );

    /** Assignment operator
     */
    QgsFeature &operator=( const QgsFeature &rhs );  // SIP_SKIP

    /**
     * Compares two features
     */
    bool operator==( const QgsFeature &other ) const;  // SIP_SKIP

    /**
     * Compares two features
     */
    bool operator!=( const QgsFeature &other ) const; // SIP_SKIP

    virtual ~QgsFeature();

    /** Get the feature ID for this feature.
     * @returns feature ID
     * @see setId()
     */
    QgsFeatureId id() const;

    /** Sets the feature ID for this feature.
     * @param id feature id
     * @see id
     */
    void setId( QgsFeatureId id );

    /** Returns the feature's attributes.
     * @link attributes @endlink method.
     * @returns list of feature's attributes
     * @see setAttributes
     * @note added in QGIS 2.9
     * @note Alternatively in Python: iterate feature, eg. @code [attr for attr in feature] @endcode
     */
    QgsAttributes attributes() const;

    /** Sets the feature's attributes.
     * The feature will be valid after.
     * @param attrs attribute list
     * @see setAttribute
     * @see attributes
     */
    void setAttributes( const QgsAttributes &attrs );

    /** Set an attribute's value by field index.
     * The feature will be valid if it was successful.
     * @param field the index of the field to set
     * @param attr the value of the attribute
     * @return false, if the field index does not exist
     * @note For Python: raises a KeyError exception instead of returning false
     * @note Alternatively in Python: @code feature[field] = attr @endcode
     * @see setAttributes
     */
#ifndef SIP_RUN
    bool setAttribute( int field, const QVariant &attr );
#else
    bool setAttribute( int field, const QVariant &attr / GetWrapper / );
    % MethodCode
    bool rv;

    if ( a1Wrapper == Py_None )
    {
      rv = sipCpp->setAttribute( a0, QVariant( QVariant::Int ) );
    }
    else
    {
      rv = sipCpp->setAttribute( a0, *a1 );
    }

    if ( !rv )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }

    sipRes = rv;
    % End
#endif

    /** Initialize this feature with the given number of fields. Discard any previously set attribute data.
     * @param fieldCount Number of fields to initialize
     */
    void initAttributes( int fieldCount );

    /** Deletes an attribute and its value.
     * @param field the index of the field
     * @see setAttribute
     * @note For Python: raises a KeyError exception if the field is not found
     * @note Alternatively in Python: @code del feature[field] @endcode
     */
    void deleteAttribute( int field );
#ifdef SIP_RUN
    % MethodCode
    if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
      sipCpp->deleteAttribute( a0 );
    else
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

    /** Returns the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     * @see setValid
     */
    bool isValid() const;

    /** Sets the validity of the feature.
     * @param validity set to true if feature is valid
     * @see isValid
     */
    void setValid( bool validity );

    /** Returns true if the feature has an associated geometry.
     * @see geometry()
     * @note added in QGIS 3.0.
     */
    bool hasGeometry() const;

    /** Returns the geometry associated with this feature. If the feature has no geometry,
     * an empty QgsGeometry object will be returned.
     * @see hasGeometry()
     * @see setGeometry()
     */
    QgsGeometry geometry() const;

    /** Set the feature's geometry. The feature will be valid after.
     * @param geometry new feature geometry
     * @see geometry()
     * @see clearGeometry()
     */
    void setGeometry( const QgsGeometry &geometry );

    /** Removes any geometry associated with the feature.
     * @see setGeometry()
     * @see hasGeometry()
     * @note added in QGIS 3.0
     */
    void clearGeometry();

    /** Assign a field map with the feature to allow attribute access by attribute name.
     *  @param fields The attribute fields which this feature holds
     *  @param initAttributes If true, attributes are initialized. Clears any data previously assigned.
     *                        C++: Defaults to false
     *                        Python: Defaults to true
     * @note added in QGIS 2.9
     * @see fields
     */
#ifndef SIP_RUN
    void setFields( const QgsFields &fields, bool initAttributes = false );
#else
    void setFields( const QgsFields &fields, bool initAttributes = true );
#endif

    /** Returns the field map associated with the feature.
     * @see setFields
     */
    QgsFields fields() const;

    /** Insert a value into attribute. Returns false if attribute name could not be converted to index.
     *  Field map must be associated using @link setFields @endlink before this method can be used.
     *  The feature will be valid if it was successful
     *  @param name The name of the field to set
     *  @param value The value to set
     *  @return false if attribute name could not be converted to index (C++ only)
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @note Alternatively in Python: @code feature[name] = attr @endcode
     *  @see setFields
     */
#ifndef SIP_RUN
    bool setAttribute( const QString &name, const QVariant &value );
#else
    void setAttribute( const QString &name, const QVariant &value / GetWrapper / );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
    {
      if ( a1Wrapper == Py_None )
      {
        sipCpp->setAttribute( *a0, QVariant( QVariant::Int ) );
      }
      else
      {
        sipCpp->setAttribute( fieldIdx, *a1 );
      }
    }
    % End
#endif

    /** Removes an attribute value by field name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param name The name of the field to delete
     *  @return false if attribute name could not be converted to index (C++ only)
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @note Alternatively in Python: @code del feature[name] @endcode
     *  @see setFields
     */
    bool deleteAttribute( const QString &name );
#ifdef SIP_RUN
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
      sipRes = false;
    }
    else
    {
      sipCpp->deleteAttribute( fieldIdx );
      sipRes = true;
    }
    % End
#endif

    /** Lookup attribute value from attribute name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param name The name of the attribute to get
     *  @return The value of the attribute (C++: Invalid variant if no such name exists )
     *  @note For Python: raises a KeyError exception if the field is not found
     *  @note Alternatively in Python: @code feature[name] @endcode
     *  @see setFields
     */
#ifndef SIP_RUN
    QVariant attribute( const QString &name ) const;
#else
    SIP_PYOBJECT attribute( const QString &name ) const;
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( sipCpp->attribute( fieldIdx ) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
    % End
#endif

    /** Lookup attribute value from its index. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param fieldIdx The index of the attribute to get
     *  @return The value of the attribute (C++: Invalid variant if no such index exists )
     *  @note For Python: raises a KeyError exception if the field is not found
     *  @note Alternatively in Python: @code feature[fieldIdx] @endcode
     *  @see setFields
     */
#ifndef SIP_RUN
    QVariant attribute( int fieldIdx ) const;
#else
    SIP_PYOBJECT attribute( int fieldIdx ) const;
    % MethodCode
    {
      if ( a0 < 0 || a0 >= sipCpp->attributes().count() )
      {
        PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
        sipIsErr = 1;
      }
      else
      {
        QVariant *v = new QVariant( sipCpp->attribute( a0 ) );
        sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
      }
    }
    % End
#endif

    /** Utility method to get attribute index from name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param fieldName name of field to get attribute index of
     *  @returns -1 if field does not exist or field map is not associated.
     *  @see setFields
     */
    int fieldNameIndex( const QString &fieldName ) const;

    //! Allows direct construction of QVariants from features.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    QExplicitlySharedDataPointer<QgsFeaturePrivate> d;

}; // class QgsFeature

//! Writes the feature to stream out. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsFeature &feature ); // SIP_SKIP
//! Reads a feature from stream in into feature. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsFeature &feature ); // SIP_SKIP

// key = feature id, value = changed attributes
#ifndef SIP_RUN
typedef QMap<QgsFeatureId, QgsAttributeMap> QgsChangedAttributesMap;
#else
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;
#endif

// key = feature id, value = changed geometry
#ifndef SIP_RUN
typedef QMap<QgsFeatureId, QgsGeometry> QgsGeometryMap;
#else
typedef QMap<qint64, QgsGeometry> QgsGeometryMap;
#endif

#ifndef SIP_RUN
typedef QSet<QgsFeatureId> QgsFeatureIds;
#else
typedef QSet<qint64> QgsFeatureIds;
#endif

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;


#ifdef SIP_RUN
typedef QMap<int, QgsField> QgsFieldMap;
#endif

typedef QList<QgsFeature> QgsFeatureList;

uint qHash( const QgsFeature &key, uint seed = 0 ); // SIP_SKIP

Q_DECLARE_METATYPE( QgsFeature )
Q_DECLARE_METATYPE( QgsFeatureList )

#endif
