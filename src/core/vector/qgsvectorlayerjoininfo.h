/***************************************************************************
    qgsvectorlayerjoininfo.h
    ---------------------
    begin                : January 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYERJOININFO_H
#define QGSVECTORLAYERJOININFO_H

#include <QHash>
#include <QString>
#include <QStringList>

#include "qgsfeature.h"

#include "qgsvectorlayerref.h"

/**
 * \ingroup core
 * \brief Defines left outer join from our vector layer to some other vector layer.
 * The join is done based on [our layer].targetField = [join layer].joinField
 *
 */
class CORE_EXPORT QgsVectorLayerJoinInfo
{
  public:

    QgsVectorLayerJoinInfo() = default;

    //! Sets weak reference to the joined layer
    void setJoinLayer( QgsVectorLayer *layer ) { mJoinLayerRef = QgsVectorLayerRef( layer ); }
    //! Returns joined layer (may be NULLPTR if the reference was set by layer ID and not resolved yet)
    QgsVectorLayer *joinLayer() const { return mJoinLayerRef.get(); }

    //! Sets ID of the joined layer. It will need to be overwritten by setJoinLayer() to a reference to real layer
    void setJoinLayerId( const QString &layerId ) { mJoinLayerRef = QgsVectorLayerRef( layerId ); }
    //! ID of the joined layer - may be used to resolve reference to the joined layer
    QString joinLayerId() const { return mJoinLayerRef.layerId; }

    //! Sets name of the field of our layer that will be used for join
    void setTargetFieldName( const QString &fieldName ) { mTargetFieldName = fieldName; }
    //! Returns name of the field of our layer that will be used for join
    QString targetFieldName() const { return mTargetFieldName; }

    //! Sets name of the field of joined layer that will be used for join
    void setJoinFieldName( const QString &fieldName ) { mJoinFieldName = fieldName; }
    //! Returns name of the field of joined layer that will be used for join
    QString joinFieldName() const { return mJoinFieldName; }

    //! Sets prefix of fields from the joined layer. If NULLPTR, joined layer's name will be used.
    void setPrefix( const QString &prefix ) { mPrefix = prefix; }
    //! Returns prefix of fields from the joined layer. If NULLPTR, joined layer's name will be used.
    QString prefix() const { return mPrefix; }

    //! Sets whether values from the joined layer should be cached in memory to speed up lookups
    void setUsingMemoryCache( bool enabled );

    /**
     * Returns whether values from the joined layer should be cached in memory to speed up lookups.
     * Will return FALSE if upsertOnEdit is enabled.
     */
    bool isUsingMemoryCache() const;

    /**
     * Returns whether the form has to be dynamically updated with joined fields
     *  when  a feature is being created in the target layer.
     */
    bool isDynamicFormEnabled() const { return mDynamicForm; }

    /**
     * Sets whether the form has to be dynamically updated with joined fields
     *  when a feature is being created in the target layer.
     */
    void setDynamicFormEnabled( bool enabled ) { mDynamicForm = enabled; }

    /**
     * Returns whether joined fields may be edited through the form of
     *  the target layer.
     */
    bool isEditable() const { return mEditable; }

    /**
     * Sets whether the form of the target layer allows editing joined fields.
     */
    void setEditable( bool enabled );

    /**
     * Returns whether a feature created on the target layer has to impact
     *  the joined layer by creating a new feature if necessary.
     */
    bool hasUpsertOnEdit() const { return mUpsertOnEdit; }

    /**
     * Sets whether a feature created on the target layer has to impact
     *  the joined layer by creating a new feature if necessary.
     */
    void setUpsertOnEdit( bool enabled ) { mUpsertOnEdit = enabled; }

    /**
     * Returns whether a feature deleted on the target layer has to impact the
     *  joined layer by deleting the corresponding joined feature.
     */
    bool hasCascadedDelete() const { return mCascadedDelete; }

    /**
     * Sets whether a feature deleted on the target layer has to impact the
     *  joined layer by deleting the corresponding joined feature.
     */
    void setCascadedDelete( bool enabled ) { mCascadedDelete = enabled; }

    /**
     * Returns the prefixed name of the field.
     * \param field the field
     * \returns the prefixed name of the field
     */
    QString prefixedFieldName( const QgsField &field ) const;

    /**
     * Extract the join feature from the target feature for the current
     *  join layer information.
     * \param feature A feature from the target layer
     * \returns the corresponding joined feature
     */
    QgsFeature extractJoinedFeature( const QgsFeature &feature ) const;

    /**
     * Sets a list of fields to ignore whatever happens.
     *
     * \deprecated use setJoinFieldNamesBlockList() instead
     */
    Q_DECL_DEPRECATED void setJoinFieldNamesBlackList( const QStringList &blackList ) SIP_DEPRECATED { mBlockList = blackList; }

    /**
     * Returns the list of fields to ignore.
     *
     * \deprecated use joinFieldNamesBlockList() instead
     */
    Q_DECL_DEPRECATED QStringList joinFieldNamesBlackList() const SIP_DEPRECATED { return mBlockList; }

    /**
     * Sets a \a list of fields to ignore whatever happens.
     *
     * \see joinFieldNamesBlockList()
     * \since QGIS 3.14
     */
    void setJoinFieldNamesBlockList( const QStringList &list ) { mBlockList = list; }

    /**
     * Returns the list of fields to ignore.
     *
     * \see setJoinFieldNamesBlockList()
     * \since QGIS 3.14
     */
    QStringList joinFieldNamesBlockList() const { return mBlockList; }

    /**
     * Returns TRUE if blocklisted fields is not empty or if a subset of names
     * has been set.
     *
     */
    bool hasSubset( bool blocklisted = true ) const;

    /**
     * Returns the list of field names to use for joining considering
     * blocklisted fields and subset.
     *
     * \warning This method is NOT thread safe, and MUST be called from the thread where the vector layers
     * participating in the join reside. See variant which accepts a QgsFields argument for a thread safe alternative.
     *
     */
    static QStringList joinFieldNamesSubset( const QgsVectorLayerJoinInfo &info, bool blocklisted = true );

    /**
     * Returns the list of field names to use for joining considering
     * blocklisted fields and subset.
     *
     * This method is thread safe.
     *
     * \since QGIS 3.30
     */
    static QStringList joinFieldNamesSubset( const QgsVectorLayerJoinInfo &info, const QgsFields &joinLayerFields, bool blocklisted = true );

    bool operator==( const QgsVectorLayerJoinInfo &other ) const
    {
      return mTargetFieldName == other.mTargetFieldName &&
             mJoinLayerRef.layerId == other.mJoinLayerRef.layerId &&
             mJoinFieldName == other.mJoinFieldName &&
             mJoinFieldsSubset == other.mJoinFieldsSubset &&
             mMemoryCache == other.mMemoryCache &&
             mPrefix == other.mPrefix;
    }

    /**
     * Sets the subset of fields to be used from joined layer.
     *
     * Ownership of \a fileNamesSubset is transferred. A \a fieldNameSubset of NULLPTR indicates that all fields should be used.
     *
     * \see joinFieldNamesSubset()
    */
    void setJoinFieldNamesSubset( QStringList *fieldNamesSubset SIP_TRANSFER ) { mJoinFieldsSubset = std::shared_ptr<QStringList>( fieldNamesSubset ); }

    /**
     * Returns the subset of fields to be used from joined layer.
     *
     * All fields will be used if NULLPTR is returned.
     *
     * \see setJoinFieldNamesSubset()
     *
    */
    QStringList *joinFieldNamesSubset() const { return mJoinFieldsSubset.get(); }

  protected:
    //! Join field in the target layer
    QString mTargetFieldName;
    //! Weak reference to the joined layer
    QgsVectorLayerRef mJoinLayerRef;
    //! Join field in the source layer
    QString mJoinFieldName;

    /**
     * An optional prefix. If it is a Null string "{layername}_" will be used
     */
    QString mPrefix;

    //! True if the join is cached in virtual memory
    bool mMemoryCache = false;

    //! Subset of fields to use from joined layer. NULLPTR = use all fields
    std::shared_ptr<QStringList> mJoinFieldsSubset;

    // caching support

    friend class QgsVectorLayerJoinBuffer;
    friend class QgsVectorLayerFeatureIterator;

    //! True if the cached join attributes need to be updated
    bool cacheDirty = true;

    bool mDynamicForm = false;

    bool mEditable = false;

    bool mUpsertOnEdit = false;

    bool mCascadedDelete = false;

    QStringList mBlockList;

    //! Cache for joined attributes to provide fast lookup (size is 0 if no memory caching)
    QHash< QString, QgsAttributes> cachedAttributes;

};


#endif // QGSVECTORLAYERJOININFO_H
