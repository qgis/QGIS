
typedef qint64 QgsFeatureId;
typedef QMap<int, QVariant> QgsAttributeMap;
typedef QVector<QVariant> QgsAttributes;

// QgsAttributes is implemented as a Python list of Python objects.
%MappedType QgsAttributes
{
%TypeHeaderCode
#include <qgsfeature.h> // NO_SIPIFY
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ( ( l = PyList_New(sipCpp->size() ) ) == NULL )
        return NULL;

    // Set the list elements.
    for ( int i = 0; i < sipCpp->size(); ++i )
    {
      QVariant *v = new QVariant( sipCpp->at( i ) );
      PyObject *tobj;

      if ( ( tobj = sipConvertFromNewType( v, sipType_QVariant,Py_None ) ) == NULL )
      {
        Py_DECREF( l );
        delete v;

        return NULL;
      }

      PyList_SET_ITEM( l, i, tobj );
    }

    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
      if (!PyList_Check(sipPy))
        return 0;

      for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
        if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), sipType_QVariant, SIP_NOT_NONE))
          return 0;

      return 1;
    }

    QgsAttributes *qv = new QgsAttributes;

    for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
      int state;
      PyObject *obj = PyList_GET_ITEM(sipPy, i);
      QVariant *t;
      if ( obj == Py_None )
      {
        t = new QVariant( QVariant::Int );
      }
      else
      {
        t = reinterpret_cast<QVariant *>(sipConvertToType(obj, sipType_QVariant, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
          sipReleaseType(t, sipType_QVariant, state);

          delete qv;
          return 0;
        }
      }

      qv->append(*t);

      sipReleaseType(t, sipType_QVariant, state);
    }

    *sipCppPtr = qv;

    return sipGetState(sipTransferObj);
%End
};

// key = feature id, value = changed attributes
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<qint64, QgsGeometry> QgsGeometryMap;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;

typedef QMap<int, QgsField> QgsFieldMap;

/** \ingroup core
 * The feature class encapsulates a single feature including its id,
 * geometry and a list of field/values attributes.
 * \note QgsFeature objects are implicitly shared.
 * @author Gary E.Sherman
 */
class QgsFeature
{
%TypeHeaderCode
#include <qgsfeature.h>
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipType_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
%End
%Docstring
/** \ingroup core
 The feature class encapsulates a single feature including its id,
 geometry and a list of field/values attributes.
 \note QgsFeature objects are implicitly shared.
 @author Gary E.Sherman
/

%End


  public:

    SIP_PYOBJECT __iter__();
%MethodCode
    QgsAttributes attributes = sipCpp->attributes();
    PyObject *attrs = sipConvertFromType( &attributes, sipType_QgsAttributes, Py_None );
    sipRes = PyObject_GetIter(attrs);
%End

    SIP_PYOBJECT __getitem__(int key);
%MethodCode
  QgsAttributes attrs = sipCpp->attributes();
  if (a0 < 0 || a0 >= attrs.count())
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    QVariant *v = new QVariant( attrs.at(a0) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End

    SIP_PYOBJECT __getitem__(const QString &name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    QVariant *v = new QVariant( sipCpp->attribute(fieldIdx) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End

    void __setitem__(int key, QVariant value /GetWrapper/);
%MethodCode
  bool rv;

  if ( a1Wrapper == Py_None )
  {
    rv = sipCpp->setAttribute(a0, QVariant( QVariant::Int ) );
  }
  else
  {
    rv = sipCpp->setAttribute(a0, *a1);
  }

  if ( !rv )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End

    void __setitem__(const QString &key, QVariant value /GetWrapper/);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    if ( a1Wrapper == Py_None )
    {
      sipCpp->setAttribute(*a0, QVariant( QVariant::Int ) );
    }
    else
    {
      sipCpp->setAttribute(fieldIdx, *a1);
    }
  }
%End

    void __delitem__(int key);
%MethodCode
  if (a0 >= 0 && a0 < sipCpp->attributes().count())
    sipCpp->deleteAttribute(a0);
  else
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End

    void __delitem__(const QString &name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
    sipCpp->deleteAttribute(fieldIdx);
%End

    QgsFeature( qint64 id = 0 );

    QgsFeature( const QgsFields &fields, qint64 id = 0 );

    QgsFeature( const QgsFeature &rhs );

    ~QgsFeature();

    QgsFeatureId id() const;

    void setId( QgsFeatureId id );

    QgsAttributes attributes() const;

    void setAttributes( const QgsAttributes &attrs );

    bool setAttribute( int field, const QVariant &attr /GetWrapper/);
%MethodCode
  bool rv;

  if ( a1Wrapper == Py_None )
  {
    rv = sipCpp->setAttribute(a0, QVariant( QVariant::Int ) );
  }
  else
  {
    rv = sipCpp->setAttribute(a0, *a1);
  }

  if ( !rv )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }

  sipRes = rv;
%End

    void initAttributes( int fieldCount );

    void deleteAttribute( int field );
%MethodCode
  if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
    sipCpp->deleteAttribute(a0);
  else
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End

    bool isValid() const;

    void setValid( bool validity );

    bool hasGeometry() const;

    QgsGeometry geometry() const;

    void setGeometry( const QgsGeometry &geometry );

    void clearGeometry();

    void setFields( const QgsFields &fields, bool initAttributes = true );

    QgsFields fields() const;

    void setAttribute( const QString &name, const QVariant &value /GetWrapper/ );
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    if ( a1Wrapper == Py_None )
    {
      sipCpp->setAttribute(*a0, QVariant( QVariant::Int ) );
    }
    else
    {
      sipCpp->setAttribute(fieldIdx, *a1);
    }
  }
%End

    bool deleteAttribute( const QString &name );
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
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
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    QVariant *v = new QVariant( sipCpp->attribute(fieldIdx) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End

    SIP_PYOBJECT attribute( int fieldIdx ) const;
%MethodCode
  {
    if (a0 < 0 || a0 >= sipCpp->attributes().count())
    {
      PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( sipCpp->attribute(a0) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
  }
%End

    int fieldNameIndex( const QString &fieldName ) const;

    //! Allows direct construction of QVariants from features.
    operator QVariant() const;

}; // class QgsFeature

typedef QSet<qint64> QgsFeatureIds;
