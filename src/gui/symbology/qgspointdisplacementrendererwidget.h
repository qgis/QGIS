/***************************************************************************
                              qgspointdisplacementrendererwidget.h
                              ------------------------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTDISPLACEMENTRENDERERWIDGET_H
#define QGSPOINTDISPLACEMENTRENDERERWIDGET_H

#include "ui_qgspointdisplacementrendererwidgetbase.h"
#include "qgis_sip.h"
#include "qgsrendererwidget.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgis_gui.h"

class QgsPointDisplacementRenderer;

/**
 * \ingroup gui
 * \class QgsPointDisplacementRendererWidget
 */
class GUI_EXPORT QgsPointDisplacementRendererWidget: public QgsRendererWidget, public QgsExpressionContextGenerator, private Ui::QgsPointDisplacementRendererWidgetBase
{
    Q_OBJECT
  public:
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;
    QgsPointDisplacementRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsPointDisplacementRendererWidget() override;

    QgsFeatureRenderer *renderer() override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

    QgsExpressionContext createExpressionContext() const override;

  private:
    QgsPointDisplacementRenderer *mRenderer = nullptr;

    void blockAllSignals( bool block );
    void setupBlankUi( const QString &layerName );

  private slots:
    void mLabelFieldComboBox_currentIndexChanged( const QString &text );
    void mRendererComboBox_currentIndexChanged( int index );
    void mPlacementComboBox_currentIndexChanged( int index );
    void labelFontChanged();
    void mCircleWidthSpinBox_valueChanged( double d );
    void mCircleColorButton_colorChanged( const QColor &newColor );
    void mDistanceSpinBox_valueChanged( double d );
    void mDistanceUnitWidget_changed();
    void mLabelColorButton_colorChanged( const QColor &newColor );
    void mCircleModificationSpinBox_valueChanged( double d );
    void mLabelDistanceFactorSpinBox_valueChanged( double d );
    void mScaleDependentLabelsCheckBox_stateChanged( int state );
    void minLabelScaleChanged( double scale );
    void mRendererSettingsButton_clicked();
    void centerSymbolChanged();
    void updateRendererFromWidget();
};

#endif // QGSPOINTDISPLACEMENTRENDERERWIDGET_H
