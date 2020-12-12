/***************************************************************************
                         qgspointcloudlayerrenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYERRENDERER_H
#define QGSPOINTCLOUDLAYERRENDERER_H

#include "qgis_core.h"
#include "qgscolorramp.h"
#include "qgsmaplayerrenderer.h"
#include "qgsreadwritecontext.h"
#include "qgspointcloudindex.h"
#include "qgsgeometry.h"

#include "qgserror.h"
#include "qgspointcloudindex.h"
#include "qgsidentifycontext.h"
#include "qgspointcloudrenderer.h"

#include <QDomElement>
#include <QString>
#include <QPainter>

class QgsRenderContext;
class QgsPointCloudLayer;
class QgsPointCloudRenderer;
class QgsPointCloudRenderContext;

#define SIP_NO_FILE

class CORE_EXPORT QgsPointCloudIdentifyResults
{
  public:

    /**
     * Constructor for QgsPointCloudIdentifyResults.
     */
    QgsPointCloudIdentifyResults() = default;

    /**
     * \brief Constructor. Creates valid result.
     *  \param format the result format
     *  \param results the results
     */
    QgsPointCloudIdentifyResults( const QMap<QString, QVariant> &results );

    /**
     * \brief Constructor. Creates invalid result with error.
     *  \param error the error
     */
    QgsPointCloudIdentifyResults( const QgsError &error );

    virtual ~QgsPointCloudIdentifyResults() = default;

    //! \brief Returns TRUE if valid
    bool isValid() const { return mValid; }

    /**
     * Returns the identify results. Results are different for each format:
     *
     * - QgsRaster::IdentifyFormatValue: a map of values for each band, where keys are band numbers (from 1).
     * - QgsRaster::IdentifyFormatFeature: a map of WMS sublayer keys and lists of QgsFeatureStore values.
     * - QgsRaster::IdentifyFormatHtml: a map of WMS sublayer keys and HTML strings.
     */
    QMap<QString, QVariant> results() const { return mResults; }

    //! Returns the last error
    QgsError error() const { return mError; }

    //! Sets the last error
    void setError( const QgsError &error ) { mError = error;}

  private:
    //! \brief Is valid
    bool mValid = false;

    //! \brief Results
    QMap<QString, QVariant> mResults;

    //! \brief Error
    QgsError mError;
};

/**
 * \ingroup core
 *
 * Implementation of threaded rendering for point cloud layers.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 * \note Not available in Python bindings
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudLayerRenderer: public QgsMapLayerRenderer
{
  public:

    //! Ctor
    explicit QgsPointCloudLayerRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context );
    ~QgsPointCloudLayerRenderer();

    bool render() override;
    bool forceRasterRender() const override;
    QVector<QMap<QString, QString>> identify( const QgsGeometry &geometry, const QgsIdentifyContext &identifyContext );

  private:

    //! Traverses tree and returns all nodes in specified depth
    QList<IndexedPointCloudNode> traverseTree( const QgsPointCloudIndex *pc, const QgsRenderContext &context, IndexedPointCloudNode n, float maxErrorPixels, float nodeErrorPixels );
    //! Traverses tree and returns all nodes that intersects with a \a geometry in specified depth
    QVector<IndexedPointCloudNode> traverseTree( const QgsPointCloudIndex *pc, const QgsRenderContext &context, IndexedPointCloudNode n, float maxErrorPixels, float nodeErrorPixels, const QgsGeometry &geometry );

    QgsPointCloudLayer *mLayer = nullptr;

    std::unique_ptr< QgsPointCloudRenderer > mRenderer;

    QgsVector3D mScale;
    QgsVector3D mOffset;

    QgsPointCloudAttributeCollection mAttributes;
    QgsGeometry mCloudExtent;

    /**
     * Retrieves the x and y coordinate for the point at index \a i.
     */
    static void pointXY( QgsPointCloudRenderContext &context, const char *ptr, int i, double &x, double &y )
    {
      const qint32 ix = *reinterpret_cast< const qint32 * >( ptr + i * context.pointRecordSize() + context.xOffset() );
      const qint32 iy = *reinterpret_cast< const qint32 * >( ptr + i * context.pointRecordSize() + context.yOffset() );
      x = context.offset().x() + context.scale().x() * ix;
      y = context.offset().y() + context.scale().y() * iy;
    }

    /**
     * Retrieves the z value for the point at index \a i.
     */
    static double pointZ( QgsPointCloudRenderContext &context, const char *ptr, int i )
    {
      const qint32 iz = *reinterpret_cast<const qint32 * >( ptr + i * context.pointRecordSize() + context.zOffset() );
      return context.offset().z() + context.scale().z() * iz;
    }
};

#endif // QGSPOINTCLOUDLAYERRENDERER_H
