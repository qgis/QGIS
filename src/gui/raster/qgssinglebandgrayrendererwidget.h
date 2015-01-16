/***************************************************************************
                         qgssinglebandgrayrendererwidget.h
                         ---------------------------------
    begin                : March 2012
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

#ifndef QGSSINGLEBANDGRAYRENDERERWIDGET_H
#define QGSSINGLEBANDGRAYRENDERERWIDGET_H

#include "qgsrasterminmaxwidget.h"
#include "qgsrasterrendererwidget.h"
#include "ui_qgssinglebandgrayrendererwidgetbase.h"

class GUI_EXPORT QgsSingleBandGrayRendererWidget: public QgsRasterRendererWidget, private Ui::QgsSingleBandGrayRendererWidgetBase
{
    Q_OBJECT
  public:
    QgsSingleBandGrayRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsSingleBandGrayRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) { return new QgsSingleBandGrayRendererWidget( layer, theExtent ); }

    QgsRasterRenderer* renderer() override;

    void setFromRenderer( const QgsRasterRenderer* r );

    QString min( int index = 0 ) override { Q_UNUSED( index ); return mMinLineEdit->text(); }
    QString max( int index = 0 ) override { Q_UNUSED( index ); return mMaxLineEdit->text(); }
    void setMin( QString value, int index = 0 ) override { Q_UNUSED( index ); mMinLineEdit->setText( value ); }
    void setMax( QString value, int index = 0 ) override { Q_UNUSED( index ); mMaxLineEdit->setText( value ); }
    int selectedBand( int index = 0 ) override { Q_UNUSED( index ); return mGrayBandComboBox->currentIndex() + 1; }

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin );

  private slots:
    void on_mGrayBandComboBox_currentIndexChanged( int index );

  private:
    QgsRasterMinMaxWidget * mMinMaxWidget;
};

#endif // QGSSINGLEBANDGRAYRENDERERWIDGET_H
