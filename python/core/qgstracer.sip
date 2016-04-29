/** \ingroup core
 * Utility class that construct a planar graph from the input vector
 * layers and provides shortest path search for tracing of existing
 * features.
 *
 * @note added in QGIS 2.14
 */
class QgsTracer : QObject
{
%TypeHeaderCode
#include <qgstracer.h>
%End

  public:
    QgsTracer();
    ~QgsTracer();

    //! Get layers used for tracing
    QList<QgsVectorLayer*> layers() const;
    //! Set layers used for tracing
    void setLayers( const QList<QgsVectorLayer*>& layers );

    //! Return true if reprojection to destination CRS is enabled
    bool hasCrsTransformEnabled() const;
    //! Set whether to do reprojection to destination CRS
    void setCrsTransformEnabled( bool enabled );

    //! Get CRS used for tracing
    QgsCoordinateReferenceSystem destinationCrs() const;
    //! Set CRS used for tracing
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

    //! Get extent to which graph's features will be limited (empty extent means no limit)
    QgsRectangle extent() const;
    //! Set extent to which graph's features will be limited (empty extent means no limit)
    void setExtent( const QgsRectangle& extent );

    //! Get maximum possible number of features in graph. If the number is exceeded, graph is not created.
    int maxFeatureCount() const;
    //! Get maximum possible number of features in graph. If the number is exceeded, graph is not created.
    void setMaxFeatureCount( int count );

    //! Build the internal data structures. This may take some time
    //! depending on how big the input layers are. It is not necessary
    //! to call this method explicitly - it will be called by findShortestPath()
    //! if necessary.
    bool init();

    //! Whether the internal data structures have been initialized
    bool isInitialized() const;

    //! Whether there was an error during graph creation due to noding exception,
    //! indicating some input data topology problems
    //! @note added in QGIS 2.16
    bool hasTopologyProblem() const;

    //! Possible errors that may happen when calling findShortestPath()
    enum PathError
    {
      ErrNone,               //!< No error
      ErrTooManyFeatures,    //!< Max feature count threshold was reached while reading features
      ErrPoint1,             //!< Start point cannot be joined to the graph
      ErrPoint2,             //!< End point cannot be joined to the graph
      ErrNoPath,             //!< Points are not connected in the graph
    };

    //! Given two points, find the shortest path and return points on the way.
    //! The optional "error" argument may receive error code (PathError enum) if it is not null
    //! @return array of points - trace of linestrings of other features (empty array one error)
    QVector<QgsPoint> findShortestPath( const QgsPoint& p1, const QgsPoint& p2, QgsTracer::PathError* error /Out/ = nullptr );

    //! Find out whether the point is snapped to a vertex or edge (i.e. it can be used for tracing start/stop)
    bool isPointSnapped( const QgsPoint& pt );

  protected:
    //! Allows derived classes to setup the settings just before the tracer is initialized.
    //! This allows the configuration to be set in a lazy way only when it is really necessary.
    //! Default implementation does nothing.
    virtual void configure();

  protected slots:
    //! Destroy the existing graph structure if any (de-initialize)
    void invalidateGraph();
};
