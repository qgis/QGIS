/***************************************************************************
    qgsmasksymbollayerwidget.h
    ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMASKSYMBOLLAYERWIDGET_H
#define QGSMASKSYMBOLLAYERWIDGET_H

#include "ui_qgsmasksymbollayerwidgetbase.h"
#include "qgis_sip.h"
#include "qgssymbollayerwidget.h"
#include "qgis_gui.h"

class QgsMaskMarkerSymbolLayer;
class QgsSymbolLayerSelectionWidget;

/**
 * \ingroup gui
 * \class QgsMaskMarkerSymbolLayerWidget
 * Symbol layer widget for the handling of QgsMaskMarkerSymbolLayer.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsMaskMarkerSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::QgsMaskSymbolLayerWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param layer the layer where this symbol layer is applied
     * \param parent the parent widget
     */
    QgsMaskMarkerSymbolLayerWidget( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Static creation method
     * \param layer the layer where this symbol layer is applied
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *layer ) SIP_FACTORY { return new QgsMaskMarkerSymbolLayerWidget( layer ); }

    //! Update the current symbol layer displayed
    void setSymbolLayer( QgsSymbolLayer *layer ) override;

    //! Returns the current symbol layer
    QgsSymbolLayer *symbolLayer() override;

  private:
    //! Current symbol layer
    QgsMaskMarkerSymbolLayer *mLayer = nullptr;
};

#endif
