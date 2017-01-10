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
#include "qgis_gui.h"

class QgsContrastEnhancement;
class QgsMultiBandColorRenderer;
class QgsRasterDataProvider;
class QgsRasterLayer;
class QLineEdit;
class QgsRasterMinMaxWidget;

/** \ingroup gui
 * \class QgsMultiBandColorRendererWidget
 */
class GUI_EXPORT QgsMultiBandColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsMultiBandColorRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsMultiBandColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) { return new QgsMultiBandColorRendererWidget( layer, theExtent ); }

    QgsRasterRenderer* renderer() override;
    void setMapCanvas( QgsMapCanvas* canvas ) override;

    void setFromRenderer( const QgsRasterRenderer* r );

    QString min( int index = 0 ) override;
    QString max( int index = 0 ) override;
    void setMin( const QString& value, int index = 0 ) override;
    void setMax( const QString& value, int index = 0 ) override;
    int selectedBand( int index = 0 ) override;
    void doComputations() override;
    QgsRasterMinMaxWidget* minMaxWidget() override { return mMinMaxWidget; }

  public slots:
    //! called when new min/max values are loaded
    void loadMinMax( int theBandNo, double theMin, double theMax );

  private slots:
    //void on_mLoadPushButton_clicked();
    void onBandChanged( int );
    void on_mRedMinLineEdit_textChanged( const QString & );
    void on_mRedMaxLineEdit_textChanged( const QString & );
    void on_mGreenMinLineEdit_textChanged( const QString & );
    void on_mGreenMaxLineEdit_textChanged( const QString & );
    void on_mBlueMinLineEdit_textChanged( const QString & );
    void on_mBlueMaxLineEdit_textChanged( const QString & );

  private:
    void createValidators();
    void setCustomMinMaxValues( QgsMultiBandColorRenderer* r, const QgsRasterDataProvider* provider, int redBand, int GreenBand,
                                int blueBand );
    //! Reads min/max values from contrast enhancement and fills values into the min/max line edits
    void setMinMaxValue( const QgsContrastEnhancement* ce, QLineEdit* minEdit, QLineEdit* maxEdit );
    QgsRasterMinMaxWidget * mMinMaxWidget;
    bool mDisableMinMaxWidgetRefresh;

    void minMaxModified();
};

#endif // QGSMULTIBANDCOLORRENDERERWIDGET_H
