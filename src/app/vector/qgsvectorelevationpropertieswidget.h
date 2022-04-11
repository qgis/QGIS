/***************************************************************************
    qgsvectorrelevationpropertieswidget.h
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORELEVATIONPROPERTIESWIDGET_H
#define QGSVECTORELEVATIONPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerelevationproperties.h"

#include "ui_qgsvectorelevationpropertieswidgetbase.h"

class QgsVectorLayer;
class QgsPropertyOverrideButton;

class QgsVectorElevationPropertiesWidget : public QgsMapLayerConfigWidget, private Ui::QgsVectorElevationPropertiesWidgetBase, private QgsExpressionContextGenerator
{
    Q_OBJECT
  public:

    QgsVectorElevationPropertiesWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) override;

    QgsExpressionContext createExpressionContext() const override;

  public slots:
    void apply() override;

  private slots:

    void onChanged();
    void clampingChanged();
    void bindingChanged();
    void toggleSymbolWidgets();
    void updateProperty();

  private:

    // TODO -- consider moving these to a common elevation properties widget base class

    /**
     * Registers a property override button, setting up its initial value, connections and description.
     * \param button button to register
     * \param key corresponding data defined property key
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsMapLayerElevationProperties::Property key );

    /**
     * Updates all property override buttons to reflect the widgets's current properties.
     */
    void updateDataDefinedButtons();

    /**
     * Updates a specific property override \a button to reflect the widgets's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );
    QgsExpressionContext mContext;
    QgsPropertyCollection mPropertyCollection;

    QgsVectorLayer *mLayer = nullptr;
    bool mBlockUpdates = false;

};


class QgsVectorElevationPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsVectorElevationPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
    QString layerPropertiesPagePositionHint() const override;
};



#endif // QGSVECTORELEVATIONPROPERTIESWIDGET_H
