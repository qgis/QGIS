/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayereditbuffer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsVectorLayerEditBuffer : QObject
{

%TypeHeaderCode
#include "qgsvectorlayereditbuffer.h"
%End
  public:
    QgsVectorLayerEditBuffer( QgsVectorLayer *layer );

    virtual bool isModified() const;
%Docstring
Returns true if the provider has been modified since the last commit
%End


    virtual bool addFeature( QgsFeature &f );
%Docstring
Adds a feature

:param f: feature to add

:return: True in case of success and False in case of error
%End

    virtual bool addFeatures( QgsFeatureList &features );
%Docstring
Insert a copy of the given features into the layer  (but does not commit it)
%End

    virtual bool deleteFeature( QgsFeatureId fid );
%Docstring
Delete a feature from the layer (but does not commit it)
%End

    virtual bool deleteFeatures( const QgsFeatureIds &fid );
%Docstring
Deletes a set of features from the layer (but does not commit it)
%End

    virtual bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geom );
%Docstring
Change feature's geometry
%End

    virtual bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );
%Docstring
Changed an attribute value (but does not commit it)
%End

    virtual bool addAttribute( const QgsField &field );
%Docstring
Add an attribute field (but does not commit it)
returns true if the field was added *
%End

    virtual bool deleteAttribute( int attr );
%Docstring
Delete an attribute field (but does not commit it)
%End

    virtual bool renameAttribute( int attr, const QString &newName );
%Docstring
Renames an attribute field (but does not commit it)

:param attr: attribute index
:param newName: new name of field

.. versionadded:: 2.16
%End

    virtual bool commitChanges( QStringList &commitErrors );
%Docstring
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
%End

    virtual void rollBack();
%Docstring
Stop editing and discard the edits
%End

    QgsFeatureMap addedFeatures() const;
%Docstring
Returns a map of new features which are not committed.

.. seealso:: :py:func:`isFeatureAdded`
%End

    bool isFeatureAdded( QgsFeatureId id ) const;
%Docstring
Returns true if the specified feature ID has been added but not committed.

:param id: feature ID

.. versionadded:: 3.0

.. seealso:: :py:func:`addedFeatures`
%End

    QgsChangedAttributesMap changedAttributeValues() const;
%Docstring
Returns a map of features with changed attributes values which are not committed.

.. seealso:: :py:func:`isFeatureAttributesChanged`
%End

    bool isFeatureAttributesChanged( QgsFeatureId id ) const;
%Docstring
Returns true if the specified feature ID has had an attribute changed but not committed.

:param id: feature ID

.. versionadded:: 3.0

.. seealso:: :py:func:`changedAttributeValues`
%End

    QgsAttributeList deletedAttributeIds() const;
%Docstring
Returns a list of deleted attributes fields which are not committed. The list is kept sorted.

.. seealso:: :py:func:`isAttributeDeleted`
%End

    bool isAttributeDeleted( int index ) const;
%Docstring
Returns true if the specified attribute has been deleted but not committed.

:param index: attribute index

.. versionadded:: 3.0

.. seealso:: :py:func:`deletedAttributeIds`
%End

    QList<QgsField> addedAttributes() const;
%Docstring
Returns a list of added attributes fields which are not committed.
%End

    QgsGeometryMap changedGeometries() const;
%Docstring
Returns a map of features with changed geometries which are not committed.

.. seealso:: :py:func:`hasFeatureGeometryChange`
%End

    bool isFeatureGeometryChanged( QgsFeatureId id ) const;
%Docstring
Returns true if the specified feature ID has had its geometry changed but not committed.

:param id: feature ID

.. versionadded:: 3.0

.. seealso:: :py:func:`changedGeometries`
%End

    QgsFeatureIds deletedFeatureIds() const;
%Docstring
Returns a list of deleted feature IDs which are not committed.

.. seealso:: :py:func:`isFeatureDeleted`
%End

    bool isFeatureDeleted( QgsFeatureId id ) const;
%Docstring
Returns true if the specified feature ID has been deleted but not committed.

:param id: feature ID

.. versionadded:: 3.0

.. seealso:: :py:func:`deletedFeatureIds`
%End


  protected slots:
    void undoIndexChanged( int index );

  signals:
    void layerModified();
%Docstring
This signal is emitted when modifications has been done on layer
%End

    void featureAdded( QgsFeatureId fid );
    void featureDeleted( QgsFeatureId fid );

    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geom );
%Docstring
Emitted when a feature's geometry is changed.

:param fid: feature ID
:param geom: new feature geometry
%End

    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );
    void attributeAdded( int idx );
    void attributeDeleted( int idx );

    void attributeRenamed( int idx, const QString &newName );
%Docstring
Emitted when an attribute has been renamed

:param idx: attribute index
:param newName: new attribute name

.. versionadded:: 2.16
%End

    void committedAttributesDeleted( const QString &layerId, const QgsAttributeList &deletedAttributes );
%Docstring
Signals emitted after committing changes
%End
    void committedAttributesAdded( const QString &layerId, const QList<QgsField> &addedAttributes );

    void committedAttributesRenamed( const QString &layerId, const QgsFieldNameMap &renamedAttributes );
%Docstring
Emitted after committing an attribute rename

:param layerId: ID of layer
:param renamedAttributes: map of field index to new name

.. versionadded:: 2.16
%End
    void committedFeaturesAdded( const QString &layerId, const QgsFeatureList &addedFeatures );
    void committedFeaturesRemoved( const QString &layerId, const QgsFeatureIds &deletedFeatureIds );
    void committedAttributeValuesChanges( const QString &layerId, const QgsChangedAttributesMap &changedAttributesValues );
    void committedGeometriesChanges( const QString &layerId, const QgsGeometryMap &changedGeometries );

  protected:

    QgsVectorLayerEditBuffer();
%Docstring
Constructor for QgsVectorLayerEditBuffer
%End

    void updateFields( QgsFields &fields );

    void updateFeatureGeometry( QgsFeature &f );
%Docstring
Update feature with uncommitted geometry updates
%End

    void updateChangedAttributes( QgsFeature &f );
%Docstring
Update feature with uncommitted attribute updates
%End

    void handleAttributeAdded( int index );
%Docstring
Update added and changed features after addition of an attribute
%End

    void handleAttributeDeleted( int index );
%Docstring
Update added and changed features after removal of an attribute
%End

    void updateAttributeMapIndex( QgsAttributeMap &attrs, int index, int offset ) const;
%Docstring
Updates an index in an attribute map to a new value (for updates of changed attributes)
%End

    void updateLayerFields();

  protected:










};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayereditbuffer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
