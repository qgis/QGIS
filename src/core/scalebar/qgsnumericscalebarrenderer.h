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
 * \brief A scale bar style that draws text in the form of '1:XXXXX'.
 */
class CORE_EXPORT QgsNumericScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    QgsNumericScaleBarRenderer() = default;

    QString id() const override;
    QString visibleName() const override;
    int sortKey() const override;
    Flags flags() const override;
    QgsNumericScaleBarRenderer *clone() const override SIP_FACTORY;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

    QSizeF calculateBoxSize( QgsRenderContext &context,
                             const QgsScaleBarSettings &settings,
                             const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

    /**
     * \deprecated QGIS 3.14. Use the one with render context instead.
     */
    Q_DECL_DEPRECATED QSizeF calculateBoxSize( const QgsScaleBarSettings &settings, const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override SIP_DEPRECATED ;

  private:

    //! Returns the text for the scale bar or an empty string in case of error
    QString scaleText( double scale, const QgsScaleBarSettings &settings ) const;

};

#endif // QGSNUMERICSCALEBARRENDERER_H
