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

#include <QDomElement>
#include <QString>
#include <QPainter>

class QgsRenderContext;
class QgsPointCloudLayer;
class QgsPointCloudRenderer;
class QgsPointCloudRenderContext;

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Configuration of the 2d renderer
 */
class CORE_EXPORT QgsPointCloudRendererConfig
{
  public:
    //! Ctor
    QgsPointCloudRendererConfig();
    //! Copy constructor
    QgsPointCloudRendererConfig( const QgsPointCloudRendererConfig &other );
    //! Assignment constructor
    QgsPointCloudRendererConfig &operator= ( const QgsPointCloudRendererConfig &other );

    //! Returns z min
    double zMin() const;
    //! Sets z min
    void setZMin( double value );

    //! Returns z max
    double zMax() const;

    //! Sets z max
    void setZMax( double value );

    //! Returns pen width
    int penWidth() const;

    //! Sets pen width
    void setPenWidth( int value );

    //! Returns color ramp
    QgsColorRamp *colorRamp() const;

    //! Sets color ramp (ownership is transferrred)
    void setColorRamp( QgsColorRamp *value SIP_TRANSFER );

    //! Returns maximum allowed screen error in pixels
    float maximumScreenError() const;

    QString attribute() const;
    void setAttribute( const QString &attribute );

  private:
    double mZMin = 0, mZMax = 0;
    QString mAttribute;
    int mPenWidth = 1;
    std::unique_ptr<QgsColorRamp> mColorRamp;
    float mMaximumScreenError = 5;
};

///@endcond

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

  private:

    //! Traverses tree and returns all nodes in specified depth
    QList<IndexedPointCloudNode> traverseTree( const QgsPointCloudIndex *pc, const QgsRenderContext &context, IndexedPointCloudNode n, float maxErrorPixels, float nodeErrorPixels );

    QgsPointCloudLayer *mLayer = nullptr;

    std::unique_ptr< QgsPointCloudRenderer > mRenderer;

    QgsPointCloudRendererConfig mConfig;

    QgsPointCloudAttributeCollection mAttributes;

    // int imgW, imgH; // DO WE NEED AT ALL?
    // QgsPointCloudDataBounds mBounds; // DO WE NEED AT ALL?

    // some stats
    int nodesDrawn = 0;
    int pointsDrawn = 0;

    void drawData( QgsPointCloudRenderContext &context, const QgsPointCloudBlock *data, const QgsPointCloudRendererConfig &config );

};

#endif // QGSPOINTCLOUDLAYERRENDERER_H
