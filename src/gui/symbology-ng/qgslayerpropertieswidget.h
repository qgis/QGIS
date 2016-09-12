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

class QgsSymbol;
class QgsSymbolLayer;
class QgsSymbolLayerWidget;
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
    QgsLayerPropertiesWidget( QgsSymbolLayer* layer, const QgsSymbol* symbol, const QgsVectorLayer* vl, QWidget* parent = nullptr );

    /** Sets the context in which the symbol widget is shown, eg the associated map canvas and expression contexts.
     * @param context symbol widget context
     * @see context()
     * @note added in QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext& context );

    /** Returns the context in which the symbol widget is shown, eg the associated map canvas and expression contexts.
     * @see setContext()
     * @note added in QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Set the widget in dock mode which tells the widget to emit panel
     * widgets and not open dialogs
     * @param dockMode True to enable dock mode.
     */
    virtual void setDockMode( bool dockMode ) override;

  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

  signals:
    void changed();
    void changeLayer( QgsSymbolLayer* );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayer* layer );

  protected: // data
    QgsSymbolLayer* mLayer;

    const QgsSymbol* mSymbol;
    const QgsVectorLayer* mVectorLayer;

  private slots:
    void reloadLayer();

  private:

    QgsSymbolWidgetContext mContext;

};

#endif //QGSLAYERPROPERTIESWIDGET_H
