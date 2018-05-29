/***************************************************************************
                            qgsscalebarrenderer.h
                            ---------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALEBARRENDERER_H
#define QGSSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QRectF>
#include <QList>

class QgsRenderContext;
class QgsScaleBarSettings;

/**
 * \ingroup core
 * \class QgsScaleBarRenderer
 * Abstract base class for scale bar renderers.
 *
 * Scalebar renderer subclasses implement custom drawing logic, with the possibility to implement
 * custom labeling.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsScaleBarRenderer
{
  public:

    /**
     * Contains parameters regarding scalebar calculations.
     * \note The need to attribute the parameters vary depending on the targeted scalebar.
     */
    struct ScaleBarContext
    {

      /**
       * The width, in millimeters, of each individual segment drawn.
       * \note The number of map units per segment needs to be set via QgsScaleBarSettings::setUnitsPerSegment.
       */
      double segmentWidth { 0.0 };

      /**
       * Destination size for scalebar. This is used for scalebars which
       * alter their appearance or alignment based on the desired scalebar
       * size (e.g. correctly aligning text in a numeric scale bar).
       */
      QSizeF size;

      //! Scale denominator
      double scale { 1.0 };

    };

    /**
     * Constructor for QgsScaleBarRenderer.
     */
    QgsScaleBarRenderer() = default;
    virtual ~QgsScaleBarRenderer() = default;

    /**
     * Returns the unique name for this style.
     */
    virtual QString name() const = 0;

    /**
     * Draws the scalebar using the specified \a settings and \a scaleContext to a destination render \a context.
     */
    virtual void draw( QgsRenderContext &context,
                       const QgsScaleBarSettings &settings,
                       const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const = 0;

    /**
     * Calculates the required box size (in millimeters) for a scalebar using the specified \a settings and \a scaleContext.
     */
    virtual QSizeF calculateBoxSize( const QgsScaleBarSettings &settings,
                                     const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

  protected:

    /**
     * Draws default scalebar labels using the specified \a settings and \a scaleContext to a destination render \a context.
     */
    void drawDefaultLabels( QgsRenderContext &context,
                            const QgsScaleBarSettings &settings,
                            const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const;

    /**
     * Returns the text used for the first label in the scalebar.
     */
    QString firstLabelString( const QgsScaleBarSettings &settings ) const;

    /**
     * Returns the x-offset (in millimeters) used for the first label in the scalebar.
     * \deprecated Use the version with QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED double firstLabelXOffset( const QgsScaleBarSettings &settings ) const SIP_DEPRECATED;

    /**
     * Returns the x-offset (in render context painter units) used for the first label in the scalebar.
     * \since QGIS 3.2
     */
    double firstLabelXOffset( const QgsScaleBarSettings &settings, const QgsRenderContext &context ) const;

    /**
     * Returns a list of positions for each segment within the scalebar.
     */
    QList<double> segmentPositions( const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const;

    /**
     * Returns a list of widths of each segment of the scalebar.
     */
    QList<double> segmentWidths( const QgsScaleBarRenderer::ScaleBarContext &scaleContext, const QgsScaleBarSettings &settings ) const;

};

#endif //QGSSCALEBARRENDERER_H
