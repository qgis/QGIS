/***************************************************************************
                         qgstriangularmesh.h
                         -------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
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

#ifndef QGSMESHVECTORRENDERER_H
#define QGSMESHVECTORRENDERER_H


#define SIP_NO_FILE

#include <QVector>
#include <QSize>

#include "qgis_core.h"
#include "qgsmeshdataprovider.h"
#include "qgsrendercontext.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"

///@cond PRIVATE

/**
 * \ingroup core
 *
 * Helper private class for rendering vector datasets (e.g. velocity)
 *
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class QgsMeshVectorRenderer
{
  public:
    //! Ctor
    QgsMeshVectorRenderer( const QgsTriangularMesh &m,
                           const QgsMeshDataBlock &datasetValues,
                           const QVector<double> &datasetValuesMag,
                           double datasetMagMaximumValue,
                           double datasetMagMinimumValue,
                           bool dataIsOnVertices,
                           const QgsMeshRendererVectorSettings &settings,
                           QgsRenderContext &context,
                           QSize size );
    //! Dtor
    ~QgsMeshVectorRenderer();

    /**
     * Draws vector arrows in the context's painter based on settings
     */
    void draw();

  private:
    //! Draws for data defined on vertices
    void drawVectorDataOnVertices( const QList<int> &trianglesInExtent );
    //! Draws for data defined on face centers
    void drawVectorDataOnFaces( const QList<int> &trianglesInExtent );
    //! Draws data on user-defined grid
    void drawVectorDataOnGrid( const QList<int> &trianglesInExtent );
    //! Draws arrow from start point and vector data
    void drawVectorArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );
    //! Calculates the end point of the arrow based on start point and vector data
    bool calcVectorLineEnd( QgsPointXY &lineEnd,
                            double &vectorLength,
                            double &cosAlpha,
                            double &sinAlpha, //out
                            const QgsPointXY &lineStart,
                            double xVal,
                            double yVal,
                            double magnitude //in
                          );

    /**
     * Calculates the buffer size
     * needed to draw arrows which have
     * start or end point outside the
     * visible canvas extent (in pixels)
     */
    double calcExtentBufferSize() const;

    const QgsTriangularMesh &mTriangularMesh;
    const QgsMeshDataBlock &mDatasetValues;
    const QVector<double> &mDatasetValuesMag; //magnitudes
    double mMinMag = 0.0;
    double mMaxMag = 0.0;
    QgsRenderContext &mContext;
    const QgsMeshRendererVectorSettings &mCfg;
    bool mDataOnVertices = true;
    QSize mOutputSize;
    QgsRectangle mBufferedExtent;
};

///@endcond



#endif // QGSMESHVECTORRENDERER_H
