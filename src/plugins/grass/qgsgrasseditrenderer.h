/***************************************************************************
    qgsgrasseditrenderer.h
                             -------------------
    begin                : February, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSEDITRENDERER_H
#define QGSGRASSEDITRENDERER_H

#include "qgis.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"

#include "qgscategorizedsymbolrendererv2.h"
#include "qgsrendererv2widget.h"

class QgsGrassEditRenderer : public QgsFeatureRendererV2
{
  public:

    QgsGrassEditRenderer();

    virtual ~QgsGrassEditRenderer();

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QList<QString> usedAttributes() override;

    virtual QgsFeatureRendererV2* clone() const override;

    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) override;

    virtual QString dump() const override;

    QgsFeatureRendererV2 *lineRenderer() const { return mLineRenderer; }
    QgsFeatureRendererV2 *pointRenderer() const { return mMarkerRenderer; }

    void setLineRenderer( QgsFeatureRendererV2 *renderer );
    void setMarkerRenderer( QgsFeatureRendererV2 *renderer );

    virtual QDomElement save( QDomDocument& doc ) override;

    static QgsFeatureRendererV2* create( QDomElement& element );

  protected:
    QgsFeatureRendererV2 *mLineRenderer;
    QgsFeatureRendererV2 *mMarkerRenderer;
};

class QgsGrassEditRendererWidget : public QgsRendererV2Widget
{
    Q_OBJECT
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsGrassEditRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsGrassEditRendererWidget();

    virtual QgsFeatureRendererV2* renderer() override;

  protected:
    QgsGrassEditRenderer* mRenderer;

    QgsRendererV2Widget* mLineRendererWidget;
    QgsRendererV2Widget* mPointRendererWidget;
};

#endif // QGSGRASSEDITRENDERER_H
