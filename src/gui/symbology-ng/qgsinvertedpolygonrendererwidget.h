/***************************************************************************
    qgsinvertedpolygonrendererwidget.h
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINVERTEDPOLYGONRENDERERWIDGET_H
#define QGSINVERTEDPOLYGONRENDERERWIDGET_H

#include "ui_qgsinvertedpolygonrendererwidgetbase.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgsrendererv2widget.h"

class QMenu;

/**
 * A widget used represent options of a QgsInvertedPolygonRenderer
 *
 * @note added in 2.4
 */
class GUI_EXPORT QgsInvertedPolygonRendererWidget : public QgsRendererV2Widget, private Ui::QgsInvertedPolygonRendererWidgetBase
{
    Q_OBJECT

  public:
    /** static creation method
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
    QgsInvertedPolygonRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    /** @returns the current feature renderer */
    virtual QgsFeatureRendererV2* renderer() override;

  protected:
    /** the mask renderer */
    QScopedPointer<QgsInvertedPolygonRenderer> mRenderer;
    /** the widget used to represent the mask's embedded renderer */
    QScopedPointer<QgsRendererV2Widget> mEmbeddedRendererWidget;

  private slots:
    void on_mRendererComboBox_currentIndexChanged( int index );
    void on_mMergePolygonsCheckBox_stateChanged( int state );
};


#endif // QGSMASKRENDERERV2WIDGET_H
