/***************************************************************************
                              qgsannotationwidget.h
                              ------------------------
  begin                : February 25, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONWIDGET_H
#define QGSANNOTATIONWIDGET_H

#include "ui_qgsannotationwidgetbase.h"
#include "qgis_app.h"
#include <memory>

class QgsMapCanvasAnnotationItem;
class QgsMarkerSymbol;
class QgsFillSymbol;

/**
 * A configuration widget to configure the annotation item properties. Usually embedded by QgsAnnotation
subclass configuration dialogs*/
class APP_EXPORT QgsAnnotationWidget: public QWidget, private Ui::QgsAnnotationWidgetBase
{
    Q_OBJECT
  public:
    QgsAnnotationWidget( QgsMapCanvasAnnotationItem *item, QWidget *parent = nullptr, Qt::WindowFlags f = 0 );

    void apply();

  signals:

    //! Emitted when the background color of the annotation is changed
    void backgroundColorChanged( const QColor &color );

  private:
    QgsMapCanvasAnnotationItem *mItem = nullptr;

    void blockAllSignals( bool block );
    void updateCenterIcon();
    void updateFillIcon();
};

#endif // QGSANNOTATIONWIDGET_H
