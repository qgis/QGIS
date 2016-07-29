/***************************************************************************
    qgsellipsesymbollayerv2widget.h
    ---------------------
    begin                : June 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSELLIPSESYMBOLLAYERV2WIDGET_H
#define QGSELLIPSESYMBOLLAYERV2WIDGET_H

#include "ui_widget_ellipse.h"
#include "qgssymbollayerv2widget.h"

class QgsEllipseSymbolLayerV2;

/** \ingroup gui
 * \class QgsEllipseSymbolLayerV2Widget
 */
class GUI_EXPORT QgsEllipseSymbolLayerV2Widget: public QgsSymbolLayerV2Widget, private Ui::WidgetEllipseBase
{
    Q_OBJECT

  public:
    QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsEllipseSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsEllipseSymbolLayerV2* mLayer;

  private:
    void blockComboSignals( bool block );

  private slots:
    void on_mShapeListWidget_itemSelectionChanged();
    void on_mWidthSpinBox_valueChanged( double d );
    void on_mHeightSpinBox_valueChanged( double d );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mOutlineStyleComboBox_currentIndexChanged( int index );
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_btnChangeColorBorder_colorChanged( const QColor& newColor );
    void on_btnChangeColorFill_colorChanged( const QColor& newColor );

    void on_mSymbolWidthUnitWidget_changed();
    void on_mOutlineWidthUnitWidget_changed();
    void on_mSymbolHeightUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

    void penJoinStyleChanged();

    void setOffset();
};

#endif // QGSELLIPSESYMBOLLAYERV2WIDGET_H
