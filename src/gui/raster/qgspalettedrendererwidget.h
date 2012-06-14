/***************************************************************************
                         qgspalettedrendererwidget.h
                         ---------------------------
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

#ifndef QGSPALETTEDRENDERERWIDGET_H
#define QGSPALETTEDRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "ui_qgspalettedrendererwidgetbase.h"

class QgsRasterLayer;

class QgsPalettedRendererWidget: public QgsRasterRendererWidget, private Ui::QgsPalettedRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsPalettedRendererWidget( QgsRasterLayer* layer );
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer ) { return new QgsPalettedRendererWidget( layer ); }
    ~QgsPalettedRendererWidget();

    QgsRasterRenderer* renderer();

    void setFromRenderer( const QgsRasterRenderer* r );

  private slots:
    void on_mTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column );
};

#endif // QGSPALETTEDRENDERERWIDGET_H
