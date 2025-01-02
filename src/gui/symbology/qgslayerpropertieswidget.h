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
#include "qgsexpressioncontext.h"
#include "qgssymbolwidgetcontext.h"
#include "qgssymbollayer.h"

class QgsSymbol;
class QgsSymbolLayer;
class QgsSymbolLayerWidget;
class QgsVectorLayer;
class QgsMapCanvas;
class QgsPanelWidget;

#include <QMap>
#include <QStandardItemModel>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsLayerPropertiesWidget
 * \brief A widget which allows configuration of the properties of a single QgsSymbolLayer.
 *
 * This widget includes a combobox box for selection of the symbol layer type, and a panel for
 * showing configuration properties for the selected symbol layer type.
 */
class GUI_EXPORT QgsLayerPropertiesWidget : public QgsPanelWidget, public QgsExpressionContextGenerator, private Ui::LayerPropertiesWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLayerPropertiesWidget.
     * \param layer the symbol layer
     * \param symbol the symbol
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsLayerPropertiesWidget( QgsSymbolLayer *layer, const QgsSymbol *symbol, QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * \param dockMode TRUE to enable dock mode.
     */
    void setDockMode( bool dockMode ) override;

  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

  signals:

    /**
     * Emitted when the symbol layer configuration is changed in the widget.
     */
    void changed();

    /**
     * Emitted when the symbol \a layer is changed in the widget.
     */
    void changeLayer( QgsSymbolLayer *layer );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayer *layer );

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );

  protected: // data
    QgsSymbolLayer *mLayer = nullptr;

    const QgsSymbol *mSymbol = nullptr;
    QgsVectorLayer *mVectorLayer = nullptr;

  private slots:
    void reloadLayer();
    void mEnabledCheckBox_toggled( bool enabled );
    void updateProperty();

  private:
    QgsSymbolWidgetContext mContext;
};

#endif //QGSLAYERPROPERTIESWIDGET_H
