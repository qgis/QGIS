/***************************************************************************
    qgsvectorfieldengine.h
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDENGINE_H
#define QGSVECTORFIELDENGINE_H

#include <memory>

#include "qgsinterpolatedlinerenderer.h"
#include "qgsvectorfieldsettings.h"

#include <QSize>

#define SIP_NO_FILE

class QgsCoordinateTransform;
class QgsPointXY;
class QgsRenderContext;
class QgsScopedQPainterState;

/**
 * \brief Class for rendering vector field data.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 4.4
 */
class QgsVectorFieldEngine
{
  public:
    //! Ctor
    QgsVectorFieldEngine( double datasetMagMaximumValue, double datasetMagMinimumValue, const QgsVectorFieldSettings &settings, QgsRenderContext &context, QSize size );
    //! Dtor
    ~QgsVectorFieldEngine();

    QgsVectorFieldEngine( const QgsVectorFieldEngine & ) = delete;
    QgsVectorFieldEngine &operator=( const QgsVectorFieldEngine & ) = delete;

    /**
     * Draws a single glyph at \a lineStart, in painter coordinates, using the symbology of the
     * settings the engine was constructed with.
     *
     * Does nothing for the symbologies which are not drawn glyph by glyph
     */
    void drawGlyph( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );

  private:
    /**
     * Draws a single arrow starting at \a lineStart, in painter coordinates.
     */
    void drawArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );

    /**
     * Draws a single wind barb centered on \a lineStart, in painter coordinates.
     */
    void drawWindBarb( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );

    //! Calculates the end point of the arrow based on start point and vector data
    bool calcVectorLineEnd(
      QgsPointXY &lineEnd,
      double &vectorLength,
      double &cosAlpha,
      double &sinAlpha, //out
      const QgsPointXY &lineStart,
      double xVal,
      double yVal,
      double magnitude //in
    );

    double mMinMag = 0.0;
    double mMaxMag = 0.0;

    QgsRenderContext &mContext;
    const QgsVectorFieldSettings mCfg;
    QgsInterpolatedLineColor mVectorColoring;

    QSize mOutputSize;

    std::unique_ptr<QgsCoordinateTransform> mGeographicTransform;
    std::unique_ptr<QgsScopedQPainterState> mScopedPainterState;
};

#endif // QGSVECTORFIELDENGINE_H
