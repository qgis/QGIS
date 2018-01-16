/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeature.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/











class QgsFeature
{
%Docstring
The feature class encapsulates a single feature including its id,
geometry and a list of field/values attributes.

.. note::

   QgsFeature objects are implicitly shared.
%End

%TypeHeaderCode
#include "qgsfeature.h"
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipType_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
%End
  public:

    SIP_PYOBJECT __iter__();
%MethodCode
    QgsAttributes attributes = sipCpp->attributes();
    PyObject *attrs = sipConvertFromType( &attributes, sipType_QgsAttributes, Py_None );
    sipRes = PyObject_GetIter( attrs );
%End

    SIP_PYOBJECT __getitem__( int key );
%MethodCode
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
%End

    SIP_PYOBJECT __getitem__( const QString &name );
%MethodCode
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
%End

    void __setitem__( int key, QVariant value /GetWrapper/ );
%MethodCode
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
%End

    void __setitem__( const QString &key, QVariant value /GetWrapper/ );
%MethodCode
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
%End

    void __delitem__( int key );
%MethodCode
    if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
      sipCpp->deleteAttribute( a0 );
    else
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
%End

    void __delitem__( const QString &name );
%MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
      sipCpp->deleteAttribute( fieldIdx );
%End

    QgsFeature( qint64 id = 0 );
%Docstring
Constructor for QgsFeature

:param id: feature id
%End

    QgsFeature( const QgsFields &fields, qint64 id = 0 );
%Docstring
Constructor for QgsFeature

:param fields: feature's fields
:param id: feature id
%End

    QgsFeature( const QgsFeature &rhs );
%Docstring
Copy constructor
%End




    virtual ~QgsFeature();

    QgsFeatureId id() const;
%Docstring
Get the feature ID for this feature.

:return: feature ID

.. seealso:: :py:func:`setId`
%End

    void setId( QgsFeatureId id );
%Docstring
Sets the feature ID for this feature.

:param id: feature id

.. seealso:: :py:func:`id`
%End

    QgsAttributes attributes() const;
%Docstring
Returns the feature's attributes.

:return: list of feature's attributes

.. seealso:: :py:func:`setAttributes`

.. versionadded:: 2.9

.. note::

   Alternatively in Python: iterate feature, eg. @code [attr for attr in feature] @endcode
%End

    void setAttributes( const QgsAttributes &attrs );
%Docstring
Sets the feature's attributes.
The feature will be valid after.

:param attrs: attribute list

.. seealso:: :py:func:`setAttribute`

.. seealso:: :py:func:`attributes`
%End

    bool setAttribute( int field, const QVariant &attr /GetWrapper/ );
%Docstring
Set an attribute's value by field index.
The feature will be valid if it was successful.

:param field: the index of the field to set
:param attr: the value of the attribute

:return: false, if the field index does not exist

.. note::

   For Python: raises a KeyError exception instead of returning false

.. note::

   Alternatively in Python: @code feature[field] = attr @endcode

.. seealso:: :py:func:`setAttributes`
%End
%MethodCode
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
%End

    void initAttributes( int fieldCount );
%Docstring
Initialize this feature with the given number of fields. Discard any previously set attribute data.

:param fieldCount: Number of fields to initialize
%End

    void deleteAttribute( int field );
%Docstring
Deletes an attribute and its value.

:param field: the index of the field

.. seealso:: :py:func:`setAttribute`

.. note::

   For Python: raises a KeyError exception if the field is not found

.. note::

   Alternatively in Python: @code del feature[field] @endcode
%End
%MethodCode
    if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
      sipCpp->deleteAttribute( a0 );
    else
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
%End

    bool isValid() const;
%Docstring
Returns the validity of this feature. This is normally set by
the provider to indicate some problem that makes the feature
invalid or to indicate a null feature.

.. seealso:: :py:func:`setValid`
%End

    void setValid( bool validity );
%Docstring
Sets the validity of the feature.

:param validity: set to true if feature is valid

.. seealso:: :py:func:`isValid`
%End

    bool hasGeometry() const;
%Docstring
Returns true if the feature has an associated geometry.

.. seealso:: :py:func:`geometry`

.. versionadded:: 3.0.
%End

    QgsGeometry geometry() const;
%Docstring
Returns the geometry associated with this feature. If the feature has no geometry,
an empty QgsGeometry object will be returned.

.. seealso:: :py:func:`hasGeometry`

.. seealso:: :py:func:`setGeometry`
%End

    void setGeometry( const QgsGeometry &geometry );
%Docstring
Set the feature's geometry. The feature will be valid after.

:param geometry: new feature geometry

.. seealso:: :py:func:`geometry`

.. seealso:: :py:func:`clearGeometry`
%End

    void clearGeometry();
%Docstring
Removes any geometry associated with the feature.

.. seealso:: :py:func:`setGeometry`

.. seealso:: :py:func:`hasGeometry`

.. versionadded:: 3.0
%End

    void setFields( const QgsFields &fields, bool initAttributes = true  );
%Docstring
Assign a field map with the feature to allow attribute access by attribute name.

:param fields: The attribute fields which this feature holds
:param initAttributes: If true, attributes are initialized. Clears any data previously assigned.
C++: Defaults to false
Python: Defaults to true

.. versionadded:: 2.9

.. seealso:: :py:func:`fields`
%End

    QgsFields fields() const;
%Docstring
Returns the field map associated with the feature.

.. seealso:: :py:func:`setFields`
%End

    void setAttribute( const QString &name, const QVariant &value /GetWrapper/ );
%Docstring
Insert a value into attribute. Returns false if attribute name could not be converted to index.
Field map must be associated using setFields() before this method can be used.
The feature will be valid if it was successful

:param name: The name of the field to set
:param value: The value to set

:return: false if attribute name could not be converted to index (C++ only)

.. note::

   For Python: raises a KeyError exception instead of returning false

.. note::

   Alternatively in Python: @code feature[name] = attr @endcode

.. seealso:: :py:func:`setFields`
%End
%MethodCode
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
%End

    bool deleteAttribute( const QString &name );
%Docstring
Removes an attribute value by field name. Field map must be associated using setFields()
before this method can be used.

:param name: The name of the field to delete

:return: false if attribute name could not be converted to index (C++ only)

.. note::

   For Python: raises a KeyError exception instead of returning false

.. note::

   Alternatively in Python: @code del feature[name] @endcode

.. seealso:: :py:func:`setFields`
%End
%MethodCode
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
%End

    SIP_PYOBJECT attribute( const QString &name ) const;
%Docstring
Lookup attribute value from attribute name. Field map must be associated using setFields()
before this method can be used.

:param name: The name of the attribute to get

:return: The value of the attribute (C++: Invalid variant if no such name exists )

.. note::

   For Python: raises a KeyError exception if the field is not found

.. note::

   Alternatively in Python: @code feature[name] @endcode

.. seealso:: :py:func:`setFields`
%End
%MethodCode
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
%End

    SIP_PYOBJECT attribute( int fieldIdx ) const;
%Docstring
Lookup attribute value from its index. Field map must be associated using setFields()
before this method can be used.

:param fieldIdx: The index of the attribute to get

:return: The value of the attribute (C++: Invalid variant if no such index exists )

.. note::

   For Python: raises a KeyError exception if the field is not found

.. note::

   Alternatively in Python: @code feature[fieldIdx] @endcode

.. seealso:: :py:func:`setFields`
%End
%MethodCode
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
%End

    int fieldNameIndex( const QString &fieldName ) const;
%Docstring
Utility method to get attribute index from name. Field map must be associated using setFields()
before this method can be used.

:param fieldName: name of field to get attribute index of

:return: -1 if field does not exist or field map is not associated.

.. seealso:: :py:func:`setFields`
%End

    operator QVariant() const;

}; // class QgsFeature


typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;


typedef QMap<qint64, QgsGeometry> QgsGeometryMap;


typedef QSet<qint64> QgsFeatureIds;

typedef QList<QgsFeature> QgsFeatureList;



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfeature.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
