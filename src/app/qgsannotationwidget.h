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
class QgsSettingsEntryBool;

/**
 * A configuration widget to configure the annotation item properties.
 *
 * Usually embedded by QgsAnnotation subclass configuration dialogs.
*/
class APP_EXPORT QgsAnnotationWidget : public QWidget, private Ui::QgsAnnotationWidgetBase
{
    Q_OBJECT
  public:
    static const QgsSettingsEntryBool *settingLiveUpdate;

    QgsAnnotationWidget( QgsMapCanvasAnnotationItem *item, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    //! Returns the annotation frame symbol fill color
    QColor backgroundColor();

    void apply();

  private:
    void frameStyleChanged();

  signals:

    //! Emitted when the background color of the annotation is changed
    void backgroundColorChanged( const QColor &color );

    //! \since QGIS 3.32 Emitted when any property of the annotation is changed
    void changed();

  private:
    QgsMapCanvasAnnotationItem *mItem = nullptr;

    void blockAllSignals( bool block );
    void updateCenterIcon();
    void updateFillIcon();
};

#endif // QGSANNOTATIONWIDGET_H
