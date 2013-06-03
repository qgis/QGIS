
typedef qint64 QgsFeatureId;

// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

typedef QVector<QVariant> QgsAttributes;

// key = feature id, value = changed attributes
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<qint64, QgsGeometry> QgsGeometryMap;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;

typedef QMap<int, QgsField> QgsFieldMap;

class QgsFeature
{
%TypeHeaderCode
#include <qgsfeature.h>
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipClass_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
%End

  public:

    SIP_PYOBJECT __getattr__(const QString& name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  QString altname = QString(*a0).replace("_"," ");
  int altfieldIdx = sipCpp->fieldNameIndex(altname);

  if (fieldIdx >= 0)
  {
      QVariant* v = new QVariant( sipCpp->attribute(fieldIdx) );
      sipRes = sipConvertFromInstance(v, sipClass_QVariant, Py_None);
  }
  else if( altfieldIdx >= 0 )
  {
      QVariant* v = new QVariant( sipCpp->attribute(altfieldIdx) );
      sipRes = sipConvertFromInstance(v, sipClass_QVariant, Py_None);
  }
  else
  {
    PyObject* key = PyString_FromString(a0->toStdString().c_str());
    sipRes = PyObject_GenericGetAttr(sipSelf, key );
  }
%End

    void __setattr__(const QString& key, QVariant value);
%MethodCode
    int fieldIdx = sipCpp->fieldNameIndex(*a0);
    QString altname = QString(*a0).replace("_"," ");
    int altfieldIdx = sipCpp->fieldNameIndex(altname);

    if (fieldIdx >= 0)
    {
        sipCpp->setAttribute(fieldIdx, *a1);
    }
    else if( altfieldIdx >= 0 )
    {
        sipCpp->setAttribute(altfieldIdx, *a1);
    }
    else
    {
        PyObject* key = PyString_FromString(a0->toStdString().c_str());
        PyObject* value = sipConvertFromType( a1, sipType_QVariant, sipSelf);
        PyObject_GenericSetAttr(sipSelf, key, value);
    }
%End

    void __delattr__(const QString& key);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
  else
    sipCpp->deleteAttribute(fieldIdx);
%End

    SIP_PYOBJECT __getitem__(int key);
%MethodCode
  const QgsAttributes& attrs = sipCpp->attributes();
  if (a0 < 0 || a0 >= attrs.count())
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
  else
  {
    QVariant* v = new QVariant(attrs[a0]);
    sipRes = sipConvertFromInstance(v, sipClass_QVariant, Py_None);
  }
%End

    SIP_PYOBJECT __getitem__(const QString& name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
  else
  {
    QVariant* v = new QVariant( sipCpp->attribute(fieldIdx) );
    sipRes = sipConvertFromInstance(v, sipClass_QVariant, Py_None);
  }
%End

    void __setitem__(int key, QVariant value);
%MethodCode
  sipCpp->setAttribute(a0, *a1);
%End

    void __setitem__(const QString& key, QVariant value);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
  else
  {
    sipCpp->setAttribute(fieldIdx, *a1);
  }
%End

    void __delitem__(int key);
%MethodCode
  if (a0 >= 0 && a0 < sipCpp->attributes().count())
    sipCpp->deleteAttribute(a0);
  else
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
%End

    void __delitem__(const QString& name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
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
    bool setAttribute( int field, const QVariant& attr );
    void initAttributes( int fieldCount );

    /**Deletes an attribute and its value*/
    void deleteAttribute( int field );

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
    QgsGeometry *geometry();

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

    /** Assign a field map with the feature to allow attribute access by
     * attribute name
     *
     *  @param fields         The attribute fields which this feature holds
     *  @param initAttributes If true, attributes are initialized. Clears any
     *  data previously assigned.
     *                        C++: Defaults to false
     *                        Python: Defaults to true
     *  @note added in 2.0
     */
    void setFields( const QgsFields* fields, bool initAttributes = true );

    /** Get associated field map. may be NULL
     *  @note added in 2.0
     */
    const QgsFields* fields() const;

    /** Insert a value into attribute. Returns false if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    bool setAttribute( const QString& name, QVariant value );

    /** Remove an attribute value. Returns false if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    bool deleteAttribute( const QString& name );

    /** Lookup attribute value from attribute name. Returns invalid variant if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    QVariant attribute( const QString& name ) const;

    /** Utility method to get attribute index from name. Returns -1 if field does not exist or field map is not associated.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    int fieldNameIndex( const QString& fieldName ) const;

}; // class QgsFeature

typedef QSet<QgsFeatureId> QgsFeatureIds;
