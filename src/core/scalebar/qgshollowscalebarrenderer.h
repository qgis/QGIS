/***************************************************************************
                            qgshollowscalebarrenderer.h
                            ------------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHOLLOWSCALEBARRENDERER_H
#define QGSHOLLOWSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgsscalebarrenderer.h"
#include <QString>

/**
 * \class QgsHollowScaleBarRenderer
 * \ingroup core
 * \brief Scalebar style that draws a single box with alternating color for the segments, with horizontal lines through
 * alternating segments. AKA "South African" style.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsHollowScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    QgsHollowScaleBarRenderer() = default;

    QString id() const override;
    QString visibleName() const override;
    Flags flags() const override;
    int sortKey() const override;
    QgsHollowScaleBarRenderer *clone() const override SIP_FACTORY;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;
    bool applyDefaultSettings( QgsScaleBarSettings &settings ) const override;

};

#endif // QGSHOLLOWSCALEBARRENDERER_H
