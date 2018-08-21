/***************************************************************************
                         qgssinglebandpseudocolorrendererwidget.h
                         ----------------------------------------
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

#ifndef QGSSINGLEBANDCOLORRENDERERWIDGET_H
#define QGSSINGLEBANDCOLORRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "qgis_sip.h"
#include "qgscolorrampshader.h"
#include "qgsrasterrenderer.h"
#include "ui_qgssinglebandpseudocolorrendererwidgetbase.h"
#include "qgis_gui.h"

class QgsRasterMinMaxWidget;

/**
 * \ingroup gui
 * \class QgsSingleBandPseudoColorRendererWidget
 *
 * Single band pseudo color renderer widget consists of a color ramp shader widget,
 * a raster min max widget and a band selector.
 *
 */
class GUI_EXPORT QgsSingleBandPseudoColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsSingleBandPseudoColorRendererWidgetBase
{

    Q_OBJECT

  public:

    //! Creates new raster renderer widget
    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    //! Creates new raster renderer widget
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsSingleBandPseudoColorRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() override;
    void setMapCanvas( QgsMapCanvas *canvas ) override;
    void doComputations() override;
    QgsRasterMinMaxWidget *minMaxWidget() override;

    //! Returns the current raster band number
    int currentBand() const;

    //! Set state of the widget from renderer settings
    void setFromRenderer( const QgsRasterRenderer *r );

  public slots:
    //! called when new min/max values are loaded
    void loadMinMax( int bandNo, double min, double max );
    //! called when the color ramp tree has changed
    void loadMinMaxFromTree( double min, double max );

  private slots:
    void bandChanged();
    void mMinLineEdit_textChanged( const QString & );
    void mMaxLineEdit_textChanged( const QString & );
    void mMinLineEdit_textEdited( const QString &text );
    void mMaxLineEdit_textEdited( const QString &text );

  private:
    void setLineEditValue( QLineEdit *lineEdit, double value );
    double lineEditValue( const QLineEdit *lineEdit ) const;

    QgsRasterMinMaxWidget *mMinMaxWidget = nullptr;
    int mMinMaxOrigin;

    void minMaxModified();
};

#endif // QGSSINGLEBANDCOLORRENDERERWIDGET_H
