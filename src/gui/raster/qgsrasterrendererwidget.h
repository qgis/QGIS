/***************************************************************************
                         qgsrasterrendererwidget.h
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

#ifndef QGSRASTERRENDERERWIDGET_H
#define QGSRASTERRENDERERWIDGET_H

#include <QWidget>

class QgsRasterLayer;
class QgsRasterRenderer;

class GUI_EXPORT QgsRasterRendererWidget: public QWidget
{
  public:
    QgsRasterRendererWidget( QgsRasterLayer* layer ) { mRasterLayer = layer; }
    virtual ~QgsRasterRendererWidget() {}

    virtual QgsRasterRenderer* renderer() = 0;

    void setRasterLayer( QgsRasterLayer* layer ) { mRasterLayer = layer; }
    const QgsRasterLayer* rasterLayer() const { return mRasterLayer; }

  protected:
    QgsRasterLayer* mRasterLayer;
    /**Returns a band name for display. First choice is color name, otherwise band number*/
    QString displayBandName( int band ) const;
};

#endif // QGSRASTERRENDERERWIDGET_H
