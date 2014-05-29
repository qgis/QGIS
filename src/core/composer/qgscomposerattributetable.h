/***************************************************************************
                         qgscomposerattributetable.h
                         ---------------------------
    begin                : April 2010
    copyright            : (C) 2010 by Marco Hugentobler
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

#ifndef QGSCOMPOSERATTRIBUTETABLE_H
#define QGSCOMPOSERATTRIBUTETABLE_H

#include "qgscomposertable.h"

class QgsComposerMap;
class QgsVectorLayer;

/**Helper class for sorting tables, takes into account sorting column and ascending / descending*/
class CORE_EXPORT QgsComposerAttributeTableCompare
{
  public:
    QgsComposerAttributeTableCompare();
    bool operator()( const QgsAttributeMap& m1, const QgsAttributeMap& m2 );

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

/**A table class that displays a vector attribute table*/
class CORE_EXPORT QgsComposerAttributeTable: public QgsComposerTable
{
    Q_OBJECT
  public:
    QgsComposerAttributeTable( QgsComposition* composition );
    ~QgsComposerAttributeTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerAttributeTable; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Writes properties specific to attribute tables
     * @param elem an existing QDomElement in which to store the attribute table's properties.
     * @param doc QDomDocument for the destination xml.
     * @see readXML
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /**Reads the properties specific to an attribute table from xml.
     * @param itemElem a QDomElement holding the attribute table's desired properties.
     * @param doc QDomDocument for the source xml.
     * @see writeXML
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

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

    /**Resets the attribute table's columns to match the vector layer's fields
     * @note added in 2.3
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
    void setMaximumNumberOfFeatures( int features );

    /**Returns the maximum number of features to be shown by the table.
     * @returns maximum number of features
     * @see setMaximumNumberOfFeatures
     */
    int maximumNumberOfFeatures() const { return mMaximumNumberOfFeatures; }

    /**Sets attribute table to only show features which are visible in a composer map item. Changing
     * this setting forces the table to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * @param visibleOnly set to true to show only visible features
     * @see displayOnlyVisibleFeatures
     * @see setComposerMap
     */
    void setDisplayOnlyVisibleFeatures( bool visibleOnly );

    /**Returns true if the table is set to show only features visible on a corresponding
     * composer map item.
     * @returns true if table only shows visible features
     * @see composerMap
     * @see setDisplayOnlyVisibleFeatures
     */
    bool displayOnlyVisibleFeatures() const { return mShowOnlyVisibleFeatures; }

    /**Returns true if a feature filter is active on the attribute table
     * @returns bool state of the feature filter
     * @note added in 2.3
     * @see setFilterFeatures
     * @see featureFilter
     */
    bool filterFeatures() const { return mFilterFeatures; }

    /**Sets whether the feature filter is active for the attribute table. Changing
     * this setting forces the table to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * @param filter Set to true to enable the feature filter
     * @note added in 2.3
     * @see filterFeatures
     * @see setFeatureFilter
     */
    void setFilterFeatures( bool filter );

    /**Returns the current expression used to filter features for the table. The filter is only
     * active if filterFeatures() is true.
     * @returns feature filter expression
     * @note added in 2.3
     * @see setFeatureFilter
     * @see filterFeatures
     */
    QString featureFilter() const { return mFeatureFilter; }

    /**Sets the expression used for filtering features in the table. The filter is only
     * active if filterFeatures() is set to true. Changing this setting forces the table
     * to refetch features from its vector layer, and may result in
     * the table changing size to accommodate the new displayed feature attributes.
     * @param expression filter to use for selecting which features to display in the table
     * @note added in 2.3
     * @see featureFilter
     * @see setFilterFeatures
     */
    void setFeatureFilter( const QString& expression );

    /**Returns the attributes fields which are shown by the table.
     * @returns a QSet of integers refering to the attributes in the vector layer. If
     * result is an empty QSet than all feature attributes will be shown.
     * @see setDisplayAttributes
     * @deprecated use QgsComposerTable::columns() instead
     */
    Q_DECL_DEPRECATED QSet<int> displayAttributes() const;

    /**Sets the attributes to display in the table.
     * @param attr QSet of integer values refering to the attributes from the vector layer to show.
     * Set to an empty QSet to show all feature attributes.
     * @param refresh set to true to force the table to refetch features from its vector layer
     * and immediately update the display of the table. This may result in the table changing size
     * to accommodate the new displayed feature attributes.
     * @see displayAttributes
     */
    void setDisplayAttributes( const QSet<int>& attr, bool refresh = true );

    /**Returns the attribute field aliases, which control how fields are named in the table's
     * header row.
     * @returns a QMap of integers to strings, where the string is the field's alias and the
     * integer is the field index from the associated vector layer.
     * @see setFieldAliasMap
     * @deprecated use QgsComposerTable::columns() instead
     */
    Q_DECL_DEPRECATED QMap<int, QString> fieldAliasMap() const;

    /**Sets the attribute field aliases, which control how fields are named in the table's
     * header row.
     * @param map QMap of integers to strings, where the string is the alias to use for the
     * corresponding field, and the integer is the field index from the vector layer
     * @see fieldAliasMap
     * @deprecated use QgsComposerTable::columns() instead
     */
    Q_DECL_DEPRECATED void setFieldAliasMap( const QMap<int, QString>& map );

    /**Adapts mMaximumNumberOfFeatures depending on the rectangle height. Calling this forces
     * the table to refetch features from its vector layer and immediately updates the display
     * of the table.
     * @see maximumNumberOfFeatures
     * @see setMaximumNumberOfFeatures
     */
    void setSceneRect( const QRectF& rectangle );

    /**Sets the attributes to use to sort the table's features.
     * @param att QList integers/bool pairs, where the integer refers to the attribute index and
     * the bool sets the sort order for the attribute. If true the attribute is sorted ascending,
     * if false, the attribute is sorted in descending order. Note that features are sorted
     * after the maximum number of displayed features have been fetched from the vector layer's
     * provider.
     * @see sortAttributes
     * @deprecated use QgsComposerTable::columns() instead
     * @note not available in python bindings
     */
    Q_DECL_DEPRECATED void setSortAttributes( const QList<QPair<int, bool> > att );

    /**Returns the attributes used to sort the table's features.
     * @returns a QList of integer/bool pairs, where the integer refers to the attribute index and
     * the bool to the sort order for the attribute. If true the attribute is sorted ascending,
     * if false, the attribute is sorted in descending order.
     * @note not available in python bindings
     */
    QList<QPair<int, bool> > sortAttributes() const;

    /**Queries the attribute table's vector layer for attributes to show in the table.
     * @param attributeMaps list of QgsAttributeMaps where the fetched feature attributes will be stored
     * @returns true if attributes were successfully fetched
     * @note not available in python bindings
     */
    bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps );

  private:
    /**Associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Associated composer map (used to display the visible features)*/
    const QgsComposerMap* mComposerMap;
    /**Maximum number of features that is displayed*/
    int mMaximumNumberOfFeatures;

    /**Shows only the features that are visible in the associated composer map (true by default)*/
    bool mShowOnlyVisibleFeatures;

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

  signals:
    /**This signal is emitted if the maximum number of feature changes (interactively)*/
    void maximumNumberOfFeaturesChanged( int n );
};

#endif // QGSCOMPOSERATTRIBUTETABLE_H
