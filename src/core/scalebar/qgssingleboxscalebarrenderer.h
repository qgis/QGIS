/***************************************************************************
                            qgssingleboxscalebarrenderer.h
                            ------------------------------
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

#ifndef QGSSINGLEBOXSCALEBARRENDERER_H
#define QGSSINGLEBOXSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgsscalebarrenderer.h"

#include <QString>

/**
 * \class QgsSingleBoxScaleBarRenderer
 * \ingroup core
 * \brief Scalebar style that draws a single box with alternating
 * color for the segments.
 */
class CORE_EXPORT QgsSingleBoxScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    QgsSingleBoxScaleBarRenderer() = default;

    [[nodiscard]] QString id() const override;
    [[nodiscard]] QString visibleName() const override;
    [[nodiscard]] int sortKey() const override;
    [[nodiscard]] Flags flags() const override;
    [[nodiscard]] QgsSingleBoxScaleBarRenderer *clone() const override SIP_FACTORY;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

    bool applyDefaultSettings( QgsScaleBarSettings &settings ) const override;
};

#endif // QGSSINGLEBOXSCALEBARRENDERER_H
