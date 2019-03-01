/***************************************************************************
                          qgsvectorlayerjoinbuffer.h
                          ---------------------------
    begin                : Feb 09, 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERJOINBUFFER_H
#define QGSVECTORLAYERJOINBUFFER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsfeaturesink.h"

#include <QHash>
#include <QString>


typedef QList< QgsVectorLayerJoinInfo > QgsVectorJoinList;


/**
 * \ingroup core
 * Manages joined fields for a vector layer*/
class CORE_EXPORT QgsVectorLayerJoinBuffer : public QObject, public QgsFeatureSink
{
    Q_OBJECT
  public:
    QgsVectorLayerJoinBuffer( QgsVectorLayer *layer = nullptr );

    /**
     * Joins another vector layer to this layer
      \param joinInfo join object containing join layer id, target and source field
      \returns (since 2.6) whether the join was successfully added
    */
    bool addJoin( const QgsVectorLayerJoinInfo &joinInfo );

    /**
     * Removes a vector layer join
      \returns true if join was found and successfully removed
    */
    bool removeJoin( const QString &joinLayerId );

    /**
     * Updates field map with joined attributes
      \param fields map to append joined attributes
     */
    void updateFields( QgsFields &fields );

    //! Calls cacheJoinLayer() for all vector joins
    void createJoinCaches();

    //! Saves mVectorJoins to xml under the layer node
    void writeXml( QDomNode &layer_node, QDomDocument &document ) const;

    /**
     * Reads joins from project file.
     * Does not resolve layer IDs to layers - call resolveReferences() afterwards
     */
    void readXml( const QDomNode &layer_node );

    /**
     * Resolves layer IDs of joined layers using given project's available layers
     * \since QGIS 3.0
     */
    void resolveReferences( QgsProject *project );

    //! Quick way to test if there is any join at all
    bool containsJoins() const { return !mVectorJoins.isEmpty(); }

    const QgsVectorJoinList &vectorJoins() const { return mVectorJoins; }

    /**
     * Finds the vector join for a layer field index.
     * \param index this layers attribute index
     * \param fields fields of the vector layer (including joined fields)
     * \param sourceFieldIndex Output: field's index in source layer
     * \returns the vector layer join info
     */
    const QgsVectorLayerJoinInfo *joinForFieldIndex( int index, const QgsFields &fields, int &sourceFieldIndex SIP_OUT ) const;

    /**
     * Find out what is the first index of the join within fields. Returns -1 if join is not present
     * \since QGIS 2.6
     */
    int joinedFieldsOffset( const QgsVectorLayerJoinInfo *info, const QgsFields &fields );

    /**
     * Returns a vector of indices for use in join based on field names from the layer
     * \since QGIS 2.6
     */
    static QVector<int> joinSubsetIndices( QgsVectorLayer *joinLayer, const QStringList &joinFieldsSubset );

    /**
     * Returns joins where the field of a target layer is considered as an id.
     * \param field the field of a target layer
     * \returns a list of vector joins
     * \since QGIS 3.0
     */
    QList<const QgsVectorLayerJoinInfo *> joinsWhereFieldIsId( const QgsField &field ) const;

    /**
     * Returns the joined feature corresponding to the feature.
     * \param info the vector join information
     * \param feature the feature of the target layer
     * \since QGIS 3.0
     */
    QgsFeature joinedFeatureOf( const QgsVectorLayerJoinInfo *info, const QgsFeature &feature ) const;

    /**
     * Returns the targeted feature corresponding to the joined feature.
     * \param info the vector join information
     * \param feature the feature of the joined layer
     * \since QGIS 3.0
     */
    QgsFeature targetedFeatureOf( const QgsVectorLayerJoinInfo *info, const QgsFeature &feature ) const;

    /**
     * Returns TRUE if the join information is about auxiliary layer, FALSE otherwise
     *
     * \param info The join information
     *
     * \returns TRUE if the join information is about auxiliary layer, FALSE otherwise
     *
     * \since QGIS 3.0
     */
    bool isAuxiliaryJoin( const QgsVectorLayerJoinInfo &info ) const;

    /**
     * Create a copy of the join buffer
     * \since QGIS 2.6
     */
    QgsVectorLayerJoinBuffer *clone() const SIP_FACTORY;

    /**
     * Adds a list of features in joined layers. Features given in parameter
     * are those added in target layer. If a corresponding joined feature yet
     * exists in a joined layer, then this feature is just updated. Note that
     * if a corresponding joined feature has only empty fields, then it's not
     * created nor added.
     *
     * \param features The list of features added in the target layer
     * \param flags Unused parameter
     *
     * \returns FALSE if an error happened, TRUE otherwise
     *
     * \since QGIS 3.0
     */
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) override;

    /**
     * Changes attribute value in joined layers. The feature id given in
     * parameter is the one added in target layer. If the corresponding joined
     * feature does not exist in a joined layer, then it's automatically
     * created if its fields are not empty.
     *
     * \param fid The feature id
     * \param field The field to update
     * \param newValue The new value of the attribute
     * \param oldValue The old value of the attribute
     *
     * \returns FALSE if an error happened, TRUE otherwise
     *
     * \since QGIS 3.0
     */
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    /**
     * Changes attributes' values in joined layers. The feature id given in
     * parameter is the one added in target layer. If the corresponding joined
     * feature does not exist in a joined layer, then it's automatically
     * created if its fields are not empty.
     *
     * \param fid The feature id
     * \param newValues The new values for attributes
     * \param oldValues The old values for attributes
     *
     * \returns FALSE if an error happened, TRUE otherwise
     *
     * \since QGIS 3.0
     */
    bool changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues = QgsAttributeMap() );

    /**
     * Deletes a feature from joined layers. The feature id given in
     * parameter is the one coming from the target layer.
     *
     * \param fid The feature id from the target layer to delete
     *
     * \returns FALSE if an error happened, TRUE otherwise
     *
     * \since QGIS 3.0
     */
    bool deleteFeature( QgsFeatureId fid ) const;

    /**
     * Deletes a list of features from joined layers. Feature ids given
     * in a parameter are those coming from the target layer.
     *
     * \param fids Feature ids from the target layer to delete
     *
     * \returns FALSE if an error happened, TRUE otherwise
     *
     * \since QGIS 3.0
     */
    bool deleteFeatures( const QgsFeatureIds &fids ) const;

  signals:

    /**
     * Emitted whenever the list of joined fields changes (e.g. added join or joined layer's fields change)
     * \since QGIS 2.6
     */
    void joinedFieldsChanged();

  private slots:
    void joinedLayerUpdatedFields();

    void joinedLayerModified();

    void joinedLayerWillBeDeleted();

  private:
    void connectJoinedLayer( QgsVectorLayer *vl );

  private:

    QgsVectorLayer *mLayer = nullptr;

    //! Joined vector layers
    QgsVectorJoinList mVectorJoins;

    //! Caches attributes of join layer in memory if QgsVectorJoinInfo.memoryCache is TRUE (and the cache is not already there)
    void cacheJoinLayer( QgsVectorLayerJoinInfo &joinInfo );

    //! Main mutex to protect most data members that can be modified concurrently
    QMutex mMutex;
};

#endif // QGSVECTORLAYERJOINBUFFER_H
