/***************************************************************************
                            qgssteppedlinescalebarrenderer.h
                            --------------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTEPPEDLINESCALEBARRENDERER_H
#define QGSSTEPPEDLINESCALEBARRENDERER_H

#include "qgis_core.h"
#include "qgsscalebarrenderer.h"
#include <QString>

/**
 * \class QgsSteppedLineScaleBarRenderer
 * \ingroup core
 * Scalebar style that draws a stepped line representation of a scalebar.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsSteppedLineScaleBarRenderer: public QgsScaleBarRenderer
{
  public:

    /**
     * Constructor for QgsSteppedLineScaleBarRenderer.
     */
    QgsSteppedLineScaleBarRenderer() = default;

    QString id() const override;
    QString visibleName() const override;
    int sortKey() const override;
    Flags flags() const override;
    QgsSteppedLineScaleBarRenderer *clone() const override SIP_FACTORY;

    void draw( QgsRenderContext &context,
               const QgsScaleBarSettings &settings,
               const QgsScaleBarRenderer::ScaleBarContext &scaleContext ) const override;

};

#endif // QGSSTEPPEDLINESCALEBARRENDERER_H
