/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstracer.h                                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsTracer : QObject
{
%Docstring
Utility class that construct a planar graph from the input vector
layers and provides shortest path search for tracing of existing
features.

.. versionadded:: 2.14
%End

%TypeHeaderCode
#include "qgstracer.h"
%End
  public:

    QgsTracer();
%Docstring
Constructor for QgsTracer.
%End
    ~QgsTracer();

    QList<QgsVectorLayer *> layers() const;
%Docstring
Get layers used for tracing
%End
    void setLayers( const QList<QgsVectorLayer *> &layers );
%Docstring
Set layers used for tracing
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
Returns the CRS used for tracing.

.. seealso:: :py:func:`setDestinationCrs`
%End

    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );
%Docstring
Sets the ``crs`` and transform ``context`` used for tracing.

.. seealso:: :py:func:`destinationCrs`
%End

    QgsRectangle extent() const;
%Docstring
Get extent to which graph's features will be limited (empty extent means no limit)
%End
    void setExtent( const QgsRectangle &extent );
%Docstring
Set extent to which graph's features will be limited (empty extent means no limit)
%End

    double offset() const;
%Docstring
Get offset in map units that should be applied to the traced paths returned from findShortestPath().
Positive offset for right side, negative offset for left side.

.. versionadded:: 3.0
%End

    void setOffset( double offset );
%Docstring
Set offset in map units that should be applied to the traced paths returned from findShortestPath().
Positive offset for right side, negative offset for left side.

.. versionadded:: 3.0
%End

    void offsetParameters( int &quadSegments /Out/, int &joinStyle /Out/, double &miterLimit /Out/ );
%Docstring
Get extra parameters for offset curve algorithm (used when offset is non-zero)

.. versionadded:: 3.0
%End

    void setOffsetParameters( int quadSegments, int joinStyle, double miterLimit );
%Docstring
Set extra parameters for offset curve algorithm (used when offset is non-zero)

.. versionadded:: 3.0
%End

    int maxFeatureCount() const;
%Docstring
Get maximum possible number of features in graph. If the number is exceeded, graph is not created.
%End
    void setMaxFeatureCount( int count );
%Docstring
Get maximum possible number of features in graph. If the number is exceeded, graph is not created.
%End

    bool init();
%Docstring
Build the internal data structures. This may take some time
depending on how big the input layers are. It is not necessary
to call this method explicitly - it will be called by findShortestPath()
if necessary.
%End

    bool isInitialized() const;
%Docstring
Whether the internal data structures have been initialized
%End

    bool hasTopologyProblem() const;
%Docstring
Whether there was an error during graph creation due to noding exception,
indicating some input data topology problems

.. versionadded:: 2.16
%End

    enum PathError
    {
      ErrNone,
      ErrTooManyFeatures,
      ErrPoint1,
      ErrPoint2,
      ErrNoPath,
    };

    QVector<QgsPointXY> findShortestPath( const QgsPointXY &p1, const QgsPointXY &p2, PathError *error /Out/ = 0 );
%Docstring
Given two points, find the shortest path and return points on the way.
The optional "error" argument may receive error code (PathError enum) if it is not null

:return: array of points - trace of linestrings of other features (empty array one error)
%End

    bool isPointSnapped( const QgsPointXY &pt );
%Docstring
Find out whether the point is snapped to a vertex or edge (i.e. it can be used for tracing start/stop)
%End

  protected:

    virtual void configure();
%Docstring
Allows derived classes to setup the settings just before the tracer is initialized.
This allows the configuration to be set in a lazy way only when it is really necessary.
Default implementation does nothing.
%End

  protected slots:
    void invalidateGraph();
%Docstring
Destroy the existing graph structure if any (de-initialize)
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstracer.h                                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
