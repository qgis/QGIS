
typedef QMap<qint64, QgsFeature> QgsFeatureMap;

class QgsVectorLayerEditBuffer : QObject
{
%TypeHeaderCode
#include "qgsvectorlayereditbuffer.h"
%End

  public:
    QgsVectorLayerEditBuffer( QgsVectorLayer* layer );
    ~QgsVectorLayerEditBuffer();

    /** Returns true if the provider has been modified since the last commit */
    virtual bool isModified() const;


    /** Adds a feature
        @param f feature to add
        @return True in case of success and False in case of error
     */
    virtual bool addFeature( QgsFeature& f );

    /** Insert a copy of the given features into the layer  (but does not commit it) */
    virtual bool addFeatures( QgsFeatureList& features );

    /** Delete a feature from the layer (but does not commit it) */
    virtual bool deleteFeature( QgsFeatureId fid );

    /** Deletes a set of features from the layer (but does not commit it) */
    virtual bool deleteFeatures( const QgsFeatureIds& fid );

    /** Change feature's geometry */
    virtual bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );

    /** Changed an attribute value (but does not commit it) */
    virtual bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    /** Add an attribute field (but does not commit it)
        returns true if the field was added */
    virtual bool addAttribute( const QgsField &field );

    /** Delete an attribute field (but does not commit it) */
    virtual bool deleteAttribute( int attr );

    /** Renames an attribute field (but does not commit it)
     * @param attr attribute index
     * @param newName new name of field
     * @note added in QGIS 2.16
    */
    virtual bool renameAttribute( int attr, const QString& newName );

    /**
      Attempts to commit any changes to disk.  Returns the result of the attempt.
      If a commit fails, the in-memory changes are left alone.

      This allows editing to continue if the commit failed on e.g. a
      disallowed value in a Postgres database - the user can re-edit and try
      again.

      The commits occur in distinct stages,
      (add attributes, add features, change attribute values, change
      geometries, delete features, delete attributes)
      so if a stage fails, it's difficult to roll back cleanly.
      Therefore any error message also includes which stage failed so
      that the user has some chance of repairing the damage cleanly.
     */
    virtual bool commitChanges( QStringList& commitErrors );

    /** Stop editing and discard the edits */
    virtual void rollBack();

    /** Returns a map of new features which are not committed.
     * @see isFeatureAdded()
    */
    QgsFeatureMap addedFeatures() const;

    /** Returns true if the specified feature ID has been added but not committed.
     * @param id feature ID
     * @note added in QGIS 3.0
     * @see addedFeatures()
     */
    bool isFeatureAdded( QgsFeatureId id ) const;

    /** Returns a map of features with changed attributes values which are not committed.
     * @see isFeatureAttributesChanged()
    */
    QgsChangedAttributesMap changedAttributeValues() const;

    /** Returns true if the specified feature ID has had an attribute changed but not committed.
     * @param id feature ID
     * @note added in QGIS 3.0
     * @see changedAttributeValues()
     */
    bool isFeatureAttributesChanged( QgsFeatureId id ) const;

    /** Returns a list of deleted attributes fields which are not committed. The list is kept sorted.
     * @see isAttributeDeleted()
    */
    QgsAttributeList deletedAttributeIds() const;

    /** Returns true if the specified attribute has been deleted but not committed.
     * @param index attribute index
     * @note added in QGIS 3.0
     * @see deletedAttributeIds()
     */
    bool isAttributeDeleted( int index ) const;

    /** Returns a list of added attributes fields which are not committed */
    QList<QgsField> addedAttributes() const;

    /** Returns a map of features with changed geometries which are not committed.
     * @see hasFeatureGeometryChange()
     */
    QgsGeometryMap changedGeometries() const;

    /** Returns true if the specified feature ID has had its geometry changed but not committed.
     * @param id feature ID
     * @note added in QGIS 3.0
     * @see changedGeometries()
     */
    bool isFeatureGeometryChanged( QgsFeatureId id ) const;

    /** Returns a list of deleted feature IDs which are not committed.
     * @see isFeatureDeleted()
    */
    QgsFeatureIds deletedFeatureIds() const;

    /** Returns true if the specified feature ID has been deleted but not committed.
     * @param id feature ID
     * @note added in QGIS 3.0
     * @see deletedFeatureIds()
     */
    bool isFeatureDeleted( QgsFeatureId id ) const;


    //QString dumpEditBuffer();

  protected slots:
    void undoIndexChanged( int index );

  signals:
    /** This signal is emitted when modifications has been done on layer */
    void layerModified();

    void featureAdded( QgsFeatureId fid );
    void featureDeleted( QgsFeatureId fid );
    void geometryChanged( QgsFeatureId fid, QgsGeometry &geom );
    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );
    void attributeAdded( int idx );
    void attributeDeleted( int idx );

    /** Emitted when an attribute has been renamed
     * @param idx attribute index
     * @param newName new attribute name
     * @note added in QGSI 2.16
     */
    void attributeRenamed( int idx, const QString& newName );

    /** Signals emitted after committing changes */
    void committedAttributesDeleted( const QString& layerId, const QgsAttributeList& deletedAttributes );
    void committedAttributesAdded( const QString& layerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& layerId, const QgsFeatureList& addedFeatures );

    /** Emitted after committing an attribute rename
     * @param layerId ID of layer
     * @param renamedAttributes map of field index to new name
     * @note added in QGIS 2.16
     */
    void committedAttributesRenamed( const QString& layerId, const QgsFieldNameMap& renamedAttributes );

    void committedFeaturesRemoved( const QString& layerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& layerId, const QgsChangedAttributesMap& changedAttributesValues );
    void committedGeometriesChanges( const QString& layerId, const QgsGeometryMap& changedGeometries );

  protected:

    void updateFields( QgsFields& fields );

    /** Update feature with uncommitted geometry updates */
    void updateFeatureGeometry( QgsFeature &f );

    /** Update feature with uncommitted attribute updates */
    void updateChangedAttributes( QgsFeature &f );

    /** Update added and changed features after addition of an attribute */
    void handleAttributeAdded( int index );

    /** Update added and changed features after removal of an attribute */
    void handleAttributeDeleted( int index );


    /** Updates an index in an attribute map to a new value (for updates of changed attributes) */
    void updateAttributeMapIndex( QgsAttributeMap& attrs, int index, int offset ) const;

    void updateLayerFields();
};
