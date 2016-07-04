/***************************************************************************
    qgslayerpropertieswidget.h
    ---------------------
    begin                : June 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERPROPERTIESWIDGET_H
#define QGSLAYERPROPERTIESWIDGET_H

#include "ui_widget_layerproperties.h"

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsSymbolLayerV2Widget;
class QgsVectorLayer;
class QgsMapCanvas;
class QgsPanelWidget;

class SymbolLayerItem;

#include <QMap>
#include <QStandardItemModel>

/** \ingroup gui
 * \class QgsLayerPropertiesWidget
 */
class GUI_EXPORT QgsLayerPropertiesWidget : public QgsPanelWidget, private Ui::LayerPropertiesWidget
{
    Q_OBJECT

  public:
    QgsLayerPropertiesWidget( QgsSymbolLayerV2* layer, const QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent = nullptr );

    /** Returns the expression context used for the widget, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the properties widget.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const { return mPresetExpressionContext; }

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * @param dockMode True to enable dock mode.
     */
    virtual void setDockMode( bool dockMode ) override;

  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the properties widget.
     * @param context expression context pointer. Ownership is not transferred and the object must
     * be kept alive for the lifetime of the properties widget.
     * @note added in QGIS 2.12
     * @see expressionContext()
     */
    void setExpressionContext( QgsExpressionContext* context );

  signals:
    void changed();
    void changeLayer( QgsSymbolLayerV2* );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayerV2* layer );

  protected: // data
    QgsSymbolLayerV2* mLayer;

    const QgsSymbolV2* mSymbol;
    const QgsVectorLayer* mVectorLayer;

  private slots:
    void reloadLayer();

  private:
    QgsExpressionContext* mPresetExpressionContext;
    QgsMapCanvas* mMapCanvas;

};

#endif //QGSLAYERPROPERTIESWIDGET_H
