/***************************************************************************
                            qgsticksscalebarrenderer.h
                            --------------------------
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

#ifndef QGSTICKSSCALEBARRENDERER_H
#define QGSTICKSSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgsscalebarrenderer.h"

/**
 * \class QgsTicksScaleBarRenderer
 * \ingroup core
 * A scale bar that draws segments using short ticks.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsTicksScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    //! Tick positions
    enum TickPosition
    {
      TicksUp, //!< Render ticks above line
      TicksDown, //!< Render ticks below line
      TicksMiddle, //!< Render ticks crossing line
    };

    /**
     * Constructor for QgsTicksScaleBarRenderer.
     */
    QgsTicksScaleBarRenderer() = default;

    QString name() const override;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

    /**
     * Sets the \a position for tick marks in the scalebar.
     * \see tickPosition()
     */
    void setTickPosition( TickPosition position ) { mTickPosition = position; }

    /**
     * Returns the position for tick marks in the scalebar.
     * \see setTickPosition()
     */
    TickPosition tickPosition() const { return mTickPosition; }

  private:

    TickPosition mTickPosition = TicksMiddle;
};

#endif // QGSTICKSSCALEBARRENDERER_H
