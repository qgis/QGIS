/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerattributetablev2.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerAttributeTableCompareV2
{
%Docstring
 Helper class for sorting tables, takes into account sorting column and ascending / descending
%End

%TypeHeaderCode
#include "qgscomposerattributetablev2.h"
%End
  public:

    QgsComposerAttributeTableCompareV2();
%Docstring
 Constructor for QgsComposerAttributeTableCompareV2.
%End
    bool operator()( const QgsComposerTableRow &m1, const QgsComposerTableRow &m2 );
%Docstring
 :rtype: bool
%End

    void setSortColumn( int col );
%Docstring
 Sets column number to sort by
 \param col column number for sorting
%End

    void setAscending( bool asc );
%Docstring
 Sets sort order for column sorting
 \param asc set to true to sort in ascending order, false to sort in descending order
%End

};


class QgsComposerAttributeTableV2: QgsComposerTableV2
{
%Docstring
 A table that displays attributes from a vector layer
%End

%TypeHeaderCode
#include "qgscomposerattributetablev2.h"
%End
  public:

    enum ContentSource
    {
      LayerAttributes,
      AtlasFeature,
      RelationChildren
    };

    QgsComposerAttributeTableV2( QgsComposition *composition /TransferThis/, bool createUndoCommands );

    virtual QString displayName() const;

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const;
%Docstring
 Writes properties specific to attribute tables
 \param elem an existing QDomElement in which to store the attribute table's properties.
 \param doc QDomDocument for the destination xml.
 \param ignoreFrames ignore frames
.. seealso:: readXml
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false );
%Docstring
 Reads the properties specific to an attribute table from xml.
 \param itemElem a QDomElement holding the attribute table's desired properties.
 \param doc QDomDocument for the source xml.
 \param ignoreFrames ignore frames
.. seealso:: writeXml
 :rtype: bool
%End

    virtual void addFrame( QgsComposerFrame *frame /Transfer/, bool recalcFrameSizes = true );

    void setSource( const ContentSource source );
%Docstring
 Sets the source for attributes to show in table body.
 \param source content source
.. seealso:: source
%End

    ContentSource source() const;
%Docstring
 Returns the source for attributes shown in the table body.
 :return: content source
.. seealso:: setSource
 :rtype: ContentSource
%End

    QgsVectorLayer *sourceLayer();
%Docstring
 Returns the source layer for the table, considering the table source mode. For example,
 if the table is set to atlas feature mode, then the source layer will be the
 atlas coverage layer. If the table is set to layer attributes mode, then
 the source layer will be the user specified vector layer.
 :return: actual source layer
 :rtype: QgsVectorLayer
%End

    void setVectorLayer( QgsVectorLayer *layer );
%Docstring
 Sets the vector layer from which to display feature attributes
 \param layer Vector layer for attribute table
.. seealso:: vectorLayer
%End

    QgsVectorLayer *vectorLayer() const;
%Docstring
 Returns the vector layer the attribute table is currently using
 :return: attribute table's current vector layer
.. seealso:: setVectorLayer
 :rtype: QgsVectorLayer
%End

    void setRelationId( const QString &relationId );
%Docstring
 Sets the relation id from which to display child features
 \param relationId id for relation to display child features from
.. seealso:: relationId
.. seealso:: setSource
.. note::

   only used if table source is set to RelationChildren
%End

    QString relationId() const;
%Docstring
 Returns the relation id which the table displays child features from
 :return: relation id
.. seealso:: setRelationId
.. seealso:: source
.. note::

   only used if table source is set to RelationChildren
 :rtype: str
%End

    void resetColumns();
%Docstring
 Resets the attribute table's columns to match the vector layer's fields
.. seealso:: setVectorLayer
%End

    void setComposerMap( const QgsComposerMap *map );
%Docstring
 Sets the composer map to use to limit the extent of features shown in the
 attribute table. This setting only has an effect if setDisplayOnlyVisibleFeatures is
 set to true. Changing the composer map forces the table to refetch features from its
 vector layer, and may result in the table changing size to accommodate the new displayed
 feature attributes.
 \param map QgsComposerMap which drives the extents of the table's features
.. seealso:: composerMap
.. seealso:: setDisplayOnlyVisibleFeatures
%End

    const QgsComposerMap *composerMap() const;
%Docstring
 Returns the composer map whose extents are controlling the features shown in the
 table. The extents of the map are only used if displayOnlyVisibleFeatures() is true.
 :return: composer map controlling the attribute table
.. seealso:: setComposerMap
.. seealso:: displayOnlyVisibleFeatures
 :rtype: QgsComposerMap
%End

    void setMaximumNumberOfFeatures( const int features );
%Docstring
 Sets the maximum number of features shown by the table. Changing this setting may result
 in the attribute table changing its size to accommodate the new number of rows, and requires
 the table to refetch features from its vector layer.
 \param features maximum number of features to show in the table
.. seealso:: maximumNumberOfFeatures
%End

    int maximumNumberOfFeatures() const;
%Docstring
 Returns the maximum number of features to be shown by the table.
 :return: maximum number of features
.. seealso:: setMaximumNumberOfFeatures
 :rtype: int
%End

    void setUniqueRowsOnly( const bool uniqueOnly );
%Docstring
 Sets attribute table to only show unique rows.
 \param uniqueOnly set to true to show only unique rows. Duplicate rows
 will be stripped from the table.
.. seealso:: uniqueRowsOnly
%End

    bool uniqueRowsOnly() const;
%Docstring
 Returns true if the table is set to show only unique rows.
 :return: true if table only shows unique rows and is stripping out
 duplicate rows.
.. seealso:: setUniqueRowsOnly
 :rtype: bool
%End

    void setDisplayOnlyVisibleFeatures( const bool visibleOnly );
%Docstring
 Sets attribute table to only show features which are visible in a composer map item. Changing
 this setting forces the table to refetch features from its vector layer, and may result in
 the table changing size to accommodate the new displayed feature attributes.
 \param visibleOnly set to true to show only visible features
.. seealso:: displayOnlyVisibleFeatures
.. seealso:: setComposerMap
%End

    bool displayOnlyVisibleFeatures() const;
%Docstring
 Returns true if the table is set to show only features visible on a corresponding
 composer map item.
 :return: true if table only shows visible features
.. seealso:: composerMap
.. seealso:: setDisplayOnlyVisibleFeatures
 :rtype: bool
%End

    void setFilterToAtlasFeature( const bool filterToAtlas );
%Docstring
 Sets attribute table to only show features which intersect the current atlas
 feature.
 \param filterToAtlas set to true to show only features which intersect
 the atlas feature
.. seealso:: filterToAtlasFeature
%End

    bool filterToAtlasFeature() const;
%Docstring
 Returns true if the table is set to only show features which intersect the current atlas
 feature.
 :return: true if table only shows features which intersect the atlas feature
.. seealso:: setFilterToAtlasFeature
 :rtype: bool
%End

    bool filterFeatures() const;
%Docstring
 Returns true if a feature filter is active on the attribute table
 :return: bool state of the feature filter
.. seealso:: setFilterFeatures
.. seealso:: featureFilter
 :rtype: bool
%End

    void setFilterFeatures( const bool filter );
%Docstring
 Sets whether the feature filter is active for the attribute table. Changing
 this setting forces the table to refetch features from its vector layer, and may result in
 the table changing size to accommodate the new displayed feature attributes.
 \param filter Set to true to enable the feature filter
.. seealso:: filterFeatures
.. seealso:: setFeatureFilter
%End

    QString featureFilter() const;
%Docstring
 Returns the current expression used to filter features for the table. The filter is only
 active if filterFeatures() is true.
 :return: feature filter expression
.. seealso:: setFeatureFilter
.. seealso:: filterFeatures
 :rtype: str
%End

    void setFeatureFilter( const QString &expression );
%Docstring
 Sets the expression used for filtering features in the table. The filter is only
 active if filterFeatures() is set to true. Changing this setting forces the table
 to refetch features from its vector layer, and may result in
 the table changing size to accommodate the new displayed feature attributes.
 \param expression filter to use for selecting which features to display in the table
.. seealso:: featureFilter
.. seealso:: setFilterFeatures
%End

    void setDisplayedFields( const QStringList &fields, bool refresh = true );
%Docstring
 Sets the attributes to display in the table.
 \param fields list of fields names from the vector layer to show.
 Set to an empty list to show all feature attributes.
 \param refresh set to true to force the table to refetch features from its vector layer
 and immediately update the display of the table. This may result in the table changing size
 to accommodate the new displayed feature attributes.
.. versionadded:: 2.16
%End


    void setWrapString( const QString &wrapString );
%Docstring
 Sets a string to wrap the contents of the table cells by. Occurrences of this string will
 be replaced by a line break.
 \param wrapString string to replace with line break
.. versionadded:: 2.12
.. seealso:: wrapString
%End

    QString wrapString() const;
%Docstring
 Returns the string used to wrap the contents of the table cells by. Occurrences of this string will
 be replaced by a line break.
 :return: string which will be replaced with line break
.. versionadded:: 2.12
.. seealso:: setWrapString
 :rtype: str
%End


    virtual QgsExpressionContext createExpressionContext() const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerattributetablev2.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
