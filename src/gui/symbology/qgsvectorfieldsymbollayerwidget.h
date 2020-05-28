/***************************************************************************
    qgsvectorfieldsymbollayerwidget.h
    ---------------------
    begin                : October 2011
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
#ifndef QGSVECTORFIELDSYMBOLLAYERWIDGET_H
#define QGSVECTORFIELDSYMBOLLAYERWIDGET_H

#include "qgssymbollayerwidget.h"
#include "qgis_sip.h"
#include "ui_widget_vectorfield.h"
#include "qgis_gui.h"

class QgsVectorFieldSymbolLayer;

/**
 * \ingroup gui
 * \class QgsVectorFieldSymbolLayerWidget
 */
class GUI_EXPORT QgsVectorFieldSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::WidgetVectorFieldBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsVectorFieldSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsVectorFieldSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsVectorFieldSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsVectorFieldSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsVectorFieldSymbolLayer *mLayer = nullptr;

  private slots:
    void mScaleSpinBox_valueChanged( double d );
    void mXAttributeComboBox_currentIndexChanged( int index );
    void mYAttributeComboBox_currentIndexChanged( int index );
    void mCartesianRadioButton_toggled( bool checked );
    void mPolarRadioButton_toggled( bool checked );
    void mHeightRadioButton_toggled( bool checked );
    void mDegreesRadioButton_toggled( bool checked );
    void mRadiansRadioButton_toggled( bool checked );
    void mClockwiseFromNorthRadioButton_toggled( bool checked );
    void mCounterclockwiseFromEastRadioButton_toggled( bool checked );
    void mDistanceUnitWidget_changed();
};

#endif // QGSVECTORFIELDSYMBOLLAYERWIDGET_H
