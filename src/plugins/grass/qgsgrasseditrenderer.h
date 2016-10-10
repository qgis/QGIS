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
#include "qgscategorizedsymbolrenderer.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"

#include "qgscategorizedsymbolrenderer.h"
#include "qgsrendererwidget.h"

class QgsGrassEditRenderer : public QgsFeatureRenderer
{
  public:

    QgsGrassEditRenderer();

    virtual ~QgsGrassEditRenderer();

    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    virtual void stopRender( QgsRenderContext& context ) override;

    virtual QSet<QString> usedAttributes() const override;

    virtual QgsFeatureRenderer* clone() const override;

    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;

    virtual QString dump() const override;

    QgsFeatureRenderer *lineRenderer() const { return mLineRenderer; }
    QgsFeatureRenderer *pointRenderer() const { return mMarkerRenderer; }

    void setLineRenderer( QgsFeatureRenderer *renderer );
    void setMarkerRenderer( QgsFeatureRenderer *renderer );

    virtual QDomElement save( QDomDocument& doc ) override;

    static QgsFeatureRenderer* create( QDomElement& element );

  protected:
    QgsFeatureRenderer *mLineRenderer;
    QgsFeatureRenderer *mMarkerRenderer;
};

class QgsGrassEditRendererWidget : public QgsRendererWidget
{
    Q_OBJECT
  public:
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );

    QgsGrassEditRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );
    ~QgsGrassEditRendererWidget();

    virtual QgsFeatureRenderer* renderer() override;

  protected:
    QgsGrassEditRenderer* mRenderer;

    QgsRendererWidget* mLineRendererWidget;
    QgsRendererWidget* mPointRendererWidget;
};

#endif // QGSGRASSEDITRENDERER_H
