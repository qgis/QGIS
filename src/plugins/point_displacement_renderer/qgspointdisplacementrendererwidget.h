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
#include "qgsrendererv2widget.h"

class QgsPointDisplacementRenderer;

class QgsPointDisplacementRendererWidget: public QgsRendererV2Widget, private Ui::QgsPointDisplacementRendererWidgetBase
{
    Q_OBJECT
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    QgsPointDisplacementRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsPointDisplacementRendererWidget();

    QgsFeatureRendererV2* renderer();

  private:
    QgsPointDisplacementRenderer* mRenderer;
    QgsRendererV2Widget* mEmbeddedRendererWidget;

    void blockAllSignals( bool block );
    void updateCenterIcon();
    void setupBlankUi( const QString& layerName );

  private slots:
    void on_mLabelFieldComboBox_currentIndexChanged( const QString& text );
    void on_mRendererComboBox_currentIndexChanged( int index );
    void on_mLabelFontButton_clicked();
    void on_mCircleWidthSpinBox_valueChanged( double d );
    void on_mCircleColorButton_clicked();
    void on_mLabelColorButton_clicked();
    void on_mCircleModificationSpinBox_valueChanged( double d );
    void on_mScaleDependentLabelsCheckBox_stateChanged( int state );
    void on_mMaxScaleDenominatorEdit_textChanged( const QString & text );
    void on_mCenterSymbolPushButton_clicked();
    void on_mRendererSettingsButton_clicked();
};

#endif // QGSPOINTDISPLACEMENTRENDERERWIDGET_H
