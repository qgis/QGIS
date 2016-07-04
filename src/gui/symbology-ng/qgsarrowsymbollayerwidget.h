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
#include "qgssymbollayerv2widget.h"

class QgsArrowSymbolLayer;

/** \ingroup gui
 * \class QgsArrowSymbolLayerWidget
 */
class GUI_EXPORT QgsArrowSymbolLayerWidget: public QgsSymbolLayerV2Widget, private Ui::QgsArrowSymbolLayerWidgetBase
{
    Q_OBJECT

  public:
    /** Constructor
     * @param layer the layer where this symbol layer is applied
     * @param parent the parent widget
     */
    QgsArrowSymbolLayerWidget( const QgsVectorLayer* layer, QWidget* parent = nullptr );

    /** Static creation method
     * @param layer the layer where this symbol layer is applied
     */
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* layer ) { return new QgsArrowSymbolLayerWidget( layer ); }

    /** Set the symbol layer */
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    /** Get the current symbol layer */
    virtual QgsSymbolLayerV2* symbolLayer() override;

  private:
    QgsArrowSymbolLayer* mLayer;

  private slots:
    void on_mArrowWidthSpin_valueChanged( double d );
    void on_mArrowWidthUnitWidget_changed();

    void on_mArrowStartWidthSpin_valueChanged( double d );
    void on_mArrowStartWidthUnitWidget_changed();

    void on_mHeadLengthSpin_valueChanged( double d );
    void on_mHeadLengthUnitWidget_changed();
    void on_mHeadThicknessSpin_valueChanged( double d );
    void on_mHeadThicknessUnitWidget_changed();

    void on_mHeadTypeCombo_currentIndexChanged( int );
    void on_mArrowTypeCombo_currentIndexChanged( int );

    void on_mOffsetSpin_valueChanged( double d );
    void on_mOffsetUnitWidget_changed();

    void on_mCurvedArrowChck_stateChanged( int );
    void on_mRepeatArrowChck_stateChanged( int );
};

#endif
