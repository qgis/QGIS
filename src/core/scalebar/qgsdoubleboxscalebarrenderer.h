/***************************************************************************
                            qgsdoubleboxscalebarrenderer.h
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

#ifndef QGSDOUBLEBOXSCALEBARRENDERER_H
#define QGSDOUBLEBOXSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgsscalebarrenderer.h"
#include <QString>

/**
 * \class QgsDoubleBoxScaleBarRenderer
 * \ingroup core
 * Double box with alternating colors.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDoubleBoxScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    /**
     * Constructor for QgsDoubleBoxScaleBarRenderer.
     */
    QgsDoubleBoxScaleBarRenderer() = default;

    QString name() const override { return QStringLiteral( "Double Box" ); }

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

};

#endif // QGSDOUBLEBOXSCALEBARRENDERER_H
