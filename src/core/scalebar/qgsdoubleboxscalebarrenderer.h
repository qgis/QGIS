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
 * \brief Double box with alternating colors.
 */
class CORE_EXPORT QgsDoubleBoxScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    QgsDoubleBoxScaleBarRenderer() = default;

    QString id() const override;
    QString visibleName() const override;
    Flags flags() const override;
    int sortKey() const override;
    QgsDoubleBoxScaleBarRenderer *clone() const override SIP_FACTORY;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

};

#endif // QGSDOUBLEBOXSCALEBARRENDERER_H
