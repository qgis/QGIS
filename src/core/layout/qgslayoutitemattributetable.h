/***************************************************************************
                         qgslayoutitemattributetable.h
                         ---------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMATTRIBUTETABLE_H
#define QGSLAYOUTITEMATTRIBUTETABLE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayouttable.h"
#include "qgsvectorlayerref.h"

class QgsLayoutItemMap;
class QgsVectorLayer;

/**
 * \ingroup core
 * A layout table subclass that displays attributes from a vector layer.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutItemAttributeTable: public QgsLayoutTable
{
    Q_OBJECT

  public:

    /**
     * Specifies the content source for the attribute table
     */
    enum ContentSource
    {
      LayerAttributes = 0, //!< Table shows attributes from features in a vector layer
      AtlasFeature, //!< Table shows attributes from the current atlas feature
      RelationChildren //!< Table shows attributes from related child features
    };

    /**
     * Constructor for QgsLayoutItemAttributeTable, attached to the specified \a layout.
     *
     * Ownership is transferred to the layout.
     */
    QgsLayoutItemAttributeTable( QgsLayout *layout SIP_TRANSFERTHIS );

    int type() const override;
    QIcon icon() const override;
    QString displayName() const override;

    /**
     * Returns a new QgsLayoutItemAttributeTable for the specified parent \a layout.
     */
    static QgsLayoutItemAttributeTable *create( QgsLayout *layout ) SIP_FACTORY;

    /**
     * Sets the \a source for attributes to show in table body.
     * \see source()
     */
    void setSource( ContentSource source );

    /**
     * Returns the source for attributes shown in the table body.
     * \see setSource()
     */
    ContentSource source() const { return mSource; }

    /**
     * Returns the source layer for the table, considering the table source mode. For example,
     * if the table is set to atlas feature mode, then the source layer will be the
     * atlas coverage layer. If the table is set to layer attributes mode, then
     * the source layer will be the user specified vector layer.
     */
    QgsVectorLayer *sourceLayer() const;

    /**
     * Sets the vector \a layer from which to display feature attributes.
     *
     * This is only considered if the table source() is LayerAttributes.
     *
     * \see vectorLayer()
     */
    void setVectorLayer( QgsVectorLayer *layer );

    /**
     * Returns the vector layer the attribute table is currently using.
     *
     * This is only considered if the table source() is LayerAttributes.
     *
     * \see setVectorLayer()
     * \see sourceLayer()
     */
    QgsVectorLayer *vectorLayer() const { return mVectorLayer.get(); }

    /**
     * Sets the relation \a id from which to display child features
     * \see relationId()
     * \see setSource()
     * \note Only used if table source() is set to RelationChildren.
     */
    void setRelationId( const QString &id );

    /**
     * Returns the relation id which the table displays child features from.
     * \see setRelationId()
     * \see source()
     * \note Only used if table source() is set to RelationChildren.
     */
    QString relationId() const { return mRelationId; }

    /**
     * Resets the attribute table's columns to match the vector layer's fields.
     * \see setVectorLayer()
     */
    void resetColumns();

    /**
     * Sets a layout \a map to use to limit the extent of features shown in the
     * attribute table. This setting only has an effect if setDisplayOnlyVisibleFeatures is
     * set to TRUE. Changing the map forces the table to refetch features from its
     * vector layer, and may result in the table changing size to accommodate the new displayed
     * feature attributes.
     * \see map()
     * \see setDisplayOnlyVisibleFeatures
     */
    void setMap( QgsLayoutItemMap *map );

    /**
     * Returns the layout map whose extents are controlling the features shown in the
     * table. The extents of the map are only used if displayOnlyVisibleFeatures() is TRUE.
     * \see setMap()
     * \see displayOnlyVisibleFeatures()
     */
    QgsLayoutItemMap *map() const { return mMap; }

    /**
     * Sets the maximum number of \a features shown by the table. Changing this setting may result
     * in the attribute table changing its size to accommodate the new number of rows, and requires
     * the table to refetch features from its vector layer.
     * \see maximumNumberOfFeatures()
     */
    void setMaximumNumberOfFeatures( int features );

    /**
     * Returns the maximum number of features to be shown by the table.
     * \see setMaximumNumberOfFeatures()
     */
    int maximumNumberOfFeatures() const { return mMaximumNumberOfFeatures; }

    /**
     * Sets attribute table to only show unique rows.
     *
     * Set \a uniqueOnly to TRUE to show only unique rows. Duplicate rows
     * will be stripped from the table.
     *
     * \see uniqueRowsOnly()
     */
    void setUniqueRowsOnly( bool uniqueOnly );

    /**
     * Returns TRUE if the table is set to show only unique rows.
     *
     * \see setUniqueRowsOnly()
     */
    bool uniqueRowsOnly() const { return mShowUniqueRowsOnly; }

    /**
     * Sets the attribute table to only show features which are visible in a map item. Changing
     * this setting forces the table to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     *
     * \see displayOnlyVisibleFeatures()
     * \see setMap()
     */
    void setDisplayOnlyVisibleFeatures( bool visibleOnly );

    /**
     * Returns TRUE if the table is set to show only features visible on a corresponding
     * map item.
     *
     * \see map()
     * \see setDisplayOnlyVisibleFeatures()
     */
    bool displayOnlyVisibleFeatures() const { return mShowOnlyVisibleFeatures; }

    /**
     * Sets attribute table to only show features which intersect the current atlas
     * feature.
     *
     * \see filterToAtlasFeature()
     */
    void setFilterToAtlasFeature( bool filterToAtlas );

    /**
     * Returns TRUE if the table is set to only show features which intersect the current atlas
     * feature.
     *
     * \see setFilterToAtlasFeature()
     */
    bool filterToAtlasFeature() const { return mFilterToAtlasIntersection; }

    /**
     * Returns TRUE if a feature filter is active on the attribute table.
     *
     * \see setFilterFeatures()
     * \see featureFilter()
     */
    bool filterFeatures() const { return mFilterFeatures; }

    /**
     * Sets whether the feature filter is active for the attribute table. Changing
     * this setting forces the table to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * \see filterFeatures()
     * \see setFeatureFilter()
     */
    void setFilterFeatures( bool filter );

    /**
     * Returns the current expression used to filter features for the table. The filter is only
     * active if filterFeatures() is TRUE.
     *
     * \see setFeatureFilter()
     * \see filterFeatures()
     */
    QString featureFilter() const { return mFeatureFilter; }

    /**
     * Sets the \a expression used for filtering features in the table. The filter is only
     * active if filterFeatures() is set to TRUE. Changing this setting forces the table
     * to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     *
     * \see featureFilter()
     * \see setFilterFeatures()
     */
    void setFeatureFilter( const QString &expression );

    /**
     * Sets the attributes to display in the table.
     * \param fields list of fields names from the vector layer to show.
     * Set to an empty list to show all feature attributes.
     * \param refresh set to TRUE to force the table to refetch features from its vector layer
     * and immediately update the display of the table. This may result in the table changing size
     * to accommodate the new displayed feature attributes.
     */
    void setDisplayedFields( const QStringList &fields, bool refresh = true );

    /**
     * Returns the attributes used to sort the table's features.
     * \returns a QList of integer/bool pairs, where the integer refers to the attribute index and
     * the bool to the sort order for the attribute. If TRUE the attribute is sorted ascending,
     * if FALSE, the attribute is sorted in descending order.
     * \note not available in Python bindings
     */
    QVector< QPair<int, bool> > sortAttributes() const SIP_SKIP;

    /**
     * Sets a string to wrap the contents of the table cells by. Occurrences of this string will
     * be replaced by a line break.
     * \param wrapString string to replace with line break
     * \see wrapString()
     */
    void setWrapString( const QString &wrapString );

    /**
     * Returns the string used to wrap the contents of the table cells by. Occurrences of this string will
     * be replaced by a line break.
     * \see setWrapString()
     */
    QString wrapString() const { return mWrapString; }

    /**
     * Queries the attribute table's vector layer for attributes to show in the table.
     * \param contents table content
     * \returns TRUE if attributes were successfully fetched
     * \note not available in Python bindings
     */
    bool getTableContents( QgsLayoutTableContents &contents ) override SIP_SKIP;

    QgsExpressionContext createExpressionContext() const override;
    void finalizeRestoreFromXml() override;

    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties ) override;

  protected:

    bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;

  private:

    //! Attribute source
    ContentSource mSource = LayerAttributes;
    //! Associated vector layer
    QgsVectorLayerRef mVectorLayer;

    //! Data defined vector layer - only
    QPointer< QgsVectorLayer > mDataDefinedVectorLayer;

    //! Relation id, if in relation children mode
    QString mRelationId;

    //! Current vector layer, if in atlas feature mode
    QgsVectorLayer *mCurrentAtlasLayer = nullptr;

    //! Associated map (used to display the visible features)
    QgsLayoutItemMap *mMap = nullptr;
    QString mMapUuid;

    //! Maximum number of features that is displayed
    int mMaximumNumberOfFeatures = 30;

    //! True if only unique rows should be shown
    bool mShowUniqueRowsOnly = false;

    //! Shows only the features that are visible in the associated layout map (TRUE by default)
    bool mShowOnlyVisibleFeatures = false;

    //! Shows only the features that intersect the current atlas feature
    bool mFilterToAtlasIntersection = false;

    //! True if feature filtering enabled
    bool mFilterFeatures = false;

    //! Feature filter expression
    QString mFeatureFilter;

    QString mWrapString;

    /**
     * Returns a list of attribute indices corresponding to displayed fields in the table.
     * \note kept for compatibility with 2.0 api only
     */
    QList<int> fieldsToDisplay() const;

    /**
     * Restores a field alias map from a pre 2.4 format project file format
     * \param map QMap of integers to strings, where the string is the alias to use for the
     * corresponding field, and the integer is the field index from the vector layer
     */
    void restoreFieldAliasMap( const QMap<int, QString> &map );

    /**
     * Replaces occurrences of the wrap character with line breaks.
     * \param variant input cell contents
     */
    QVariant replaceWrapChar( const QVariant &variant ) const;

  private slots:
    //! Checks if this vector layer will be removed (and sets mVectorLayer to 0 if yes)
    void removeLayer( const QString &layerId );

    void atlasLayerChanged( QgsVectorLayer *layer );

};

#endif // QGSLAYOUTITEMATTRIBUTETABLE_H
