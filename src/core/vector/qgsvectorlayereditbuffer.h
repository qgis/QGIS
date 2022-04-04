/***************************************************************************
    qgsvectorlayereditbuffer.h
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYEREDITBUFFER_H
#define QGSVECTORLAYEREDITBUFFER_H

#include "qgis_core.h"
#include <QList>
#include <QSet>

#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"

class QgsVectorLayer;
class QgsVectorLayerEditBufferGroup;

typedef QList<int> QgsAttributeList SIP_SKIP;
typedef QSet<int> QgsAttributeIds SIP_SKIP;
typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

/**
 * \ingroup core
 * \class QgsVectorLayerEditBuffer
 */
class CORE_EXPORT QgsVectorLayerEditBuffer : public QObject
{
    Q_OBJECT
  public:
    QgsVectorLayerEditBuffer( QgsVectorLayer *layer );

    //! Returns TRUE if the provider has been modified since the last commit
    virtual bool isModified() const;

    /**
     * Adds a feature
     * \param f feature to add
     * \returns TRUE in case of success and FALSE in case of error
     */
    virtual bool addFeature( QgsFeature &f );

    //! Insert a copy of the given features into the layer  (but does not commit it)
    virtual bool addFeatures( QgsFeatureList &features );

    //! Delete a feature from the layer (but does not commit it)
    virtual bool deleteFeature( QgsFeatureId fid );

    //! Deletes a set of features from the layer (but does not commit it)
    virtual bool deleteFeatures( const QgsFeatureIds &fid );

    //! Change feature's geometry
    virtual bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geom );

    //! Changed an attribute value (but does not commit it)
    virtual bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    /**
     * Changes values of attributes (but does not commit it).
     * \returns TRUE if attributes are well updated, FALSE otherwise
     * \since QGIS 3.0
     */
    virtual bool changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues );

    /**
     * Adds an attribute field (but does not commit it)
     * returns TRUE if the field was added
    */
    virtual bool addAttribute( const QgsField &field );

    //! Deletes an attribute field (but does not commit it)
    virtual bool deleteAttribute( int attr );

    /**
     * Renames an attribute field (but does not commit it)
     * \param attr attribute index
     * \param newName new name of field
     * \since QGIS 2.16
    */
    virtual bool renameAttribute( int attr, const QString &newName );

    /**
     * Attempts to commit any changes to disk.  Returns the result of the attempt.
     * If a commit fails, the in-memory changes are left alone.
     *
     * This allows editing to continue if the commit failed on e.g. a
     * disallowed value in a Postgres database - the user can re-edit and try
     * again.
     *
     * The commits occur in distinct stages,
     * (add attributes, add features, change attribute values, change
     * geometries, delete features, delete attributes)
     * so if a stage fails, it's difficult to roll back cleanly.
     * Therefore any error message also includes which stage failed so
     * that the user has some chance of repairing the damage cleanly.
     */
    virtual bool commitChanges( QStringList &commitErrors );

    //! Stop editing and discard the edits
    virtual void rollBack();

    /**
     * Returns a map of new features which are not committed.
     * \see isFeatureAdded()
    */
    QgsFeatureMap addedFeatures() const { return mAddedFeatures; }

    /**
     * Returns a list of the features IDs for all newly added or edited features
     * in the buffer.
     *
     * \since QGIS 3.20
     */
    QgsFeatureIds allAddedOrEditedFeatures() const;

    /**
     * Returns TRUE if the specified feature ID has been added but not committed.
     * \param id feature ID
     * \see addedFeatures()
     * \since QGIS 3.0
     */
    bool isFeatureAdded( QgsFeatureId id ) const { return mAddedFeatures.contains( id ); }

    /**
     * Returns a map of features with changed attributes values which are not committed.
     * \see isFeatureAttributesChanged()
    */
    QgsChangedAttributesMap changedAttributeValues() const { return mChangedAttributeValues; }

    /**
     * Returns TRUE if the specified feature ID has had an attribute changed but not committed.
     * \param id feature ID
     * \see changedAttributeValues()
     * \since QGIS 3.0
     */
    bool isFeatureAttributesChanged( QgsFeatureId id ) const { return mChangedAttributeValues.contains( id ); }

    /**
     * Returns a list of deleted attributes fields which are not committed. The list is kept sorted.
     * \see isAttributeDeleted()
    */
    QgsAttributeList deletedAttributeIds() const { return mDeletedAttributeIds; }

    /**
     * Returns TRUE if the specified attribute has been deleted but not committed.
     * \param index attribute index
     * \see deletedAttributeIds()
     * \since QGIS 3.0
     */
    bool isAttributeDeleted( int index ) const { return mDeletedAttributeIds.contains( index ); }

    /**
     * Returns a list of added attributes fields which are not committed.
     */
    QList<QgsField> addedAttributes() const { return mAddedAttributes; }

    /**
     * Returns a map of features with changed geometries which are not committed.
     * \see isFeatureGeometryChanged()
     */
    QgsGeometryMap changedGeometries() const { return mChangedGeometries; }

    /**
     * Returns TRUE if the specified feature ID has had its geometry changed but not committed.
     * \param id feature ID
     * \see changedGeometries()
     * \since QGIS 3.0
     */
    bool isFeatureGeometryChanged( QgsFeatureId id ) const { return mChangedGeometries.contains( id ); }

    /**
     * Returns a list of deleted feature IDs which are not committed.
     * \see isFeatureDeleted()
    */
    QgsFeatureIds deletedFeatureIds() const { return mDeletedFeatureIds; }

    /**
     * Returns TRUE if the specified feature ID has been deleted but not committed.
     * \param id feature ID
     * \see deletedFeatureIds()
     * \since QGIS 3.0
     */
    bool isFeatureDeleted( QgsFeatureId id ) const { return mDeletedFeatureIds.contains( id ); }

    /**
     * Updates \a fields
     * \note Not available in Python bindings
     * \since QGIS 3.18
     */
    void updateFields( QgsFields &fields ) SIP_SKIP;

    /**
     * Returns the parent edit buffer group for this edit buffer, or nullptr if not part of a group.
     * \since QGIS 3.26
     */
    QgsVectorLayerEditBufferGroup *editBufferGroup() const;

    /**
     * Set the parent edit buffer group for this edit buffer.
     * \since QGIS 3.26
     */
    void setEditBufferGroup( QgsVectorLayerEditBufferGroup *editBufferGroup );

    //QString dumpEditBuffer();

  protected slots:
    void undoIndexChanged( int index );

  signals:
    //! Emitted when modifications has been done on layer
    void layerModified();

    void featureAdded( QgsFeatureId fid );
    void featureDeleted( QgsFeatureId fid );

    /**
     * Emitted when a feature's geometry is changed.
     * \param fid feature ID
     * \param geom new feature geometry
     */
    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geom );

    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );
    void attributeAdded( int idx );
    void attributeDeleted( int idx );

    /**
     * Emitted when an attribute has been renamed
     * \param idx attribute index
     * \param newName new attribute name
     * \since QGIS 2.16
     */
    void attributeRenamed( int idx, const QString &newName );

    //! Signals emitted after committing changes
    void committedAttributesDeleted( const QString &layerId, const QgsAttributeList &deletedAttributes );
    void committedAttributesAdded( const QString &layerId, const QList<QgsField> &addedAttributes );

    /**
     * Emitted after committing an attribute rename
     * \param layerId ID of layer
     * \param renamedAttributes map of field index to new name
     * \since QGIS 2.16
     */
    void committedAttributesRenamed( const QString &layerId, const QgsFieldNameMap &renamedAttributes );
    void committedFeaturesAdded( const QString &layerId, const QgsFeatureList &addedFeatures );
    void committedFeaturesRemoved( const QString &layerId, const QgsFeatureIds &deletedFeatureIds );
    void committedAttributeValuesChanges( const QString &layerId, const QgsChangedAttributesMap &changedAttributesValues );
    void committedGeometriesChanges( const QString &layerId, const QgsGeometryMap &changedGeometries );

  protected:

    //! Constructor for QgsVectorLayerEditBuffer
    QgsVectorLayerEditBuffer() = default;

    //! Update feature with uncommitted geometry updates
    void updateFeatureGeometry( QgsFeature &f );

    //! Update feature with uncommitted attribute updates
    void updateChangedAttributes( QgsFeature &f );

    //! Update added and changed features after addition of an attribute
    void handleAttributeAdded( int index );

    //! Update added and changed features after removal of an attribute
    void handleAttributeDeleted( int index );

    //! Updates an index in an attribute map to a new value (for updates of changed attributes)
    void updateAttributeMapIndex( QgsAttributeMap &attrs, int index, int offset ) const;

    void updateLayerFields();

  protected:
    QgsVectorLayer *L = nullptr;

    friend class QgsVectorLayerUndoCommand;
    friend class QgsVectorLayerUndoCommandAddFeature;
    friend class QgsVectorLayerUndoCommandDeleteFeature;
    friend class QgsVectorLayerUndoCommandChangeGeometry;
    friend class QgsVectorLayerUndoCommandChangeAttribute;
    friend class QgsVectorLayerUndoCommandAddAttribute;
    friend class QgsVectorLayerUndoCommandDeleteAttribute;
    friend class QgsVectorLayerUndoCommandRenameAttribute;

    friend class QgsVectorLayerUndoPassthroughCommand;
    friend class QgsVectorLayerUndoPassthroughCommandAddFeatures;
    friend class QgsVectorLayerUndoPassthroughCommandDeleteFeatures;
    friend class QgsVectorLayerUndoPassthroughCommandChangeGeometry;
    friend class QgsVectorLayerUndoPassthroughCommandChangeAttribute;
    friend class QgsVectorLayerUndoPassthroughCommandChangeAttributes;
    friend class QgsVectorLayerUndoPassthroughCommandAddAttribute;
    friend class QgsVectorLayerUndoPassthroughCommandDeleteAttribute;
    friend class QgsVectorLayerUndoPassthroughCommandRenameAttribute;
    friend class QgsVectorLayerUndoPassthroughCommandUpdate;

    /**
     * Deleted feature IDs which are not committed.  Note a feature can be added and then deleted
     * again before the change is committed - in that case the added feature would be removed
     * from mAddedFeatures only and *not* entered here.
     */
    QgsFeatureIds mDeletedFeatureIds;

    //! New features which are not committed.
    QgsFeatureMap mAddedFeatures;

    //! Changed attributes values which are not committed
    QgsChangedAttributesMap mChangedAttributeValues;

    //! Deleted attributes fields which are not committed. The list is kept sorted.
    QgsAttributeList mDeletedAttributeIds;

    //! Added attributes fields which are not committed
    QList<QgsField> mAddedAttributes;

    //! Renamed attributes which are not committed.
    QgsFieldNameMap mRenamedAttributes;

    //! Changed geometries which are not committed.
    QgsGeometryMap mChangedGeometries;

    QgsVectorLayerEditBufferGroup *mEditBufferGroup = nullptr;

    friend class QgsGrassProvider; //GRASS provider totally abuses the edit buffer

  private:

    friend class QgsVectorLayerEditBufferGroup;

    /**
     * Check geometry of added features for compatibility with data provider
     * \param commitErrors will be extended in case of error
     * \returns TRUE if compatible
     */
    bool commitChangesCheckGeometryTypeCompatibility( QStringList &commitErrors );

    /**
     * Delete attributes
     * \param attributesDeleted is set if one or more attributes where deleted
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesDeleteAttributes( bool &attributesDeleted, QStringList &commitErrors );

    /**
     * Rename attributes
     * \param attributesRenamed is set if one or more attributes where renamed
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesRenameAttributes( bool &attributesRenamed, QStringList &commitErrors );

    /**
     * Add new attributes
     * \param attributesAdded is set if one or more attributes where added
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesAddAttributes( bool &attributesAdded, QStringList &commitErrors );

    /**
     * Check that delete/rename/add attributes operations where successful
     * \param oldFields are the fields before delete/rename/add operations
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesCheckAttributesModifications( const QgsFields oldFields, QStringList &commitErrors );

    /**
     * Change attributes
     * \param attributesChanged is set if one or more attributes where changed
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesChangeAttributes( bool &attributesChanged, QStringList &commitErrors );

    /**
     * Delete features
     * \param featuresDeleted is set if one or more features where deleted
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesDeleteFeatures( bool &featuresDeleted, QStringList &commitErrors );

    /**
     * Add new features
     * \param featuresAdded is set if one or more features where deleted
     * \param commitErrors will be extended in case of error
     * \returns TRUE on success
     */
    bool commitChangesAddFeatures( bool &featuresAdded, QStringList &commitErrors );
};

#endif // QGSVECTORLAYEREDITBUFFER_H
