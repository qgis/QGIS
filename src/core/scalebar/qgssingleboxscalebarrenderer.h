/***************************************************************************
                            qgssingleboxscalebarrenderer.h
                            ------------------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
 * Scalebar style that draws a single box with alternating
 * color for the segments.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsSingleBoxScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    /**
     * Constructor for QgsSingleBoxScaleBarRenderer.
     */
    QgsSingleBoxScaleBarRenderer() = default;

    QString id() const override;
    QString visibleName() const override;
    int sortKey() const override;
    Flags flags() const override;
    QgsSingleBoxScaleBarRenderer *clone() const override SIP_FACTORY;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

};

#endif // QGSSINGLEBOXSCALEBARRENDERER_H
