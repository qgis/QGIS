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

#include "qgsrasterrendererwidget.h"
#include "ui_qgsmultibandcolorrendererwidgetbase.h"

class QgsContrastEnhancement;
class QgsMultiBandColorRenderer;
class QgsRasterDataProvider;
class QgsRasterLayer;
class QLineEdit;

class QgsMultiBandColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsMultiBandColorRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsMultiBandColorRendererWidget( QgsRasterLayer* layer );
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer ) { return new QgsMultiBandColorRendererWidget( layer ); }
    ~QgsMultiBandColorRendererWidget();

    QgsRasterRenderer* renderer();

    void setFromRenderer( const QgsRasterRenderer* r );

  private slots:
    void on_mLoadPushButton_clicked();

  private:
    void createValidators();
    void setCustomMinMaxValues( QgsMultiBandColorRenderer* r, const QgsRasterDataProvider* provider, int redBand, int GreenBand,
                                int blueBand );
    /**Reads min/max values from contrast enhancement and fills values into the min/max line edits*/
    void setMinMaxValue( const QgsContrastEnhancement* ce, QLineEdit* minEdit, QLineEdit* maxEdit );
    void loadMinMaxValueForBand( int band, QLineEdit* minEdit, QLineEdit* maxEdit );
};

#endif // QGSMULTIBANDCOLORRENDERERWIDGET_H
