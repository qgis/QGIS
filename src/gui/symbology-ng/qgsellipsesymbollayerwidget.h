/***************************************************************************
    qgsellipsesymbollayerwidget.h
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
#include "qgis.h"
#include "qgssymbollayerwidget.h"
#include "qgis_gui.h"

class QgsEllipseSymbolLayer;

/** \ingroup gui
 * \class QgsEllipseSymbolLayerWidget
 */
class GUI_EXPORT QgsEllipseSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::WidgetEllipseBase
{
    Q_OBJECT

  public:
    QgsEllipseSymbolLayerWidget( const QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = 0 );

    static QgsSymbolLayerWidget *create( const QgsVectorLayer *vl ) SIP_FACTORY { return new QgsEllipseSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayer *layer ) override;
    virtual QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsEllipseSymbolLayer *mLayer = nullptr;

  private:
    void blockComboSignals( bool block );

  private slots:
    void on_mShapeListWidget_itemSelectionChanged();
    void on_mWidthSpinBox_valueChanged( double d );
    void on_mHeightSpinBox_valueChanged( double d );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mStrokeStyleComboBox_currentIndexChanged( int index );
    void on_mStrokeWidthSpinBox_valueChanged( double d );
    void on_btnChangeColorStroke_colorChanged( const QColor &newColor );
    void on_btnChangeColorFill_colorChanged( const QColor &newColor );

    void on_mSymbolWidthUnitWidget_changed();
    void on_mStrokeWidthUnitWidget_changed();
    void on_mSymbolHeightUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

    void penJoinStyleChanged();

    void setOffset();
};

#endif // QGSELLIPSESYMBOLLAYERV2WIDGET_H
