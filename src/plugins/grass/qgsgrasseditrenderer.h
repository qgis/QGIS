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

    ~QgsGrassEditRenderer() override;

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;

    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;

    void stopRender( QgsRenderContext &context ) override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

    QgsFeatureRenderer *clone() const override;

    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    QString dump() const override;

    QgsFeatureRenderer *lineRenderer() const { return mLineRenderer; }
    QgsFeatureRenderer *pointRenderer() const { return mMarkerRenderer; }

    void setLineRenderer( QgsFeatureRenderer *renderer );
    void setMarkerRenderer( QgsFeatureRenderer *renderer );

    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;

    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context );

  protected:
    QgsFeatureRenderer *mLineRenderer = nullptr;
    QgsFeatureRenderer *mMarkerRenderer = nullptr;
};

class QgsGrassEditRendererWidget : public QgsRendererWidget
{
    Q_OBJECT
  public:
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );

    QgsGrassEditRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsGrassEditRendererWidget() override;

    QgsFeatureRenderer *renderer() override;

  protected:
    QgsGrassEditRenderer *mRenderer = nullptr;

    QgsRendererWidget *mLineRendererWidget = nullptr;
    QgsRendererWidget *mPointRendererWidget = nullptr;
};

#endif // QGSGRASSEDITRENDERER_H
