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

#include <QList>
#include <QSet>

#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsrectangle.h"
#include "qgssnapper.h"

class QgsVectorLayer;

typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;
typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class CORE_EXPORT QgsVectorLayerEditBuffer : public QObject
{
    Q_OBJECT
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

    /** delete a feature from the layer (but does not commit it) */
    virtual bool deleteFeature( QgsFeatureId fid );

    /** change feature's geometry */
    virtual bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );

    /** changed an attribute value (but does not commit it) */
    virtual bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    /** add an attribute field (but does not commit it)
        returns true if the field was added */
    virtual bool addAttribute( const QgsField &field );

    /** delete an attribute field (but does not commit it) */
    virtual bool deleteAttribute( int attr );


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



    /** New features which are not commited. */
    inline const QgsFeatureMap& addedFeatures() { return mAddedFeatures; }

    /** Changed attributes values which are not commited */
    inline const QgsChangedAttributesMap& changedAttributeValues() { return mChangedAttributeValues; }

    /** deleted attributes fields which are not commited. The list is kept sorted. */
    inline const QgsAttributeList& deletedAttributeIds() { return mDeletedAttributeIds; }

    /** added attributes fields which are not commited */
    inline const QList<QgsField>& addedAttributes() { return mAddedAttributes; }

    /** Changed geometries which are not commited. */
    inline const QgsGeometryMap& changedGeometries() { return mChangedGeometries; }

    inline const QgsFeatureIds deletedFeatureIds() { return mDeletedFeatureIds; }
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

    /** Signals emitted after committing changes */
    void committedAttributesDeleted( const QString& layerId, const QgsAttributeList& deletedAttributes );
    void committedAttributesAdded( const QString& layerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& layerId, const QgsFeatureList& addedFeatures );
    void committedFeaturesRemoved( const QString& layerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& layerId, const QgsChangedAttributesMap& changedAttributesValues );
    void committedGeometriesChanges( const QString& layerId, const QgsGeometryMap& changedGeometries );

  protected:

    QgsVectorLayerEditBuffer() : L( NULL ) {}

    void updateFields( QgsFields& fields );

    /** Update feature with uncommited geometry updates */
    void updateFeatureGeometry( QgsFeature &f );

    /** Update feature with uncommited attribute updates */
    void updateChangedAttributes( QgsFeature &f );

    /** update added and changed features after addition of an attribute */
    void handleAttributeAdded( int index );

    /** update added and changed features after removal of an attribute */
    void handleAttributeDeleted( int index );


    /** Updates an index in an attribute map to a new value (for updates of changed attributes) */
    void updateAttributeMapIndex( QgsAttributeMap& attrs, int index, int offset ) const;

    void updateLayerFields();

  protected:
    QgsVectorLayer* L;
    friend class QgsVectorLayer;

    friend class QgsVectorLayerUndoCommand;
    friend class QgsVectorLayerUndoCommandAddFeature;
    friend class QgsVectorLayerUndoCommandDeleteFeature;
    friend class QgsVectorLayerUndoCommandChangeGeometry;
    friend class QgsVectorLayerUndoCommandChangeAttribute;
    friend class QgsVectorLayerUndoCommandAddAttribute;
    friend class QgsVectorLayerUndoCommandDeleteAttribute;

    /** Deleted feature IDs which are not commited.  Note a feature can be added and then deleted
        again before the change is committed - in that case the added feature would be removed
        from mAddedFeatures only and *not* entered here.
     */
    QgsFeatureIds mDeletedFeatureIds;

    /** New features which are not commited. */
    QgsFeatureMap mAddedFeatures;

    /** Changed attributes values which are not commited */
    QgsChangedAttributesMap mChangedAttributeValues;

    /** deleted attributes fields which are not commited. The list is kept sorted. */
    QgsAttributeList mDeletedAttributeIds;

    /** added attributes fields which are not commited */
    QList<QgsField> mAddedAttributes;

    /** Changed geometries which are not commited. */
    QgsGeometryMap mChangedGeometries;

};

#endif // QGSVECTORLAYEREDITBUFFER_H
