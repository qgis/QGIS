/***************************************************************************
  qgs25drendererwidget.h - Qgs25DRendererWidget

 ---------------------
 begin                : 14.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGS25DRENDERERWIDGET_H
#define QGS25DRENDERERWIDGET_H

#include "ui_qgs25drendererwidgetbase.h"
#include "qgsrendererv2widget.h"
#include "qgs25drenderer.h"

/** \ingroup gui
 * \class Qgs25DRendererWidget
 */
class GUI_EXPORT Qgs25DRendererWidget : public QgsRendererV2Widget, Ui::Qgs25DRendererWidgetBase
{
    Q_OBJECT

  public:
    /** Static creation method
     * @param layer the layer where this renderer is applied
     * @param style
     * @param renderer the mask renderer (will take ownership)
     */
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    /** Constructor
     * @param layer the layer where this renderer is applied
     * @param style
     * @param renderer the mask renderer (will take ownership)
     */
    Qgs25DRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsFeatureRendererV2* renderer() override;

  private slots:
    void updateRenderer();

  private:
    void apply() override;

    Qgs25DRenderer* mRenderer;
};

#endif // QGS25DRENDERERWIDGET_H
