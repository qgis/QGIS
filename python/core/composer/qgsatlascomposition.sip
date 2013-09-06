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

    QgsComposerMap* composerMap() const;
    void setComposerMap( QgsComposerMap* map );

    bool hideCoverage() const;
    void setHideCoverage( bool hide );

    bool fixedScale() const;
    void setFixedScale( bool fixed );

    float margin() const;
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

    /** Begins the rendering. */
    void beginRender();
    /** Ends the rendering. Restores original extent */
    void endRender();

    /** Returns the number of features in the coverage layer */
    int numFeatures() const;

    /** Prepare the atlas map for the given feature. Sets the extent and context variables */
    void prepareForFeature( int i );

    /** Returns the current filename. Must be called after prepareForFeature( i ) */
    const QString& currentFilename() const;

    void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    void readXML( const QDomElement& elem, const QDomDocument& doc );

    QgsComposition* composition();

  signals:
    /** emitted when one of the parameters changes */
    void parameterChanged();
};
