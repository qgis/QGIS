/***************************************************************************
    qgsarrowsymbollayerwidget.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARROWSYMBOLLAYERWIDGET_H
#define QGSARROWSYMBOLLAYERWIDGET_H

#include "ui_qgsarrowsymbollayerwidgetbase.h"
#include "qgis_sip.h"
#include "qgssymbollayerwidget.h"
#include "qgis_gui.h"

class QgsArrowSymbolLayer;

/**
 * \ingroup gui
 * \class QgsArrowSymbolLayerWidget
 */
class GUI_EXPORT QgsArrowSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::QgsArrowSymbolLayerWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param layer the layer where this symbol layer is applied
     * \param parent the parent widget
     */
    QgsArrowSymbolLayerWidget( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Static creation method
     * \param layer the layer where this symbol layer is applied
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *layer ) SIP_FACTORY { return new QgsArrowSymbolLayerWidget( layer ); }

    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  private:
    QgsArrowSymbolLayer *mLayer = nullptr;

  private slots:
    void mArrowWidthSpin_valueChanged( double d );
    void mArrowWidthUnitWidget_changed();

    void mArrowStartWidthSpin_valueChanged( double d );
    void mArrowStartWidthUnitWidget_changed();

    void mHeadLengthSpin_valueChanged( double d );
    void mHeadLengthUnitWidget_changed();
    void mHeadThicknessSpin_valueChanged( double d );
    void mHeadThicknessUnitWidget_changed();

    void mHeadTypeCombo_currentIndexChanged( int );
    void mArrowTypeCombo_currentIndexChanged( int );

    void mOffsetSpin_valueChanged( double d );
    void mOffsetUnitWidget_changed();

    void mCurvedArrowChck_stateChanged( int );
    void mRepeatArrowChck_stateChanged( int );
};

#endif
