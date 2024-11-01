/***************************************************************************
                         qgspointcloudrgbrendererwidget.h
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDRGBRENDERERWIDGET_H
#define QGSPOINTCLOUDRGBRENDERERWIDGET_H

#include "qgspointcloudrendererwidget.h"
#include "ui_qgspointcloudrgbrendererwidgetbase.h"
#include "qgis_gui.h"

class QgsContrastEnhancement;
class QgsPointCloudLayer;
class QgsStyle;
class QLineEdit;
class QgsPointCloudRgbRenderer;

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsPointCloudRgbRendererWidget : public QgsPointCloudRendererWidget, private Ui::QgsPointCloudRgbRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsPointCloudRgbRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style );
    static QgsPointCloudRendererWidget *create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * );

    QgsPointCloudRenderer *renderer() override;
  private slots:

    void mRedMinLineEdit_textChanged( const QString & );
    void mRedMaxLineEdit_textChanged( const QString & );
    void mGreenMinLineEdit_textChanged( const QString & );
    void mGreenMaxLineEdit_textChanged( const QString & );
    void mBlueMinLineEdit_textChanged( const QString & );
    void mBlueMaxLineEdit_textChanged( const QString & );

    void emitWidgetChanged();

    void redAttributeChanged();
    void greenAttributeChanged();
    void blueAttributeChanged();

  private:
    void setFromRenderer( const QgsPointCloudRenderer *r );

    void createValidators();
    void setCustomMinMaxValues( QgsPointCloudRgbRenderer *r );
    //! Reads min/max values from contrast enhancement and fills values into the min/max line edits
    void setMinMaxValue( const QgsContrastEnhancement *ce, QLineEdit *minEdit, QLineEdit *maxEdit );

    void minMaxModified();

    bool mBlockChangedSignal = false;
    int mDisableMinMaxWidgetRefresh = 0;
};

///@endcond

#endif // QGSPOINTCLOUDRGBRENDERERWIDGET_H
