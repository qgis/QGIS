/***************************************************************************
                         qgsmultibandcolorrendererwidget.h
                         ---------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMULTIBANDCOLORRENDERERWIDGET_H
#define QGSMULTIBANDCOLORRENDERERWIDGET_H

#include "qgsrasterminmaxwidget.h"
#include "qgsrasterrendererwidget.h"
#include "ui_qgsmultibandcolorrendererwidgetbase.h"

class QgsContrastEnhancement;
class QgsMultiBandColorRenderer;
class QgsRasterDataProvider;
class QgsRasterLayer;
class QLineEdit;

class GUI_EXPORT QgsMultiBandColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsMultiBandColorRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsMultiBandColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) { return new QgsMultiBandColorRendererWidget( layer, theExtent ); }
    ~QgsMultiBandColorRendererWidget();

    QgsRasterRenderer* renderer();

    void setFromRenderer( const QgsRasterRenderer* r );

    QString min( int index = 0 );
    QString max( int index = 0 );
    void setMin( QString value, int index = 0 );
    void setMax( QString value, int index = 0 );
    //QString stdDev( ) { return QString::number( mStdDevSpinBox->value() ); }
    //void setStdDev( QString value ) { mStdDevSpinBox->setValue( value.toDouble() ); }
    int selectedBand( int index = 0 );

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax );

  private slots:
    //void on_mLoadPushButton_clicked();
    void onBandChanged( int );

  private:
    void createValidators();
    void setCustomMinMaxValues( QgsMultiBandColorRenderer* r, const QgsRasterDataProvider* provider, int redBand, int GreenBand,
                                int blueBand );
    /**Reads min/max values from contrast enhancement and fills values into the min/max line edits*/
    void setMinMaxValue( const QgsContrastEnhancement* ce, QLineEdit* minEdit, QLineEdit* maxEdit );
    QgsRasterMinMaxWidget * mMinMaxWidget;
};

#endif // QGSMULTIBANDCOLORRENDERERWIDGET_H
