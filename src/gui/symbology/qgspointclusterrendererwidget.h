/***************************************************************************
                              qgspointclusterrendererwidget.h
                              -------------------------------
  begin                : February 2016
  copyright            : (C) 2016 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLUSTERRENDERERWIDGET_H
#define QGSPOINTCLUSTERRENDERERWIDGET_H

#include "ui_qgspointclusterrendererwidgetbase.h"
#include "qgis_sip.h"
#include "qgsrendererwidget.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgis_gui.h"

class QgsPointClusterRenderer;

/**
 * \class QgsPointClusterRendererWidget
 * \ingroup gui
 * A widget which allows configuration of the properties for a QgsPointClusterRenderer.
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsPointClusterRendererWidget: public QgsRendererWidget, public QgsExpressionContextGenerator, private Ui::QgsPointClusterRendererWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Returns a new QgsPointClusterRendererWidget.
     * \param layer associated vector layer
     * \param style style collection
     * \param renderer source QgsPointClusterRenderer renderer
     * \returns new QgsRendererWidget
     */
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Constructor for QgsPointClusterRendererWidget.
     * \param layer associated vector layer
     * \param style style collection
     * \param renderer source QgsPointClusterRenderer renderer
     */
    QgsPointClusterRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );

    ~QgsPointClusterRendererWidget() override;

    QgsFeatureRenderer *renderer() override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

    QgsExpressionContext createExpressionContext() const override;

  private:
    QgsPointClusterRenderer *mRenderer = nullptr;

    void blockAllSignals( bool block );
    void setupBlankUi( const QString &layerName );

  private slots:

    void mRendererComboBox_currentIndexChanged( int index );
    void mDistanceSpinBox_valueChanged( double d );
    void mDistanceUnitWidget_changed();
    void mRendererSettingsButton_clicked();
    void centerSymbolChanged();
    void updateRendererFromWidget();
};

#endif // QGSPOINTCLUSTERRENDERERWIDGET_H
