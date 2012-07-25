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

#include "qgsrasterrendererwidget.h"
#include "ui_qgssinglebandgrayrendererwidgetbase.h"

class GUI_EXPORT QgsSingleBandGrayRendererWidget: public QgsRasterRendererWidget, private Ui::QgsSingleBandGrayRendererWidgetBase
{
    Q_OBJECT
  public:
    QgsSingleBandGrayRendererWidget( QgsRasterLayer* layer );
    ~QgsSingleBandGrayRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer ) { return new QgsSingleBandGrayRendererWidget( layer ); }

    QgsRasterRenderer* renderer();

    void setFromRenderer( const QgsRasterRenderer* r );

    QString min( int index = 0 ) { Q_UNUSED( index ); return mMinLineEdit->text(); }
    QString max( int index = 0 ) { Q_UNUSED( index ); return mMaxLineEdit->text(); }
    void setMin( QString value, int index = 0 ) { Q_UNUSED( index ); mMinLineEdit->setText( value ); }
    void setMax( QString value, int index = 0 ) { Q_UNUSED( index ); mMaxLineEdit->setText( value ); }
    QString stdDev( ) { return QString::number( mStdDevSpinBox->value() ); }
    void setStdDev( QString value ) { mStdDevSpinBox->setValue( value.toDouble() ); }
    int selectedBand( int index = 0 ) { Q_UNUSED( index ); return mGrayBandComboBox->currentIndex() + 1; }

  private slots:
    void on_mLoadPushButton_clicked();
};

#endif // QGSSINGLEBANDGRAYRENDERERWIDGET_H
