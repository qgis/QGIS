

// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

// key = feature id, value = changed attributes
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<qint64, QgsGeometry> QgsGeometryMap;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;


class QgsFeature
{
%TypeHeaderCode
#include <qgsfeature.h>
%End
    
  public:

    SIP_PYOBJECT __getitem__(int key);
%MethodCode
  const QgsAttributeMap& attrMap = sipCpp->attributeMap();
  QgsAttributeMap::const_iterator it = attrMap.find(a0);
  if (it == attrMap.end())
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
  else
  {
    QVariant* v = new QVariant(it.value());
    sipRes = sipConvertFromInstance(v, sipClass_QVariant, Py_None);
  }
%End

    void __setitem__(int key, QVariant value);
%MethodCode
  sipCpp->addAttribute(a0, *a1);
%End

    void __delitem__(int key);
%MethodCode
  if (sipCpp->attributeMap().contains(a0))
    sipCpp->deleteAttribute(a0);
  else
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
%End

    //! Constructor
    QgsFeature(qint64 id = 0, QString typeName = "" );

    /** copy ctor needed due to internal pointer */
    QgsFeature(const QgsFeature & rhs );

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
    void setFeatureId(qint64 id);


    /** returns the feature's type name
     */
    QString typeName() const;


    /** sets the feature's type name
     */
    void setTypeName(QString typeName);

    /**
     * Get the attributes for this feature.
     * @return A std::map containing the field name/value mapping
     */
    const QMap<int, QVariant> & attributeMap() const;


    /**Sets all the attributes in one go*/
    void setAttributeMap(const QMap<int, QVariant> & attributeMap);

    /** Clear attribute map
     * added in 1.5
     */
    void clearAttributeMap();

    /** 
     * Add an attribute to the map
     */
    void addAttribute(int field, QVariant attr);

    /**Deletes an attribute and its value*/
    void deleteAttribute(int field);

    /**Changes an existing attribute value
        @param field index of the field
        @param attr attribute name and value to be set */
    void changeAttribute(int field, QVariant attr);

    /**
     * Return the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     */
    bool isValid() const;

    /** 
     * Set the validity of the feature.
     */
    void setValid(bool validity);
    
    /**
     * Return the dirty state of this feature.
     * Dirty is set if (e.g.) the feature's geometry has been modified in-memory.
     */
    bool isDirty() const;

    /** 
     * Reset the dirtiness of the feature.  (i.e. make clean)
     * You would normally do this after it's saved to permanent storage (e.g. disk, an ACID-compliant database)
     */
    void clean();
    
    /**
     * Get the geometry object associated with this feature
     */
    QgsGeometry * geometry();
    
    /**
     * Get the geometry object associated with this feature
     * The caller assumes responsibility for the QgsGeometry*'s destruction.
     */
    QgsGeometry * geometryAndOwnership() /Factory/;
    
    /** Set this feature's geometry from another QgsGeometry object (deep copy)
     */
    void setGeometry(const QgsGeometry& geom);
    
    /** Set this feature's geometry (takes geometry ownership)
     */
    // use only the version that copies the geometry (it's safer)
    //void setGeometry(QgsGeometry* geom /Transfer/);
    
    /** 
     * Set this feature's geometry from WKB
     *
     * This feature assumes responsibility for destroying geom.
     */
    void setGeometryAndOwnership(unsigned char * geom /Transfer/, size_t length);
    
    
}; // class QgsFeature

