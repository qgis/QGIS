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
#include "qgis.h"
#include "qgsrendererwidget.h"
#include "qgis_gui.h"

class Qgs25DRenderer;

/**
 * \ingroup gui
 * \class Qgs25DRendererWidget
 */
class GUI_EXPORT Qgs25DRendererWidget : public QgsRendererWidget, protected Ui::Qgs25DRendererWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Static creation method
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the mask renderer (will not take ownership)
     */
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer SIP_TRANSFER ) SIP_FACTORY;

    /**
     * Constructor
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the mask renderer (will not take ownership)
     */
    Qgs25DRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer SIP_TRANSFER );

    QgsFeatureRenderer *renderer() override;

  private slots:
    void updateRenderer();

  private:
    void apply() override SIP_FORCE;

    Qgs25DRenderer *mRenderer = nullptr;

    friend class QgsAppScreenShots;
};

#endif // QGS25DRENDERERWIDGET_H
