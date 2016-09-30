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
#include "qgsrendererwidget.h"

class QgsPointClusterRenderer;

/** \class QgsPointClusterRendererWidget
 * \ingroup gui
 * A widget which allows configuration of the properties for a QgsPointClusterRenderer.
 * \note added in QGIS 3.0
 */

class GUI_EXPORT QgsPointClusterRendererWidget: public QgsRendererWidget, private Ui::QgsPointClusterRendererWidgetBase
{
    Q_OBJECT
  public:

    /** Returns a new QgsPointClusterRendererWidget.
     * @param layer associated vector layer
     * @param style style collection
     * @param renderer source QgsPointClusterRenderer renderer
     * @returns new QgsRendererWidget
     */
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );

    /** Constructor for QgsPointClusterRendererWidget.
     * @param layer associated vector layer
     * @param style style collection
     * @param renderer source QgsPointClusterRenderer renderer
     */
    QgsPointClusterRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );

    ~QgsPointClusterRendererWidget();

    QgsFeatureRenderer* renderer() override;
    void setContext( const QgsSymbolWidgetContext& context ) override;

  private:
    QgsPointClusterRenderer* mRenderer;

    void blockAllSignals( bool block );
    void updateCenterIcon();
    void setupBlankUi( const QString& layerName );

  private slots:

    void on_mRendererComboBox_currentIndexChanged( int index );
    void on_mDistanceSpinBox_valueChanged( double d );
    void on_mDistanceUnitWidget_changed();
    void on_mCenterSymbolPushButton_clicked();
    void on_mRendererSettingsButton_clicked();
    void updateCenterSymbolFromWidget();
    void cleanUpSymbolSelector( QgsPanelWidget* container );
    void updateRendererFromWidget();
};

#endif // QGSPOINTCLUSTERRENDERERWIDGET_H
