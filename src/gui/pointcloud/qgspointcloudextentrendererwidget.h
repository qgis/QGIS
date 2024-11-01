/***************************************************************************
                         qgspointcloudextentrendererwidget.h
    ---------------------
    begin                : December 2020
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

#ifndef QGSPOINTCLOUDEXTENTRENDERERWIDGET_H
#define QGSPOINTCLOUDEXTENTRENDERERWIDGET_H

#include "qgspointcloudrendererwidget.h"
#include "ui_qgspointcloudextentrendererwidgetbase.h"
#include "qgis_gui.h"

class QgsContrastEnhancement;
class QgsPointCloudLayer;
class QgsStyle;
class QgsPointCloudExtentRenderer;

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsPointCloudExtentRendererWidget : public QgsPointCloudRendererWidget, private Ui::QgsPointCloudExtentRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsPointCloudExtentRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style );
    static QgsPointCloudRendererWidget *create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * );

    QgsPointCloudRenderer *renderer() override;

  private slots:

    void emitWidgetChanged();

  private:
    void setFromRenderer( const QgsPointCloudRenderer *r );

    bool mBlockChangedSignal = false;
};

///@endcond

#endif // QGSPOINTCLOUDEXTENTRENDERERWIDGET_H
