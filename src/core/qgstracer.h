/***************************************************************************
 qgstracer.h
 --------------------------------------
 Date                 : January 2016
 Copyright            : (C) 2016 by Martin Dobias
 Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSTRACER_H
#define QGSTRACER_H

class QgsVectorLayer;

#include "qgis_core.h"
#include <QSet>
#include <QVector>
#include <memory>

#include "qgsfeatureid.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsrectangle.h"
#include "qgsgeometry.h"

struct QgsTracerGraph;
class QgsFeatureRenderer;
class QgsRenderContext;

/**
 * \ingroup core
 * \brief Utility class that construct a planar graph from the input vector
 * layers and provides shortest path search for tracing of existing
 * features.
 *
 * \since QGIS 2.14
 */
class CORE_EXPORT QgsTracer : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsTracer.
     */
    QgsTracer();
    ~QgsTracer() override;

    //! Gets layers used for tracing
    QList<QgsVectorLayer *> layers() const { return mLayers; }
    //! Sets layers used for tracing
    void setLayers( const QList<QgsVectorLayer *> &layers );

    /**
     * Returns the CRS used for tracing.
     * \see setDestinationCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const { return mCRS; }

    /**
     * Sets the \a crs and transform \a context used for tracing.
     * \see destinationCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    /**
     * Sets the \a renderContext used for tracing only on visible features.
     * \since QGIS 3.4
     */
    void setRenderContext( const QgsRenderContext *renderContext );

    //! Gets extent to which graph's features will be limited (empty extent means no limit)
    QgsRectangle extent() const { return mExtent; }
    //! Sets extent to which graph's features will be limited (empty extent means no limit)
    void setExtent( const QgsRectangle &extent );

    /**
     * Gets offset in map units that should be applied to the traced paths returned from findShortestPath().
     * Positive offset for right side, negative offset for left side.
     * \since QGIS 3.0
     */
    double offset() const { return mOffset; }

    /**
     * Set offset in map units that should be applied to the traced paths returned from findShortestPath().
     * Positive offset for right side, negative offset for left side.
     * \since QGIS 3.0
     */
    void setOffset( double offset );

    // TODO QGIS 4.0 -- use Qgis::JoinStyle instead of int!

    /**
     * Gets extra parameters for offset curve algorithm (used when offset is non-zero)
     * \since QGIS 3.0
     */
    void offsetParameters( int &quadSegments SIP_OUT, int &joinStyle SIP_OUT, double &miterLimit SIP_OUT );

    // TODO QGIS 4.0 -- use Qgis::JoinStyle instead of int!

    /**
     * Set extra parameters for offset curve algorithm (used when offset is non-zero)
     * \since QGIS 3.0
     */
    void setOffsetParameters( int quadSegments, int joinStyle, double miterLimit );

    //! Gets maximum possible number of features in graph. If the number is exceeded, graph is not created.
    int maxFeatureCount() const { return mMaxFeatureCount; }
    //! Gets maximum possible number of features in graph. If the number is exceeded, graph is not created.
    void setMaxFeatureCount( int count ) { mMaxFeatureCount = count; }

    /**
     * Build the internal data structures. This may take some time
     * depending on how big the input layers are. It is not necessary
     * to call this method explicitly - it will be called by findShortestPath()
     * if necessary.
     */
    bool init();

    //! Whether the internal data structures have been initialized
    bool isInitialized() const { return static_cast< bool >( mGraph ); }

    /**
     * Whether there was an error during graph creation due to noding exception,
     * indicating some input data topology problems
     * \since QGIS 2.16
     */
    bool hasTopologyProblem() const { return mHasTopologyProblem; }

    //! Possible errors that may happen when calling findShortestPath()
    enum PathError
    {
      ErrNone,               //!< No error
      ErrTooManyFeatures,    //!< Max feature count threshold was reached while reading features
      ErrPoint1,             //!< Start point cannot be joined to the graph
      ErrPoint2,             //!< End point cannot be joined to the graph
      ErrNoPath,             //!< Points are not connected in the graph
    };

    /**
     * Given two points, find the shortest path and return points on the way.
     * The optional "error" argument may receive error code (PathError enum) if it is not NULLPTR
     * \returns array of points - trace of linestrings of other features (empty array one error)
     */
    QVector<QgsPointXY> findShortestPath( const QgsPointXY &p1, const QgsPointXY &p2, PathError *error SIP_OUT = nullptr );

    //! Find out whether the point is snapped to a vertex or edge (i.e. it can be used for tracing start/stop)
    bool isPointSnapped( const QgsPointXY &pt );

  protected:

    /**
     * Allows derived classes to setup the settings just before the tracer is initialized.
     * This allows the configuration to be set in a lazy way only when it is really necessary.
     * Default implementation does nothing.
     */
    virtual void configure() {}

  protected slots:
    //! Destroy the existing graph structure if any (de-initialize)
    void invalidateGraph();

  private:
    bool initGraph();

  private slots:
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom );
    void onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );
    void onDataChanged( );
    void onStyleChanged( );
    void onLayerDestroyed( QObject *obj );

  private:
    //! Graph data structure for path searching
    std::unique_ptr< QgsTracerGraph > mGraph;
    //! Input layers for the graph building
    QList<QgsVectorLayer *> mLayers;
    //! Destination CRS in which graph is built and tracing done
    QgsCoordinateReferenceSystem mCRS;
    //! Coordinate transform context
    QgsCoordinateTransformContext mTransformContext;
    //! Render context
    std::unique_ptr<QgsRenderContext> mRenderContext;
    //! Extent for graph building (empty extent means no limit)
    QgsRectangle mExtent;

    //! Offset in map units that should be applied to the traced paths
    double mOffset = 0;
    //! Offset parameter: Number of segments (approximation of circle quarter) when using round join style
    int mOffsetSegments = 8;
    //! Offset parameter: Join style (1 = round, 2 = miter, 3 = bevel)
    Qgis::JoinStyle mOffsetJoinStyle = Qgis::JoinStyle::Miter;
    //! Offset parameter: Limit for miter join style
    double mOffsetMiterLimit = 5.;

    /**
     * Limit of how many features can be in the graph (0 means no limit).
     * This is to avoid possibly long graph preparation for complicated layers
     */
    int mMaxFeatureCount = 0;

    /**
     * A flag indicating that there was an error during graph creation
     * due to noding exception, indicating some input data topology problems
     */
    bool mHasTopologyProblem = false;
};


#endif // QGSTRACER_H
