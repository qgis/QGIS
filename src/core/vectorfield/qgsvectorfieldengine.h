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
#include "qgspointxy.h"
#include "qgsvectorfieldsettings.h"

#include <QSize>

#define SIP_NO_FILE

class QgsCoordinateTransform;
class QgsFeedback;
class QgsRasterBlockFeedback;
class QgsRenderContext;
class QgsScopedQPainterState;
class QgsVectorFieldValueSource;

/**
 * \ingroup core
 *
 * \brief Responsible for rendering vector field data.
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
     * Returns the distance, in painter units, by which the rendered extent must be grown so that
     * glyphs centered just outside of it are still drawn.
     *
     * Returns 0 for the symbologies which are not drawn glyph by glyph.
     */
    double glyphExtentBuffer() const;

    /**
     * Draws a single glyph at \a lineStart, in painter coordinates, using the symbology of the
     * settings the engine was constructed with.
     *
     * Does nothing for the symbologies which are not drawn glyph by glyph, see drawStreamlines()
     * and drawTraces().
     */
    void drawGlyph( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );

    /**
     * Draws one glyph per position of the placement grid described by the settings the engine was
     * constructed with, sampling the vector field from \a source, which the engine takes ownership of.
     *
     * The grid is laid out from \a anchor, a point which does not depend on how the render is split
     * into blocks, so that the same pattern is drawn across block boundaries, and falls back to the
     * layout of \a source itself when the settings do not ask for a user defined grid.
     *
     * Does nothing for the symbologies which are not drawn glyph by glyph, see drawStreamlines()
     * and drawTraces().
     */
    void drawGlyphs( std::unique_ptr<QgsVectorFieldValueSource> source, const QgsPointXY &anchor, QgsFeedback *feedback = nullptr );

    /**
     * Integrates and draws streamlines over the whole rendered extent, sampling the vector field
     * from \a source, which the engine takes ownership of.
     *
     * \a feedback is used to interrupt the rendering of the color ramp background image.
     */
    void drawStreamlines( std::unique_ptr<QgsVectorFieldValueSource> source, QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * Seeds particles over the whole rendered extent, moves them one time step and draws their
     * traces, sampling the vector field from \a source, which the engine takes ownership of.
     */
    void drawTraces( std::unique_ptr<QgsVectorFieldValueSource> source );

  private:
    //! The positions at which the glyphs of the rendered extent are drawn, in map coordinates
    struct GlyphLayout
    {
        QgsPointXY origin;
        double spacingX = 0;
        double spacingY = 0;

        //! Returns TRUE if the layout describes a grid which can be walked
        bool isValid() const { return spacingX > 0 && spacingY > 0; }
    };

    /**
     * Returns the positions at which the glyphs of the rendered extent are drawn, laid out from
     * \a anchor, falling back to the layout of \a source when the settings do not ask for a user
     * defined grid.
     */
    GlyphLayout glyphLayout( const QgsPointXY &anchor, const QgsVectorFieldValueSource &source ) const;

    //! Draws a single arrow starting at \a lineStart, in painter coordinates.
    void drawArrow( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );

    //! Draws a single wind barb centered on \a lineStart, in painter coordinates.
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
