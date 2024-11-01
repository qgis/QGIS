/***************************************************************************
    qgspointcloudattributebyramprendererwidget.h
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

#ifndef QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERERWIDGET_H
#define QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERERWIDGET_H

#include "qgspointcloudrendererwidget.h"
#include "ui_qgspointcloudattributebyramprendererwidgetbase.h"
#include "qgis_gui.h"

class QgsPointCloudLayer;
class QgsStyle;
class QLineEdit;
class QgsPointCloudAttributeByRampRenderer;

#define SIP_NO_FILE

///@cond PRIVATE

class GUI_EXPORT QgsPointCloudAttributeByRampRendererWidget : public QgsPointCloudRendererWidget, private Ui::QgsPointCloudAttributeByRampRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsPointCloudAttributeByRampRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style );
    static QgsPointCloudRendererWidget *create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * );

    QgsPointCloudRenderer *renderer() override;
  private slots:

    void emitWidgetChanged();
    void minMaxChanged();
    void attributeChanged();
    void setMinMaxFromLayer();

  private:
    void setFromRenderer( const QgsPointCloudRenderer *r );

    bool mBlockChangedSignal = false;
    bool mBlockMinMaxChanged = false;
    bool mBlockSetMinMaxFromLayer = false;

    double mProviderMin = std::numeric_limits<double>::quiet_NaN();
    double mProviderMax = std::numeric_limits<double>::quiet_NaN();
};

///@endcond

#endif // QGSPOINTCLOUDATTRIBUTEBYRAMPRENDERERWIDGET_H
