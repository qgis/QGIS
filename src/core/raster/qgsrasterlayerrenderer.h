/***************************************************************************
  qgsrasterlayerrenderer.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERLAYERRENDERER_H
#define QGSRASTERLAYERRENDERER_H

#define SIP_NO_FILE

#include "qgsmaplayerrenderer.h"

class QPainter;

class QgsMapToPixel;
class QgsRasterLayer;
class QgsRasterPipe;
struct QgsRasterViewPort;
class QgsRenderContext;

class QgsRasterLayerRenderer;

#include "qgsrasterinterface.h"

///@cond PRIVATE

/**
 * \ingroup core
 * Specific internal feedback class to provide preview of raster layer rendering.
 * \since QGIS 3.0
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsRasterLayerRendererFeedback : public QgsRasterBlockFeedback
{
    Q_OBJECT

  public:
    //! Create feedback object based on our layer renderer
    explicit QgsRasterLayerRendererFeedback( QgsRasterLayerRenderer *r );

    //! when notified of new data in data provider it launches a preview draw of the raster
    virtual void onNewData() override;
  private:
    QgsRasterLayerRenderer *mR = nullptr;   //!< Parent renderer instance
    int mMinimalPreviewInterval;  //!< In milliseconds
    QTime mLastPreview;           //!< When last preview has been generated
};

///@endcond

/**
 * \ingroup core
 * Implementation of threaded rendering for raster layers.
 *
 * \since QGIS 2.4
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsRasterLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsRasterLayerRenderer( QgsRasterLayer *layer, QgsRenderContext &rendererContext );
    ~QgsRasterLayerRenderer();

    virtual bool render() override;

    virtual QgsFeedback *feedback() const override;

  private:

    QPainter *mPainter = nullptr;
    const QgsMapToPixel *mMapToPixel = nullptr;
    QgsRasterViewPort *mRasterViewPort = nullptr;

    QgsRasterPipe *mPipe = nullptr;
    QgsRenderContext &mContext;

    //! feedback class for cancelation and preview generation
    QgsRasterLayerRendererFeedback *mFeedback = nullptr;

    friend class QgsRasterLayerRendererFeedback;
};


#endif // QGSRASTERLAYERRENDERER_H
