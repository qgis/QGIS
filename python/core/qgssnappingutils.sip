
class QgsSnappingUtils : QObject
{
%TypeHeaderCode
#include <qgssnappingutils.h>
%End

  public:
    QgsSnappingUtils( QObject* parent /TransferThis/ = 0 );
    ~QgsSnappingUtils();

    // main actions

    /** Get a point locator for the given layer. If such locator does not exist, it will be created */
    QgsPointLocator* locatorForLayer( QgsVectorLayer* vl );

    /** Snap to map according to the current configuration (mode). Optional filter allows discarding unwanted matches. */
    QgsPointLocator::Match snapToMap( QPoint point, QgsPointLocator::MatchFilter* filter = 0 );
    QgsPointLocator::Match snapToMap( const QgsPoint& pointMap, QgsPointLocator::MatchFilter* filter = 0 );

    /** Snap to current layer */
    QgsPointLocator::Match snapToCurrentLayer( QPoint point, int type, QgsPointLocator::MatchFilter* filter = 0 );

    // environment setup

    /** Assign current map settings to the utils - used for conversion between screen coords to map coords */
    void setMapSettings( const QgsMapSettings& settings );
    const QgsMapSettings& mapSettings() const;

    /** Set current layer so that if mode is SnapCurrentLayer we know which layer to use */
    void setCurrentLayer( QgsVectorLayer* layer );
    QgsVectorLayer* currentLayer() const;


    // configuration

    //! modes for "snap to background"
    enum SnapToMapMode
    {
      SnapCurrentLayer,    //!< snap just to current layer (tolerance and type from defaultSettings())
      SnapAllLayers,       //!< snap to all rendered layers (tolerance and type from defaultSettings())
      SnapAdvanced,        //!< snap according to the configuration set in setLayers()
    };

    /** Set how the snapping to map is done */
    void setSnapToMapMode( SnapToMapMode mode );
    /** Find out how the snapping to map is done */
    SnapToMapMode snapToMapMode() const;

    enum IndexingStrategy
    {
      IndexAlwaysFull,    //!< For all layers build index of full extent. Uses more memory, but queries are faster.
      IndexNeverFull,     //!< For all layers only create temporary indexes of small extent. Low memory usage, slower queries.
      IndexHybrid         //!< For "big" layers using IndexNeverFull, for the rest IndexAlwaysFull. Compromise between speed and memory usage.
    };

    /** Set a strategy for indexing geometry data - determines how fast and memory consuming the data structures will be */
    void setIndexingStrategy( IndexingStrategy strategy );
    /** Find out which strategy is used for indexing - by default hybrid indexing is used */
    IndexingStrategy indexingStrategy() const;

    /** Configure options used when the mode is snap to current layer or to all layers */
    void setDefaultSettings( int type, double tolerance, QgsTolerance::UnitType unit );
    /** Query options used when the mode is snap to current layer or to all layers */
    void defaultSettings( int& type /Out/, double& tolerance /Out/, QgsTolerance::UnitType& unit /Out/ );

    /**
     * Configures how a certain layer should be handled in a snapping operation
     */
    struct LayerConfig
    {
      LayerConfig( QgsVectorLayer* l, QgsPointLocator::Types t, double tol, QgsTolerance::UnitType u );

      bool operator==( const QgsSnappingUtils::LayerConfig& other ) const;
      bool operator!=( const QgsSnappingUtils::LayerConfig& other ) const;

      //! The layer to configure.
      QgsVectorLayer* layer;
      //! To which geometry properties of this layers a snapping should happen.
      QgsPointLocator::Types type;
      //! The range around snapping targets in which snapping should occur.
      double tolerance;
      //! The units in which the tolerance is specified.
      QgsTolerance::UnitType unit;
    };

    /** Set layers which will be used for snapping */
    void setLayers( const QList<QgsSnappingUtils::LayerConfig>& layers );
    /** Query layers used for snapping */
    QList<QgsSnappingUtils::LayerConfig> layers() const;

    /** Set whether to consider intersections of nearby segments for snapping */
    void setSnapOnIntersections( bool enabled );
    /** Query whether to consider intersections of nearby segments for snapping */
    bool snapOnIntersections() const;

    /** Get extra information about the instance
     * @note added in QGIS 2.14
     */
    QString dump();

  public slots:
    /** Read snapping configuration from the project */
    void readConfigFromProject();

  signals:
    /** Emitted when snapping configuration has been changed
     * @note added in QGIS 2.14
     */
    void configChanged();

  protected:
    //! Called when starting to index - can be overridden and e.g. progress dialog can be provided
    virtual void prepareIndexStarting( int count );
    //! Called when finished indexing a layer. When index == count the indexing is complete
    virtual void prepareIndexProgress( int index );
};
