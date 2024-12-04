/***************************************************************************
  qgsinterpolatedlinesymbollayerwidget.h - QgsInterpolatedLineSymbolLayerWidget

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
#ifndef QGSINTERPOLATEDLINESYMBOLLAYERWIDGET_H
#define QGSINTERPOLATEDLINESYMBOLLAYERWIDGET_H

#include "qgsrendererwidget.h"
#include "qgssymbollayerwidget.h"
#include "qgsinterpolatedlinerenderer.h"
#include "ui_qgsinterpolatedlinesymbollayerwidgetbase.h"

/**
 * \ingroup gui
 * \brief QgsInterpolatedLineSymbolLayerWidget
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsInterpolatedLineSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::QgsInterpolatedLineSymbolLayerWidgetBase
{
    Q_OBJECT
  public:
    /**
     * Constructor
     * \param layer the layer where this symbol layer is applied
     * \param parent the parent widget
     */
    QgsInterpolatedLineSymbolLayerWidget( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Static creation method
     * \param layer the layer where this symbol layer is applied
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *layer ) SIP_FACTORY { return new QgsInterpolatedLineSymbolLayerWidget( layer ); }

    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  private slots:
    void apply();
    void updateVisibleWidget();
    void onReloadMinMaxValueWidth();
    void onReloadMinMaxValueColor();
    void reloadMinMaxWidthFromLayer();
    void reloadMinMaxColorFromLayer();
    void onColorMinMaxLineTextChanged();
    void onColorMinMaxLineTextEdited();

  private:
    QgsInterpolatedLineSymbolLayer *mLayer = nullptr;

    double mMinimumForWidthFromLayer = std::numeric_limits<double>::quiet_NaN();
    double mMaximumForWidthFromLayer = std::numeric_limits<double>::quiet_NaN();

    double mMaximumForColorFromLayer = std::numeric_limits<double>::quiet_NaN();
    double mMinimumForColorFromLayer = std::numeric_limits<double>::quiet_NaN();

    QgsInterpolatedLineWidth interpolatedLineWidth();
    QgsInterpolatedLineColor interpolatedLineColor();
    double lineEditValue( QLineEdit *lineEdit );
    void setLineEditValue( QLineEdit *lineEdit, double value );
};

#endif // QGSINTERPOLATEDLINESYMBOLLAYERWIDGET_H
