/***************************************************************************
    qgsheatmaprendererwidget.h
    --------------------------
    begin                : November 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHEATMAPRENDERERWIDGET_H
#define QGSHEATMAPRENDERERWIDGET_H

#include "ui_qgsheatmaprendererwidgetbase.h"
#include "qgsheatmaprenderer.h"
#include "qgsrendererv2widget.h"

class QMenu;

class GUI_EXPORT QgsHeatmapRendererWidget : public QgsRendererV2Widget, private Ui::QgsHeatmapRendererWidgetBase
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
    QgsHeatmapRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    /** @returns the current feature renderer */
    virtual QgsFeatureRendererV2* renderer() override;

  protected:
    QgsHeatmapRenderer* mRenderer;

  private slots:

    void applyColorRamp();
    void on_mRadiusUnitWidget_changed();
    void on_mRadiusSpinBox_valueChanged( double d );
    void on_mMaxSpinBox_valueChanged( double d );
    void on_mQualitySlider_valueChanged( int v );
    void on_mInvertCheckBox_toggled( bool v );
    void weightExpressionChanged( QString expression );

};


#endif // QGSHEATMAPRENDERERWIDGET_H
