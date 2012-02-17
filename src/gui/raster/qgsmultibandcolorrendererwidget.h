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

class QgsRasterLayer;

class QgsMultiBandColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsMultiBandColorRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsMultiBandColorRendererWidget( QgsRasterLayer* layer );
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer ) { return new QgsMultiBandColorRendererWidget( layer ); }
    ~QgsMultiBandColorRendererWidget();

    QgsRasterRenderer* renderer();
};

#endif // QGSMULTIBANDCOLORRENDERERWIDGET_H
