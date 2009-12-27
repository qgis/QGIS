

class QgsFeature
{
%TypeHeaderCode
#include <qgsfeature.h>
%End
    
  public:

    //! Constructor
    QgsFeature(int id = 0, QString typeName = "" );

    /** copy ctor needed due to internal pointer */
    QgsFeature(const QgsFeature & rhs );

    //! Destructor
    ~QgsFeature();

    
    /**
     * Get the feature id for this feature
     * @return Feature id
     */
    int id() const;

    /**
     * Set the feature id for this feature
     * @param id Feature id
     */
     void setFeatureId(int id);


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

