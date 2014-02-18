/** \ingroup MapComposer
 * Class used to render an Atlas, iterating over geometry features.
 * prepareForFeature() modifies the atlas map's extent to zoom on the given feature.
 * This class is used for printing, exporting to PDF and images.
 * */
class QgsAtlasComposition : QObject
{
%TypeHeaderCode
#include <qgsatlascomposition.h>
%End

public:
    QgsAtlasComposition( QgsComposition* composition );
    ~QgsAtlasComposition();

    /** Is the atlas generation enabled ? */
    bool enabled() const;
    void setEnabled( bool e );

    /**Returns the map used by the atlas
     * @deprecated Use QgsComposerMap::atlasDriven() instead
     */
    QgsComposerMap* composerMap() const;
    /**Sets the map used by the atlas
     * @deprecated Use QgsComposerMap::setAtlasDriven( true ) instead
     */
    void setComposerMap( QgsComposerMap* map );

    bool hideCoverage() const;
    void setHideCoverage( bool hide );

    /**Returns whether the atlas map uses a fixed scale
     * @deprecated Use QgsComposerMap::atlasFixedScale() instead
     */
    bool fixedScale() const;
    /**Sets whether the atlas map should use a fixed scale
     * @deprecated Use QgsComposerMap::setAtlasFixedScale( bool ) instead
     */
    void setFixedScale( bool fixed );

    /**Returns the margin for the atlas map
     * @deprecated Use QgsComposerMap::atlasMargin() instead
     */
    float margin() const;
    /**Sets the margin for the atlas map
     * @deprecated Use QgsComposerMap::setAtlasMargin( double ) instead
     */
    void setMargin( float margin );

    QString filenamePattern() const;
    void setFilenamePattern( const QString& pattern );

    QgsVectorLayer* coverageLayer() const;
    void setCoverageLayer( QgsVectorLayer* lmap );

    bool singleFile() const;
    void setSingleFile( bool single );

    bool sortFeatures() const;
    void setSortFeatures( bool doSort );

    bool sortAscending() const;
    void setSortAscending( bool ascending );

    bool filterFeatures() const;
    void setFilterFeatures( bool doFilter );

    QString featureFilter() const;
    void setFeatureFilter( const QString& expression );

    int sortKeyAttributeIndex() const;
    void setSortKeyAttributeIndex( int idx );

    /** Begins the rendering. Returns true if successful, false if no matching atlas
      features found.*/
    bool beginRender();
    /** Ends the rendering. Restores original extent */
    void endRender();

    /** Returns the number of features in the coverage layer */
    int numFeatures() const;

    /** Prepare the atlas map for the given feature. Sets the extent and context variables */
    void prepareForFeature( int i );

    /** Prepare the atlas map for the given feature. Sets the extent and context variables */
    void prepareForFeature( QgsFeature * feat );

    /** Returns the current filename. Must be called after prepareForFeature( i ) */
    const QString& currentFilename() const;

    void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    void readXML( const QDomElement& elem, const QDomDocument& doc );

    QgsComposition* composition();

    /** Requeries the current atlas coverage layer and applies filtering and sorting. Returns
      number of matching features. Must be called after prepareForFeature( i ) */
    int updateFeatures();

    void nextFeature();
    void prevFeature();
    void lastFeature();
    void firstFeature();

    /** Returns the current atlas feature. Must be called after prepareForFeature( i ). */
    QgsFeature* currentFeature();

    /** Recalculates the bounds of an atlas driven map */
    void prepareMap( QgsComposerMap* map );

  signals:
    /** emitted when one of the parameters changes */
    void parameterChanged();

    /** emitted when atlas is enabled or disabled */
    void toggled( bool );

    /**Is emitted when the atlas has an updated status bar message for the composer window*/
    void statusMsgChanged( QString message );

    /**Is emitted when the coverage layer for an atlas changes*/
    void coverageLayerChanged( QgsVectorLayer* layer );

};
