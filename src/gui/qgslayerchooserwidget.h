/***************************************************************************
                             QgsLayerChooserwidget.cpp
                             -------------------------
    begin                : September 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERCHOOSERWIDGET_H
#define QGSLAYERCHOOSERWIDGET_H

#include "qgsmaplayer.h"
#include "qgis.h"

#include <QObject>
#include <QFlags>
#include <QComboBox>


/**
  * Manage a widget to allow selecting a layer
  * from the list of layers.
  * Different types of widgets are managed using
  * inherited classes: QgsLayerChooserCombo
  *
  * A QgsLayerChooserWidget::LayerFilter can be used
  * to filter the layers.
  *
  * In case of a vector layer, a QgsFieldChooserWidget
  * can be assiociated to allow the selection of fields.
  */
class GUI_EXPORT QgsLayerChooserWidget : public QObject
{
    Q_OBJECT

  public:
    /**
     * @brief The visibility of the layer in the widget depending on the result of the QgsLayerChooserWidget::LayerFilter
     */
    enum DisplayStatus
    {
      enabled,
      disabled,
      hidden
    };

    /**
     * Filter the layers to list in the widget.
     * @see QgsLayerChooserWidget
     */
    class LayerFilter
    {
      public:
        /**
         * @brief defines if the layer should be listed or not.
         * @param layer the map layer
         * @return QgsLayerChooserWidget::DisplayStatus (enabled, disabled or hidden)
         */
        virtual DisplayStatus acceptLayer( QgsMapLayer* layer ) { Q_UNUSED( layer ); return enabled; }
    };

    class VectorLayerFilter : public LayerFilter
    {
        virtual DisplayStatus acceptLayer( QgsMapLayer* layer );
    };

    class RasterLayerFilter : public LayerFilter
    {
        virtual DisplayStatus acceptLayer( QgsMapLayer* layer );
    };

    /** constructor */
    QgsLayerChooserWidget( QObject *parent = 0 );

    /** initialize the widget to show the layers. Must be redefined in subclasses. */
    virtual bool initWidget( QWidget* widget ) = 0;

    /**
     * @brief set the filter to be used to determine layers visibility
     * @param filter QgsLayerChooserWidget::LayerFilter
     */
    void setFilter( LayerFilter* filter );

    /** get the currently selected layer in the widget */
    virtual QgsMapLayer* getLayer() const = 0;

    /** get the id of the currently selected layer in the widget */
    QString getLayerId() const;

  signals:
    /** layerChanged is emitted whenever the selected layer in the widget has changed */
    void layerChanged( QgsMapLayer* );

  public slots:
    /**
     * @brief set the current layer in the widget
     * @note subclass must declare "using QgsLayerChooserWidget::setLayer;"
     * in header to avoid shadowing
     */
    void setLayer( QString layerid );

    /** set the current layer in the widget */
    virtual void setLayer( QgsMapLayer* layer ) = 0;

  protected:
    /**
     * @brief add a layer to the widget list. This method is pure virtual.
     * @param layer the map layer
     * @param display the QgsLayerChooserWidget::DisplayStatus
     */
    virtual void addLayer( QgsMapLayer* layer, DisplayStatus display ) = 0;
    /** remove the layer items in the widget. Pure virtual. */
    virtual void clearWidget() = 0;
    /** populates the widget with layers names */
    void populateLayers();

  protected slots:
    /** intermediate method to map the QgsMapLayerRegistry::layersAdded signal to the populateLayers method */
    void populateLayers( QList<QgsMapLayer*> layerList ) {Q_UNUSED( layerList ); return populateLayers();}
    /** intermediate method to map the QgsMapLayerRegistry::layersRemoved signal to the populateLayers method */
    void populateLayers( QStringList layerList ) {Q_UNUSED( layerList ); return populateLayers();}

  private:
    LayerFilter* mFilter;
};

#endif // QGSLAYERCHOOSERWIDGET_H
