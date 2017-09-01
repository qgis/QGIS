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
    ~QgsTracer();

    QList<QgsVectorLayer *> layers() const;
%Docstring
Get layers used for tracing
 :rtype: list of QgsVectorLayer
%End
    void setLayers( const QList<QgsVectorLayer *> &layers );
%Docstring
Set layers used for tracing
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
Get CRS used for tracing
 :rtype: QgsCoordinateReferenceSystem
%End
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Set CRS used for tracing
%End

    QgsRectangle extent() const;
%Docstring
Get extent to which graph's features will be limited (empty extent means no limit)
 :rtype: QgsRectangle
%End
    void setExtent( const QgsRectangle &extent );
%Docstring
Set extent to which graph's features will be limited (empty extent means no limit)
%End

    int maxFeatureCount() const;
%Docstring
Get maximum possible number of features in graph. If the number is exceeded, graph is not created.
 :rtype: int
%End
    void setMaxFeatureCount( int count );
%Docstring
Get maximum possible number of features in graph. If the number is exceeded, graph is not created.
%End

    bool init();
%Docstring
if necessary.
 :rtype: bool
%End

    bool isInitialized() const;
%Docstring
Whether the internal data structures have been initialized
 :rtype: bool
%End

    bool hasTopologyProblem() const;
%Docstring
.. versionadded:: 2.16
 :rtype: bool
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
:return: array of points - trace of linestrings of other features (empty array one error)
 :rtype: list of QgsPointXY
%End

    bool isPointSnapped( const QgsPointXY &pt );
%Docstring
Find out whether the point is snapped to a vertex or edge (i.e. it can be used for tracing start/stop)
 :rtype: bool
%End

  protected:
    virtual void configure();
%Docstring
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
