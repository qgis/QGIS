/***************************************************************************
  qgsattributetablerendererwidget.h - QgsAttributeTableRendererWidget

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTETABLERENDERERWIDGET_H
#define QGSATTRIBUTETABLERENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "ui_qgsattributetablerendererwidgetbase.h"
#include "qgis_gui.h"
#include <QObject>

class GUI_EXPORT QgsAttributeTableRendererWidget : public QgsRasterRendererWidget, private Ui::QgsAttributeTableRendererWidgetBase
{
    Q_OBJECT
  public:

    //! Creates new raster renderer widget
    QgsAttributeTableRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    //! Creates new raster renderer widget
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsAttributeTableRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() SIP_FACTORY override;

};

#endif // QGSATTRIBUTETABLERENDERERWIDGET_H
