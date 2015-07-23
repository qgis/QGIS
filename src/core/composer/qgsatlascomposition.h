/***************************************************************************
                             qgsatlascomposermap.h
                             ---------------------
    begin                : October 2012
    copyright            : (C) 2005 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATLASCOMPOSITION_H
#define QGSATLASCOMPOSITION_H

#include "qgscoordinatetransform.h"
#include "qgsfeature.h"

#include <memory>
#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QStringList>

class QgsComposerMap;
class QgsComposition;
class QgsVectorLayer;
class QgsExpression;

/** \ingroup MapComposer
 * Class used to render an Atlas, iterating over geometry features.
 * prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
 * This class is used for printing, exporting to PDF and images.
 * @note This class should not be created directly. For the atlas to function correctly
 * the atlasComposition() property for QgsComposition should be used to retrieve a
 * QgsAtlasComposition which is automatically created and attached to the composition.
 */
class CORE_EXPORT QgsAtlasComposition : public QObject
{
    Q_OBJECT
  public:
    QgsAtlasComposition( QgsComposition* composition );
    ~QgsAtlasComposition();

    /** Returns whether the atlas generation is enabled
     * @returns true if atlas is enabled
     * @see setEnabled
     */
    bool enabled() const { return mEnabled; }

    /** Sets whether the atlas is enabled
     * @param enabled set to true to enable to atlas
     * @see enabled
     */
    void setEnabled( bool enabled );

    /** Returns the map used by the atlas
     * @deprecated Use QgsComposerMap::atlasDriven() instead
     */
    Q_DECL_DEPRECATED QgsComposerMap* composerMap() const;

    /** Sets the map used by the atlas
     * @deprecated Use QgsComposerMap::setAtlasDriven( true ) instead
     */
    Q_DECL_DEPRECATED void setComposerMap( QgsComposerMap* map );

    /** Returns true if the atlas is set to hide the coverage layer
     * @returns true if coverage layer is hidden
     * @see setHideCoverage
     */
    bool hideCoverage() const { return mHideCoverage; }

    /** Sets whether the coverage layer should be hidden in map items in the composition
     * @param hide set to true to hide the coverage layer
     * @see hideCoverage
     */
    void setHideCoverage( bool hide );

    /** Returns whether the atlas map uses a fixed scale
     * @deprecated since 2.4 Use QgsComposerMap::atlasScalingMode() instead
     */
    Q_DECL_DEPRECATED bool fixedScale() const;

    /** Sets whether the atlas map should use a fixed scale
     * @deprecated since 2.4 Use QgsComposerMap::setAtlasScalingMode() instead
     */
    Q_DECL_DEPRECATED void setFixedScale( bool fixed );

    /** Returns the margin for the atlas map
     * @deprecated Use QgsComposerMap::atlasMargin() instead
     */
    Q_DECL_DEPRECATED float margin() const;

    /** Sets the margin for the atlas map
     * @deprecated Use QgsComposerMap::setAtlasMargin( double ) instead
     */
    Q_DECL_DEPRECATED void setMargin( float margin );

    /** Returns the filename expression used for generating output filenames for each
     * atlas page.
     * @returns filename pattern
     * @see setFilenamePattern
     * @see filenamePatternErrorString
     * @note This property has no effect when exporting to PDF if singleFile() is true
     */
    QString filenamePattern() const { return mFilenamePattern; }

    /** Sets the filename expression used for generating output filenames for each
     * atlas page.
     * @returns true if filename expression could be successful set, false if expression is invalid
     * @param pattern expression to use for output filenames
     * @see filenamePattern
     * @see filenamePatternErrorString
     * @note This method has no effect when exporting to PDF if singleFile() is true
     */
    bool setFilenamePattern( const QString& pattern );

    /** Returns an error string from parsing the filename expression.
     * @returns filename pattern parser error
     * @see setFilenamePattern
     * @see filenamePattern
     */
    QString filenamePatternErrorString() const { return mFilenameParserError; }

    /** Returns the coverage layer used for the atlas features
     * @returns atlas coverage layer
     * @see setCoverageLayer
     */
    QgsVectorLayer* coverageLayer() const { return mCoverageLayer; }

    /** Sets the coverage layer to use for the atlas features
     * @param layer vector coverage layer
     * @see coverageLayer
     */
    void setCoverageLayer( QgsVectorLayer* layer );

    /** Returns whether the atlas will be exported to a single file. This is only
     * applicable for PDF exports.
     * @returns true if atlas will be exported to a single file
     * @see setSingleFile
     * @note This property is only used for PDF exports.
     */
    bool singleFile() const { return mSingleFile; }

    /** Sets whether the atlas should be exported to a single file. This is only
     * applicable for PDF exports.
     * @param single set to true to export atlas to a single file.
     * @see singleFile
     * @note This method is only used for PDF exports.
     */
    void setSingleFile( bool single ) { mSingleFile = single; }

    bool sortFeatures() const { return mSortFeatures; }
    void setSortFeatures( bool doSort ) { mSortFeatures = doSort; }

    bool sortAscending() const { return mSortAscending; }
    void setSortAscending( bool ascending ) { mSortAscending = ascending; }

    bool filterFeatures() const { return mFilterFeatures; }
    void setFilterFeatures( bool doFilter ) { mFilterFeatures = doFilter; }

    QString featureFilter() const { return mFeatureFilter; }
    void setFeatureFilter( const QString& expression ) { mFeatureFilter = expression; }

    /** Returns an error string from parsing the feature filter expression.
     * @returns filename pattern parser error
     * @see setFilenamePattern
     * @see filenamePattern
     */
    QString featureFilterErrorString() const { return mFilterParserError; }

    QString sortKeyAttributeName() const { return mSortKeyAttributeName; }
    void setSortKeyAttributeName( QString fieldName ) { mSortKeyAttributeName = fieldName; }

    Q_DECL_DEPRECATED int sortKeyAttributeIndex() const;
    Q_DECL_DEPRECATED void setSortKeyAttributeIndex( int idx );

    /** Returns the current list of predefined scales for the atlas. This is used
     * for maps which are set to the predefined atlas scaling mode.
     * @returns a vector of doubles representing predefined scales
     * @see setPredefinedScales
     * @see QgsComposerMap::atlasScalingMode
    */
    const QVector<qreal>& predefinedScales() const { return mPredefinedScales; }

    /** Sets the list of predefined scales for the atlas. This is used
     * for maps which are set to the predefined atlas scaling mode.
     * @param scales a vector of doubles representing predefined scales
     * @see predefinedScales
     * @see QgsComposerMap::atlasScalingMode
     */
    void setPredefinedScales( const QVector<qreal>& scales );

    /** Begins the rendering. Returns true if successful, false if no matching atlas
      features found.*/
    bool beginRender();
    /** Ends the rendering. Restores original extent */
    void endRender();

    /** Returns the number of features in the coverage layer */
    int numFeatures() const;

    /** Prepare the atlas map for the given feature. Sets the extent and context variables
     * @param i feature number
     * @param updateMaps set to true to redraw maps and recalculate their extent
     * @returns true if feature was successfully prepared
    */
    bool prepareForFeature( const int i, const bool updateMaps = true );

    /** Prepare the atlas map for the given feature. Sets the extent and context variables
     * @returns true if feature was successfully prepared
    */
    bool prepareForFeature( const QgsFeature *feat );

    /** Returns the current filename. Must be called after prepareForFeature( i ) */
    const QString& currentFilename() const;

    void writeXML( QDomElement& elem, QDomDocument& doc ) const;

    /** Reads general atlas settings from xml
     * @param elem a QDomElement holding the atlas properties.
     * @param doc QDomDocument for the source xml.
     * @see readXMLMapSettings
     * @note This method should be called before restoring composer item properties
     */
    void readXML( const QDomElement& elem, const QDomDocument& doc );

    /** Reads old (pre 2.2) map related atlas settings from xml
     * @param elem a QDomElement holding the atlas map properties.
     * @param doc QDomDocument for the source xml.
     * @see readXMLMapSettings
     * @note This method should be called after restoring composer item properties
     * @note added in version 2.5
     */
    void readXMLMapSettings( const QDomElement& elem, const QDomDocument& doc );

    QgsComposition* composition() { return mComposition; }

    /** Requeries the current atlas coverage layer and applies filtering and sorting. Returns
      number of matching features. Must be called after prepareForFeature( i ) */
    int updateFeatures();

    /** Returns the current atlas feature. Must be called after prepareForFeature( i ). */
    QgsFeature* currentFeature() { return &mCurrentFeature; }

    /** Returns the current feature number.
     * @note added in QGIS 2.12
     */
    int currentFeatureNumber() const { return mCurrentFeatureNo; }

    /** Recalculates the bounds of an atlas driven map */
    void prepareMap( QgsComposerMap* map );

  public slots:

    /** Refreshes the current atlas feature, by refetching its attributes from the vector layer provider
     * @note added in QGIS 2.5
    */
    void refreshFeature();

    void nextFeature();
    void prevFeature();
    void lastFeature();
    void firstFeature();

  signals:
    /** Emitted when one of the parameters changes */
    void parameterChanged();

    /** Emitted when atlas is enabled or disabled */
    void toggled( bool );

    /** Is emitted when the atlas has an updated status bar message for the composer window*/
    void statusMsgChanged( QString message );

    /** Is emitted when the coverage layer for an atlas changes*/
    void coverageLayerChanged( QgsVectorLayer* layer );

    /** Is emitted when atlas rendering has begun*/
    void renderBegun();

    /** Is emitted when atlas rendering has ended*/
    void renderEnded();

    /** Is emitted when the current atlas feature changes*/
    void featureChanged( QgsFeature* feature );

    /** Is emitted when the number of features for the atlas changes.
     * @note added in QGIS 2.12
     */
    void numberFeaturesChanged( int numFeatures );

  private:
    /** Updates the filename expression.
     * @returns true if expression was successfully parsed, false if expression is invalid
     */
    bool updateFilenameExpression();

    /** Evaluates filename for current feature
     * @returns true if feature filename was successfully evaluated
    */
    bool evalFeatureFilename();

    QgsComposition* mComposition;

    bool mEnabled;
    bool mHideCoverage;
    QString mFilenamePattern;
    QgsVectorLayer* mCoverageLayer;
    bool mSingleFile;

    QgsCoordinateTransform mTransform;
    QString mCurrentFilename;
    // feature ordering
    bool mSortFeatures;
    // sort direction
    bool mSortAscending;

    // current atlas feature number
    int mCurrentFeatureNo;

  public:
    typedef QMap< QgsFeatureId, QVariant > SorterKeys;

  private slots:
    void removeLayers( QStringList layers );

  private:
    // value of field that is used for ordering of features
    SorterKeys mFeatureKeys;
    // key (attribute index) used for ordering
    QString mSortKeyAttributeName;

    // feature filtering
    bool mFilterFeatures;
    // feature expression filter
    QString mFeatureFilter;

    // id of each iterated feature (after filtering and sorting)
    QVector<QgsFeatureId> mFeatureIds;

    QgsFeature mCurrentFeature;

    QScopedPointer<QgsExpression> mFilenameExpr;

    // bounding box of the current feature transformed into map crs
    QgsRectangle mTransformedFeatureBounds;

    QString mFilenameParserError;
    QString mFilterParserError;

    //forces all atlas enabled maps to redraw
    void updateAtlasMaps();

    //computes the extent of the current feature, in the crs of the specified map
    void computeExtent( QgsComposerMap *map );

    //list of predefined scales
    QVector<qreal> mPredefinedScales;
};

#endif



