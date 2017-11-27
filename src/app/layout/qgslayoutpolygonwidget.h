/***************************************************************************
                         qgslayoutpolygonwidget.h
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

#ifndef QGSLAYOUTPOLYGONWIDGET_H
#define QGSLAYOUTPOLYGONWIDGET_H

#include "ui_qgslayoutpolygonwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitempolygon.h"

/**
 * Input widget for QgsLayoutItemPolygon
 */
class QgsLayoutPolygonWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPolygonWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsLayoutPolygonWidget( QgsLayoutItemPolygon *polygon );

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private:
    QgsLayoutItemPolygon *mPolygon = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();

  private slots:
    void symbolChanged();
};

#endif // QGSLAYOUTPOLYGONWIDGET_H
