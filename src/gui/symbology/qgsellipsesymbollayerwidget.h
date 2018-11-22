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
#ifndef QGSELLIPSESYMBOLLAYERWIDGET_H
#define QGSELLIPSESYMBOLLAYERWIDGET_H

#include "ui_widget_ellipse.h"
#include "qgis.h"
#include "qgssymbollayerwidget.h"
#include "qgis_gui.h"

class QgsEllipseSymbolLayer;

/**
 * \ingroup gui
 * \class QgsEllipseSymbolLayerWidget
 */
class GUI_EXPORT QgsEllipseSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::WidgetEllipseBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsEllipseSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsEllipseSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsEllipseSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsEllipseSymbolLayer *mLayer = nullptr;

  private:
    void blockComboSignals( bool block );

  private slots:
    void mShapeListWidget_itemSelectionChanged();
    void mWidthSpinBox_valueChanged( double d );
    void mHeightSpinBox_valueChanged( double d );
    void mRotationSpinBox_valueChanged( double d );
    void mStrokeStyleComboBox_currentIndexChanged( int index );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void btnChangeColorStroke_colorChanged( const QColor &newColor );
    void btnChangeColorFill_colorChanged( const QColor &newColor );

    void mSymbolWidthUnitWidget_changed();
    void mStrokeWidthUnitWidget_changed();
    void mSymbolHeightUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void mVerticalAnchorComboBox_currentIndexChanged( int index );

    void penJoinStyleChanged();

    void setOffset();
};

#endif // QGSELLIPSESYMBOLLAYERWIDGET_H
