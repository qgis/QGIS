/***************************************************************************
                         qgspointcloudrenderer.h
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

#ifndef QGSPOINTCLOUDRENDERER_H
#define QGSPOINTCLOUDRENDERER_H

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

#ifndef SIP_RUN
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
    //! Assigment contructor
    QgsPointCloudRendererConfig &operator= ( const QgsPointCloudRendererConfig &other );

    //! Returns z min
    double zMin() const;
    //! Sets z min
    void setZMin( double value );

    //! Returns z max
    double zMax() const;

    //! Sets z max
    void setZMax( double value );

    //! Retuns pen width
    int penWidth() const;

    //! Sets pen width
    void setPenWidth( int value );

    //! Returns color ramp
    QgsColorRamp *colorRamp() const;

    //! Sets color ramp
    void setColorRamp( const QgsColorRamp *value );

  private:
    double mZMin = 0, mZMax = 0;
    int mPenWidth = 1;
    std::unique_ptr<QgsColorRamp> mColorRamp;
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
    QgsPointCloudLayer *mLayer = nullptr;

    QgsPointCloudRendererConfig mConfig;

    // int imgW, imgH; // DO WE NEED AT ALL?
    // QgsPointCloudDataBounds mBounds; // DO WE NEED AT ALL?

    // some stats
    int nodesDrawn = 0;
    int pointsDrawn = 0;

    void drawData( QPainter *painter, const QVector<qint32> &data, const QgsPointCloudRendererConfig &config );
};
#endif


#endif // QGSPOINTCLOUDRENDERER_H
