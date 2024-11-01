/***************************************************************************
    qgsembeddedsymbolrendererwidget.h
    ---------------------
    begin                : March 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEMBEDDEDSYMBOLRENDERERWIDGET_H
#define QGSEMBEDDEDSYMBOLRENDERERWIDGET_H

#include "ui_qgsembeddedsymbolrendererwidgetbase.h"
#include "qgis_sip.h"
#include "qgsrendererwidget.h"
#include "qgis_gui.h"

class QMenu;
class QgsEmbeddedSymbolRenderer;

/**
 * \ingroup gui
 * \brief A widget used represent options of a QgsEmbeddedSymbolRenderer
 *
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsEmbeddedSymbolRendererWidget : public QgsRendererWidget, private Ui::QgsEmbeddedSymbolRendererWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Static creation method
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the merged feature renderer (will not take ownership)
     */
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Constructor
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the merged feature renderer (will not take ownership)
     */
    QgsEmbeddedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsEmbeddedSymbolRendererWidget() override;

    QgsFeatureRenderer *renderer() override;
    void setContext( const QgsSymbolWidgetContext &context ) override;
    QgsExpressionContext createExpressionContext() const override;

  private:
    //! The renderer
    std::unique_ptr<QgsEmbeddedSymbolRenderer> mRenderer;
};


#endif // QGSEMBEDDEDSYMBOLRENDERERWIDGET_H
