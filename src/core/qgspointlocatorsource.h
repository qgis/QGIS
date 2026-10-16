/***************************************************************************
  qgspointlocatorsource.h
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by the QGIS project
  Email                : info at qgis dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTLOCATORSOURCE_H
#define QGSPOINTLOCATORSOURCE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//


#include <memory>

#include "qgsfeatureid.h"
#include "qgsgeometry.h"

#include <QHash>
#include <QString>
#include <QVector>

#define SIP_NO_FILE

class QgsMapLayer;
class QgsVectorLayer;
class QgsAnnotationLayer;
class QgsRectangle;
class QgsRenderContext;
class QgsFeatureRenderer;
class QgsVectorLayerFeatureSource;
class QgsCoordinateTransform;

/**
 * \ingroup core
 * \brief Abstract source of snappable geometries for QgsPointLocator.
 *
 * A source decouples the point locator from the concrete layer type it indexes. It produces the
 * geometries to be indexed and resolves the identity (layer and item id) of a match. Anything a
 * source needs from the live layer must be captured on the main thread (typically in the
 * constructor) so that snappableGeometries() can run safely on a worker thread during a relaxed
 * (background) index build.
 *
 * \note not available in Python bindings
 */
class QgsPointLocatorSource
{
  public:
    virtual ~QgsPointLocatorSource() = default;

    //! A single snappable geometry, identified by a (possibly synthetic) feature id.
    struct Geometry
    {
        QgsFeatureId id = 0;
        QgsGeometry geometry;
    };

    /**
     * Produces all snappable geometries, already transformed into the locator's destination CRS
     * via \a transform, optionally restricted to \a extent (destination CRS) and to at most
     * \a maxFeaturesToIndex geometries (-1 for no limit).
     *
     * Sets \a ok to FALSE if the maximum feature count was exceeded and indexing must be aborted.
     *
     * This may be called on a worker thread, so it MUST NOT read the live layer. It may only use
     * state snapshotted on the main thread at construction.
     */
    virtual QVector<Geometry> snappableGeometries( const QgsRectangle *extent, const QgsCoordinateTransform &transform, int maxFeaturesToIndex, bool &ok ) = 0;

    //! The map layer this source indexes.
    virtual QgsMapLayer *layer() const = 0;

    //! The vector layer this source indexes, or NULLPTR for a non-vector source.
    virtual QgsVectorLayer *vectorLayer() const { return nullptr; }

    //! The item id mapped to the synthetic feature \a id, or an empty string if this source has no item ids.
    virtual QString itemId( QgsFeatureId ) const { return QString(); }

    //! An identifier for the source, used e.g. as the background indexing task name.
    virtual QString id() const = 0;

    /**
     * Releases resources only needed while (re)building the index (e.g. a cloned feature source),
     * while keeping the lightweight identity data required to resolve matches at query time.
     */
    virtual void releaseIndexingResources() {}
};

/**
 * \ingroup core
 * \brief QgsPointLocatorSource backed by a vector layer.
 * \note not available in Python bindings
 */
class QgsPointLocatorVectorSource : public QgsPointLocatorSource
{
  public:
    /**
     * Constructs a vector source for \a layer. Clones the feature source (and, if \a context is
     * not NULLPTR, the layer renderer) on the main thread. \a context is borrowed and owned by the
     * caller (the locator).
     */
    QgsPointLocatorVectorSource( QgsVectorLayer *layer, QgsRenderContext *context );
    ~QgsPointLocatorVectorSource() override;

    QVector<Geometry> snappableGeometries( const QgsRectangle *extent, const QgsCoordinateTransform &transform, int maxFeaturesToIndex, bool &ok ) override;
    QgsMapLayer *layer() const override;
    QgsVectorLayer *vectorLayer() const override { return mLayer; }
    QString id() const override;
    void releaseIndexingResources() override;

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsRenderContext *mContext = nullptr; // borrowed, owned by the locator
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
};

/**
 * \ingroup core
 * \brief QgsPointLocatorSource backed by an annotation layer.
 *
 * Snapshots the enabled items' snap geometries (cloned) and their item ids on the main thread at
 * construction, so snappableGeometries() only reprojects the snapshot and never touches the live
 * layer.
 *
 * \note not available in Python bindings
 */
class QgsPointLocatorAnnotationSource : public QgsPointLocatorSource
{
  public:
    //! Snapshots the snappable items of the annotation \a layer on the main thread.
    explicit QgsPointLocatorAnnotationSource( QgsAnnotationLayer *layer );

    QVector<Geometry> snappableGeometries( const QgsRectangle *extent, const QgsCoordinateTransform &transform, int maxFeaturesToIndex, bool &ok ) override;
    QgsMapLayer *layer() const override;
    QString itemId( QgsFeatureId id ) const override { return mItemIds.value( id ); }
    QString id() const override;
    void releaseIndexingResources() override;

  private:
    QgsAnnotationLayer *mLayer = nullptr;
    //! Cloned snap geometries in the layer CRS, keyed by synthetic feature id. Freed after indexing.
    QHash<QgsFeatureId, QgsGeometry> mSnapshot;
    //! Maps each synthetic feature id to its annotation item id. Kept alive for query-time match stamping.
    QHash<QgsFeatureId, QString> mItemIds;
};

/// @endcond

#endif // QGSPOINTLOCATORSOURCE_H
