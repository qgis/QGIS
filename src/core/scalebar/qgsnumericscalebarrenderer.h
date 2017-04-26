/***************************************************************************
                            qgsnumericscalebarrenderer.h
                            ----------------------------
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

#ifndef QGSNUMERICSCALEBARRENDERER_H
#define QGSNUMERICSCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgsscalebarrenderer.h"
#include <QString>

/**
 * \class QgsNumericScaleBarRenderer
 * \ingroup core
 * A scale bar style that draws text in the form of '1:XXXXX'.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsNumericScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    /**
     * Constructor for QgsNumericScaleBarRenderer.
     */
    QgsNumericScaleBarRenderer() = default;

    QString name() const override { return QStringLiteral( "Numeric" ); }

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

    QSizeF calculateBoxSize( const QgsScaleBarSettings &settings,
                             const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

  private:

    //! Returns the text for the scale bar or an empty string in case of error
    QString scaleText( double scale ) const;

};

#endif // QGSNUMERICSCALEBARRENDERER_H
