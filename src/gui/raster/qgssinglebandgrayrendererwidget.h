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
#include "qgis_sip.h"
#include "ui_qgssinglebandgrayrendererwidgetbase.h"
#include "qgis_gui.h"
#include "qgscolorramplegendnodesettings.h"

class QgsRasterMinMaxWidget;

/**
 * \ingroup gui
 * \class QgsSingleBandGrayRendererWidget
 */
class GUI_EXPORT QgsSingleBandGrayRendererWidget: public QgsRasterRendererWidget, private Ui::QgsSingleBandGrayRendererWidgetBase
{
    Q_OBJECT
  public:
    QgsSingleBandGrayRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsSingleBandGrayRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() SIP_FACTORY override;
    void setMapCanvas( QgsMapCanvas *canvas ) override;

    /**
     * Sets the widget state from the specified renderer.
     */
    void setFromRenderer( const QgsRasterRenderer *r );

    QString min( int index = 0 ) override { Q_UNUSED( index ) return mMinLineEdit->text(); }
    QString max( int index = 0 ) override { Q_UNUSED( index ) return mMaxLineEdit->text(); }
    void setMin( const QString &value, int index = 0 ) override;
    void setMax( const QString &value, int index = 0 ) override;
    int selectedBand( int index = 0 ) override { Q_UNUSED( index ) return mGrayBandComboBox->currentBand(); }

    QgsContrastEnhancement::ContrastEnhancementAlgorithm contrastEnhancementAlgorithm() const override;
    void setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm ) override;

    void doComputations() override;
    QgsRasterMinMaxWidget *minMaxWidget() override { return mMinMaxWidget; }

  public slots:
    //! called when new min/max values are loaded
    void loadMinMax( int bandNo, double min, double max );

  private slots:
    void bandChanged();
    void mMinLineEdit_textChanged( const QString & );
    void mMaxLineEdit_textChanged( const QString & );
    void showLegendSettings();

  private:
    QgsRasterMinMaxWidget *mMinMaxWidget = nullptr;
    bool mDisableMinMaxWidgetRefresh;
    QgsColorRampLegendNodeSettings mLegendSettings;

    void minMaxModified();
};

#endif // QGSSINGLEBANDGRAYRENDERERWIDGET_H
