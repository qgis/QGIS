/***************************************************************************
                         qgslayoutpolylinewidget.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPOLYLINEWIDGET_H
#define QGSLAYOUTPOLYLINEWIDGET_H

#include "ui_qgslayoutpolylinewidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitempolyline.h"

/**
 * Input widget for QgsLayoutItemPolyline
 */
class QgsLayoutPolylineWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPolylineWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsLayoutPolylineWidget( QgsLayoutItemPolyline *polyline );

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private:
    QgsLayoutItemPolyline *mPolyline = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

  private slots:

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();

  private slots:
    void symbolChanged();
};

#endif // QGSLAYOUTPOLYLINEWIDGET_H
