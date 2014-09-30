/** \ingroup MapComposer
 * Class used to render an Atlas, iterating over geometry features.
 * prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
 * This class is used for printing, exporting to PDF and images.
 * @note This class should not be created directly. For the atlas to function correctly
 * the atlasComposition() property for QgsComposition should be used to retrieve a
 * QgsAtlasComposition which is automatically created and attached to the composition.
 */
class QgsAtlasComposition : QObject
{
%TypeHeaderCode
#include <qgsatlascomposition.h>
%End

public:
    QgsAtlasComposition( QgsComposition* composition );
    ~QgsAtlasComposition();

    /**Returns whether the atlas generation is enabled
     * @returns true if atlas is enabled
     * @see setEnabled
     */
    bool enabled() const;

    /**Sets whether the atlas is enabled
     * @param enabled set to true to enable to atlas
     * @see enabled
     */
    void setEnabled( bool enabled );

    /**Returns the map used by the atlas
     * @deprecated Use QgsComposerMap::atlasDriven() instead
     */
    QgsComposerMap* composerMap() const /Deprecated/;

    /**Sets the map used by the atlas
     * @deprecated Use QgsComposerMap::setAtlasDriven( true ) instead
     */
    void setComposerMap( QgsComposerMap* map ) /Deprecated/;

    /**Returns true if the atlas is set to hide the coverage layer
     * @returns true if coverage layer is hidden
     * @see setHideCoverage
     */
    bool hideCoverage() const;

    /**Sets whether the coverage layer should be hidden in map items in the composition
     * @param hide set to true to hide the coverage layer
     * @see hideCoverage
     */
    void setHideCoverage( bool hide );

    /**Returns whether the atlas map uses a fixed scale
     * @deprecated since 2.4 Use QgsComposerMap::atlasScalingMode() instead
     */
    bool fixedScale() const /Deprecated/;

    /**Sets whether the atlas map should use a fixed scale
     * @deprecated since 2.4 Use QgsComposerMap::setAtlasScalingMode() instead
     */
    void setFixedScale( bool fixed ) /Deprecated/;

    /**Returns the margin for the atlas map
     * @deprecated Use QgsComposerMap::atlasMargin() instead
     */
    float margin() const /Deprecated/;

    /**Sets the margin for the atlas map
     * @deprecated Use QgsComposerMap::setAtlasMargin( double ) instead
     */
    void setMargin( float margin ) /Deprecated/;

    /**Returns the filename expression used for generating output filenames for each
     * atlas page.
     * @returns filename pattern
     * @see setFilenamePattern
     * @see filenamePatternErrorString
     * @note This property has no effect when exporting to PDF if singleFile() is true
     */
    QString filenamePattern() const;

    /**Sets the filename expression used for generating output filenames for each
     * atlas page.
     * @returns true if filename expression could be successful set, false if expression is invalid
     * @param pattern expression to use for output filenames
     * @see filenamePattern
     * @see filenamePatternErrorString
     * @note This method has no effect when exporting to PDF if singleFile() is true
     */
    bool setFilenamePattern( const QString& pattern );

    /**Returns an error string from parsing the filename expression.
     * @returns filename pattern parser error
     * @see setFilenamePattern
     * @see filenamePattern
     */
    QString filenamePatternErrorString() const;

    /**Returns the coverage layer used for the atlas features
     * @returns atlas coverage layer
     * @see setCoverageLayer
     */
    QgsVectorLayer* coverageLayer() const;

    /**Sets the coverage layer to use for the atlas features
     * @param layer vector coverage layer
     * @see coverageLayer
     */
    void setCoverageLayer( QgsVectorLayer* layer );

    /**Returns whether the atlas will be exported to a single file. This is only
     * applicable for PDF exports.
     * @returns true if atlas will be exported to a single file
     * @see setSingleFile
     * @note This property is only used for PDF exports.
     */
    bool singleFile() const;

    /**Sets whether the atlas should be exported to a single file. This is only
     * applicable for PDF exports.
     * @param single set to true to export atlas to a single file.
     * @see singleFile
     * @note This method is only used for PDF exports.
     */
    void setSingleFile( bool single );

    bool sortFeatures() const;
    void setSortFeatures( bool doSort );

    bool sortAscending() const;
    void setSortAscending( bool ascending );

    bool filterFeatures() const;
    void setFilterFeatures( bool doFilter );

    QString featureFilter() const;
    void setFeatureFilter( const QString& expression );

    /**Returns an error string from parsing the feature filter expression.
     * @returns filename pattern parser error
     * @see setFilenamePattern
     * @see filenamePattern
     */
    QString featureFilterErrorString() const;

    QString sortKeyAttributeName() const;
    void setSortKeyAttributeName( QString fieldName );

    int sortKeyAttributeIndex() const /Deprecated/;
    void setSortKeyAttributeIndex( int idx ) /Deprecated/;

    /**Returns the current list of predefined scales for the atlas. This is used
     * for maps which are set to the predefined atlas scaling mode.
     * @returns a vector of doubles representing predefined scales
     * @see setPredefinedScales
     * @see QgsComposerMap::atlasScalingMode
    */
    const QVector<qreal>& predefinedScales() const;

    /**Sets the list of predefined scales for the atlas. This is used
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

    /**Prepare the atlas map for the given feature. Sets the extent and context variables
     * @param i feature number
     * @param updateMaps set to true to redraw maps and recalculate their extent
     * @returns true if feature was successfully prepared
    */
    bool prepareForFeature( const int i, const bool updateMaps = true );

    /**Prepare the atlas map for the given feature. Sets the extent and context variables
     * @returns true if feature was successfully prepared
    */
    bool prepareForFeature( const QgsFeature * feat );

    /** Returns the current filename. Must be called after prepareForFeature( i ) */
    const QString& currentFilename() const;

    void writeXML( QDomElement& elem, QDomDocument& doc ) const;

    /**Reads general atlas settings from xml
     * @param elem a QDomElement holding the atlas properties.
     * @param doc QDomDocument for the source xml.
     * @see readXMLMapSettings
     * @note This method should be called before restoring composer item properties
     */
    void readXML( const QDomElement& elem, const QDomDocument& doc );

    /**Reads old (pre 2.2) map related atlas settings from xml
     * @param elem a QDomElement holding the atlas map properties.
     * @param doc QDomDocument for the source xml.
     * @see readXMLMapSettings
     * @note This method should be called after restoring composer item properties
     * @note added in version 2.5
     */
    void readXMLMapSettings( const QDomElement& elem, const QDomDocument& doc );

    QgsComposition* composition();

    /** Requeries the current atlas coverage layer and applies filtering and sorting. Returns
      number of matching features. Must be called after prepareForFeature( i ) */
    int updateFeatures();

    /** Returns the current atlas feature. Must be called after prepareForFeature( i ). */
    QgsFeature* currentFeature();

    /** Recalculates the bounds of an atlas driven map */
    void prepareMap( QgsComposerMap* map );

  public slots:

    /**Refreshes the current atlas feature, by refetching its attributes from the vector layer provider
     * @note added in QGIS 2.5
    */
    void refreshFeature();

    void nextFeature();
    void prevFeature();
    void lastFeature();
    void firstFeature();

  signals:
    /** emitted when one of the parameters changes */
    void parameterChanged();

    /** emitted when atlas is enabled or disabled */
    void toggled( bool );

    /**Is emitted when the atlas has an updated status bar message for the composer window*/
    void statusMsgChanged( QString message );

    /**Is emitted when the coverage layer for an atlas changes*/
    void coverageLayerChanged( QgsVectorLayer* layer );

    /**Is emitted when atlas rendering has begun*/
    void renderBegun();

    /**Is emitted when atlas rendering has ended*/
    void renderEnded();

    /**Is emitted when the current atlas feature changes*/
    void featureChanged( QgsFeature* feature );

};
