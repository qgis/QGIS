/***************************************************************************
                         qgscomposerattributetablev2.h
                         ---------------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERATTRIBUTETABLEV2_H
#define QGSCOMPOSERATTRIBUTETABLEV2_H

#include "qgscomposertablev2.h"
#include "qgscomposerattributetable.h"

class QgsComposerMap;
class QgsVectorLayer;

/**Helper class for sorting tables, takes into account sorting column and ascending / descending*/
class CORE_EXPORT QgsComposerAttributeTableCompareV2
{
  public:
    QgsComposerAttributeTableCompareV2();
    bool operator()( const QgsComposerTableRow& m1, const QgsComposerTableRow& m2 );

    /**Sets column number to sort by
     * @param col column number for sorting
     */
    void setSortColumn( int col ) { mCurrentSortColumn = col; }

    /**Sets sort order for column sorting
     * @param asc set to true to sort in ascending order, false to sort in descending order
     */
    void setAscending( bool asc ) { mAscending = asc; }

  private:
    int mCurrentSortColumn;
    bool mAscending;
};


/**A table that displays attributes from a vector layer*/
class CORE_EXPORT QgsComposerAttributeTableV2: public QgsComposerTableV2
{
    Q_OBJECT

  public:

    /*! Specifies the content source for the attribute table
     */
    enum ContentSource
    {
      LayerAttributes = 0, /*!< table shows attributes from features in a vector layer */
      AtlasFeature, /*!< table shows attributes from the current atlas feature */
      RelationChildren /*!< table shows attributes from related child features */
    };

    QgsComposerAttributeTableV2( QgsComposition* composition, bool createUndoCommands );
    ~QgsComposerAttributeTableV2();

    virtual QString displayName() const override;

    /**Writes properties specific to attribute tables
     * @param elem an existing QDomElement in which to store the attribute table's properties.
     * @param doc QDomDocument for the destination xml.
     * @param ignoreFrames ignore frames
     * @see readXML
     */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const override;

    /**Reads the properties specific to an attribute table from xml.
     * @param itemElem a QDomElement holding the attribute table's desired properties.
     * @param doc QDomDocument for the source xml.
     * @param ignoreFrames ignore frames
     * @see writeXML
     */
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false ) override;

    virtual void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true ) override;

    /**Sets the source for attributes to show in table body.
     * @param source content source
     * @see source
     */
    void setSource( const ContentSource source );

    /**Returns the source for attributes shown in the table body.
     * @returns content source
     * @see setSource
     */
    ContentSource source() const { return mSource; }

    /**Returns the source layer for the table, considering the table source mode. Eg,
     * if the table is set to atlas feature mode, then the source layer will be the
     * atlas coverage layer. If the table is set to layer attributes mode, then
     * the source layer will be the user specified vector layer.
     * @returns actual source layer
     */
    QgsVectorLayer* sourceLayer();

    /**Sets the vector layer from which to display feature attributes
     * @param layer Vector layer for attribute table
     * @see vectorLayer
     */
    void setVectorLayer( QgsVectorLayer* layer );

    /**Returns the vector layer the attribute table is currently using
     * @returns attribute table's current vector layer
     * @see setVectorLayer
     */
    QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

    /**Sets the relation id from which to display child features
     * @param relationId id for relation to display child features from
     * @see relationId
     * @see setSource
     * @note only used if table source is set to RelationChildren
     */
    void setRelationId( const QString relationId );

    /**Returns the relation id which the table displays child features from
     * @returns relation id
     * @see setRelationId
     * @see source
     * @note only used if table source is set to RelationChildren
     */
    QString relationId() const { return mRelationId; }

    /**Resets the attribute table's columns to match the vector layer's fields
     * @see setVectorLayer
     */
    void resetColumns();

    /**Sets the composer map to use to limit the extent of features shown in the
     * attribute table. This setting only has an effect if setDisplayOnlyVisibleFeatures is
     * set to true. Changing the composer map forces the table to refetch features from its
     * vector layer, and may result in the table changing size to accommodate the new displayed
     * feature attributes.
     * @param map QgsComposerMap which drives the extents of the table's features
     * @see composerMap
     * @see setDisplayOnlyVisibleFeatures
     */
    void setComposerMap( const QgsComposerMap* map );

    /**Returns the composer map whose extents are controlling the features shown in the
     * table. The extents of the map are only used if displayOnlyVisibleFeatures() is true.
     * @returns composer map controlling the attribute table
     * @see setComposerMap
     * @see displayOnlyVisibleFeatures
     */
    const QgsComposerMap* composerMap() const { return mComposerMap; }

    /**Sets the maximum number of features shown by the table. Changing this setting may result
     * in the attribute table changing its size to accommodate the new number of rows, and requires
     * the table to refetch features from its vector layer.
     * @param features maximum number of features to show in the table
     * @see maximumNumberOfFeatures
     */
    void setMaximumNumberOfFeatures( const int features );

    /**Returns the maximum number of features to be shown by the table.
     * @returns maximum number of features
     * @see setMaximumNumberOfFeatures
     */
    int maximumNumberOfFeatures() const { return mMaximumNumberOfFeatures; }

    /**Sets attribute table to only show unique rows.
     * @param uniqueOnly set to true to show only unique rows. Duplicate rows
     * will be stripped from the table.
     * @see uniqueRowsOnly
     */
    void setUniqueRowsOnly( const bool uniqueOnly );

    /**Returns true if the table is set to show only unique rows.
     * @returns true if table only shows unique rows and is stripping out
     * duplicate rows.
     * @see setUniqueRowsOnly
     */
    bool uniqueRowsOnly() const { return mShowUniqueRowsOnly; }

    /**Sets attribute table to only show features which are visible in a composer map item. Changing
     * this setting forces the table to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * @param visibleOnly set to true to show only visible features
     * @see displayOnlyVisibleFeatures
     * @see setComposerMap
     */
    void setDisplayOnlyVisibleFeatures( const bool visibleOnly );

    /**Returns true if the table is set to show only features visible on a corresponding
     * composer map item.
     * @returns true if table only shows visible features
     * @see composerMap
     * @see setDisplayOnlyVisibleFeatures
     */
    bool displayOnlyVisibleFeatures() const { return mShowOnlyVisibleFeatures; }

    /**Sets attribute table to only show features which intersect the current atlas
     * feature.
     * @param filterToAtlas set to true to show only features which intersect
     * the atlas feature
     * @see filterToAtlasFeature
     */
    void setFilterToAtlasFeature( const bool filterToAtlas );

    /**Returns true if the table is set to only show features which intersect the current atlas
     * feature.
     * @returns true if table only shows features which intersect the atlas feature
     * @see setFilterToAtlasFeature
     */
    bool filterToAtlasFeature() const { return mFilterToAtlasIntersection; }

    /**Returns true if a feature filter is active on the attribute table
     * @returns bool state of the feature filter
     * @see setFilterFeatures
     * @see featureFilter
     */
    bool filterFeatures() const { return mFilterFeatures; }

    /**Sets whether the feature filter is active for the attribute table. Changing
     * this setting forces the table to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * @param filter Set to true to enable the feature filter
     * @see filterFeatures
     * @see setFeatureFilter
     */
    void setFilterFeatures( const bool filter );

    /**Returns the current expression used to filter features for the table. The filter is only
     * active if filterFeatures() is true.
     * @returns feature filter expression
     * @see setFeatureFilter
     * @see filterFeatures
     */
    QString featureFilter() const { return mFeatureFilter; }

    /**Sets the expression used for filtering features in the table. The filter is only
     * active if filterFeatures() is set to true. Changing this setting forces the table
     * to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * @param expression filter to use for selecting which features to display in the table
     * @see featureFilter
     * @see setFilterFeatures
     */
    void setFeatureFilter( const QString& expression );

    /**Sets the attributes to display in the table.
     * @param attr QSet of integer values refering to the attributes from the vector layer to show.
     * Set to an empty QSet to show all feature attributes.
     * @param refresh set to true to force the table to refetch features from its vector layer
     * and immediately update the display of the table. This may result in the table changing size
     * to accommodate the new displayed feature attributes.
     * @see displayAttributes
     */
    void setDisplayAttributes( const QSet<int>& attr, bool refresh = true );

    /**Returns the attributes used to sort the table's features.
     * @returns a QList of integer/bool pairs, where the integer refers to the attribute index and
     * the bool to the sort order for the attribute. If true the attribute is sorted ascending,
     * if false, the attribute is sorted in descending order.
     * @note not available in python bindings
     */
    QList<QPair<int, bool> > sortAttributes() const;

    /**Queries the attribute table's vector layer for attributes to show in the table.
     * @param contents table content
     * @returns true if attributes were successfully fetched
     * @note not available in python bindings
     */
    bool getTableContents( QgsComposerTableContents &contents ) override;

  private:

    /**Attribute source*/
    ContentSource mSource;
    /**Associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Relation id, if in relation children mode*/
    QString mRelationId;

    /**Current vector layer, if in atlas feature mode*/
    QgsVectorLayer* mCurrentAtlasLayer;

    /**Associated composer map (used to display the visible features)*/
    const QgsComposerMap* mComposerMap;
    /**Maximum number of features that is displayed*/
    int mMaximumNumberOfFeatures;

    /**True if only unique rows should be shown*/
    bool mShowUniqueRowsOnly;

    /**Shows only the features that are visible in the associated composer map (true by default)*/
    bool mShowOnlyVisibleFeatures;

    /**Shows only the features that intersect the current atlas feature*/
    bool mFilterToAtlasIntersection;

    /**True if feature filtering enabled*/
    bool mFilterFeatures;
    /**Feature filter expression*/
    QString mFeatureFilter;

    /**Returns a list of attribute indices corresponding to displayed fields in the table.
     * @note kept for compatibility with 2.0 api only
     */
    QList<int> fieldsToDisplay() const;

    /**Restores a field alias map from a pre 2.4 format project file format
     * @param map QMap of integers to strings, where the string is the alias to use for the
     * corresponding field, and the integer is the field index from the vector layer
     */
    void restoreFieldAliasMap( const QMap<int, QString>& map );

  private slots:
    /**Checks if this vector layer will be removed (and sets mVectorLayer to 0 if yes) */
    void removeLayer( QString layerId );

    void atlasLayerChanged( QgsVectorLayer* layer );

};

#endif // QGSCOMPOSERATTRIBUTETABLEV2_H
