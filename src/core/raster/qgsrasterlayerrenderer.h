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

#include "qgsmaplayerrenderer.h"

class QPainter;

class QgsMapToPixel;
class QgsRasterBlockFeedback;
class QgsRasterLayer;
class QgsRasterPipe;
struct QgsRasterViewPort;
class QgsRenderContext;

class QgsRasterLayerRenderer;

#include "qgsrasterinterface.h"


/** \ingroup core
 * Implementation of threaded rendering for raster layers.
 *
 * @note added in 2.4
 * @note not available in Python bindings
 */
class QgsRasterLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsRasterLayerRenderer( QgsRasterLayer* layer, QgsRenderContext& rendererContext );
    ~QgsRasterLayerRenderer();

    virtual bool render() override;

    virtual QgsFeedback* feedback() const override;

  protected:

    QPainter* mPainter;
    const QgsMapToPixel* mMapToPixel;
    QgsRasterViewPort* mRasterViewPort;

    QgsRasterPipe* mPipe;
    QgsRenderContext& mContext;

    /** \ingroup core
     * Specific internal feedback class to provide preview of raster layer rendering.
     * @note added in 3.0
     * @note not available in Python bindings
     */
    class Feedback : public QgsRasterBlockFeedback
    {
      public:
        //! Create feedback object based on our layer renderer
        explicit Feedback( QgsRasterLayerRenderer* r );

        //! when notified of new data in data provider it launches a preview draw of the raster
        virtual void onNewData() override;
      private:
        QgsRasterLayerRenderer* mR;   //!< parent renderer instance
        int mMinimalPreviewInterval;  //!< in miliseconds
        QTime mLastPreview;           //!< when last preview has been generated
    };

    //! feedback class for cancellation and preview generation
    Feedback* mFeedback;
};


#endif // QGSRASTERLAYERRENDERER_H
