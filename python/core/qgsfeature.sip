
typedef qint64 QgsFeatureId;

// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

typedef QVector<QVariant> QgsAttributes;
// QgsAttributes is implemented as a Python list of Python objects.
%MappedType QgsAttributes /DocType="list-of-attributes"/
{
%TypeHeaderCode
#include <qvector.h>
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ( ( l = PyList_New(sipCpp->size() ) ) == NULL )
        return NULL;

    // Set the list elements.
    for ( int i = 0; i < sipCpp->size(); ++i )
    {
      QVariant* v = new QVariant( sipCpp->at( i ) );
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

    QVector<QVariant> *qv = new QVector<QVariant>;

    for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
      int state;
      PyObject* obj = PyList_GET_ITEM(sipPy, i);
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
 *
 * @author Gary E.Sherman
 */
class QgsFeature
{
%TypeHeaderCode
#include <qgsfeature.h>
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipClass_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
%End

  public:

    SIP_PYOBJECT __iter__();
%MethodCode
    PyObject *attrs = sipConvertFromType( &sipCpp->attributes(), sipType_QgsAttributes, Py_None );
    sipRes = PyObject_GetIter(attrs);
%End

    SIP_PYOBJECT __getitem__(int key);
%MethodCode
  const QgsAttributes& attrs = sipCpp->attributes();
  if (a0 < 0 || a0 >= attrs.count())
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    QVariant* v = new QVariant( attrs.at(a0) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End

    SIP_PYOBJECT __getitem__(const QString& name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    QVariant* v = new QVariant( sipCpp->attribute(fieldIdx) );
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

    void __setitem__(const QString& key, QVariant value /GetWrapper/);
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

    void __delitem__(const QString& name);
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

    //! Constructor
    QgsFeature( qint64 id = 0 );

    QgsFeature( const QgsFields& fields, qint64 id = 0 );

    /** copy ctor needed due to internal pointer */
    QgsFeature( const QgsFeature & rhs );

    //! Destructor
    ~QgsFeature();

    /**
     * Get the feature id for this feature
     * @return Feature id
     */
    qint64 id() const;

    /**
     * Set the feature id for this feature
     * @param id Feature id
     */
    void setFeatureId( qint64 id );

    const QgsAttributes& attributes() const;
    //QgsAttributes& attributes();
    void setAttributes(const QgsAttributes& attrs);
    bool setAttribute( int field, const QVariant& attr /GetWrapper/);
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

    /**
     * Initialize this feature with the given number of fields. Discard any previously set attribute data.
     * @param fieldCount Number of fields to initialize
     */
    void initAttributes( int fieldCount );

    /**
     * Deletes an attribute and its value
     *
     * @param field The index of the field
     *
     * @note For Python: raises a KeyError exception if the field is not found
     */
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
    /**
     * Return the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     */
    bool isValid() const;

    /**
     * Set the validity of the feature.
     */
    void setValid( bool validity );

    /**
     * Get the geometry object associated with this feature
     */
    QgsGeometry *geometry() const;

    /**
     * Get the geometry object associated with this feature
     * The caller assumes responsibility for the QgsGeometry*'s destruction.
     */
    QgsGeometry *geometryAndOwnership() /Factory/;

    /** Set this feature's geometry from another QgsGeometry object (deep copy)
     */
    void setGeometry( const QgsGeometry& geom );

    /** Set this feature's geometry (takes geometry ownership)
     * @note not available in python bindings
     */
    // void setGeometry( QgsGeometry* geom /Transfer/ );

    /**
     * Set this feature's geometry from WKB
     *
     * This feature assumes responsibility for destroying geom.
     */
    void setGeometryAndOwnership( unsigned char * geom /Transfer/, size_t length );

    /** Assign a field map with the feature to allow attribute access by attribute name
     *
     *  @param fields         The attribute fields which this feature holds. When used from python, make sure
     *                        a copy of the fields is held by python, as ownership stays there.
     *                        I.e. Do not call feature.setFields( myDataProvider.fields() ) but instead call
     *                        myFields = myDataProvider.fields()
     *                        feature.setFields( myFields )
     *  @param initAttributes If true, attributes are initialized. Clears any data previously assigned.
     *                        C++: Defaults to false
     *                        Python: Defaults to true
     *  @note added in 2.0
     *
     * TODO: QGIS3 - take reference, not pointer
     */
    void setFields( const QgsFields* fields, bool initAttributes = true );

    /** Get associated field map.
     *  @note added in 2.0
     *
     * TODO: QGIS 3 - return reference or value, not pointer
     */
    const QgsFields* fields() const;

    /** Insert a value into attribute. Returns false if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *
     *  @param name The name of the field to set
     *  @param value The value to set
     *
     *  @return false if attribute name could not be converted to index (C++ only)
     *
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @note added in 2.0
     */
    void setAttribute( const QString& name, QVariant value /GetWrapper/ );
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
    /** Remove an attribute value.
     *  Returns false if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *
     *  @param name The name of the field to delete
     *
     *  @return false if attribute name could not be converted to index (C++ only)
     *
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @note added in 2.0
     */
    bool deleteAttribute( const QString& name );
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    sipCpp->deleteAttribute( fieldIdx );
  }
%End
    /** Lookup attribute value from attribute name.
     *  Returns invalid variant if attribute name could not be converted to index (C++ only)
     *  Field map must be associated to make this work.
     *
     *  @param name The name of the attribute to get
     *
     *  @return The value of the attribute (C++: Invalid variant if no such name exists )
     *
     *  @note For Python: raises a KeyError exception if field is not found
     *  @note added in 2.0
     */
    SIP_PYOBJECT attribute( const QString& name ) const;
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    QVariant* v = new QVariant( sipCpp->attribute(fieldIdx) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End
    /** Utility method to get attribute index from name. Returns -1 if field does not exist or field map is not associated.
     *  Field map must be associated to make this work.
     *
     *  @note added in 2.0
     */
    int fieldNameIndex( const QString& fieldName ) const;

}; // class QgsFeature

typedef QSet<QgsFeatureId> QgsFeatureIds;
