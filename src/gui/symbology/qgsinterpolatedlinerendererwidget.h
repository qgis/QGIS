/***************************************************************************
  qgsinterpolatedlinerendererwidget.h - QgsInterpolatedLineRendererWidget

 ---------------------
 begin                : 23.3.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINTERPOLATEDLINERENDERERWIDGET_H
#define QGSINTERPOLATEDLINERENDERERWIDGET_H

#include "qgsrendererwidget.h"
#include "qgsinterpolatedlinerenderer.h"
#include "ui_qgsinterpolatedlinerendererwidgetbase.h"


/**
 * \ingroup gui
 * \class QgsInterpolatedLineRendererWidget
 * \since QGIS 3.20
 */
class QgsInterpolatedLineRendererWidget : public QgsRendererWidget, private Ui::QgsInterpolatedLineRendererWidgetBase,  private QgsExpressionContextGenerator
{
    Q_OBJECT
  public:

    /**
     * Static creation method
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the renderer (will not take ownership)
     */
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Constructor
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the mask renderer (will not take ownership)
     */
    QgsInterpolatedLineRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );


    QgsFeatureRenderer *renderer() override;

  private slots:
    void reloadMinMaxFromLayer();
    void onLoadColorMinmax();
    void onColorMinMaxLineTextChanged();
    void onColorMinMaxLineTextEdited();
    void onFixedWidthChanged();

  private:
    std::unique_ptr<QgsInterpolatedLineFeatureRenderer> mRenderer;

    double mMaximumFromLayer = std::numeric_limits<double>::quiet_NaN();
    double mMinimumFromLayer = std::numeric_limits<double>::quiet_NaN();

    QgsExpressionContext createExpressionContext() const override;

    double lineEditValue( QLineEdit *lineEdit );
    void setLineEditValue( QLineEdit *lineEdit, double value );
    void syncToRenderer();
};

#endif // QGSINTERPOLATEDLINERENDERERWIDGET_H
