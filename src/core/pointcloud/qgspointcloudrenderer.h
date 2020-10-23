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


class CORE_EXPORT QgsPointCloudRendererConfig
{
  public:
    QgsPointCloudRendererConfig();
    QgsPointCloudRendererConfig( const QgsPointCloudRendererConfig &other );
    QgsPointCloudRendererConfig &operator= ( const QgsPointCloudRendererConfig &other );

    double zMin() const;
    void setZMin( double value );

    double zMax() const;
    void setZMax( double value );

    int penWidth() const;
    void setPenWidth( int value );

    QgsColorRamp *colorRamp() const;

    // TODO should it clone?
    void setColorRamp( const QgsColorRamp *value );

  private:
    double mZMin = 0, mZMax = 0;
    int mPenWidth = 1;
    std::unique_ptr<QgsColorRamp> mColorRamp;
};


/**
 * \ingroup core
 *
 * Represents a 2D renderer of point cloud data
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRenderer: public QgsMapLayerRenderer
{
  public:

    explicit QgsPointCloudRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context );
    ~QgsPointCloudRenderer();

    bool render() override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

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


#endif // QGSPOINTCLOUDRENDERER_H
